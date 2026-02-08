# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)


#set(RMAKER_PATH $ENV{HOME}/esp-rainmaker)

#message(STATUS "RMAKER_PATH está definido: ${RMAKER_PATH}")

# Add RainMaker components and other common application components
#set(EXTRA_COMPONENT_DIRS ${RMAKER_PATH}/components ${RMAKER_PATH}/examples/common)

#message(STATUS "RMAKER_PATH está definido: ${RMAKER_PATH}")

# Add RainMaker components and other common application components
#set(EXTRA_COMPONENT_DIRS ${RMAKER_PATH}/components ${RMAKER_PATH}/examples/common)

set(PROJECT_VER "2.0beta")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(GasBoiler)
