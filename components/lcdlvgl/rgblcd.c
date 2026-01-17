

#include "rgblcd.h"



#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

#include "driver/spi_master.h"

#include  "esp_lcd_touch_xpt2046.h"
#include "app_interface.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "events_lcd.h"
#include "lv_main.h"


#define	I2C_HOST	0
esp_lcd_touch_handle_t tp = NULL;
static esp_timer_handle_t timer_backlight;
xQueueHandle event_queue;


static const char *TAG = "rgblcd";
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Refresh Rate = 18000000/(1+40+20+800)/(1+10+5+480) = 42Hz
#define LCD_H_RES              480
#define LCD_V_RES              272
#define LCD_HSYNC              0
#define LCD_HBP                40
#define LCD_HFP                20
#define LCD_VSYNC              0
#define LCD_VBP                10
#define LCD_VFP                5

#define LCD_BK_LIGHT_ON_LEVEL  1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL


#if CONFIG_DOUBLE_FB
#define EXAMPLE_LCD_NUM_FB             2
#else
#define EXAMPLE_LCD_NUM_FB             1
#endif // CONFIG_EXAMPLE_USE_DOUBLE_FB


#define EXAMPLE_DATA_BUS_WIDTH         16
#define EXAMPLE_PIXEL_SIZE             2
#define LV_COLOR_FORMAT        LV_COLOR_FORMAT_RGB565


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your Application ///////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define EXAMPLE_LVGL_DRAW_BUF_LINES    50 // number of display lines in each draw buffer
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2
#define EXAMPLE_LVGL_TASK_PRIORITY     2

// LVGL library is not thread-safe, this example will call LVGL APIs from different tasks, so use a mutex to protect it
static _lock_t lvgl_api_lock;

xQueueHandle event_queue;
lv_display_t *display;

/********************************************************************************* */
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<< CONFIG_RELAY_GPIO) | (1ULL << CONFIG_PIN_NUM_BK_LIGHT))




static void backlight_off(void *arg);











const esp_timer_create_args_t backlight_shot_timer_args = {
        .callback = &backlight_off,
        /* name is optional, but may help identify the timer when debugging */
        .name = "end schedule"
};






static bool notify_lvgl_flush_ready(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // pass the draw buffer to the driver
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);

}

static void increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static void lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t time_till_next_ms = 0;
    
	event_queue = xQueueCreate(5, sizeof(event_lcd_t));

    while (1) {
        _lock_acquire(&lvgl_api_lock);
        time_till_next_ms = lv_timer_handler();
        wait_event_lcd();
        _lock_release(&lvgl_api_lock);

        // in case of task watch dog timeout, set the minimal delay to 10ms
        if (time_till_next_ms < 10) {
            time_till_next_ms = 10;
        }

        usleep(1000 * time_till_next_ms);
    }
}


static void set_lcd_backlight(uint32_t level)
{
#if CONFIG_PIN_NUM_BK_LIGHT >= 0
    gpio_set_level(CONFIG_PIN_NUM_BK_LIGHT, level);
#endif
}

void init_lcdrgb(void)
{

    init_gpios_app();
    set_lcd_backlight(LCD_BK_LIGHT_OFF_LEVEL);


    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = EXAMPLE_DATA_BUS_WIDTH,
        .dma_burst_size = 64,
        .num_fbs = EXAMPLE_LCD_NUM_FB,
#if CONFIG_EXAMPLE_USE_BOUNCE_BUFFER
        .bounce_buffer_size_px = 20 * EXAMPLE_LCD_H_RES,
#endif
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = CONFIG_RGB_LCD_PIN_NUM_DISP_EN,
        .pclk_gpio_num = CONFIG_RGB_LCD_PIN_NUM_PCLK,
        .vsync_gpio_num = CONFIG_RGB_LCD_PIN_NUM_VSYNC,
        .hsync_gpio_num = CONFIG_RGB_LCD_PIN_NUM_HSYNC,
        .de_gpio_num = CONFIG_RGB_LCD_PIN_NUM_DE,
        .data_gpio_nums = {
            CONFIG_RGB_LCD_PIN_NUM_DATA0,
            CONFIG_RGB_LCD_PIN_NUM_DATA1,
            CONFIG_RGB_LCD_PIN_NUM_DATA2,
            CONFIG_RGB_LCD_PIN_NUM_DATA3,
            CONFIG_RGB_LCD_PIN_NUM_DATA4,
            CONFIG_RGB_LCD_PIN_NUM_DATA5,
            CONFIG_RGB_LCD_PIN_NUM_DATA6,
            CONFIG_RGB_LCD_PIN_NUM_DATA7,
            CONFIG_RGB_LCD_PIN_NUM_DATA8,
            CONFIG_RGB_LCD_PIN_NUM_DATA9,
            CONFIG_RGB_LCD_PIN_NUM_DATA10,
            CONFIG_RGB_LCD_PIN_NUM_DATA11,
            CONFIG_RGB_LCD_PIN_NUM_DATA12,
            CONFIG_RGB_LCD_PIN_NUM_DATA13,
            CONFIG_RGB_LCD_PIN_NUM_DATA14,
            CONFIG_RGB_LCD_PIN_NUM_DATA15,
#if CONFIG_EXAMPLE_LCD_DATA_LINES > 16
            EXAMPLE_PIN_NUM_DATA16,
            EXAMPLE_PIN_NUM_DATA17,
            EXAMPLE_PIN_NUM_DATA18,
            EXAMPLE_PIN_NUM_DATA19,
            EXAMPLE_PIN_NUM_DATA20,
            EXAMPLE_PIN_NUM_DATA21,
            EXAMPLE_PIN_NUM_DATA22,
            EXAMPLE_PIN_NUM_DATA23
#endif
        },
        .timings = {
            .pclk_hz = CONFIG_RGB_LCD_PIXEL_CLOCK_HZ,
            .h_res = CONFIG_LCD_H_RES,
            .v_res = CONFIG_LCD_V_RES,
            .hsync_back_porch = LCD_HBP,
            .hsync_front_porch = LCD_HFP,
            .hsync_pulse_width = LCD_HSYNC,
            .vsync_back_porch = LCD_VBP,
            .vsync_front_porch = LCD_VFP,
            .vsync_pulse_width = LCD_VSYNC,
            .flags = {
                .pclk_active_neg = true,
            },
        },
        .flags.fb_in_psram = true, // allocate frame buffer in PSRAM
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

    ESP_LOGI(TAG, "Initialize RGB LCD panel");
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_LOGI(TAG, "Turn on LCD backlight");
    set_lcd_backlight(LCD_BK_LIGHT_ON_LEVEL);

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // create a lvgl display
    display = lv_display_create(CONFIG_LCD_H_RES, CONFIG_LCD_V_RES);
    // associate the rgb panel handle to the display
    lv_display_set_user_data(display, panel_handle);
    // set color depth
    lv_display_set_color_format(display, LV_COLOR_FORMAT);
    // create draw buffers
    void *buf1 = NULL;
    void *buf2 = NULL;
#if CONFIG_DOUBLE_FB
    ESP_LOGI(TAG, "Use frame buffers as LVGL draw buffers");
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));
    // set LVGL draw buffers and direct mode
    lv_display_set_buffers(display, buf1, buf2, CONFIG_LCD_H_RES * CONFIG_LCD_V_RES * EXAMPLE_PIXEL_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT);
#else
    ESP_LOGI(TAG, "Allocate LVGL draw buffers");
    // it's recommended to allocate the draw buffer from internal memory, for better performance
    size_t draw_buffer_sz = CONFIG_LCD_H_RES * EXAMPLE_LVGL_DRAW_BUF_LINES * EXAMPLE_PIXEL_SIZE;
    buf1 = heap_caps_malloc(draw_buffer_sz, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    assert(buf1);
    // set LVGL draw buffers and partial mode
    lv_display_set_buffers(display, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif // CONFIG_EXAMPLE_USE_DOUBLE_FB

    // set the callback which can copy the rendered image to an area of the display
    lv_display_set_flush_cb(display, lvgl_flush_cb);


    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, display));


    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));


    xTaskCreatePinnedToCore(lvgl_port_task, "LVGL", 5120, NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL,1);

    // Lock the mutex due to the LVGL APIs are not thread-safe
    _lock_acquire(&lvgl_api_lock);
    create_screen();
    init_app_touch_xpt2046(display);
    timing_backlight();

    _lock_release(&lvgl_api_lock);



}

/*************************************************** driver touch ********************************************** */


void lv_cancel_timing_backlight() {

	if (esp_timer_is_active(timer_backlight)) {
		ESP_LOGI(TAG, "se cancela temporizador");
		esp_timer_stop(timer_backlight);
	}


}

esp_err_t backlight_on() {


    
    if (gpio_get_level(CONFIG_PIN_NUM_BK_LIGHT) == 0) {
		gpio_set_level(CONFIG_PIN_NUM_BK_LIGHT, 1);
	} else {
        lv_cancel_timing_backlight();
	}
    if (!esp_timer_is_active(timer_backlight)) {

        ESP_ERROR_CHECK(esp_timer_start_once(timer_backlight, (CONFIG_TIME_OFF_BACKLIGHT * 1000000)));

    }
    

	return ESP_OK;
}



void backlight_off(void *arg) {


gpio_set_level(CONFIG_PIN_NUM_BK_LIGHT, 0);

}





void timing_backlight() {

    ESP_ERROR_CHECK(esp_timer_create(&backlight_shot_timer_args, &timer_backlight));
    ESP_ERROR_CHECK(esp_timer_start_once(timer_backlight, (CONFIG_TIME_OFF_BACKLIGHT * 1000000)));

}





static void lvgl_touch_cb(lv_indev_t * drv, lv_indev_data_t * data) {

    uint16_t x[1];
    uint16_t y[1];
    uint16_t strength[1];
    uint8_t count = 0;




    // Update touch point data.
   ESP_ERROR_CHECK(esp_lcd_touch_read_data(tp));

    data->state = LV_INDEV_STATE_REL;

    if (esp_lcd_touch_get_coordinates(tp, x, y, strength, &count, 1))
    {
        data->point.x = x[0];
        data->point.y = y[0];
        data->state = LV_INDEV_STATE_PR;
        backlight_on();

    }

    data->continue_reading = false;


}




void init_app_touch_xpt2046(lv_disp_t *display) {


	// Inicializamos el bus

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = 12,
        .mosi_io_num = 11,
        .miso_io_num = 13,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = CONFIG_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));


	esp_lcd_panel_io_handle_t tp_io_handle = NULL;
	esp_lcd_panel_io_spi_config_t tp_io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(38);
    tp_io_config.spi_mode = 3;
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &tp_io_config, &tp_io_handle));

	    esp_lcd_touch_config_t tp_cfg = {
	        .x_max = CONFIG_LCD_H_RES,
	        .y_max = CONFIG_LCD_V_RES,
	        .rst_gpio_num = -1,
	        .int_gpio_num = 18,
	        .flags = {
	            .swap_xy = 0,
	            .mirror_x = 0,
	            .mirror_y = 0,
	        },
	    };

	    ESP_LOGI(TAG, "Initialize touch controller XPT2046");
	    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &tp));

	    ESP_LOGE(TAG, "tp es nulo y no puede ser");

        lv_indev_t * indev = lv_indev_create();
        
        lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev, lvgl_touch_cb);


}


void init_gpios_app() {
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
	io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

}