/* QMK 0.22.14のquantum_keycodes.hをinclude pathに置き、ホストCコンパイラで実行する。 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "quantum_keycodes.h"
#include "../qmk_firmware/keyboards/keyball/keyball39/keymaps/my_custom/config.h"

typedef uint32_t layer_state_t;
typedef struct { int8_t x, y, h, v; uint8_t buttons; } report_mouse_t;
typedef struct {
    struct { bool pressed, valid; } event;
    struct { uint8_t count; } tap;
} keyrecord_t;

/* 時刻・イベント・レイヤー制御だけを置き換え、製品の判定関数を直接検証する。 */
#define IS_NOEVENT(event) (!(event).valid)
#define IS_MOUSEKEY(keycode) ((keycode) >= KC_MS_UP && (keycode) <= KC_MS_ACCEL2)
#define TEST_SCRL_MO 0x7E0C
static uint32_t test_now;
static layer_state_t test_layers;
static bool test_enabled = true;
static bool test_toggle;
static unsigned test_resets;
static uint32_t timer_read32(void) { return test_now; }
static uint8_t get_auto_mouse_layer(void) { return AUTO_MOUSE_DEFAULT_LAYER; }
static bool get_auto_mouse_enable(void) { return test_enabled; }
static bool get_auto_mouse_toggle(void) { return test_toggle; }
static bool layer_state_is(uint8_t layer) { return (test_layers & ((layer_state_t)1 << layer)) != 0; }
static bool is_mouse_record_kb(uint16_t keycode, keyrecord_t *record) { (void)record; return keycode == TEST_SCRL_MO; }
static void auto_mouse_reset_trigger(bool pressed) {
    assert(pressed);
    ++test_resets;
    test_layers = 0;
}
static int test_mouse_tracker;
static unsigned test_delay_clears;
static void auto_mouse_keyevent(bool pressed) {
    test_mouse_tracker += pressed ? 1 : -1;
    ++test_delay_clears;
}

#include "../qmk_firmware/keyboards/keyball/keyball39/keymaps/my_custom/features/auto_mouse_guard_qmk.h"

static unsigned checks;
#define CHECK(condition) do { ++checks; assert(condition); } while (0)

static void reset_test(void) {
    memset(&keyball_auto_mouse_guard, 0, sizeof(keyball_auto_mouse_guard));
    keyball_auto_mouse_owned = false;
    keyball_auto_mouse_buttons_down = false;
    keyball_auto_mouse_held_keys = 0;
    test_now = 1000;
    test_layers = 0;
    test_enabled = true;
    test_toggle = false;
    test_resets = 0;
    test_mouse_tracker = 0;
    test_delay_clears = 0;
}

static void raw(int8_t x, int8_t y) {
    report_mouse_t report = {0};
    report.x = x;
    report.y = y;
    keyball_auto_mouse_observe_report(report);
}

static bool activate(void) {
    report_mouse_t report = {0};
    return auto_mouse_activation(report);
}

static void key(uint16_t keycode, bool pressed, uint8_t taps) {
    keyrecord_t record = {0};
    record.event.valid = true;
    record.event.pressed = pressed;
    record.tap.count = taps;
    keyball_auto_mouse_on_key(keycode, &record);
    keyball_auto_mouse_layer_changed(test_layers);
}

static void enter_auto_layer(void) {
    raw(KEYBALL_AUTO_MOUSE_DISTANCE, 0);
    CHECK(activate());
    test_layers = (layer_state_t)1 << AUTO_MOUSE_DEFAULT_LAYER;
    keyball_auto_mouse_layer_changed(test_layers);
    CHECK(keyball_auto_mouse_owned);
}

int main(void) {
    reset_test();
    raw(1, 0);
    CHECK(!activate());
    for (int i = 1; i < KEYBALL_AUTO_MOUSE_DISTANCE; ++i) {
        test_now += 100;
        raw(1, 0);
    }
    CHECK(activate());
    CHECK(!activate());

    reset_test();
    raw(7, 0);
    test_now += 250;
    raw(1, 0);
    CHECK(activate());
    reset_test();
    raw(7, 0);
    test_now += 251;
    raw(1, 0);
    CHECK(!activate());

    reset_test();
    raw(7, 0);
    key(KC_Q, true, 1);
    raw(127, 127);
    CHECK(!activate());
    test_now += 500;
    raw(127, 0);
    CHECK(!activate());
    ++test_now;
    raw(1, 0);
    CHECK(!activate());
    raw(7, 0);
    CHECK(activate());

    reset_test();
    key(KC_Q, true, 1);
    test_now += 400;
    key(KC_Q, false, 1);
    test_now += 101;
    raw(100, 0);
    CHECK(!activate());
    test_now += 400;
    raw(8, 0);
    CHECK(activate());

    reset_test();
    for (int i = 0; i < 8; ++i) raw(1, 0);
    CHECK(activate()); /* hookのskip中にもrawを取り込む。 */
    reset_test();
    raw(-128, -128);
    CHECK(activate());
    reset_test();
    test_now = 65530;
    key(KC_Q, true, 1);
    test_now += 501;
    raw(8, 0);
    CHECK(activate());
    reset_test();
    test_now = UINT32_MAX - 200;
    key(KC_Q, true, 1);
    test_now = 299;
    raw(8, 0);
    CHECK(!activate());
    test_now = 300;
    raw(8, 0);
    CHECK(activate());
    test_now = UINT32_MAX - 100;
    raw(8, 0);
    CHECK(activate()); /* 解除済み抑制を古い時刻から復活させない。 */

    reset_test();
    enter_auto_layer();
    raw(0, 0);
    CHECK(!activate());
    raw(1, 0);
    CHECK(activate());
    CHECK(!activate());
    test_layers = 0;
    keyball_auto_mouse_layer_changed(test_layers);
    raw(1, 0);
    CHECK(!activate());

    reset_test();
    key(KC_Q, true, 1);
    report_mouse_t explicit_report = {0};
    explicit_report.buttons = 1;
    CHECK(auto_mouse_activation(explicit_report));
    explicit_report.buttons = 0;
    explicit_report.v = 1;
    CHECK(auto_mouse_activation(explicit_report));

    reset_test();
    enter_auto_layer();
    key(KC_Q, true, 1);
    CHECK(test_resets == 1 && !layer_state_is(AUTO_MOUSE_DEFAULT_LAYER));
    CHECK(keyball_auto_mouse_guard.typing_blocked);
    raw(8, 0);
    CHECK(!activate()); /* Qの次のJはbaseレイヤーから解決される。 */

    const uint16_t manual_keys[] = {TO(1), TG(1), MO(1), LM(1, 0), TT(1), LT(1, KC_A)};
    for (unsigned i = 0; i < sizeof(manual_keys) / sizeof(manual_keys[0]); ++i) {
        reset_test();
        enter_auto_layer();
        key(manual_keys[i], true, 0);
        key(KC_Q, true, 1);
        CHECK(test_resets == 0 && layer_state_is(AUTO_MOUSE_DEFAULT_LAYER));
    }
    const uint16_t modifiers[] = {KC_LEFT_CTRL, MT(0x01, KC_A), LT(2, KC_A)};
    for (unsigned i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); ++i) {
        reset_test();
        enter_auto_layer();
        key(modifiers[i], true, 0);
        CHECK(test_resets == 0);
    }
    const uint16_t tap_keys[] = {MT(0x01, KC_A), LT(1, KC_A)};
    for (unsigned i = 0; i < sizeof(tap_keys) / sizeof(tap_keys[0]); ++i) {
        reset_test();
        enter_auto_layer();
        key(tap_keys[i], true, 1);
        CHECK(test_resets == 1);
    }
    const uint16_t mouse_keys[] = {KC_MS_BTN1, TEST_SCRL_MO};
    for (unsigned i = 0; i < sizeof(mouse_keys) / sizeof(mouse_keys[0]); ++i) {
        reset_test();
        enter_auto_layer();
        key(mouse_keys[i], true, 0);
        key(KC_Q, true, 1);
        CHECK(test_resets == 0 && keyball_auto_mouse_held_keys == 1);
        key(mouse_keys[i], false, 0);
        key(KC_Q, true, 1);
        CHECK(test_resets == 1 && keyball_auto_mouse_held_keys == 0);
    }
    reset_test();
    enter_auto_layer();
    explicit_report.buttons = 1;
    keyball_auto_mouse_observe_report(explicit_report);
    key(KC_Q, true, 1);
    CHECK(test_resets == 0);
    reset_test();
    enter_auto_layer();
    test_toggle = true;
    key(KC_Q, true, 1);
    CHECK(test_resets == 0);

    /* スクロールは打鍵直後でもQMKの遅延ごと解除し、続くボール移動も抑制しない。 */
    report_mouse_t scroll_reports[3] = {{0}, {0}, {0}};
    scroll_reports[0].v = 1;
    scroll_reports[1].h = -1;
    scroll_reports[2].buttons = 1;
    for (unsigned i = 0; i < sizeof(scroll_reports) / sizeof(scroll_reports[0]); ++i) {
        reset_test();
        key(KC_Q, true, 1);
        key(KC_Q, false, 1);
        keyball_auto_mouse_observe_report(scroll_reports[i]);
        CHECK(!keyball_auto_mouse_guard.typing_blocked);
        CHECK(test_delay_clears > 0 && test_mouse_tracker == 0);
        CHECK(auto_mouse_activation(scroll_reports[i]));
        raw(KEYBALL_AUTO_MOUSE_DISTANCE, 0);
        CHECK(activate());
    }

    /* 移動のないレポートでは打鍵後の抑制を解除しない。 */
    reset_test();
    key(KC_Q, true, 1);
    raw(0, 0);
    CHECK(keyball_auto_mouse_guard.typing_blocked && test_delay_clears == 0);

    /* レイヤー上のボタンとSCRL_MOも打鍵後の抑制を解除する。 */
    for (unsigned i = 0; i < sizeof(mouse_keys) / sizeof(mouse_keys[0]); ++i) {
        reset_test();
        key(KC_Q, true, 1);
        key(mouse_keys[i], true, 0);
        CHECK(!keyball_auto_mouse_guard.typing_blocked);
        key(mouse_keys[i], false, 0);
        raw(KEYBALL_AUTO_MOUSE_DISTANCE, 0);
        CHECK(activate());
    }

    /* レイヤーやModのホールド操作は打鍵ではなく、押下でも解放でも抑制を始めない。 */
    const uint16_t hold_keys[] = {LT(2, KC_SPACE), MT(0x01, KC_A), MO(2), KC_LEFT_CTRL};
    for (unsigned i = 0; i < sizeof(hold_keys) / sizeof(hold_keys[0]); ++i) {
        reset_test();
        key(hold_keys[i], true, 0);
        CHECK(!keyball_auto_mouse_guard.typing_blocked);
        key(hold_keys[i], false, 0);
        CHECK(!keyball_auto_mouse_guard.typing_blocked);
        raw(KEYBALL_AUTO_MOUSE_DISTANCE, 0);
        CHECK(activate());
    }

    /* ホールド解放より前の文字入力の抑制は短縮しない。 */
    reset_test();
    key(KC_Q, true, 1);
    key(LT(2, KC_SPACE), false, 0);
    raw(KEYBALL_AUTO_MOUSE_DISTANCE, 0);
    CHECK(!activate());

    /* タップされたLT/MTと修飾付き記号は打鍵として抑制する。 */
    const uint16_t typed_keys[] = {LT(2, KC_SPACE), MT(0x01, KC_A), LSFT(KC_1)};
    for (unsigned i = 0; i < sizeof(typed_keys) / sizeof(typed_keys[0]); ++i) {
        reset_test();
        key(typed_keys[i], true, 1);
        CHECK(keyball_auto_mouse_guard.typing_blocked);
        raw(KEYBALL_AUTO_MOUSE_DISTANCE, 0);
        CHECK(!activate());
    }

    reset_test();
    raw(7, 0);
    test_enabled = false;
    raw(1, 0);
    test_enabled = true;
    raw(1, 0);
    CHECK(!activate());
    key(KC_MS_BTN1, false, 0);
    CHECK(keyball_auto_mouse_held_keys == 0);
    keyrecord_t no_event = {0};
    keyball_auto_mouse_on_key(KC_Q, &no_event);
    CHECK(!keyball_auto_mouse_guard.typing_blocked);
    printf("PASS: %u checks using production guard and QMK adapter\n", checks);
    return 0;
}
