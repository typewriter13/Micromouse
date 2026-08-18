
#ifndef WIFI_H
#define WIFI_H

#include "esp_http_server.h"

extern volatile float Kp;
extern volatile float Kd;
extern volatile int left_turn_threshold;
extern volatile int right_turn_threshold;

void wifi_connection(void);
httpd_handle_t start_webserver(void);

#endif // WIFI_H