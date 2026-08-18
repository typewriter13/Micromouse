#include "../include/wifi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_netif.h"

#define SSID "Shwet"
#define PASS "shwet0908"

static const char *TAG_WIFI = "WIFI_SERVER";
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        esp_wifi_connect();
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_start();
    esp_wifi_connect();
}

esp_err_t get_handler(httpd_req_t *req)
{
    const char resp[] = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>Micromouse Tuner</title>\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
        "</head>\n"
        "<body style=\"font-family: Arial, sans-serif; margin: 20px;\">\n"
        "    <h2>Robot Dynamic Tuning</h2>\n"
        "    <form id=\"tuneForm\">\n"
        "        <b>PID Constants:</b><br>\n"
        "        Kp: <input type=\"number\" step=\"0.0001\" id=\"kpInput\" value=\"0.008\"><br><br>\n"
        "        Kd: <input type=\"number\" step=\"0.0001\" id=\"kdInput\" value=\"0.0005\"><br><br>\n"
        "        <hr>\n"
        "        <b>Turn Thresholds:</b><br>\n"
        "        Left Turn Threshold: <input type=\"number\" step=\"1\" id=\"ltInput\" value=\"4000\"><br><br>\n"
        "        Right Turn Threshold: <input type=\"number\" step=\"1\" id=\"rtInput\" value=\"4000\"><br><br>\n"
        "        <input type=\"submit\" value=\"Send to Robot\" style=\"padding: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer;\">\n"
        "    </form>\n"
        "    <br>\n"
        "    <div id=\"response\" style=\"color: green; font-weight: bold;\"></div>\n"
        "\n"
        "    <script>\n"
        "        document.getElementById(\"tuneForm\").addEventListener(\"submit\", function(event) {\n"
        "            event.preventDefault();\n"
        "            var kpVal = document.getElementById(\"kpInput\").value;\n"
        "            var kdVal = document.getElementById(\"kdInput\").value;\n"
        "            var ltVal = document.getElementById(\"ltInput\").value;\n"
        "            var rtVal = document.getElementById(\"rtInput\").value;\n"
        "            \n"
        "            var params = \"kp=\" + kpVal + \"&kd=\" + kdVal + \"&lt=\" + ltVal + \"&rt=\" + rtVal;\n"
        "\n"
        "            var xhr = new XMLHttpRequest();\n"
        "            xhr.open(\"GET\", \"/get?\" + params, true);\n"
        "            xhr.onreadystatechange = function() {\n"
        "                if (xhr.readyState === 4 && xhr.status === 200) {\n"
        "                    document.getElementById(\"response\").innerHTML = xhr.responseText;\n"
        "                }\n"
        "            };\n"
        "            xhr.send();\n"
        "        });\n"
        "    </script>\n"
        "</body>\n"
        "</html>";
    
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_handler_str(httpd_req_t *req)
{
    char *buf;
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[32];
            
            if (httpd_query_key_value(buf, "kp", param, sizeof(param)) == ESP_OK) {
                Kp = atof(param);
                ESP_LOGI(TAG_WIFI, "New Kp: %f", Kp);
            }
            if (httpd_query_key_value(buf, "kd", param, sizeof(param)) == ESP_OK) {
                Kd = atof(param);
                ESP_LOGI(TAG_WIFI, "New Kd: %f", Kd);
            }
            if (httpd_query_key_value(buf, "lt", param, sizeof(param)) == ESP_OK) {
                left_turn_threshold = atoi(param);
                ESP_LOGI(TAG_WIFI, "New Left Threshold: %d", left_turn_threshold);
            }
            if (httpd_query_key_value(buf, "rt", param, sizeof(param)) == ESP_OK) {
                right_turn_threshold = atoi(param);
                ESP_LOGI(TAG_WIFI, "New Right Threshold: %d", right_turn_threshold);
            }
        }
        free(buf);
    }

    char resp[128];
    snprintf(resp, sizeof(resp), "Updated! Kp: %.4f, Kd: %.4f, L_Thresh: %d, R_Thresh: %d", Kp, Kd, left_turn_threshold, right_turn_threshold);
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
     config.core_id = 0;
    httpd_handle_t server = NULL;
   
    
    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_handler,
        .user_ctx = NULL};

    httpd_uri_t uri_get_input = {
        .uri = "/get",
        .method = HTTP_GET,
        .handler = get_handler_str,
        .user_ctx = NULL};

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_get_input);
    }
    return server;
}