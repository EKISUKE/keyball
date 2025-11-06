/*
 * Copyright 2025 EKISUKE
 * キー入力の自分カスタマイズ用ヘッダー
 */


static void send_value_report(int16_t value)
{
#ifdef SEND_STRING_ENABLE
    const char *str = get_u16_str(value, ' ');
    // Skip padding spaces
    while (*str == ' ') {
        str++;
    }
    send_string(str);
#endif
}

bool process_record_user(uint16_t keycode, keyrecord_t *record)
{
    if (record->event.pressed == false) {
        return true;
    }
    switch (keycode)
    {
    case KBC_SAVE:
#ifdef DYNAMIC_TAPPING_TERM_ENABLE
        // 現在の g_tapping_term の値を保存
        user_config.tapping_term = g_tapping_term;
        send_value_report(user_config.tapping_term);
#endif
        eeconfig_update_user(user_config.raw);
        // kb側のセーブが効くようにtrueで返す。
        return true;
    default:
        break;
    }
    return true;
}