set(srcs "main.c"
    "st7789.c"
   
    "decode_png.c"
    "pngle.c"
    "esp_camera.c"
    "cam_hal.c"
    "sccb.c"
    "ll_cam.c"
    "sensor.c"
    "ov2640.c"
    "xclk.c"
    "bsp.c"
    )

# tjpgd library does not exist in ESP32-S2 ROM.
if(IDF_TARGET STREQUAL "esp32")
    list(APPEND srcs "decode_jpeg.c")
endif()

if(IDF_TARGET STREQUAL "esp32s3")
    list(APPEND srcs "decode_jpeg.c")
endif()

if(IDF_TARGET STREQUAL "esp32c3")
    list(APPEND srcs "decode_jpeg.c")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")
