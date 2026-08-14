idf_component_register(SRCS "main.c" "../src/encoder.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver esp_timer freertos)
