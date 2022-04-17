#include "py/runtime.h"
#include "tinyusb.h"
#include "tusb_hid.h"
#include "usb.h"
#include "tidal_usb_hid.h"

// Prevent the HID driver automatically reporting no buttons pressed after each
// report, so buttons are considered held until they're explicitly released
void tud_hid_report_complete_cb(uint8_t itf, uint8_t const *report, uint8_t len)
{
    // no-op
}

// Send up to 6 keyboard scancodes.
STATIC mp_obj_t tidal_hid_send_key(size_t n_args, const mp_obj_t *args) {
    // Extract the ints from the micropython input objects.
    uint8_t key[6] = { 0 };
    
    for (uint8_t i=0; i<n_args; i++) {
        key[i] = mp_obj_get_int_truncated(args[i]);
    }

    tinyusb_hid_keyboard_report(key);

    // Calculate the addition and convert to MicroPython object.
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(tidal_hid_send_key_obj, 0, 6, tidal_hid_send_key);


// Send a mouse move report
STATIC mp_obj_t tidal_hid_move_mouse(mp_obj_t a_obj, mp_obj_t b_obj) {
    // Extract the ints from the micropython input objects.
    int a = mp_obj_get_int(a_obj);
    int b = mp_obj_get_int(b_obj);
    
    
    tinyusb_hid_mouse_move_report(a, b, 0, 0);

    // Calculate the addition and convert to MicroPython object.
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(tidal_hid_move_mouse_obj, tidal_hid_move_mouse);

// --------------------------------------------------------------------------- //

// Define the module globals table
STATIC const mp_rom_map_elem_t tidal_hid_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_tilda_hid) },
    { MP_ROM_QSTR(MP_QSTR_send_key), MP_ROM_PTR(&tidal_hid_send_key_obj) },
    { MP_ROM_QSTR(MP_QSTR_move_mouse), MP_ROM_PTR(&tidal_hid_move_mouse_obj) },
};
STATIC MP_DEFINE_CONST_DICT(tidal_hid_module_globals, tidal_hid_module_globals_table);

// Define module object but don't register it at the top level
const mp_obj_module_t tidal_hid_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&tidal_hid_module_globals,
};
