#include "urg32.h"
#include "urg32_config.h"

#if URG32_WIFI_SCORES_ENABLE
#include "cJSON.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <stdbool.h>
#include <string.h>

static const char *TAG = "urg32_wifi";
static httpd_handle_t server;
static urg32_score_t scores[URG32_SCORE_MAX];
static int score_count;
static bool wifi_started;

static void copy_cstr(char *dst, size_t dst_size, const char *src) {
    if (!dst || dst_size == 0) {
        return;
    }
    if (!src) {
        src = "";
    }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    (void)arg;
    (void)data;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Wi-Fi connected");
    }
}

void urg32_wifi_scores_init(void) {
    if (wifi_started) {
        return;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               wifi_event,
                                               NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               wifi_event,
                                               NULL));

    wifi_config_t wc = {0};
    copy_cstr((char *)wc.sta.ssid, sizeof(wc.sta.ssid), URG32_WIFI_SSID);
    copy_cstr((char *)wc.sta.password,
              sizeof(wc.sta.password),
              URG32_WIFI_PASSWORD);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_started = true;
}

int urg32_score_submit(const char *game, const char *player, uint32_t score) {
    if (!game || !player) {
        return ESP_ERR_INVALID_ARG;
    }
    if (score_count >= URG32_SCORE_MAX) {
        score_count = URG32_SCORE_MAX - 1;
    }
    memmove(&scores[1], &scores[0], sizeof(scores[0]) * score_count);
    copy_cstr(scores[0].game, sizeof(scores[0].game), game);
    copy_cstr(scores[0].player, sizeof(scores[0].player), player);
    scores[0].score = score;
    if (score_count < URG32_SCORE_MAX) {
        score_count++;
    }
    return ESP_OK;
}

static esp_err_t get_scores(httpd_req_t *req) {
    cJSON *root = cJSON_CreateArray();
    if (!root) {
        httpd_resp_send_err(req, 500, "out of memory");
        return ESP_ERR_NO_MEM;
    }
    for (int i = 0; i < score_count; i++) {
        cJSON *o = cJSON_CreateObject();
        if (!o) {
            cJSON_Delete(root);
            httpd_resp_send_err(req, 500, "out of memory");
            return ESP_ERR_NO_MEM;
        }
        cJSON_AddStringToObject(o, "game", scores[i].game);
        cJSON_AddStringToObject(o, "player", scores[i].player);
        cJSON_AddNumberToObject(o, "score", scores[i].score);
        cJSON_AddItemToArray(root, o);
    }
    char *json = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    if (json) {
        httpd_resp_sendstr(req, json);
        cJSON_free(json);
    } else {
        httpd_resp_sendstr(req, "[]");
    }
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t read_request_body(httpd_req_t *req, char *body, size_t size) {
    if (!body || size == 0 || req->content_len <= 0) {
        httpd_resp_send_err(req, 400, "empty body");
        return ESP_FAIL;
    }
    if ((size_t)req->content_len >= size) {
        httpd_resp_send_err(req, 400, "body too large");
        return ESP_FAIL;
    }

    size_t received = 0;
    while (received < (size_t)req->content_len) {
        int n = httpd_req_recv(req,
                               body + received,
                               req->content_len - received);
        if (n == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;
        }
        if (n <= 0) {
            httpd_resp_send_err(req, 400, "failed to read body");
            return ESP_FAIL;
        }
        received += (size_t)n;
    }
    body[received] = '\0';
    return ESP_OK;
}

static esp_err_t post_score(httpd_req_t *req) {
    char body[192];
    if (read_request_body(req, body, sizeof(body)) != ESP_OK) {
        return ESP_FAIL;
    }

    cJSON *root = cJSON_Parse(body);
    if (!root) {
        httpd_resp_send_err(req, 400, "invalid json");
        return ESP_FAIL;
    }
    cJSON *game = cJSON_GetObjectItem(root, "game");
    cJSON *player = cJSON_GetObjectItem(root, "player");
    cJSON *score = cJSON_GetObjectItem(root, "score");
    if (!cJSON_IsString(game) ||
        !cJSON_IsString(player) ||
        !cJSON_IsNumber(score) ||
        score->valuedouble < 0.0 ||
        score->valuedouble > (double)UINT32_MAX) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, 400, "expected game, player, score");
        return ESP_FAIL;
    }
    int err = urg32_score_submit(game->valuestring,
                                 player->valuestring,
                                 (uint32_t)score->valuedouble);
    cJSON_Delete(root);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, 400, "invalid score");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, "{\"ok\":true}");
    return ESP_OK;
}

void urg32_scores_api_start(void) {
    if (server) {
        return;
    }
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    esp_err_t err = httpd_start(&server, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP server start failed: %s", esp_err_to_name(err));
        server = NULL;
        return;
    }
    httpd_uri_t gs = {
        .uri = "/api/scores",
        .method = HTTP_GET,
        .handler = get_scores
    };
    httpd_uri_t ps = {
        .uri = "/api/scores",
        .method = HTTP_POST,
        .handler = post_score
    };
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &gs));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &ps));
}
#else
void urg32_wifi_scores_init(void) {}
void urg32_scores_api_start(void) {}
int urg32_score_submit(const char *game, const char *player, uint32_t score) {
    (void)game;
    (void)player;
    (void)score;
    return 0;
}
#endif
