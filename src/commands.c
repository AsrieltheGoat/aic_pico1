#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "hardware/clocks.h"
#include "hardware/i2c.h"

#include "config.h"
#include "save.h"
#include "cli.h"

#include "light.h"

#include "aime.h"
#include "bana.h"
#include "nfc.h"
#include "lib/pn532.h"

#include "cardio.h"

static int fps[2];
void fps_count(int core)
{
    static uint32_t last[2] = {0};
    static int counter[2] = {0};

    counter[core]++;

    uint32_t now = time_us_32();
    if (now - last[core] < 1000000) {
        return;
    }
    last[core] = now;
    fps[core] = counter[core];
    counter[core] = 0;
}

static void display_sys()
{
    int mhz = clock_get_hz(clk_sys) / 1000000;
    printf("[System]\r\n");

    const char *cpu = "RP2040";
    #ifdef PICO_RP2350
        cpu = "RP2350";
    #endif
    printf("  Processor: %s\r\n", cpu);
    printf("  Clock: %d MHz\r\n", mhz);
}

static void display_nfc()
{
    printf("[NFC Module]\r\n");
    printf("    %s (%s)\r\n", nfc_module_name(), nfc_module_version());
}

static void display_light()
{
    printf("[Light]\r\n");
    printf("    RGB-%s (%s), LED-%s\r\n",
            aic_cfg->light.rgb_en ? "ON" : "OFF",
            light_get_rgb_order_string(aic_cfg->light.rgb_order),
            aic_cfg->light.led ? "ON" : "OFF");
    printf("    Level: Idle-%d, Active-%d\r\n", aic_cfg->light.level_idle, aic_cfg->light.level_active);
}


static void display_reader()
{
    printf("[Reader]\r\n");
    printf("    Virtual AIC: %s\r\n", aic_cfg->reader.virtual_aic ? "ON" : "OFF");
    printf("    Mode: %s\r\n", mode_name(aic_cfg->reader.mode));
    if (aic_cfg->reader.mode == MODE_AUTO) {
        printf("    Detected: %s\r\n", mode_name(aic_runtime.mode));
    }
    if ((aic_runtime.mode == MODE_AIME0) || (aic_runtime.mode == MODE_AIME1)) {
        printf("    AIME Pattern: %s\r\n", aime_get_mode_string());
    }
}

static void handle_display()
{
    display_sys();
    display_nfc();
    display_light();
    display_reader();
}

static void handle_save()
{
    save_request(true);
}

static void handle_factory_reset()
{
    config_factory_reset();
    printf("Factory reset done.\r\n");
}

static void handle_nfc()
{
    printf("NFC module: %s\r\n", nfc_module_name());

    nfc_rf_field(true);
    nfc_card_t card = nfc_detect_card();
    nfc_rf_field(false);

    printf("Card: %s", nfc_card_name_str(card.card_type));
    for (int i = 0; i < card.len; i++) {
        printf(" %02x", card.uid[i]);
    }
    printf("\r\n");
}


static void handle_i2cscan()
{
    const uint8_t scl = 27;
    const uint8_t sda = 26;
    const uint32_t freq = 400 * 1000;

    i2c_init(i2c1, freq);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_pull_up(scl);
    gpio_pull_up(sda);

    printf("I2C scan: GPIO%d=SCL GPIO%d=SDA @ %lu kHz\r\n",
           scl, sda, freq / 1000);

    bool found = false;
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        uint8_t dummy;
        int ret = i2c_read_blocking(i2c1, addr, &dummy, 1, false);
        if (ret >= 0) {
            printf("Found device at 0x%02X\r\n", addr);
            found = true;
        }
    }

    if (!found) {
        printf("No I2C devices found. PN532 normally responds at 0x24.\r\n");
    }
}

static void handle_nfcdiag()
{
    const uint8_t scl = 27;
    const uint8_t sda = 26;
    const uint32_t freq = 400 * 1000;

    uint8_t uid[8] = {0};
    uint8_t pmm[8] = {0};
    uint8_t syscode[2] = {0};
    int uid_len = 0;
    uint16_t atqa = 0;
    uint8_t sak = 0;

    i2c_init(i2c1, freq);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_pull_up(scl);
    gpio_pull_up(sda);

    bool module = pn532_init(i2c1);

    printf("PN532 diagnostic\r\n");
    printf("  Module: %s\r\n", module ? "PN532" : "Unknown");
    printf("  I2C: GPIO%d=SCL GPIO%d=SDA @ %lu kHz\r\n",
           scl, sda, freq / 1000);

    if (!module) {
        printf("  RF field: unavailable\r\n");
        printf("  Type A: none\r\n");
        printf("  FeliCa: none\r\n");
        return;
    }

    pn532_rf_field(true);
    bool type_a = pn532_poll_mifare(uid, &uid_len, &atqa, &sak);
    bool felica = pn532_poll_felica(uid, pmm, syscode, false);
    pn532_rf_field(false);

    printf("  Firmware: %s\r\n", pn532_firmware_ver());

    printf("  Type A: ");
    if (type_a) {
        printf("DETECTED UID=");
        for (int i = 0; i < uid_len; i++) {
            printf("%02X", uid[i]);
        }
        printf(" ATQA=%04X SAK=%02X\r\n", atqa, sak);
    } else {
        printf("none\r\n");
    }

    printf("  FeliCa: ");
    if (felica) {
        printf("DETECTED IDm=");
        for (int i = 0; i < 8; i++) {
            printf("%02X", uid[i]);
        }
        printf(" SYS=%02X%02X\r\n", syscode[0], syscode[1]);
    } else {
        printf("none\r\n");
    }
}

static void handle_virtual(int argc, char *argv[])
{
    const char *usage = "Usage: virtual <on|off>\r\n";
    if (argc != 1) {
        printf("%s", usage);
        return;
    }

    const char *commands[] = { "on", "off" };
    int match = cli_match_prefix(commands, 2, argv[0]);
    if (match < 0) {
        printf("%s", usage);
        return;
    }

    aic_cfg->reader.virtual_aic = (match == 0);

    aime_virtual_aic(aic_cfg->reader.virtual_aic);
    config_changed();
    display_reader();
}

static void handle_mode(int argc, char *argv[])
{
    const char *usage = "Usage: mode <auto|aime0|aime1|bana>\r\n"
                        "    auto: Auto detect\r\n"
                        "    aime0: Sega Aime 0\r\n"
                        "    aime1: Sega Aime 1\r\n"
                        "    bana: Bandai Namco\r\n";
    if (argc != 1) {
        printf("%s", usage);
        return;
    }

    reader_mode_t newmode = MODE_NONE;
    const char *commands[] = { "auto", "aime0", "aime1", "bana" };
    int match = cli_match_prefix(commands, 4, argv[0]);
    switch (match) {
        case 0:
            newmode = MODE_AUTO;
            break;
        case 1:
            newmode = MODE_AIME0;
            break;
        case 2:
            newmode = MODE_AIME1;
            break;
        case 3:
            newmode = MODE_BANA;
            break;
        default:
            printf("%s", usage);
            return;
    }

    aic_cfg->reader.mode = newmode;
    aic_runtime.mode = (newmode == MODE_AUTO) ? MODE_NONE : newmode;
    config_changed();
    display_reader();
}

static void handle_light(int argc, char *argv[])
{
    const char *usage = "Usage: light <rgb|led|both|off>\r\n";
    if (argc != 1) {
        printf("%s", usage);
        return;
    }

    const char *commands[] = { "rgb", "led", "both", "off" };
    int match = cli_match_prefix(commands, 4, argv[0]);
    switch (match) {
        case 0:
            aic_cfg->light.rgb_en = true;
            aic_cfg->light.led = false;
            break;
        case 1:
            aic_cfg->light.rgb_en = false;
            aic_cfg->light.led = true;
            break;
        case 2:
            aic_cfg->light.rgb_en = true;
            aic_cfg->light.led = true;
            break;
        case 3:
            aic_cfg->light.rgb_en = false;
            aic_cfg->light.led = false;
            break;
        default:
            printf("%s", usage);
            return;
    }
    config_changed();
    display_light();
}

static void handle_rgb_order(int argc, char *argv[])
{
    const char *usage = "Usage: rgb-order <grb|brg|rgb|rbg>\r\n";
    if (argc != 1) {
        printf("%s", usage);
        return;
    }

    const char *commands[] = { "grb", "brg", "rgb", "rbg" };
    int match = cli_match_prefix(commands, 4, argv[0]);
    if (match < 0) {
        printf("%s", usage);
        return;
    }

    aic_cfg->light.rgb_order = match;
    light_set_rgb_order(aic_cfg->light.rgb_order);
    config_changed();
    printf("RGB order set to %s.\r\n", commands[match]);
}

static void handle_level(int argc, char *argv[])
{
    const char *usage = "Usage: level <dimmed> <active>\r\n"
                        "    dimmed, active: [0..255]\r\n";
    if (argc != 2) {
        printf(usage);
        return;
    }

    int idle = cli_extract_non_neg_int(argv[0], 0);
    int active = cli_extract_non_neg_int(argv[1], 0);
    if ((idle < 0) || (idle > 255) ||
        (active < 0) || (active > 255)) {
        printf(usage);
        return;
    }

    aic_cfg->light.level_idle = idle;
    aic_cfg->light.level_active = active;

    config_changed();
    display_light();
}


static void handle_debug()
{
    aic_runtime.debug = !aic_runtime.debug;
    nfc_runtime.debug = aic_runtime.debug;
    printf("Debug: %s\r\n", aic_runtime.debug ? "ON" : "OFF");
}

void commands_init()
{
    cli_register("display", handle_display, "Display all settings.");
    cli_register("save", handle_save, "Save config to flash.");
    cli_register("factory", handle_factory_reset, "Reset everything to default.");
    cli_register("nfc", handle_nfc, "NFC module.");
    cli_register("virtual", handle_virtual, "Virtual AIC card.");
    cli_register("mode", handle_mode, "Reader mode/protocol.");
    cli_register("light", handle_light, "Turn on/off lights.");
    cli_register("rgb-order", handle_rgb_order, "Set RGB order.");
    cli_register("level", handle_level, "Set light level.");
    cli_register("debug", handle_debug, "Toggle debug.");
    cli_register("nfcdiag", handle_nfcdiag, "Diagnose PN532 RF and card polling.");
    cli_register("i2cscan", handle_i2cscan, "Scan PN532 I2C bus on GPIO 27/26.");
}
