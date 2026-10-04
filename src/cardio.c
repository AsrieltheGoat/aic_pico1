/*
 * Cardio processing and auto PIN-entry
 * WHowe <github.com/whowechina>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "tusb.h"
#include "usb_descriptors.h"

#include "board_defs.h"

#include "nfc.h"
#include "light.h"

static struct {
    uint8_t current[9];
    uint8_t reported[9];
    uint64_t report_time;
} hid_cardio;



void cardio_report_cardio()
{
    if (!tud_hid_ready()) {
        return;
    }

    uint64_t now = time_us_64();

    if ((memcmp(hid_cardio.current, hid_cardio.reported, 9) != 0) &&
        (now - hid_cardio.report_time > 1000000)) {

        tud_hid_n_report(0x00, hid_cardio.current[0], hid_cardio.current + 1, 8);
        memcpy(hid_cardio.reported, hid_cardio.current, 9);
        hid_cardio.report_time = now;
    }
}

static void update_cardio(nfc_card_t *card)
{
    switch (card->card_type) {
        case NFC_CARD_MIFARE:
            hid_cardio.current[0] = REPORT_ID_EAMU;
            hid_cardio.current[1] = 0xe0;
            hid_cardio.current[2] = 0x04;
            if (card->len == 4) {
                memcpy(hid_cardio.current + 3, card->uid, 4);
                memcpy(hid_cardio.current + 7, card->uid, 2);
            } else if (card->len == 7) {
                memcpy(hid_cardio.current + 3, card->uid + 1, 6);
            }
            break;
        case NFC_CARD_FELICA:
            hid_cardio.current[0] = REPORT_ID_FELICA;
            memcpy(hid_cardio.current + 1, card->uid, 8);
           break;
        case NFC_CARD_VICINITY:
            hid_cardio.current[0] = REPORT_ID_EAMU;
            memcpy(hid_cardio.current + 1, card->uid, 8);
            break;
        default:
            memset(hid_cardio.current, 0, 9);
            return;
    }

    printf(" -> CardIO ");
    for (int i = 1; i < 9; i++) {
        printf("%02X", hid_cardio.current[i]);
    }
  
}

void cardio_run(bool hid_light)
{
    static nfc_card_t old_card = { 0 };

    nfc_rf_field(true);
    nfc_card_t card = nfc_detect_card();
    if (card.card_type != NFC_CARD_NONE) {
        nfc_identify_last_card();
    }
    nfc_rf_field(false);

    if (memcmp(&old_card, &card, sizeof(old_card)) == 0) {
        return;
    }

    old_card = card;

    if (!hid_light) {
        if (card.card_type != NFC_CARD_NONE) {
            light_rainbow(30, 0, aic_cfg->light.level_active);
        } else {
            light_rainbow(1, 3000, aic_cfg->light.level_idle);
        }
    }

    display_card(&card);
    update_cardio(&card);
}

void cardio_clear()
{
    memset(hid_cardio.current, 0, 9);
}
