#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "mdns.h"
#include "esp_http_server.h"

#define WIFI_SSID "Squawk7700"
#define WIFI_PASS "yoloswaggins"

esp_err_t get_handler(httpd_req_t *req)
{
  const char resp[] = "<!DOCTYPE html><html><body>Hello from ESP32!</body></html>";
  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t open_handler(httpd_req_t *req)
{
  const char resp[] = "Open";
  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

void start_webserver(void)
{
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  // Start the httpd server
  if (httpd_start(&server, &config) != ESP_OK)
  {
    return;
  }

  httpd_uri_t uri_get = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = get_handler,
    .user_ctx  = NULL
  };

  httpd_register_uri_handler(server, &uri_get);

  httpd_uri_t uri_open = {
    .uri       = "/open",
    .method    = HTTP_GET,
    .handler   = open_handler,
    .user_ctx  = NULL
  };

  httpd_register_uri_handler(server, &uri_open);
}

void mdns_setup(void)
{
  mdns_init();
  mdns_hostname_set("love");
  mdns_instance_name_set("ESP32 WebServer");
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_wifi_connect();
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    ESP_LOGI("WIFI", "got ip");
    mdns_setup();
  }
}

void wifi_init(void)
{
  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id);
  esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip);

  wifi_config_t wifi_config =
  {
    .sta =
    {
      .ssid = WIFI_SSID,
      .password = WIFI_PASS,
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
      .pmf_cfg = {
        .capable = true,
        .required = false
      },
    },
  };

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();
}

void app_main(void)
{
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  wifi_init();

  start_webserver();

  while (true)
  {
    // This task is going to do something very simple
    printf("Idle task is running...\n");

    // Delay for 1 second
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

