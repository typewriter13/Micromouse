idf_component_register(SRCS "main.c" "../src/motor.c" "../src/ir.c" "../src/tof.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver esp_adc freertos)
