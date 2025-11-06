/* Copyright 2024 EKISUKE
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------
 * 
 */

// キーボードに保存する用のユーザーコンフィグ
typedef union {
    uint32_t raw;
    struct {
        int16_t to_clickable_movement;  // クリックレイヤーが有効になるしきい値
#ifdef DYNAMIC_TAPPING_TERM_ENABLE
        int16_t tapping_term;           // タッピングタームの設定値
#endif
    };
} user_config_t;

user_config_t user_config;