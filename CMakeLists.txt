# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)
project(pwm_display C CXX ASM)  # Nome do projeto corrigido para `pwm_display`

pico_sdk_init()

# Definição correta do executável
add_executable(pwm_display main.c lib/ssd1306.c)

pico_set_program_name(pwm_display "PWM Display")
pico_set_program_version(pwm_display "1.0")

pico_enable_stdio_uart(pwm_display 1)
pico_enable_stdio_usb(pwm_display 1)

# Certifique-se de que o nome do alvo está correto
target_link_libraries(pwm_display pico_stdlib hardware_adc hardware_pwm hardware_i2c)

target_include_directories(pwm_display PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(pwm_display)
