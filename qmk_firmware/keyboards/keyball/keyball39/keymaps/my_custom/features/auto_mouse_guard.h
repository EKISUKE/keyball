#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t typing_at;
    uint32_t motion_at;
    uint8_t distance;
    bool typing_blocked;
    bool has_motion;
    bool pending_motion;
    bool was_active;
} keyball_auto_mouse_guard_t;

static inline void keyball_auto_mouse_clear_motion(keyball_auto_mouse_guard_t *guard) {
    guard->distance = 0;
    guard->has_motion = false;
    guard->pending_motion = false;
}

static inline void keyball_auto_mouse_note_typing(keyball_auto_mouse_guard_t *guard, uint32_t now) {
    keyball_auto_mouse_clear_motion(guard);
    guard->typing_at = now;
    guard->typing_blocked = true;
}

static inline bool keyball_auto_mouse_typing_blocked(keyball_auto_mouse_guard_t *guard, uint32_t now) {
    if (guard->typing_blocked && (uint32_t)(now - guard->typing_at) > AUTO_MOUSE_DELAY) {
        // 解除後は古い時刻を再評価せず、タイマー一周後の抑制復活を防ぐ。
        guard->typing_blocked = false;
    }
    return guard->typing_blocked;
}

static inline void keyball_auto_mouse_sync_layer(keyball_auto_mouse_guard_t *guard, bool active) {
    if (guard->was_active != active) {
        keyball_auto_mouse_clear_motion(guard);
        guard->was_active = active;
    }
}

static inline void keyball_auto_mouse_expire_motion(keyball_auto_mouse_guard_t *guard, uint32_t now) {
    if (guard->has_motion && (uint32_t)(now - guard->motion_at) > KEYBALL_AUTO_MOUSE_IDLE_MS) {
        keyball_auto_mouse_clear_motion(guard);
    }
}

static inline void keyball_auto_mouse_observe(keyball_auto_mouse_guard_t *guard, int8_t x, int8_t y,
                                             uint32_t now, bool enabled, bool active) {
    keyball_auto_mouse_sync_layer(guard, active);
    if (!enabled || keyball_auto_mouse_typing_blocked(guard, now)) {
        keyball_auto_mouse_clear_motion(guard);
        return;
    }
    keyball_auto_mouse_expire_motion(guard, now);
    if (!x && !y) {
        return;
    }
    // -128を反転できる型へ拡張してから加算する。
    uint16_t distance = (uint16_t)(x < 0 ? -(int16_t)x : x) + (uint16_t)(y < 0 ? -(int16_t)y : y);
    guard->motion_at = now;
    guard->has_motion = true;
    guard->pending_motion = true;
    if (!active) {
        distance += guard->distance;
        guard->distance = distance >= KEYBALL_AUTO_MOUSE_DISTANCE ? KEYBALL_AUTO_MOUSE_DISTANCE : (uint8_t)distance;
    }
}

static inline bool keyball_auto_mouse_consume(keyball_auto_mouse_guard_t *guard, uint32_t now,
                                             bool active, bool explicit_input) {
    keyball_auto_mouse_sync_layer(guard, active);
    keyball_auto_mouse_expire_motion(guard, now);
    bool movement = false;
    if (!keyball_auto_mouse_typing_blocked(guard, now)) {
        movement = active ? guard->pending_motion : guard->distance >= KEYBALL_AUTO_MOUSE_DISTANCE;
    }
    guard->pending_motion = false;
    if (active || movement || explicit_input) {
        guard->distance = 0;
    }
    return movement || explicit_input;
}
