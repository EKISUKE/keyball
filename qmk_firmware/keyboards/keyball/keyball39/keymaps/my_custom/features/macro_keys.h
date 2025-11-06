/* Copyright 2023 kamidai (@d_kamiichi)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * コード表：
 * - 公式ファームウェアのキーコード：
 *   KBC_RST(0x5DA5): Keyball 設定のリセット
 *   KBC_SAVE(0x5DA6): 現在の Keyball 設定を EEPROM に保存します
 *   CPI_I100(0x5DA7): CPI を 100 増加させます(最大:12000)
 *   CPI_D100(0x5DA8): CPI を 100 減少させます(最小:100)
 *   CPI_I1K(0x5DA9): CPI を 1000 増加させます(最大:12000)
 *   CPI_D1K(0x5DAA): CPI を 1000 減少させます(最小:100)
 *   SCRL_TO(0x5DAB): タップごとにスクロールモードの ON/OFF を切り替えます
 *   SCRL_MO(0x5DAC): キーを押している間、スクロールモードになります
 *   SCRL_DVI(0x5DAD): スクロール除数を１つ上げます(max D7 = 1/128)← 最もスクロール遅い
 *   SCRL_DVD(0x5DAE): スクロール除数を１つ下げます(min D0 = 1/1)← 最もスクロール速い
 */

enum custom_keycodes {
    KC_BACK_TO_LAYER0_BTN1 = KEYBALL_SAFE_RANGE,  // (0x7E40): レイヤー0に遷移できるBTN1
    KC_DOUBLE_CLICK_BTN1,                         // (0x7E41): 1タップでダブルクリックできるBTN1
    KC_TRIPLE_CLICK_BTN1,                         // (0x7E42): 1タップでトリプルクリックできるBTN1
    SFT_T_G_KC_A,                                 // (0x7E43):
    SFT_T_S_KC_SCLN,                              // (0x7E44):
    COMBO_BRC,                                    // (0x7E45):
    COMBO_select_BRC,                             // (0x7E46):
    COMBO_S9_S0,                                  // (0x7E47):
    COMBO_select_S9_S0,                           // (0x7E48):
    COMBO_S_BRC,                                  // (0x7E49):
    COMBO_select_S_BRC,                           // (0x7E4A):
    COMBO_sumitsuki_BRC,                          // (0x7E4B):
    COMBO_select_sumitsuki_BRC,                   // (0x7E4C):
    COMBO_MINUS_SPACE,                            // (0x7E4D):
#ifndef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    MKC_CLKTH_I,                                  // マウスレイヤーに移る際の移動値閾値を増加
    MKC_CLKTH_D,                                  // マウスレイヤーに移る際の移動値閾値を減少
#endif

    // CUSTOM_LT1_LEFT,                                    //
    // CUSTOM_LT1_LEFT,                                    //
    // CUSTOM_S9,                                    //
    // CUSTOM_S0,                                    //
    // KC_ALT_BTN1,                                  //
    // select_BRC,
    // CMD_SCRL,
};

bool process_record_my_custom(uint16_t keycode, keyrecord_t* record)
{
    if (record->event.pressed == false) {
        return true;
    }
    switch (keycode)
    {
#ifndef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    case MKC_CLKTH_I:
        user_config.to_clickable_movement += 5;
        if (user_config.to_clickable_movement > INT16_MAX) {
            user_config.to_clickable_movement = INT16_MAX;
        }
        return false;
    case MKC_CLKTH_D:
        user_config.to_clickable_movement -= 5;
        if (user_config.to_clickable_movement < 0) {
            user_config.to_clickable_movement = 0;
        }
        return false;
#endif
    case KBC_SAVE:
#ifdef DYNAMIC_TAPPING_TERM_ENABLE
        // 現在の g_tapping_term の値を保存
        user_config.tapping_term = get_tapping_term();
        //tapping_term_report();
#endif
        tap_code(KC_Q);
        eeconfig_update_user(user_config.raw);
        return false;
    }
    return true;
}

// マクロキーの処理を行う関数
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  current_keycode = keycode;             // 押下されたキーコードを保存する
  switch (keycode) {
#ifndef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    // デフォルトのマウスキーを自動クリックレイヤーで使用可能にする
    case KC_MS_BTN1:
    case KC_MS_BTN2:
    case KC_MS_BTN3:
    case KC_MS_BTN4:
    case KC_MS_BTN5: {
      if (click_layer && get_highest_layer(layer_state) == click_layer) {
        if (record->event.pressed) {
          // キーダウン時: 状態をCLICKINGに設定
          state = CLICKING;
        } else {
          // キーアップ時: クリックレイヤーを有効にして、状態をCLICKEDに設定
          enable_click_layer();
          state = CLICKED;
        }
      }
      return true;
    }
#endif

    // 以下のキーは自動クリックレイヤーで使用可能にする
    case KC_LALT:
    case KC_LSFT: {
      return true;
    }
    case SCRL_MO: {
      if (click_layer && get_highest_layer(layer_state) == click_layer) {
        if (record->event.pressed) {
          // キーダウン時: 状態をCLICKINGに設定
          state = CLICKING;
        } else {
          // キーアップ時: クリックレイヤーを有効にして、状態をCLICKEDに設定
          enable_click_layer();
          state = CLICKED;
        }
      }
    }
    case MKC_CLKTH_I:
    case MKC_CLKTH_D:
    case KBC_SAVE:
        return process_record_my_custom(keycode, record);

    // その他のキーコードの場合
    default:
      if (record->event.pressed) {
        // キーダウン時
        disable_click_layer();  // クリックレイヤーを無効化
      }


  }
  return true;
}
