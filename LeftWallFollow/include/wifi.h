#ifndef WIFI_H
#define WIFI_H

#include "esp_http_server.h"

extern volatile float Kp;
extern volatile float Kd;
extern volatile int left_turn_threshold;
extern volatile int right_turn_threshold;
extern volatile int wall_distance;
extern volatile float max_duty_cycle;
extern volatile float min_duty_cycle;

void wifi_connection(void);
httpd_handle_t start_webserver(void);

#endif
