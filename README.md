# Macropad15 - ZMK Firmware

Ban chuyen **Macropad15** (15 phim + 3 nut xoay EC11 + OLED SSD1306, Pro Micro
RP2040) tu firmware **QMK/Vial** sang **ZMK firmware v0.3**.

Repo nay la mot *config repo* ZMK hoan chinh: chua shield (ma tran, encoder,
OLED), keymap 6 layer, va module code C voi cac behavior custom duoc viet lai
cho ZMK. Build bang GitHub Actions, khong can cai dat gi tren may.

## 1. Build firmware (.uf2) qua GitHub Actions

1. Tao repo moi tren GitHub (vd `macropad15-zmk`) va day toan bo thu muc nay
   len, **giu nguyen cau truc** (`boards/`, `config/`, `src/`, `dts/`,
   `zephyr/`, `.github/`, `build.yaml`).
2. Vao tab **Actions** -> workflow "Build ZMK firmware (Macropad15)" -> doi
   khoang 10-20 phot -> tai artifact `macropad15-sparkfun_pro_micro_rp2040-zmk`
   (ben trong co file `.uf2`).

## 2. Nap firmware

1. Nhan giu nut BOOT tren Pro Micro RP2040, cam USB (hoac nhan RESET 2 lan
   nhanh) -> board hien nhu o dia `RPI-RP2`.
2. Keo file `.uf2` vao o dia do. Board tu khoi dong lai.

## 3. Keymap mac dinh

Giong he 1-1 voi ban QMK/Vial. Thu tu vi tri:

```
      (ENC1)         k0   k1   k2   k3   k4
   (ENC2)  (ENC3)    k5   k6   k7   k8   k9
                     k10  k11  k12  k13  k14
```

Nut nhan ENC1 = `&lcycle` (doi vong layer **Base -> Numpad -> Media -> Nokia**),
ENC2 = Backspace, ENC3 = Tab.

| Vi tri | Layer 0 Base | Layer 1 Layer1 | Layer 2 Numpad | Layer 3 Media | Layer 4 Nokia | Layer 5 FN |
|---|---|---|---|---|---|---|
| k0 | A | V | KP 7 | Prev | NK 7 (7stu) | F1 |
| k1 | B | C | KP 8 | Play | NK 8 (8vwx) | F2 |
| k2 | C | X | KP 9 | Next | NK 9 (9yz) | F3 |
| k3 | D | Z | `-`/`/` | Calculator | Caps Word | F4 |
| k4 | E | Up | MO(5) | MO(5) | MO(5) | (trans) |
| k5 | F | `/` | KP 4 | Home | NK 4 (4jkl) | F5 |
| k6 | G | `.` | KP 5 | Up | NK 5 (5mno) | F6 |
| k7 | H | `,` | KP 6 | End | NK 6 (6pqr) | F7 |
| k8 | I | M | KP Enter | Vol- | Enter | Sys Reset |
| k9 | J | Shift | `+`/`*` | Vol+ | Sticky Shift | Bootloader |
| k10 | K | TO(0) | KP 1 | Left | NK 1 (1abc) | F8 |
| k11 | L | F1 F2 | KP 2 | Down | NK 2 (2def) | F9 |
| k12 | M | F11 PgUp | KP 3 | Right | NK 3 (3ghi) | F10 |
| k13 | N | F11 PgDn | KP 0 | PgUp | NK 0 (0, space) | F11 |
| k14 | O | Del | KP . | PgDn | NK . (.,?!) | F12 |
| ENC1 nut | LCycle | (nhu Base) | LCycle | LCycle | LCycle | Mute |
| ENC2 nut | (nhu Base) | (nhu Base) | (nhu Base) | (nhu Base) | (nhu Base) | (trong) |
| ENC3 nut | (nhu Base) | (nhu Base) | (nhu Base) | (nhu Base) | (nhu Base) | (trong) |

Nut xoay (quay): ENC1 = volume; ENC2 = trai/phai (layer Media: bai truoc/sau);
ENC3 = len/xuong (layer Media: PgUp/PgDn). Giu phim k4 o layer 2-4 de vao layer
FN (F1..F12, Reset, Bootloader, Mute).

Cac phim dac biet:

- **`&lcycle`** — doi vong layer (behavior custom `src/lcycle.c`).
- **`&nokia 1..9/0/10`** — Nokia multitap (behavior custom
  `src/nokia_multitap.c`): bam lien tiep cung phim trong 600 ms de doi ky tu
  (1abc, 2def, ..., 9yz, 0 + khoang cach, . , ? !); ky tu vua go se bi xoa va
  thay bang ky tu ke tiep. Viet hoa khi Shift dang giu (ke ca sticky shift).
- **`&th_key A B`** — bam ngan = A, bam giu = B (behavior custom
  `src/tap_hold_key.c`): dung cho `-`/`/` va `+`/`*` o layer Numpad.
- **`&seq_f1f2`, `&seq_f11pu`, `&seq_f11pd`** — macro bam F1 roi F2, F11 roi
  PgUp/PgDn (ZMK macro thuan, giong ban ESP32-C3).

## 4. Man hinh OLED (SSD1306 128x64)

Day: VCC -> 3V3, GND -> GND, **SDA -> D14 (GP20)**, **SCL -> D10 (GP21)** (I2C0).

- Man hinh nghi: ten layer + trang thai NUM/CAPS + mod dang giu (SFT/CTL/ALT/GUI).
- Khi bam phim: **ten phim bang chu to o giua man hinh** (vd `7`, `ENTER`,
  `VOL +`, `F11`), tu dong quay ve man hinh thong tin sau 0,7 giay.
- Tu tat man hinh khi khong su dung (`CONFIG_ZMK_DISPLAY_BLANK_ON_IDLE`, du
  the cho OLED_TIMEOUT 60s cua QMK).
- Khong lap OLED: firmware van chay binh thuong (co the xoa
  `CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y` trong `config/macropad15.conf`
  de tiet kiem flash).

## 5. Doi chieu voi ban QMK/Vial

| Tinh nang | QMK/Vial | ZMK (repo nay) |
|---|---|---|
| Ma tran 4x5 ROW2COL, 3 nut nhan encoder | `keyboard.json` | `macropad15.overlay` |
| 3 encoder quay (steps 4) | `resolution: 4` | `steps = <4>` |
| 6 layer + keymap | `keymap.c` | `config/macropad15.keymap` |
| LCYCLE (doi vong layer) | code C QMK | behavior `&lcycle` (C module) |
| Nokia multitap 600ms | code C QMK | behavior `&nokia` (C module) |
| Tap/hold `-`/`/`, `+`/`*` | code C QMK (TAPPING_TERM 200) | behavior `&th_key` (C module) |
| F1F2, F11+PgUp/PgDn | code C QMK | ZMK macro goc (`&seq_*`) |
| Caps Word | `CW_TOGG` | `&caps_word` |
| One-shot Shift | `OSM(MOD_LSFT)` | `&sk LSHIFT` (sticky key) |
| RESET / BOOTLOADER | `QK_RBT` / `QK_BOOT` | `&sys_reset` / `&bootloader` |
| Encoder map theo layer | `encoder_map` | `sensor-bindings` tung layer |
| OLED ten layer + phim to + NUM/CAPS/mod | `oled_task_user` | man hinh custom `src/status_screen.c` (LVGL) |
| NKRO | `nkro: true` | `CONFIG_ZMK_HID_REPORT_TYPE_NKRO=y` |
| USB VID/PID | `0xC1A0/0x0015` | `CONFIG_USB_DEVICE_VID/PID` |

## 6. Khac biet can biet so voi Vial

- **Khong co Vial/Via**: ZMK khong sua keymap ngay tren may. Muon doi phim:
  sua `config/macropad15.keymap` roi push de build lai (hoac build local).
- **Macro chuoi ky tu** (vd go email) cua Vial: dung ZMK macro trong keymap, vd
  ```dts
  macro_mail: macro_mail {
      compatible = "zmk,behavior-macro";
      #binding-cells = <0>;
      bindings = <&kp C &kp H &kp I &kp P ...>;
  };
  ```
- **Caps Word voi Nokia multitap**: khac QMK, trang thai Caps Word khong lam
  cac ky tu multitap tu viet hoa (ZMK khong cho behavior khac doc trang thai
  nay). Van viet hoa duoc bang Shift/sticky shift truoc khi bam.
- **2 layer trong (6, 7) cua Vial** bo di: muon them layer, them node moi vao
  `keymap` trong `config/macropad15.keymap`.

## 7. Sua keymap / behavior

- **Doi phim, layer, encoder**: sua `config/macropad15.keymap`. Moi layer la
  mot node; `bindings` gom 18 vi tri theo thu tu bang o tren; `sensor-bindings`
  la 3 nut xoay (thu tu ENC1, ENC2, ENC3).
- **Doi thoi gian tap/hold va Nokia**: `CONFIG_MACROPAD15_TAPPING_TERM_MS` /
  `CONFIG_MACROPAD15_NOKIA_MULTITAP_MS` trong `config/macropad15.conf`.
- **Doi danh sach layer cua LCycle**: property `layers = <0 2 3 4>;` trong node
  `lcycle` (layer index tu 0).

## 8. Xu ly su co

- **Encoder quay nguoc chieu**: doi thu tu 2 keycode trong `sensor-bindings`
  (vd `<&inc_dec_kp C_VOL_UP C_VOL_DN>` thanh `<&inc_dec_kp C_VOL_DN C_VOL_UP>`)
  hoac doi `a-gpios`/`b-gpios` trong overlay.
- **1 nac nhay 2 buoc**: sua `steps = <4>` thanh `steps = <2>` trong overlay
  (tuong ung `resolution` cua QMK).
- **Man hinh bi lat nguoc / soi nguoc mau**: them `inversion-on;` hoac bo
  `segment-remap;`/`com-invdir;` trong node `ssd1306@3c` cua overlay.
- **Firmware cu (Vial/QMK) con giu keymap**: ZMK khong dung EEPROM Vial nen
  khong can xoa; nhung neu doi tu ZMK ve QMK thi nho nap `flash_nuke.uf2`.
- **Khong may chu/ky tu hang xoay dien**: o layer 2 (Numpad), diode cua phim
  va nut nhan encoder phai dung chieu ROW2COL nhu so do goc.

## 9. Cau truc repo

```
├── .github/workflows/build.yml   # Build ZMK qua GitHub Actions
├── build.yaml                    # Ma tran build: board + shield
├── boards/shields/macropad15/    # Shield: ma tran, encoder, OLED, keymap default
├── config/                       # west.yml + keymap + .conf (phan nguoi dung sua)
├── dts/bindings/behaviors/       # Binding yaml cho 3 behavior custom
├── src/                          # Code C: lcycle, nokia multitap, tap-hold, man hinh
├── zephyr/module.yml             # Khai bao repo la ZMK module (build code C)
├── Kconfig                       # Option module (thoi gian tap/hold, bat/tat man hinh)
└── CMakeLists.txt                # Them source C vao build
```
