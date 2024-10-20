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

    MKC_CLKTH_I,                                  // マウスレイヤーに移る際の移動値閾値を増加
    MKC_CLKTH_D,                                  // マウスレイヤーに移る際の移動値閾値を減少

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
    case KBC_SAVE:
        eeconfig_update_user(user_config.raw);
        return false;
    }
    return true;
}

// マクロキーの処理を行う関数
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  current_keycode = keycode;             // 押下されたキーコードを保存する
  bool mod_pressed = (get_mods() != 0);  // 修飾キーが押されているかを判定（0でなければ修飾キーが押されている）

  static bool is_gui_active = false;
  static bool is_ctrl_active = false;

  static bool is_lt1_pressed = false;  // レイヤー1の状態を追跡する変数
  static bool is_lt2_pressed = false;  // レイヤー2の状態を追跡する変数
  static bool is_lt3_pressed = false;  // レイヤー3の状態を追跡する変数

  // static bool is_eisuu = false;  // レイヤー1の状態を追跡する変数
  static bool is_kana = false;  // レイヤー1の状態を追跡する変数

  switch (keycode) {
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

      // レイヤー1の間は、TABは "command + タブ" になる
      // レイヤー3の間は、TABは "control + タブ" になる
    case KC_TAB: {
      if (record->event.pressed) {
        is_gui_active = true;

        if (get_highest_layer(layer_state) == 1) {
          // キーダウン時:
          register_code(KC_LGUI);
          is_gui_active = true;
          // tap_code(KC_TAB);
          enable_click_layer();
        } else if (get_highest_layer(layer_state) == 3) {
          // キーダウン時:
          register_code(KC_LCTL);
          is_ctrl_active = true;
          // tap_code(KC_TAB);
          enable_click_layer();
        }
      }
      return true;
    }


      static bool is_lt1_lang2_pressed = false;  // LT(1, KC_LNG2)の状態を追跡
      static bool is_lt1_lang1_pressed = false;  // LT(1, KC_LNG1)の状態を追跡

      // 上位レイヤーから下位レイヤーへ移動できるようにする
    case LT(1, KC_LNG2):  // レイヤー1へのキー
    case LT(1, KC_LNG1):  // レイヤー1へのキー
      if (record->event.pressed) {
        click_timer = timer_read();
        is_lt1_pressed = true;
        if (keycode == LT(1, KC_LNG2)) {
          is_lt1_lang2_pressed = true;
        } else if (keycode == LT(1, KC_LNG1)) {
          is_lt1_lang1_pressed = true;
        }
        layer_on(1);  // レイヤー1をオンにする
        disable_click_layer();

        if (is_lt2_pressed) {
          layer_off(2);  // LT2が既に押されていればレイヤー2をオフにする
        } else if (is_lt3_pressed) {
          layer_off(3);  // LT3が既に押されていればレイヤー3をオフにする
        }
      } else {
        is_lt1_pressed = false;
        if (keycode == LT(1, KC_LNG2)) {
          is_lt1_lang2_pressed = false;
        } else if (keycode == LT(1, KC_LNG1)) {
          is_lt1_lang1_pressed = false;
        }
        if (!is_lt1_lang2_pressed && !is_lt1_lang1_pressed) {
          layer_off(1);
        }
        if (is_lt2_pressed) {
          layer_on(2);  // LT2が押されていればレイヤー2をオンにする
        } else if (is_lt3_pressed) {
          layer_on(3);  // LT3が押されていればレイヤー3をオンにする
        }

        if (timer_elapsed(click_timer) < TAPPING_TERM) {
          // タッピングタイム内に放された場合はタップ動作
          if (keycode == LT(1, KC_LNG2)) {
            tap_code(KC_LNG2);
            // is_eisuu = true;
            is_kana = false;
          } else if (keycode == LT(1, KC_LNG1)) {
            tap_code(KC_LNG1);
            is_kana = true;
          }
        }

        if (is_gui_active) {
          unregister_code(KC_LGUI);
          is_gui_active = false;
        }
      }
      return false;

    case LT(2, KC_V):  // レイヤー2へのキー
    case S(KC_DOT):    // レイヤー2へのキー
      if (record->event.pressed) {
        click_timer = timer_read();
        is_lt2_pressed = true;
        layer_on(2);  // レイヤー2をオンにする
        disable_click_layer();

        if (is_lt1_pressed) {
          layer_off(1);  // LT1が既に押されていればレイヤー1をオフにする
        } else if (is_lt3_pressed) {
          layer_off(3);  // LT3が既に押されていればレイヤー3をオフにする
        }
      } else {
        is_lt2_pressed = false;
        layer_off(2);  // LTが押されていなければレイヤー2をオフにする

        if (is_lt1_pressed) {
          layer_on(1);  // LT1が押されていればレイヤー1をオンにする
        } else if (is_lt3_pressed) {
          layer_on(3);  // LT1が押されていればレイヤー1をオンにする
        }

        if (timer_elapsed(click_timer) < TAPPING_TERM) {
          // タッピングタイム内に放された場合はタップ動作
          if (keycode == LT(2, KC_V)) {
            tap_code(KC_V);
          } else if (keycode == S(KC_DOT)) {
            tap_code16(S(KC_DOT));
          }
        }
      }
      return false;

    case LT(3, KC_ESC):  // レイヤー3へのキー
      if (record->event.pressed) {
        click_timer = timer_read();
        is_lt3_pressed = true;
        layer_on(3);  // レイヤー3をオンにする
        disable_click_layer();

        if (is_lt1_pressed) {
          layer_off(1);  // LT1が既に押されていればレイヤー1をオフにする
        } else if (is_lt2_pressed) {
          layer_off(2);  // LT2が既に押されていればレイヤー2をオフにする
        }
      } else {
        is_lt3_pressed = false;
        layer_off(3);  // LT3をオフにする

        if (is_lt1_pressed) {
          layer_on(1);  // LT1が押されていればレイヤー1をオンにする
        } else if (is_lt2_pressed) {
          layer_on(2);  // LT2が押されていればレイヤー2をオンにする
        }

        if (timer_elapsed(click_timer) < TAPPING_TERM) {
          // タッピングタイム内に放された場合はタップ動作
          tap_code(KC_ESC);
        }

        if (is_ctrl_active) {
          unregister_code(KC_LCTL);
          is_ctrl_active = false;
        }
      }
      return false;

      // SFT_Tのカスタムキーコード
    case SFT_T_G_KC_A:
    case SFT_T_S_KC_SCLN:
      if (record->event.pressed) {
        // キーが押されたとき
        click_timer = timer_read();
        register_code(KC_LSFT);
      } else {
        // キーが放されたとき
        unregister_code(KC_LSFT);

        if (timer_elapsed(click_timer) < TAPPING_TERM) {
          // タッピングタイム内に放された場合はタップ動作
          if (keycode == SFT_T_G_KC_A) {
            tap_code16(G(KC_A));
          }
          if (keycode == SFT_T_S_KC_SCLN) {
            tap_code16(S(KC_SCLN));
          }
        }
      }
      return false;

      // コンボ
    case COMBO_MINUS_SPACE:
      if (record->event.pressed) {
        tap_code(KC_LNG2);  //
        tap_code(KC_MINUS);  // -　を送信
        tap_code(KC_SPACE);  //  スペース を送信
        tap_code(KC_LNG1);  //
      }
      return false;

    case COMBO_BRC:
    case COMBO_select_BRC:
    case COMBO_S9_S0:
    case COMBO_select_S9_S0:
    case COMBO_S_BRC:
    case COMBO_select_S_BRC:
    case COMBO_sumitsuki_BRC:
    case COMBO_select_sumitsuki_BRC:
      if (record->event.pressed) {
        // 選択バージョン
        if (keycode == COMBO_select_BRC || keycode == COMBO_select_S9_S0 || keycode == COMBO_select_S_BRC || keycode == COMBO_select_sumitsuki_BRC) {
          tap_code16(G(KC_X));  // カット
        }

        // if (keycode == COMBO_BRC || keycode == COMBO_S9_S0 || keycode == COMBO_S_BRC || keycode == COMBO_sumitsuki_BRC) {
        if (keycode == COMBO_BRC || keycode == COMBO_select_BRC) {
          tap_code(KC_LBRC);  // 「　を送信
          tap_code(KC_RBRC);  //  」 を送信
        } else if (keycode == COMBO_S9_S0 || keycode == COMBO_select_S9_S0) {
          tap_code16(S(KC_9));  // （　を送信
          tap_code16(S(KC_0));  //  ） を送信
        } else if (keycode == COMBO_S_BRC || keycode == COMBO_select_S_BRC) {
          tap_code16(S(KC_LBRC));  // 『　を送信
          tap_code16(S(KC_RBRC));  //  』 を送信
        } else if (keycode == COMBO_sumitsuki_BRC || keycode == COMBO_select_sumitsuki_BRC) {
          tap_code16(A(KC_9));  // 　を送信
          tap_code16(A(KC_0));  //  ） を送信
        }
        // かなの場合にKC_ENTを送信
        if (is_kana) {
          tap_code(KC_ENT);
        }
        // KC_LEFT を送信
        tap_code(KC_LEFT);
        // }

        // 選択バージョン
        if (keycode == COMBO_select_BRC || keycode == COMBO_select_S9_S0 || keycode == COMBO_select_S_BRC || keycode == COMBO_select_sumitsuki_BRC) {
          tap_code16(G(KC_V));  // ペースト
          wait_ms(130);         // 0.1秒（1300ミリ秒）待機
          tap_code(KC_RIGHT);
        }

      }
      return false;

    // 以下スワイプジェスチャー
    // クリックすると state が SWIPE になり、離したら NONE になる
    case CMD_T(KC_SPACE):
    case SFT_T(KC_LNG2):
    case ALT_T(KC_F13): {
      if (record->event.pressed) {
        // キーダウン時
        state = SWIPE;
      } else {
        // キーアップ時
        clear_mods();           // すべての修飾キーの状態をクリアする
        disable_click_layer();  // クリックレイヤーを無効化する
      }

      // 複数の修飾キーが押された場合、LEDをオフにし、スワイプ状態を解除する
      if (mod_pressed) {
#if RGBLIGHT_ENABLE
        rgbl5ight_sethsv(HSV_OFF);
#endif
        state = NONE;
        return true;  // キーのデフォルトの動作を続行させる
      }

      // スワイプが行われた場合、スワイプフラグをリセット
      if (is_swiped) {
        is_swiped = false;
        return false;  // キーのデフォルトの動作をスキップする
      }

      // スワイプが行われていなかった場合、キーのデフォルトの動作を続行させる
      if (!is_swiped) {
        return true;
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
