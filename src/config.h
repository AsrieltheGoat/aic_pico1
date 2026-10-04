/*
 * Controller Config
 * WHowe <github.com/whowechina>
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#include <mode.h>

typedef struct __attribute__((packed)) {
    struct {
        uint8_t level_idle;
        uint8_t level_active;
        uint8_t rgb_en:1;
        uint8_t rgb_order:3;
        uint8_t rgb_reserved:4;
        bool led;
    } light;
    struct {
        bool virtual_aic;
        uint8_t mode;
    } reader;
    uint8_t lcd_reserved[2];
    struct {
        uint8_t reserved[4];
    } tweak;
    uint8_t padding[3];
    uint32_t reserved;
} aic_cfg_t;

typedef volatile struct {
    bool debug;
    reader_mode_t mode;
} aic_runtime_t;

extern aic_cfg_t *aic_cfg;
extern aic_runtime_t aic_runtime;

void config_init();
void config_changed(); // Notify the config has changed
void config_factory_reset(); // Reset the config to factory default

#endif
