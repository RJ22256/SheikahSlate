// The HUD, the start screen and the map laid out across a wide window (Video > Wide HUD and menus; services/wide_ui.c)
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// The window's shape over 16:9 the layouts are made for (wide_ui_shape_wanted when the RomFS was opened), else 1
extern float g_wide_ui_shape;
// The shape the settings give (Fill wide screens and Wide HUD and menus on, the window or screen size): 1 for none
float wide_ui_shape_wanted(void);
// The layouts (names without blyt/ and .bflyt) of an unpacked layout archive, and the parts they place over the whole
// screen, changed in place for that shape; returns how many changed
int wide_ui_archive(uint8_t *arc, size_t an, const char *const *layouts, int n, float shape);

#ifdef __cplusplus
}
#endif
