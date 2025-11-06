RGBLIGHT_ENABLE = yes

OLED_ENABLE = yes

# VIA_ENABLE = yes


### 使っていないもの
### 以下はメモリ不足のため、VIAをOFFにする必要がある
# KEY_OVERRIDE_ENABLE = yes
COMBO_ENABLE = yes
# AUTO_SHIFT_ENABLE = yes

# 動的にタッピングタームを変更可能にする
DYNAMIC_TAPPING_TERM_ENABLE = yes

# ファームウェアのサイズを削減
EXTRAFLAGS += -flto
GRAVE_ESC_ENABLE = no  # 90バイト