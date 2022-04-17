#include "py/runtime.h"
#include "py/mphal.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "tusb_console.h"
#include "tidal_usb_console.h"

// USB serial read callbacks from mp
static uint8_t usb_rx_buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE];
static uint8_t usb_cdc_connected;

static void usb_callback_rx(int itf, cdcacm_event_t *event) {
    for (;;) {
        size_t len = 0;
        esp_err_t ret = tinyusb_cdcacm_read(itf, usb_rx_buf, sizeof(usb_rx_buf), &len);
        if (ret != ESP_OK) {
            break;
        }
        if (len == 0) {
            break;
        }
        for (size_t i = 0; i < len; ++i) {
            if (usb_rx_buf[i] == mp_interrupt_char) {
                mp_sched_keyboard_interrupt();
            } else {
                ringbuf_put(&stdin_ringbuf, usb_rx_buf[i]);
            }
        }
    }
}

void usb_callback_line_state_changed(int itf, cdcacm_event_t *event) {
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;
    
    // If dtr && rts are both true, the CDC is connected to a HOST.
    usb_cdc_connected = dtr && rts;
    
    // TODO: identify pattern change that esp-idf uses to signal bootloader reset
}


void usb_tx_strn(const char *str, size_t len) {
    // Write out the data to the CDC interface, but only while the USB host is connected.
    while (usb_cdc_connected && len) {
        size_t l = tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, (uint8_t *)str, len);
        str += l;
        len -= l;
        tud_cdc_n_write_flush(TINYUSB_CDC_ACM_0);
    }
}

void tidal_configure_usb_console() {
    // Configure callbacks for CDC_ACM 0 to pass input to Micropython ringbuffer
    tinyusb_config_cdcacm_t amc_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 256,
        .callback_rx = &usb_callback_rx,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = &usb_callback_line_state_changed,
        .callback_line_coding_changed = NULL
    };
    tusb_cdc_acm_init(&amc_cfg);    
}