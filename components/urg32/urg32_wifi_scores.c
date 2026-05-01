#include "urg32.h"
#include "urg32_config.h"

#if URG32_WIFI_SCORES_ENABLE
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "urg32_wifi";
static httpd_handle_t server;
static urg32_score_t scores[URG32_SCORE_MAX];
static int score_count;

static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) esp_wifi_connect();
    if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) ESP_LOGI(TAG, "Wi-Fi connected");
}

void urg32_wifi_scores_init(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event, NULL);
    wifi_config_t wc = {0};
    strncpy((char *)wc.sta.ssid, URG32_WIFI_SSID, sizeof(wc.sta.ssid));
    strncpy((char *)wc.sta.password, URG32_WIFI_PASSWORD, sizeof(wc.sta.password));
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wc);
    esp_wifi_start();
}

esp_err_t urg32_score_submit(const char *game, const char *player, uint32_t score) {
    if (score_count >= URG32_SCORE_MAX) score_count = URG32_SCORE_MAX - 1;
    memmove(&scores[1], &scores[0], sizeof(scores[0]) * score_count);
    strncpy(scores[0].game, game, sizeof(scores[0].game)-1);
    strncpy(scores[0].player, player, sizeof(scores[0].player)-1);
    scores[0].score = score;
    if (score_count < URG32_SCORE_MAX) score_count++;
    return ESP_OK;
}

static esp_err_t get_scores(httpd_req_t *req) {
    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < score_count; i++) {
        cJSON *o = cJSON_CreateObject();
        cJSON_AddStringToObject(o, "game", scores[i].game);
        cJSON_AddStringToObject(o, "player", scores[i].player);
        cJSON_AddNumberToObject(o, "score", scores[i].score);
        cJSON_AddItemToArray(root, o);
    }
    char *json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json ? json : "[]");
    cJSON_free(json); cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t post_score(httpd_req_t *req) {
    char body[192] = {0};
    int n = httpd_req_recv(req, body, sizeof(body)-1);
    if (n <= 0) { httpd_resp_send_err(req, 400, "empty body"); return ESP_FAIL; }
    cJSON *root = cJSON_Parse(body);
    if (!root) { httpd_resp_send_err(req, 400, "invalid json"); return ESP_FAIL; }
    cJSON *game = cJSON_GetObjectItem(root, "game");
    cJSON *player = cJSON_GetObjectItem(root, "player");
    cJSON *score = cJSON_GetObjectItem(root, "score");
    if (!cJSON_IsString(game) || !cJSON_IsString(player) || !cJSON_IsNumber(score)) {
        cJSON_Delete(root); httpd_resp_send_err(req, 400, "expected game, player, score"); return ESP_FAIL;
    }
    urg32_score_submit(game->valuestring, player->valuestring, (uint32_t)score->valuedouble);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "{\"ok\":true}");
    return ESP_OK;
}

void urg32_scores_api_start(void) {
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    httpd_start(&server, &cfg);
    httpd_uri_t gs = {.uri="/api/scores", .method=HTTP_GET, .handler=get_scores};
    httpd_uri_t ps = {.uri="/api/scores", .method=HTTP_POST, .handler=post_score};
    httpd_register_uri_handler(server, &gs);
    httpd_register_uri_handler(server, &ps);
}
#else
void urg32_wifi_scores_init(void) {}
void urg32_scores_api_start(void) {}
int urg32_score_submit(const char *game, const char *player, uint32_t score) { (void)game; (void)player; (void)score; return 0; }
#endif
