cmake_minimum_required(VERSION 3.10)

# 1. Project Definitions
set(TARGET_NAME simpletrackers)
set(DRIVER_NAME "driver_${TARGET_NAME}")
project(${TARGET_NAME})

# 2. Platform Setup
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM 64)
else()
    set(PLATFORM 32)
endif()

add_definitions(-D_WIN32)
set(ARCH_TARGET win${PLATFORM})

# 3. Locate OpenVR SDK
set(OPENVR_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/openvr)
set(OPENVR_INCLUDE_DIR ${OPENVR_LIB_DIR}/headers)

find_library(OPENVR_LIBRARIES
        NAMES openvr_api
        PATHS
        ${OPENVR_LIB_DIR}/bin
        ${OPENVR_LIB_DIR}/lib
        PATH_SUFFIXES
        win${PLATFORM}
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
)

if(NOT OPENVR_LIBRARIES)
    message(FATAL_ERROR "OpenVR library not found! Please ensure 'openvr_api.lib' is in ${OPENVR_LIB_DIR}/lib/win${PLATFORM}")
endif()

# 4. Define Source Files (FIXED: Added driverlog.cpp and driverlog.h)
add_library(${DRIVER_NAME} SHARED
        src/hmd_driver_factory.cpp
        src/device_provider.h
        src/device_provider.cpp
        src/tracker_device_driver.h
        src/tracker_device_driver.cpp
        src/driverlog.h  
        src/driverlog.cpp
        src/common.h
        src/vrmath.h
        src/vrmath.cpp
)

# 5. Output Directory Setup
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}/bin/${ARCH_TARGET}>)
set_target_properties(${DRIVER_NAME} PROPERTIES PREFIX "")

# 6. Linking & Includes
target_link_libraries(${DRIVER_NAME} PRIVATE ${OPENVR_LIBRARIES} ws2_32)

target_include_directories(${DRIVER_NAME} PRIVATE ${OPENVR_INCLUDE_DIR})
target_include_directories(${DRIVER_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 7. Asset Copying
add_custom_command(
        TARGET ${DRIVER_NAME}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}
)
