idf_component_register(SRCS "main.c" "../src/motor.c" "../src/ir.c" "../src/tof.c" "../src/wifi.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver esp_adc freertos esp_wifi esp_netif nvs_flash esp_http_server esp_timer)

