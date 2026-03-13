#include "ssd1306.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];
static SSD1306_t SSD1306;

static SSD1306_Scroll_t *scroll_registry[SCROLL_REGISTRY_MAX];
static uint8_t scroll_registry_count = 0;

void ssd1306_Reset(void) { ssd1306_WriteCommand(0x2E); }

void ssd1306_WriteCommand(uint8_t byte) {
  HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x00, 1, &byte, 1,
                    HAL_MAX_DELAY);
}

void ssd1306_WriteData(uint8_t *buffer, size_t buff_size) {
  HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x40, 1, buffer,
                    buff_size, HAL_MAX_DELAY);
}

void ssd1306_Init(void) {
  ssd1306_Reset();
  HAL_Delay(100);

  ssd1306_SetDisplayOn(0);

  ssd1306_WriteCommand(0x20);
  ssd1306_WriteCommand(0x00);
  ssd1306_WriteCommand(0xB0);

#ifdef SSD1306_MIRROR_VERT
  ssd1306_WriteCommand(0xC0);
#else
  ssd1306_WriteCommand(0xC8);
#endif

  ssd1306_WriteCommand(0x00);
  ssd1306_WriteCommand(0x10);
  ssd1306_WriteCommand(0x40);

  ssd1306_SetContrast(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
  ssd1306_WriteCommand(0xA0);
#else
  ssd1306_WriteCommand(0xA1);
#endif

#ifdef SSD1306_INVERSE_COLOR
  ssd1306_WriteCommand(0xA7);
#else
  ssd1306_WriteCommand(0xA6);
#endif

#if (SSD1306_HEIGHT == 128)
  ssd1306_WriteCommand(0xFF);
#else
  ssd1306_WriteCommand(0xA8);
#endif

#if (SSD1306_HEIGHT == 32)
  ssd1306_WriteCommand(0x1F);
#elif (SSD1306_HEIGHT == 64)
  ssd1306_WriteCommand(0x3F);
#elif (SSD1306_HEIGHT == 128)
  ssd1306_WriteCommand(0x3F);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

  ssd1306_WriteCommand(0xA4);
  ssd1306_WriteCommand(0xD3);
  ssd1306_WriteCommand(0x00);
  ssd1306_WriteCommand(0xD5);
  ssd1306_WriteCommand(0xF0);
  ssd1306_WriteCommand(0xD9);
  ssd1306_WriteCommand(0x22);
  ssd1306_WriteCommand(0xDA);

#if (SSD1306_HEIGHT == 32)
  ssd1306_WriteCommand(0x02);
#elif (SSD1306_HEIGHT == 64)
  ssd1306_WriteCommand(0x12);
#elif (SSD1306_HEIGHT == 128)
  ssd1306_WriteCommand(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

  ssd1306_WriteCommand(0xDB);
  ssd1306_WriteCommand(0x20);
  ssd1306_WriteCommand(0x8D);
  ssd1306_WriteCommand(0x14);
  ssd1306_SetDisplayOn(1);

  ssd1306_Fill(BLACK);
  ssd1306_UpdateScreen();

  SSD1306.CurrentX = 0;
  SSD1306.CurrentY = 0;
  SSD1306.Initialized = 1;
}

void ssd1306_Fill(SSD1306_COLOR color) {
  memset(SSD1306_Buffer, (color == BLACK) ? 0x00 : 0xFF,
         sizeof(SSD1306_Buffer));
}

void ssd1306_UpdateScreen(void) {
  for (uint8_t i = 0; i < SSD1306_HEIGHT / 8; i++) {
    ssd1306_WriteCommand(0xB0 + i);
    ssd1306_WriteCommand(0x00 + SSD1306_X_OFFSET_LOWER);
    ssd1306_WriteCommand(0x10 + SSD1306_X_OFFSET_UPPER);
    ssd1306_WriteData(&SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
  }
}

void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
  if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    return;

  if (color == WHITE)
    SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
  else
    SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

char ssd1306_WriteChar(char ch, SSD1306_Font_t Font, SSD1306_COLOR color) {
  uint32_t i, b, j;

  if (ch < 32 || ch > 126)
    return 0;

  const uint8_t char_width =
      Font.char_width ? Font.char_width[ch - 32] : Font.width;
  if (SSD1306_WIDTH < (SSD1306.CurrentX + char_width) ||
      SSD1306_HEIGHT < (SSD1306.CurrentY + Font.height)) {
    return 0;
  }

  for (i = 0; i < Font.height; i++) {
    b = Font.data[(ch - 32) * Font.height + i];
    for (j = 0; j < char_width; j++) {
      if ((b << j) & 0x8000)
        ssd1306_DrawPixel(SSD1306.CurrentX + j, SSD1306.CurrentY + i,
                          (SSD1306_COLOR)color);
      else
        ssd1306_DrawPixel(SSD1306.CurrentX + j, SSD1306.CurrentY + i,
                          (SSD1306_COLOR)!color);
    }
  }
  SSD1306.CurrentX += char_width;
  return ch;
}

char ssd1306_WriteString(char *str, SSD1306_Font_t Font, SSD1306_COLOR color) {
  while (*str) {
    if (ssd1306_WriteChar(*str, Font, color) != *str)
      return *str;
    str++;
  }
  return *str;
}

void ssd1306_SetCursor(uint8_t x, uint8_t y) {
  SSD1306.CurrentX = x;
  SSD1306.CurrentY = y;
}

void ssd1306_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                  SSD1306_COLOR color) {
  int32_t deltaX = abs(x2 - x1);
  int32_t deltaY = abs(y2 - y1);
  int32_t signX = ((x1 < x2) ? 1 : -1);
  int32_t signY = ((y1 < y2) ? 1 : -1);
  int32_t error = deltaX - deltaY;
  int32_t error2;

  ssd1306_DrawPixel(x2, y2, color);

  while ((x1 != x2) || (y1 != y2)) {
    ssd1306_DrawPixel(x1, y1, color);
    error2 = error * 2;
    if (error2 > -deltaY) {
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX) {
      error += deltaX;
      y1 += signY;
    }
  }
}

void ssd1306_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                           SSD1306_COLOR color) {
  uint8_t x_start = ((x1 <= x2) ? x1 : x2);
  uint8_t x_end = ((x1 <= x2) ? x2 : x1);
  uint8_t y_start = ((y1 <= y2) ? y1 : y2);
  uint8_t y_end = ((y1 <= y2) ? y2 : y1);

  for (uint8_t y = y_start; (y <= y_end) && (y < SSD1306_HEIGHT); y++)
    for (uint8_t x = x_start; (x <= x_end) && (x < SSD1306_WIDTH); x++)
      ssd1306_DrawPixel(x, y, color);
}

void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char *bitmap,
                        uint8_t w, uint8_t h, SSD1306_COLOR color) {
  int16_t byteWidth = (w + 7) / 8;
  uint8_t byte = 0;

  if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    return;

  for (uint8_t j = 0; j < h; j++, y++) {
    for (uint8_t i = 0; i < w; i++) {
      if (i & 7)
        byte <<= 1;
      else
        byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));

      if (byte & 0x80)
        ssd1306_DrawPixel(x + i, y, color);
    }
  }
}

void ssd1306_SetContrast(const uint8_t value) {
  const uint8_t kSetContrastControlRegister = 0x81;
  ssd1306_WriteCommand(kSetContrastControlRegister);
  ssd1306_WriteCommand(value);
}

void ssd1306_SetDisplayOn(const uint8_t on) {
  uint8_t value;
  if (on) {
    value = 0xAF;
    SSD1306.DisplayOn = 1;
  } else {
    value = 0xAE;
    SSD1306.DisplayOn = 0;
  }
  ssd1306_WriteCommand(value);
}

uint8_t ssd1306_GetDisplayOn() { return SSD1306.DisplayOn; }

SSD1306_Error_t ssd1306_FillBuffer(uint8_t *buf, uint32_t len) {
  SSD1306_Error_t ret = SSD1306_ERR;
  if (len <= SSD1306_BUFFER_SIZE) {
    memcpy(SSD1306_Buffer, buf, len);
    ret = SSD1306_OK;
  }
  return ret;
}

void ssd1306_CommandList(const uint8_t *cmds, uint8_t len) {
  uint8_t buffer[1 + len];
  buffer[0] = 0x00;
  memcpy(&buffer[1], cmds, len);
  HAL_I2C_Master_Transmit(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, buffer, len + 1,
                          HAL_MAX_DELAY);
}

static void scroll_build_pixel_buf(SSD1306_Scroll_t *scroll) {
  int font_pages = (scroll->font.height + 7) / 8;
  memset(scroll->pixel_buf, 0,
         (size_t)font_pages * SCROLL_PIXEL_COLS_MAX);

  int cur_col = 0;

  for (int ci = 0; ci < scroll->len && scroll->buf[ci]; ci++) {
    char ch = scroll->buf[ci];

    if (ch < 32 || ch > 126) {
      cur_col += scroll->font.width;
      continue;
    }

    const uint8_t cw = scroll->font.char_width
                           ? scroll->font.char_width[ch - 32]
                           : scroll->font.width;

    for (int row = 0; row < scroll->font.height; row++) {
      uint16_t b =
          (uint16_t)scroll->font.data[(ch - 32) * scroll->font.height + row];
      int page = row / 8;
      int bit = row % 8;

      for (int j = 0; j < cw; j++) {
        int dst_col = cur_col + j;
        if (dst_col >= SCROLL_PIXEL_COLS_MAX)
          goto char_done;

        if ((b << j) & 0x8000)
          scroll->pixel_buf[page * SCROLL_PIXEL_COLS_MAX + dst_col] |=
              (uint8_t)(1 << bit);
      }
    }

  char_done:
    cur_col += cw;
    if (cur_col >= SCROLL_PIXEL_COLS_MAX)
      break;
  }

  scroll->total_px = cur_col;
}

static void scroll_blit(const SSD1306_Scroll_t *scroll) {
  if (scroll->total_px == 0)
    return;

  int font_pages = (scroll->font.height + 7) / 8;
  int visible_px = (int)scroll->visible_chars * (int)scroll->font_w;
  int screen_page_start = scroll->y / 8;

  for (int page = 0; page < font_pages; page++) {
    int screen_page = screen_page_start + page;
    if (screen_page >= SSD1306_HEIGHT / 8)
      break;

    for (int col = 0; col < visible_px; col++) {
      int screen_col = (int)scroll->x + col;
      if (screen_col >= SSD1306_WIDTH)
        break;

      int src_col = (scroll->pixel_offset + col) % scroll->total_px;
      SSD1306_Buffer[screen_col + screen_page * SSD1306_WIDTH] =
          scroll->pixel_buf[page * SCROLL_PIXEL_COLS_MAX + src_col];
    }
  }
}

void ssd1306_ScrollInit(SSD1306_Scroll_t *scroll, uint8_t x, uint8_t y,
                        uint8_t visible_chars, SSD1306_Font_t font) {
  memset(scroll, 0, sizeof(SSD1306_Scroll_t));
  scroll->x = x;
  scroll->y = y;
  scroll->visible_chars = visible_chars;
  scroll->font_w = font.width;
  scroll->font = font;
}

void ssd1306_ScrollWrite(SSD1306_Scroll_t *scroll, const char *text) {
  int name_len = (int)strlen(text);

  if (name_len <= (int)scroll->visible_chars) {
    scroll->len = 0;
    scroll->pixel_offset = 0;
    scroll->total_px = 0;

    uint8_t clear_w = scroll->visible_chars * scroll->font_w;
    ssd1306_FillRectangle(scroll->x, scroll->y, scroll->x + clear_w - 1,
                          scroll->y + scroll->font.height - 1, BLACK);
    ssd1306_SetCursor(scroll->x, scroll->y);
    ssd1306_WriteString((char *)text, scroll->font, WHITE);
    ssd1306_UpdateScreen();
    return;
  }

  int total = name_len + SCROLL_GAP;
  if (total >= SCROLL_BUF_MAX)
    total = SCROLL_BUF_MAX - 1;

  memcpy(scroll->buf, text, (name_len < total) ? (size_t)name_len : (size_t)total);
  for (int i = name_len; i < total; i++)
    scroll->buf[i] = ' ';
  scroll->buf[total] = '\0';
  scroll->len = total;

  scroll_build_pixel_buf(scroll);

  scroll->pixel_offset = 0;
  scroll->last_tick = HAL_GetTick();

  scroll_blit(scroll);
  ssd1306_UpdateScreen();
}

void ssd1306_ScrollTick(SSD1306_Scroll_t *scroll) {
  if (scroll->len == 0 || scroll->total_px == 0)
    return;

  uint32_t now = HAL_GetTick();
  if ((now - scroll->last_tick) < SCROLL_INTERVAL_MS)
    return;
  scroll->last_tick = now;

  scroll->pixel_offset =
      (scroll->pixel_offset + SCROLL_SPEED_PX) % scroll->total_px;

  scroll_blit(scroll);
  ssd1306_UpdateScreen();
}

void ssd1306_ScrollRegister(SSD1306_Scroll_t *scroll) {
  if (scroll_registry_count < SCROLL_REGISTRY_MAX)
    scroll_registry[scroll_registry_count++] = scroll;
}

void ssd1306_ScrollTickAll(void) {
  for (uint8_t i = 0; i < scroll_registry_count; i++)
    ssd1306_ScrollTick(scroll_registry[i]);
}