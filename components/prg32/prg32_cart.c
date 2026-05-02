#include "prg32.h"
#include "prg32_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define PRG32_CART_PARTITION_LABEL "cart0"
#define PRG32_CART_PARTITION_SUBTYPE 0x40
#define PRG32_CART_HEADER_MIN_SIZE ((size_t)sizeof(prg32_cart_header_t))

typedef void (*prg32_cart_entry_t)(void);

static const char *TAG = "prg32_cart";

uint8_t prg32_cart_exec[PRG32_CART_RAM_SIZE] IRAM_ATTR __attribute__((aligned(16)));

static const esp_partition_t *g_cart_partition;
static SemaphoreHandle_t g_cart_lock;
static prg32_cart_header_t g_header;
static prg32_cart_entry_t g_init;
static prg32_cart_entry_t g_update;
static prg32_cart_entry_t g_draw;
static uint32_t g_generation;
static bool g_loaded;
static bool g_stored;
static char g_error[96] = "no cartridge loaded";

static void set_error(const char *msg) {
    if (!msg) {
        msg = "unknown cartridge error";
    }
    snprintf(g_error, sizeof(g_error), "%s", msg);
#if PRG32_DEBUG
    ESP_LOGW(TAG, "%s", g_error);
#endif
}

static void set_errorf(const char *fmt, ...) {
    if (!fmt) {
        set_error("unknown cartridge error");
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_error, sizeof(g_error), fmt, ap);
    va_end(ap);
#if PRG32_DEBUG
    ESP_LOGW(TAG, "%s", g_error);
#endif
}

static uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t len) {
    crc = ~crc;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xedb88320u & mask);
        }
    }
    return ~crc;
}

static int lock_cart(void) {
    if (!g_cart_lock) {
        return 0;
    }
    return xSemaphoreTake(g_cart_lock, portMAX_DELAY) == pdTRUE ? 0 : -1;
}

static void unlock_cart(void) {
    if (g_cart_lock) {
        xSemaphoreGive(g_cart_lock);
    }
}

static const esp_partition_t *cart_partition(void) {
    if (!g_cart_partition) {
        g_cart_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA,
            (esp_partition_subtype_t)PRG32_CART_PARTITION_SUBTYPE,
            PRG32_CART_PARTITION_LABEL);
    }
    return g_cart_partition;
}

static int validate_header(const prg32_cart_header_t *h,
                           size_t image_size,
                           const uint8_t **payload) {
    if (!h || !payload) {
        set_error("missing cartridge image");
        return -1;
    }
    if (image_size < PRG32_CART_HEADER_MIN_SIZE) {
        set_error("cartridge image is too small");
        return -1;
    }
    if (memcmp(h->magic, PRG32_CART_MAGIC, sizeof(h->magic)) != 0) {
        set_error("bad cartridge magic");
        return -1;
    }
    if (h->abi_major != PRG32_CART_ABI_MAJOR) {
        set_errorf("unsupported cartridge ABI major=%u expected=%u",
                   (unsigned)h->abi_major,
                   (unsigned)PRG32_CART_ABI_MAJOR);
        return -1;
    }
    if (h->header_size < PRG32_CART_HEADER_MIN_SIZE ||
        h->header_size > image_size) {
        set_error("invalid cartridge header size");
        return -1;
    }
    if (h->load_addr != (uint32_t)(uintptr_t)prg32_cart_exec) {
        set_error("cartridge linked for a different runtime address");
        return -1;
    }
    if (h->code_size == 0 || h->code_size > h->mem_size ||
        h->mem_size > PRG32_CART_RAM_SIZE) {
        set_errorf("invalid cartridge size code=%lu mem=%lu ram=%lu",
                   (unsigned long)h->code_size,
                   (unsigned long)h->mem_size,
                   (unsigned long)PRG32_CART_RAM_SIZE);
        return -1;
    }
    if ((size_t)h->header_size + h->code_size > image_size) {
        set_errorf("truncated cartridge payload image=%lu needed=%lu",
                   (unsigned long)image_size,
                   (unsigned long)((size_t)h->header_size + h->code_size));
        return -1;
    }
    if (h->init_offset >= h->code_size ||
        h->update_offset >= h->code_size ||
        h->draw_offset >= h->code_size) {
        set_errorf("entry offset outside code init=%lu update=%lu draw=%lu code=%lu",
                   (unsigned long)h->init_offset,
                   (unsigned long)h->update_offset,
                   (unsigned long)h->draw_offset,
                   (unsigned long)h->code_size);
        return -1;
    }
    if ((h->init_offset & 1u) != 0u ||
        (h->update_offset & 1u) != 0u ||
        (h->draw_offset & 1u) != 0u) {
        set_error("entry offsets must be 2-byte aligned");
        return -1;
    }
    *payload = ((const uint8_t *)h) + h->header_size;
    uint32_t crc = crc32_update(0, *payload, h->code_size);
    if (crc != h->payload_crc32) {
        set_errorf("cartridge payload CRC mismatch expected=0x%08lx got=0x%08lx",
                   (unsigned long)h->payload_crc32,
                   (unsigned long)crc);
        return -1;
    }
    return 0;
}

static int load_image_locked(const void *image, size_t image_size) {
    const prg32_cart_header_t *h = (const prg32_cart_header_t *)image;
    const uint8_t *payload = NULL;
    if (validate_header(h, image_size, &payload) != 0) {
        return -1;
    }

    g_loaded = false;
    memset(prg32_cart_exec, 0, sizeof(prg32_cart_exec));
    memcpy(prg32_cart_exec, payload, h->code_size);
    __asm__ volatile("fence.i" ::: "memory");

    memcpy(&g_header, h, sizeof(g_header));
    g_init = (prg32_cart_entry_t)(void *)(prg32_cart_exec + h->init_offset);
    g_update = (prg32_cart_entry_t)(void *)(prg32_cart_exec + h->update_offset);
    g_draw = (prg32_cart_entry_t)(void *)(prg32_cart_exec + h->draw_offset);
    g_loaded = true;
    g_generation++;
    set_error("ok");
    ESP_LOGI(TAG,
             "loaded cartridge '%s' (%lu bytes code, %lu bytes memory)",
             g_header.name,
             (unsigned long)g_header.code_size,
             (unsigned long)g_header.mem_size);
    return 0;
}

void prg32_cart_init(void) {
    if (!g_cart_lock) {
        g_cart_lock = xSemaphoreCreateMutex();
    }
    g_cart_partition = cart_partition();
    if (!g_cart_partition) {
        set_error("cart0 partition not found");
        ESP_LOGW(TAG, "%s", g_error);
        return;
    }
    prg32_cart_load_stored();
}

uintptr_t prg32_cart_load_addr(void) {
    return (uintptr_t)prg32_cart_exec;
}

size_t prg32_cart_ram_size(void) {
    return sizeof(prg32_cart_exec);
}

uint32_t prg32_cart_generation(void) {
    return g_generation;
}

int prg32_cart_is_loaded(void) {
    return g_loaded ? 1 : 0;
}

int prg32_cart_load_stored(void) {
    const esp_partition_t *part = cart_partition();
    if (!part) {
        set_error("cart0 partition not found");
        return -1;
    }

    uint8_t header_buf[sizeof(prg32_cart_header_t)];
    esp_err_t err = esp_partition_read(part, 0, header_buf, sizeof(header_buf));
    if (err != ESP_OK) {
        set_error("failed to read cart0 header");
        return -1;
    }
    prg32_cart_header_t *h = (prg32_cart_header_t *)header_buf;
    if (memcmp(h->magic, PRG32_CART_MAGIC, sizeof(h->magic)) != 0) {
        g_stored = false;
        set_error("no stored cartridge");
        return -1;
    }
    if (h->header_size < PRG32_CART_HEADER_MIN_SIZE ||
        h->code_size == 0 ||
        h->header_size + h->code_size > part->size) {
        set_error("stored cartridge header is invalid");
        return -1;
    }

    size_t image_size = h->header_size + h->code_size;
    uint8_t *image = malloc(image_size);
    if (!image) {
        set_error("out of memory reading cartridge");
        return -1;
    }
    err = esp_partition_read(part, 0, image, image_size);
    if (err != ESP_OK) {
        free(image);
        set_error("failed to read stored cartridge");
        return -1;
    }

    if (lock_cart() != 0) {
        free(image);
        set_error("failed to lock cartridge runtime");
        return -1;
    }
    int rc = load_image_locked(image, image_size);
    if (rc == 0) {
        g_stored = true;
    }
    unlock_cart();
    free(image);
    return rc;
}

int prg32_cart_install(const void *image, size_t image_size, int persist) {
    if (!image || image_size == 0) {
        set_error("missing cartridge image");
        return -1;
    }
    if (lock_cart() != 0) {
        set_error("failed to lock cartridge runtime");
        return -1;
    }
    int rc = load_image_locked(image, image_size);
    unlock_cart();
    if (rc != 0) {
        return rc;
    }

    if (persist) {
        g_stored = false;
        const esp_partition_t *part = cart_partition();
        if (!part) {
            set_error("cart0 partition not found");
            return -1;
        }
        if (image_size > part->size) {
            set_error("cartridge is larger than cart0 partition");
            return -1;
        }
        esp_err_t err = esp_partition_erase_range(part, 0, part->size);
        if (err == ESP_OK) {
            err = esp_partition_write(part, 0, image, image_size);
        }
        if (err != ESP_OK) {
            set_error("failed to persist cartridge");
            return -1;
        }
        g_stored = true;
    }
    return 0;
}

int prg32_cart_select_stored(void) {
    return prg32_cart_load_stored();
}

int prg32_cart_get_info(prg32_cart_info_t *info) {
    if (!info) {
        return -1;
    }
    memset(info, 0, sizeof(*info));
    if (lock_cart() != 0) {
        return -1;
    }
    if (g_loaded) {
        snprintf(info->name, sizeof(info->name), "%s", g_header.name);
        info->code_size = g_header.code_size;
        info->mem_size = g_header.mem_size;
    }
    info->load_addr = (uint32_t)(uintptr_t)prg32_cart_exec;
    info->generation = g_generation;
    info->loaded = g_loaded ? 1 : 0;
    info->stored = g_stored ? 1 : 0;
    unlock_cart();
    return 0;
}

static int call_entry(prg32_cart_entry_t entry) {
    if (!entry || !g_loaded) {
        return -1;
    }
    if (lock_cart() != 0) {
        return -1;
    }
    if (!g_loaded) {
        unlock_cart();
        return -1;
    }
    uintptr_t base = (uintptr_t)prg32_cart_exec;
    uintptr_t addr = (uintptr_t)entry;
    uintptr_t end = base + g_header.code_size;
    if (addr < base || addr >= end || ((addr - base) & 1u) != 0u) {
        set_error("entry pointer is outside cartridge code range");
        unlock_cart();
        return -1;
    }
    entry();
    unlock_cart();
    return 0;
}

int prg32_cart_call_init(void) {
    return call_entry(g_init);
}

int prg32_cart_call_update(void) {
    return call_entry(g_update);
}

int prg32_cart_call_draw(void) {
    return call_entry(g_draw);
}

const char *prg32_cart_last_error(void) {
    return g_error;
}
