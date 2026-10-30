# Build cac behavior + man hinh custom cua Macropad15 vao firmware ZMK.
target_sources_ifdef(CONFIG_ZMK_MACROPAD15 app PRIVATE
    src/lcycle.c
    src/nokia_multitap.c
    src/tap_hold_key.c
)

target_sources_ifdef(CONFIG_ZMK_MACROPAD15_STATUS_SCREEN app PRIVATE
    src/status_screen.c
)
