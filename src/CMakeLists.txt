# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.13)

    # initialize the SDK based on PICO_SDK_PATH
    # note: this must happen before project()
    include(pico_sdk_import.cmake)

    # Set name of project (as PROJECT_NAME) and C/C++ standards
    project(halloweenizer)
	
    # initialize the Raspberry Pi Pico SDK
    pico_sdk_init()

    # rest of your project
    add_executable(${PROJECT_NAME}
        main.c
        delaytp.s
        uartbuf.c
        mystring.c
        serialprocess.c
    )

    # Add pico_stdlib library which aggregates commonly used features(gpio, time, etc. functions)
    target_link_libraries(${PROJECT_NAME}
         pico_stdlib
    )

    # Create map/bin/hex/uf2 files
    pico_add_extra_outputs(${PROJECT_NAME})

    # Enable usb output, disable uart output
    pico_enable_stdio_usb(${PROJECT_NAME} 0)
    pico_enable_stdio_uart(${PROJECT_NAME} 1)
    



