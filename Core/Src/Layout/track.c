#include "track.h"
#include "icons.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "stdint.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <sys/_intsup.h>

SSD1306_Scroll_t track_lbl;
SSD1306_Scroll_t artist_lbl;
SSD1306_Scroll_t duration_t;

void Track_Init_UI(char track_val[], char artist_val[]) {
  ssd1306_Init();
  ssd1306_DrawBitmap(110, 0, info_battery, 24, 8, WHITE);
  ssd1306_DrawBitmap(110, 10, info_headset, 8, 8, WHITE);
  ssd1306_DrawBitmap(120, 10, info_volume, 8, 8, WHITE);
  //ssd1306_DrawBitmap(112, 20, control_memory, 12, 12, WHITE);

  ssd1306_DrawBitmap(0, 0, track_music, 8, 8, WHITE);

  ssd1306_ScrollInit(&track_lbl, 10, 0, 15, Font_6x8);
  ssd1306_ScrollWrite(&track_lbl, track_val);

  ssd1306_DrawBitmap(0, 10, track_artist, 8, 8, WHITE);
  ssd1306_ScrollInit(&artist_lbl, 10, 10, 15, Font_6x8);
  ssd1306_ScrollWrite(&artist_lbl, artist_val);
  // ssd1306_Line(0, 20, 128, 20, WHITE);
  // ssd1306_Line(106, 0, 106, 20, WHITE);
  ssd1306_DrawBitmap(26, 24, control_mix, 8, 8, WHITE);
  ssd1306_DrawBitmap(40, 24, control_back, 8, 8, WHITE);
  ssd1306_DrawBitmap(50, 24, control_pause, 8, 8, WHITE);
  ssd1306_DrawBitmap(60, 24, control_forward, 8, 8, WHITE);
  ssd1306_DrawBitmap(74, 24, control_repeat, 8, 8, WHITE);

  ssd1306_ScrollInit(&duration_t, 104, 24, 4, Font_6x8);
  ssd1306_ScrollWrite(&duration_t, "2:14");

  ssd1306_ScrollRegister(&track_lbl);
  ssd1306_ScrollRegister(&artist_lbl);

  ssd1306_UpdateScreen();
}
