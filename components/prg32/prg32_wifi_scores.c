#include "prg32.h"
#include "prg32_config.h"

#if PRG32_WIFI_ENABLE
#include "cJSON.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "prg32_wifi";
static httpd_handle_t server;
static prg32_score_t scores[PRG32_SCORE_MAX];
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

void prg32_wifi_scores_init(void) {
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
#if PRG32_WIFI_STA_ENABLE
    esp_netif_create_default_wifi_sta();
#endif
#if PRG32_WIFI_AP_ENABLE
    esp_netif_create_default_wifi_ap();
#endif

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
#if PRG32_WIFI_STA_ENABLE
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               wifi_event,
                                               NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               wifi_event,
                                               NULL));

    wifi_config_t wc = {0};
    copy_cstr((char *)wc.sta.ssid, sizeof(wc.sta.ssid), PRG32_WIFI_SSID);
    copy_cstr((char *)wc.sta.password,
              sizeof(wc.sta.password),
              PRG32_WIFI_PASSWORD);
#endif
#if PRG32_WIFI_AP_ENABLE
    wifi_config_t ap = {0};
    copy_cstr((char *)ap.ap.ssid, sizeof(ap.ap.ssid), PRG32_WIFI_AP_SSID);
    copy_cstr((char *)ap.ap.password,
              sizeof(ap.ap.password),
              PRG32_WIFI_AP_PASSWORD);
    ap.ap.ssid_len = strlen((const char *)ap.ap.ssid);
    ap.ap.channel = PRG32_WIFI_AP_CHANNEL;
    ap.ap.max_connection = PRG32_WIFI_AP_MAX_CONN;
    ap.ap.authmode = strlen((const char *)ap.ap.password) >= 8
        ? WIFI_AUTH_WPA_WPA2_PSK
        : WIFI_AUTH_OPEN;
#endif

#if PRG32_WIFI_STA_ENABLE && PRG32_WIFI_AP_ENABLE
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap));
#elif PRG32_WIFI_STA_ENABLE
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
#elif PRG32_WIFI_AP_ENABLE
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap));
#endif

    ESP_ERROR_CHECK(esp_wifi_start());
#if PRG32_WIFI_AP_ENABLE
    ESP_LOGI(TAG,
             "Upload AP started: ssid=%s password=%s url=http://192.168.4.1",
             PRG32_WIFI_AP_SSID,
             PRG32_WIFI_AP_PASSWORD);
#endif
    wifi_started = true;
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

static void add_json_u32(cJSON *obj, const char *name, uint32_t value) {
    cJSON_AddNumberToObject(obj, name, (double)value);
}

static void add_import(cJSON *imports, const char *name, uintptr_t addr) {
    add_json_u32(imports, name, (uint32_t)addr);
}

static esp_err_t send_runtime(httpd_req_t *req) {
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        httpd_resp_send_err(req, 500, "out of memory");
        return ESP_ERR_NO_MEM;
    }
    cJSON_AddStringToObject(root, "name", "PRG32");
    cJSON_AddStringToObject(root, "cart_magic", PRG32_CART_MAGIC);
    cJSON_AddNumberToObject(root, "cart_abi_major", PRG32_CART_ABI_MAJOR);
    cJSON_AddNumberToObject(root, "cart_abi_minor", PRG32_CART_ABI_MINOR);
    add_json_u32(root, "cart_load_addr", (uint32_t)prg32_cart_load_addr());
    add_json_u32(root, "cart_ram_size", (uint32_t)prg32_cart_ram_size());
#if CONFIG_PRG32_DISPLAY_QEMU_RGB
    cJSON_AddBoolToObject(root, "qemu", true);
#else
    cJSON_AddBoolToObject(root, "qemu", false);
#endif

    prg32_cart_info_t info;
    if (prg32_cart_get_info(&info) == 0) {
        cJSON *cart = cJSON_AddObjectToObject(root, "cart");
        cJSON_AddStringToObject(cart, "name", info.name);
        cJSON_AddBoolToObject(cart, "loaded", info.loaded != 0);
        cJSON_AddBoolToObject(cart, "stored", info.stored != 0);
        add_json_u32(cart, "code_size", info.code_size);
        add_json_u32(cart, "mem_size", info.mem_size);
        add_json_u32(cart, "generation", info.generation);
    }

    cJSON *imports = cJSON_AddObjectToObject(root, "imports");
    add_import(imports, "prg32_ticks_ms", (uintptr_t)prg32_ticks_ms);
    add_import(imports, "prg32_input_read", (uintptr_t)prg32_input_read);
    add_import(imports, "prg32_controller_read", (uintptr_t)prg32_controller_read);
    add_import(imports, "prg32_audio_beep", (uintptr_t)prg32_audio_beep);
    add_import(imports, "prg32_console_clear", (uintptr_t)prg32_console_clear);
    add_import(imports, "prg32_console_putc", (uintptr_t)prg32_console_putc);
    add_import(imports, "prg32_console_write", (uintptr_t)prg32_console_write);
    add_import(imports, "prg32_console_hex32", (uintptr_t)prg32_console_hex32);
    add_import(imports, "prg32_gfx_clear", (uintptr_t)prg32_gfx_clear);
    add_import(imports, "prg32_gfx_present", (uintptr_t)prg32_gfx_present);
    add_import(imports, "prg32_gfx_pixel", (uintptr_t)prg32_gfx_pixel);
    add_import(imports, "prg32_gfx_rect", (uintptr_t)prg32_gfx_rect);
    add_import(imports, "prg32_gfx_text8", (uintptr_t)prg32_gfx_text8);
    add_import(imports, "prg32_tile_clear", (uintptr_t)prg32_tile_clear);
    add_import(imports, "prg32_tile_define", (uintptr_t)prg32_tile_define);
    add_import(imports, "prg32_tile_put", (uintptr_t)prg32_tile_put);
    add_import(imports, "prg32_tile_present", (uintptr_t)prg32_tile_present);
    add_import(imports, "prg32_sprite_hitbox", (uintptr_t)prg32_sprite_hitbox);
    add_import(imports, "prg32_sprite_draw_8x8", (uintptr_t)prg32_sprite_draw_8x8);
    add_import(imports, "prg32_sprite_draw_16x16", (uintptr_t)prg32_sprite_draw_16x16);
    add_import(imports, "prg32_score_submit", (uintptr_t)prg32_score_submit);

    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    httpd_resp_set_type(req, "application/json");
    if (!json) {
        httpd_resp_sendstr(req, "{}");
        return ESP_OK;
    }
    httpd_resp_sendstr(req, json);
    cJSON_free(json);
    return ESP_OK;
}

static esp_err_t get_games(httpd_req_t *req) {
    prg32_cart_info_t info;
    prg32_cart_get_info(&info);
    cJSON *root = cJSON_CreateArray();
    if (!root) {
        httpd_resp_send_err(req, 500, "out of memory");
        return ESP_ERR_NO_MEM;
    }
    cJSON *cart = cJSON_CreateObject();
    if (!cart) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, 500, "out of memory");
        return ESP_ERR_NO_MEM;
    }
    cJSON_AddStringToObject(cart, "slot", "cart0");
    cJSON_AddStringToObject(cart, "name", info.name);
    cJSON_AddBoolToObject(cart, "loaded", info.loaded != 0);
    cJSON_AddBoolToObject(cart, "stored", info.stored != 0);
    add_json_u32(cart, "code_size", info.code_size);
    add_json_u32(cart, "mem_size", info.mem_size);
    add_json_u32(cart, "generation", info.generation);
    cJSON_AddItemToArray(root, cart);

    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    httpd_resp_set_type(req, "application/json");
    if (!json) {
        httpd_resp_sendstr(req, "[]");
        return ESP_OK;
    }
    httpd_resp_sendstr(req, json);
    cJSON_free(json);
    return ESP_OK;
}

static esp_err_t post_game(httpd_req_t *req) {
#if PRG32_GAME_UPLOAD_ENABLE
    if (req->content_len <= 0 ||
        (size_t)req->content_len > PRG32_CART_RAM_SIZE + sizeof(prg32_cart_header_t)) {
        httpd_resp_send_err(req, 400, "invalid cartridge size");
        return ESP_FAIL;
    }
    uint8_t *body = malloc((size_t)req->content_len);
    if (!body) {
        httpd_resp_send_err(req, 500, "out of memory");
        return ESP_ERR_NO_MEM;
    }
    size_t received = 0;
    while (received < (size_t)req->content_len) {
        int n = httpd_req_recv(req,
                               (char *)body + received,
                               req->content_len - received);
        if (n == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;
        }
        if (n <= 0) {
            free(body);
            httpd_resp_send_err(req, 400, "failed to read cartridge");
            return ESP_FAIL;
        }
        received += (size_t)n;
    }
    int err = prg32_cart_install(body, received, 1);
    free(body);
    if (err != 0) {
        httpd_resp_send_err(req, 400, prg32_cart_last_error());
        return ESP_FAIL;
    }
    prg32_cart_info_t info;
    prg32_cart_get_info(&info);
    char response[128];
    snprintf(response,
             sizeof(response),
             "{\"ok\":true,\"slot\":\"cart0\",\"name\":\"%s\"}",
             info.name);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response);
    return ESP_OK;
#else
    httpd_resp_send_err(req, 403, "game upload disabled");
    return ESP_FAIL;
#endif
}

static esp_err_t select_game(httpd_req_t *req) {
    (void)req;
    if (prg32_cart_select_stored() != 0) {
        httpd_resp_send_err(req, 400, prg32_cart_last_error());
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, "{\"ok\":true,\"slot\":\"cart0\"}");
    return ESP_OK;
}

void prg32_scores_api_start(void) {
    if (server) {
        return;
    }
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.max_uri_handlers = 8;
    cfg.recv_wait_timeout = 10;
    esp_err_t err = httpd_start(&server, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP server start failed: %s", esp_err_to_name(err));
        server = NULL;
        return;
    }
    httpd_uri_t rt = {
        .uri = "/api/runtime",
        .method = HTTP_GET,
        .handler = send_runtime
    };
    httpd_uri_t games_get = {
        .uri = "/api/games",
        .method = HTTP_GET,
        .handler = get_games
    };
    httpd_uri_t games_post = {
        .uri = "/api/games",
        .method = HTTP_POST,
        .handler = post_game
    };
    httpd_uri_t games_select = {
        .uri = "/api/games/select",
        .method = HTTP_POST,
        .handler = select_game
    };
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &rt));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &games_get));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &games_post));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &games_select));

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
#endif
}
#else
void prg32_wifi_scores_init(void) {}
void prg32_scores_api_start(void) {}
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
#endif
