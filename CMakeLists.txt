##
# @file CMakeLists.txt
# @brief 
#/

# MODULE_PATH
set(MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# MODULE_NAME
get_filename_component(MODULE_NAME ${MODULE_PATH} NAME)

# Arduino Board (t2/t3)
if(NOT DEFINED CONFIG_ARDUINO_BOARD)
    message(FATAL_ERROR "Use [menuconfig] choice [ARDUINO_BOARD].")
endif()
if(${CONFIG_ARDUINO_BOARD} STREQUAL "ARDUINO_CHIP_T3")
    set(ARDUINO_BOARD "t3")
elseif(${CONFIG_ARDUINO_BOARD} STREQUAL "ARDUINO_CHIP_T5")
    set(ARDUINO_BOARD "t5")
elseif(${CONFIG_ARDUINO_BOARD} STREQUAL "ARDUINO_CHIP_LN882H")
    set(ARDUINO_BOARD "ln882h")
else()
    set(CONFIG_ARDUINO_BOARD "ARDUINO_CHIP_T2")
    set(ARDUINO_BOARD "t2")
endif()

# LIB_SRCS
file(GLOB_RECURSE
    LIB_SRCS_CORE
    "${MODULE_PATH}/cores/tuya_open/*.cpp"
    "${MODULE_PATH}/cores/tuya_open/*.c"
    "${MODULE_PATH}/cores/tuya_open/api/*.cpp"
    )
file(GLOB_RECURSE
    LIB_SRCS_LIB
    "${MODULE_PATH}/libraries/BLE/src/*.cpp"
    "${MODULE_PATH}/libraries/BLE/src/*.c"
    "${MODULE_PATH}/libraries/FS/src/*.cpp"
    "${MODULE_PATH}/libraries/FS/src/*.c"
    "${MODULE_PATH}/libraries/HTTPClient/src/*.cpp"
    "${MODULE_PATH}/libraries/HTTPClient/src/*.c"
    "${MODULE_PATH}/libraries/LittleFS/src/*.cpp"
    "${MODULE_PATH}/libraries/LittleFS/src/*.c"
    "${MODULE_PATH}/libraries/Log/src/*.cpp"
    "${MODULE_PATH}/libraries/Log/src/*.c"
    "${MODULE_PATH}/libraries/MQTTClient/src/*.cpp"
    "${MODULE_PATH}/libraries/MQTTClient/src/*.c"
    "${MODULE_PATH}/libraries/SPI/src/*.cpp"
    "${MODULE_PATH}/libraries/SPI/src/*.c"
    "${MODULE_PATH}/libraries/Ticker/src/*.cpp"
    "${MODULE_PATH}/libraries/Ticker/src/*.c"
    "${MODULE_PATH}/libraries/TuyaIoT/src/*.cpp"
    "${MODULE_PATH}/libraries/TuyaIoT/src/*.c"
    )

file(GLOB_RECURSE
    LIB_SRCS_VAR
    "${MODULE_PATH}/variants/${ARDUINO_BOARD}/*.c"
    )
list(APPEND LIB_SRCS ${LIB_SRCS_CORE})
list(APPEND LIB_SRCS ${LIB_SRCS_LIB})
list(APPEND LIB_SRCS ${LIB_SRCS_VAR})

set(LIBRARIES_INC 
    "${MODULE_PATH}/libraries/BLE/src/"
    "${MODULE_PATH}/libraries/FS/src/"
    "${MODULE_PATH}/libraries/HTTPClient/src/"
    "${MODULE_PATH}/libraries/LittleFS/src/"
    "${MODULE_PATH}/libraries/Log/src/"
    "${MODULE_PATH}/libraries/MQTTClient/src/"
    "${MODULE_PATH}/libraries/SPI/src/"
    "${MODULE_PATH}/libraries/Ticker/src/"
    "${MODULE_PATH}/libraries/TuyaIoT/src/"
    )

set(LIB_PUBLIC_INC
    "${MODULE_PATH}/cores/tuya_open"
    "${LIBRARIES_INC}"
    "${MODULE_PATH}/variants/${ARDUINO_BOARD}"
    )

########################################
# Target Configure
########################################
add_library(${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        ${LIB_SRCS}
    )

target_include_directories(${MODULE_NAME}
    PUBLIC
        ${LIB_PUBLIC_INC}
    )

target_compile_options(${MODULE_NAME}
    PRIVATE
    -D${CONFIG_ARDUINO_BOARD}
    )


########################################
# Layer Configure
########################################
list(APPEND COMPONENT_LIBS ${MODULE_NAME})
# set(COMPONENT_LIBS "${COMPONENT_LIBS}" PARENT_SCOPE)
list(APPEND COMPONENT_PUBINC ${LIB_PUBLIC_INC})
# set(COMPONENT_PUBINC "${COMPONENT_PUBINC}" PARENT_SCOPE)

