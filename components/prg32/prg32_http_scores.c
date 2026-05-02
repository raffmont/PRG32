#include "prg32.h"
#include "prg32_config.h"

#if PRG32_WIFI_ENABLE

#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include <stdio.h>
#include <string.h>

static prg32_score_t scores[PRG32_SCORE_MAX];
static int score_count;

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

static int copy_json_string(char *dst, size_t dst_size, const char *src) {
    if (!dst || dst_size == 0 || !src) {
        return ESP_ERR_INVALID_ARG;
    }
    size_t out = 0;
    for (size_t in = 0; src[in]; ++in) {
        unsigned char ch = (unsigned char)src[in];
        const char *escape = NULL;
        if (ch == '"' || ch == '\\') {
            escape = ch == '"' ? "\\\"" : "\\\\";
        } else if (ch < 32) {
            ch = '?';
        }
        if (escape) {
            if (out + 2 >= dst_size) {
                return ESP_ERR_INVALID_ARG;
            }
            dst[out++] = escape[0];
            dst[out++] = escape[1];
        } else {
            if (out + 1 >= dst_size) {
                return ESP_ERR_INVALID_ARG;
            }
            dst[out++] = (char)ch;
        }
    }
    dst[out] = '\0';
    return ESP_OK;
}

int prg32_score_submit(const char *game, const char *player, uint32_t score) {
    if (!game || !player) {
        return ESP_ERR_INVALID_ARG;
    }
    if (score_count >= PRG32_SCORE_MAX) {
        score_count = PRG32_SCORE_MAX - 1;
    }
    memmove(&scores[1], &scores[0], sizeof(scores[0]) * score_count);
    copy_cstr(scores[0].game, sizeof(scores[0].game), game);
    copy_cstr(scores[0].player, sizeof(scores[0].player), player);
    scores[0].score = score;
    if (score_count < PRG32_SCORE_MAX) {
        score_count++;
    }
    return ESP_OK;
}

int prg32_score_submit_remote(const char *base_url,
                              const char *game,
                              const char *player,
                              uint32_t score) {
    if (!base_url || !game || !player) {
        return ESP_ERR_INVALID_ARG;
    }

    char url[160];
    int n = snprintf(url, sizeof(url), "%s/api/scores", base_url);
    if (n < 0 || (size_t)n >= sizeof(url)) {
        return ESP_ERR_INVALID_ARG;
    }

    char game_json[40];
    char player_json[40];
    if (copy_json_string(game_json, sizeof(game_json), game) != ESP_OK ||
        copy_json_string(player_json, sizeof(player_json), player) != ESP_OK) {
        return ESP_ERR_INVALID_ARG;
    }

    char body[160];
    n = snprintf(body,
                 sizeof(body),
                 "{\"game\":\"%s\",\"player\":\"%s\",\"score\":%lu}",
                 game_json,
                 player_json,
                 (unsigned long)score);
    if (n < 0 || (size_t)n >= sizeof(body)) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_config_t cfg = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 3000
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) {
        return ESP_ERR_NO_MEM;
    }
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, body, (int)strlen(body));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        if (status < 200 || status >= 300) {
            err = ESP_FAIL;
        }
    }
    esp_http_client_cleanup(client);
    return err;
}

#if PRG32_WIFI_SCORES_ENABLE
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
    int err = prg32_score_submit(game->valuestring,
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
#endif

void prg32_http_register_score_handlers(httpd_handle_t server) {
#if PRG32_WIFI_SCORES_ENABLE
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
#else
    (void)server;
#endif
}

#else

int prg32_score_submit(const char *game, const char *player, uint32_t score) {
    (void)game;
    (void)player;
    (void)score;
    return 0;
}

int prg32_score_submit_remote(const char *base_url,
                              const char *game,
                              const char *player,
                              uint32_t score) {
    (void)base_url;
    (void)game;
    (void)player;
    (void)score;
    return -1;
}

void prg32_http_register_score_handlers(void *server) {
    (void)server;
}

#endif
