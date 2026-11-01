#pragma once

#include "auto_mouse_guard.h"

static keyball_auto_mouse_guard_t keyball_auto_mouse_guard;
static bool keyball_auto_mouse_owned;
static bool keyball_auto_mouse_buttons_down;
static uint8_t keyball_auto_mouse_held_keys;

static bool keyball_auto_mouse_manual_target(uint16_t keycode, keyrecord_t *record) {
    uint8_t target = get_auto_mouse_layer();
    if (keycode >= QK_TO && keycode <= QK_TO_MAX) return QK_TO_GET_LAYER(keycode) == target;
    if (keycode >= QK_TOGGLE_LAYER && keycode <= QK_TOGGLE_LAYER_MAX) return QK_TOGGLE_LAYER_GET_LAYER(keycode) == target;
    if (keycode >= QK_MOMENTARY && keycode <= QK_MOMENTARY_MAX) return QK_MOMENTARY_GET_LAYER(keycode) == target;
    if (keycode >= QK_LAYER_MOD && keycode <= QK_LAYER_MOD_MAX) return QK_LAYER_MOD_GET_LAYER(keycode) == target;
#ifndef NO_ACTION_ONESHOT
    if (keycode >= QK_ONE_SHOT_LAYER && keycode <= QK_ONE_SHOT_LAYER_MAX) return QK_ONE_SHOT_LAYER_GET_LAYER(keycode) == target;
#endif
#ifndef NO_ACTION_TAPPING
    if (keycode >= QK_LAYER_TAP_TOGGLE && keycode <= QK_LAYER_TAP_TOGGLE_MAX) return QK_LAYER_TAP_TOGGLE_GET_LAYER(keycode) == target;
    if (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) return !record->tap.count && QK_LAYER_TAP_GET_LAYER(keycode) == target;
#endif
    return false;
}

static bool keyball_auto_mouse_is_typing(uint16_t keycode, keyrecord_t *record) {
    if (keycode >= KC_LEFT_CTRL && keycode <= KC_RIGHT_GUI) return false;
    if (keycode >= QK_MODS && keycode <= QK_MODS_MAX) return false;
    if (keycode >= QK_TO && keycode <= QK_TO_MAX) return false;
    if (keycode >= QK_TOGGLE_LAYER && keycode <= QK_TOGGLE_LAYER_MAX) return false;
    if (keycode >= QK_MOMENTARY && keycode <= QK_MOMENTARY_MAX) return false;
    if (keycode >= QK_DEF_LAYER && keycode <= QK_DEF_LAYER_MAX) return false;
    if (keycode >= QK_LAYER_MOD && keycode <= QK_LAYER_MOD_MAX) return false;
#ifndef NO_ACTION_ONESHOT
    if (keycode >= QK_ONE_SHOT_LAYER && keycode <= QK_ONE_SHOT_LAYER_MAX) return false;
    if (keycode >= QK_ONE_SHOT_MOD && keycode <= QK_ONE_SHOT_MOD_MAX) return false;
#endif
#ifndef NO_ACTION_TAPPING
    if (keycode >= QK_LAYER_TAP_TOGGLE && keycode <= QK_LAYER_TAP_TOGGLE_MAX) return false;
    if ((keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) ||
        (keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX)) return record->tap.count > 0;
#endif
    return true;
}

static void keyball_auto_mouse_on_key(uint16_t keycode, keyrecord_t *record) {
    if (IS_NOEVENT(record->event)) return;
    if (keyball_auto_mouse_manual_target(keycode, record)) {
        keyball_auto_mouse_owned = false;
        return;
    }
    if (IS_MOUSEKEY(keycode) || is_mouse_record_kb(keycode, record)) {
        if (record->event.pressed) {
            // ボタン操作後に抑制が残ると、続くボール移動でレイヤーを維持できない。
            keyball_auto_mouse_clear_typing(&keyball_auto_mouse_guard);
            if (keyball_auto_mouse_held_keys < UINT8_MAX) ++keyball_auto_mouse_held_keys;
        } else if (keyball_auto_mouse_held_keys) {
            --keyball_auto_mouse_held_keys;
        }
        return;
    }
    bool typing = keyball_auto_mouse_is_typing(keycode, record);
    // レイヤーやModのホールドは文字入力ではなく、スクロール用レイヤーの解放直後に操作を止めないよう除外する。
    // S(KC_1)等の記号は文字入力なので抑制するが、QMKに合わせてレイヤー解除の対象にはしない。
    if (typing || (keycode >= QK_MODS && keycode <= QK_MODS_MAX)) {
        keyball_auto_mouse_note_typing(&keyball_auto_mouse_guard, timer_read32());
    }
    if (record->event.pressed && typing &&
        get_auto_mouse_enable() && keyball_auto_mouse_owned && !get_auto_mouse_toggle() &&
        !keyball_auto_mouse_held_keys && !keyball_auto_mouse_buttons_down && layer_state_is(get_auto_mouse_layer())) {
        // QMKの起動状態が残っていても、次の文字をクリックにしない。手動切替とドラッグは維持する。
        keyball_auto_mouse_owned = false;
        auto_mouse_reset_trigger(true);
    }
}

static void keyball_auto_mouse_observe_report(report_mouse_t report) {
    bool active = layer_state_is(get_auto_mouse_layer());
    keyball_auto_mouse_buttons_down = report.buttons != 0;
    if (!active || !get_auto_mouse_enable()) keyball_auto_mouse_owned = false;
    if (report.buttons || report.h || report.v) {
        keyball_auto_mouse_clear_typing(&keyball_auto_mouse_guard);
        // 打鍵後のQMK遅延中はauto_mouse_activationが呼ばれない。遅延だけを解除する公開APIがないため、
        // 押下と解放を対で呼び、押下数を変えずに遅延タイマーを止める。
        auto_mouse_keyevent(true);
        auto_mouse_keyevent(false);
    }
    keyball_auto_mouse_observe(&keyball_auto_mouse_guard, report.x, report.y, timer_read32(), get_auto_mouse_enable(), active);
}

bool auto_mouse_activation(report_mouse_t report) {
    bool active = layer_state_is(get_auto_mouse_layer());
    bool activate = keyball_auto_mouse_consume(&keyball_auto_mouse_guard, timer_read32(), active,
                                               report.buttons || report.h || report.v);
    if (activate && !active) keyball_auto_mouse_owned = true;
    return activate;
}

static void keyball_auto_mouse_layer_changed(layer_state_t state) {
    if (!(state & ((layer_state_t)1 << get_auto_mouse_layer()))) {
        keyball_auto_mouse_owned = false;
        // layer_offの再入で、直前の打鍵による抑制まで消さない。
        keyball_auto_mouse_clear_motion(&keyball_auto_mouse_guard);
    }
}
