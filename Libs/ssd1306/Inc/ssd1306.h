/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev
 * (afiskon) in 2018. Removed SPI support and added new features to suit project
 * needs by Yusuf Yıldırım (antaresrvish) in 2026.
 * https://github.com/antaresrvish/stm32-ssd1306
 */

#ifndef __SSD1306_H__
#define __SSD1306_H__

#include <_ansi.h>
#include <stdint.h>

_BEGIN_STD_C

#include "ssd1306_conf.h"

/* STM32 HAL Selection based on Family */
#if defined(STM32WB)
#include "stm32wbxx_hal.h"
#elif defined(STM32F0)
#include "stm32f0xx_hal.h"
#elif defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#elif defined(STM32L0)
#include "stm32l0xx_hal.h"
#elif defined(STM32L1)
#include "stm32l1xx_hal.h"
#elif defined(STM32L4)
#include "stm32l4xx_hal.h"
#elif defined(STM32L5)
#include "stm32l5xx_hal.h"
#elif defined(STM32F3)
#include "stm32f3xx_hal.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal.h"
#elif defined(STM32F7)
#include "stm32f7xx_hal.h"
#elif defined(STM32G0)
#include "stm32g0xx_hal.h"
#elif defined(STM32G4)
#include "stm32g4xx_hal.h"
#elif defined(STM32C0)
#include "stm32c0xx_hal.h"
#elif defined(STM32U5)
#include "stm32u5xx_hal.h"
#else
#error                                                                         \
    "SSD1306 library was tested only on STM32F0, STM32F1, STM32F3, STM32F4, STM32F7, STM32L0, STM32L1, STM32L4, STM32H7, STM32G0, STM32G4, STM32WB, STM32C0, STM32U5 MCU families. Please modify ssd1306.h if you know what you are doing. Also please send a pull request if it turns out the library works on other MCU's as well!"
#endif

/* Offset definitions */
#ifdef SSD1306_X_OFFSET
#define SSD1306_X_OFFSET_LOWER (SSD1306_X_OFFSET & 0x0F)
#define SSD1306_X_OFFSET_UPPER ((SSD1306_X_OFFSET >> 4) & 0x07)
#else
#define SSD1306_X_OFFSET_LOWER 0
#define SSD1306_X_OFFSET_UPPER 0
#endif

/* vvv I2C config vvv */
#ifndef SSD1306_I2C_PORT
/** @brief I2C Peripheral handle used for the display. */
#define SSD1306_I2C_PORT hi2c1
#endif

#ifndef SSD1306_I2C_ADDR
/** @brief I2C Address of the SSD1306 (default 0x78 shifted, 0x3C actual). */
#define SSD1306_I2C_ADDR (0x3C << 1)
#endif
/* ^^^ I2C config ^^^ */

extern I2C_HandleTypeDef SSD1306_I2C_PORT;

/** @brief SSD1306 OLED height in pixels. */
#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT 64
#endif

/** @brief SSD1306 OLED width in pixels. */
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH 128
#endif

/** @brief Total frame buffer size in bytes. */
#ifndef SSD1306_BUFFER_SIZE
#define SSD1306_BUFFER_SIZE SSD1306_WIDTH *SSD1306_HEIGHT / 8
#endif

/** @brief Maximum character length of the software scroll buffer. */
#ifndef SCROLL_BUF_MAX
#define SCROLL_BUF_MAX 128
#endif

/**
 * @brief Number of space characters inserted between the end and the
 *        beginning of the text in the circular scroll buffer.
 */
#ifndef SCROLL_GAP
#define SCROLL_GAP 1
#endif

/** @brief Interval in milliseconds between each scroll step. */
#ifndef SCROLL_INTERVAL_MS
#define SCROLL_INTERVAL_MS 350
#endif

/** @brief Maximum number of scroll labels that can be registered globally. */
#ifndef SCROLL_REGISTRY_MAX
#define SCROLL_REGISTRY_MAX 8
#endif

/**
 * @brief Color enumeration for the display.
 */
typedef enum {
  BLACK = 0x00, /**< Pixel OFF */
  WHITE = 0x01  /**< Pixel ON  */
} SSD1306_COLOR;

/**
 * @brief Library error status enumeration.
 */
typedef enum {
  SSD1306_OK = 0x00, /**< Operation successful. */
  SSD1306_ERR = 0x01 /**< Generic error.        */
} SSD1306_Error_t;

/**
 * @brief Font definition structure.
 */
typedef struct {
  uint8_t width;  /**< Font width in pixels (used for monospaced fonts). */
  uint8_t height; /**< Font height in pixels.                            */
  const uint16_t *data;      /**< Pointer to the font bitmap data array.      */
  const uint8_t *char_width; /**< Per-character width table for proportional
                                fonts; NULL for monospaced fonts. */
} SSD1306_Font_t;

/**
 * @brief Software scrolling label state structure.
 *
 * Holds all state needed to display a continuously scrolling text label
 * on a fixed-width region of the screen.
 */
typedef struct {
  char buf[SCROLL_BUF_MAX]; /**< Circular text buffer ("text   " with gap). */
  int len;                  /**< Length of buf content; 0 means no scrolling. */
  int offset;            /**< Current rotation start index in buf.          */
  uint32_t last_tick;    /**< HAL tick value at the last scroll step.       */
  uint8_t x;             /**< X position of the label on the display.       */
  uint8_t y;             /**< Y position of the label on the display.       */
  uint8_t visible_chars; /**< Number of characters visible at once.         */
  uint8_t font_w;        /**< Width of a single character in pixels.        */
  SSD1306_Font_t font;   /**< Font used to render the scrolling text.       */
} SSD1306_Scroll_t;

/**
 * @brief Internal struct to track display state.
 */
typedef struct {
  uint16_t CurrentX;   /**< Current cursor X position. */
  uint16_t CurrentY;   /**< Current cursor Y position. */
  uint8_t Initialized; /**< Non-zero after ssd1306_Init() completes. */
  uint8_t DisplayOn;   /**< Non-zero when the display is powered on.  */
} SSD1306_t;

/**
 * @brief 2-D point coordinate structure.
 */
typedef struct {
  uint8_t x; /**< Horizontal coordinate. */
  uint8_t y; /**< Vertical coordinate.   */
} SSD1306_VERTEX;

/* =========================================================================
 *  Core API
 * ========================================================================= */

/**
 * @brief  Initializes the SSD1306 display.
 * @note   Must be called before any other library function.
 *         Resets the display, sends all configuration commands,
 *         clears the screen buffer, and pushes it to the hardware.
 */
void ssd1306_Init(void);

/**
 * @brief  Fills the entire screen buffer with the specified color.
 * @param  color  Color to fill with (BLACK or WHITE).
 * @note   Call ssd1306_UpdateScreen() afterwards to push changes to the
 * display.
 */
void ssd1306_Fill(SSD1306_COLOR color);

/**
 * @brief  Pushes the internal RAM buffer to the SSD1306 over I2C.
 * @note   Drawing functions only modify the internal buffer; call this
 *         function to make the changes visible on the physical display.
 */
void ssd1306_UpdateScreen(void);

/**
 * @brief  Draws a single pixel at the given coordinates.
 * @param  x      X-coordinate (0 … SSD1306_WIDTH  - 1).
 * @param  y      Y-coordinate (0 … SSD1306_HEIGHT - 1).
 * @param  color  Pixel color (BLACK or WHITE).
 */
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);

/**
 * @brief  Writes a single ASCII character at the current cursor position.
 * @param  ch     Character to write (printable ASCII, 32–126).
 * @param  Font   Font structure to use.
 * @param  color  Foreground color of the character.
 * @return The character written on success, or 0 on failure
 *         (out-of-bounds or unsupported character).
 */
char ssd1306_WriteChar(char ch, SSD1306_Font_t Font, SSD1306_COLOR color);

/**
 * @brief  Writes a null-terminated string at the current cursor position.
 * @param  str    Pointer to the string.
 * @param  Font   Font structure to use.
 * @param  color  Foreground color of the text.
 * @return '\0' on success, or the first character that could not be written.
 */
char ssd1306_WriteString(char *str, SSD1306_Font_t Font, SSD1306_COLOR color);

/**
 * @brief  Moves the text cursor to the specified position.
 * @param  x  New X-coordinate.
 * @param  y  New Y-coordinate.
 */
void ssd1306_SetCursor(uint8_t x, uint8_t y);

/**
 * @brief  Draws a straight line between two points using Bresenham's algorithm.
 * @param  x1     Start X-coordinate.
 * @param  y1     Start Y-coordinate.
 * @param  x2     End X-coordinate.
 * @param  y2     End Y-coordinate.
 * @param  color  Color of the line.
 */
void ssd1306_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                  SSD1306_COLOR color);

/**
 * @brief  Draws and fills a rectangle defined by two corner points.
 * @param  x1     X-coordinate of the first corner.
 * @param  y1     Y-coordinate of the first corner.
 * @param  x2     X-coordinate of the opposite corner.
 * @param  y2     Y-coordinate of the opposite corner.
 * @param  color  Fill color (BLACK or WHITE).
 */
void ssd1306_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                           SSD1306_COLOR color);

/**
 * @brief  Renders a monochrome bitmap image into the screen buffer.
 * @param  x       Top-left X-coordinate of the bitmap.
 * @param  y       Top-left Y-coordinate of the bitmap.
 * @param  bitmap  Pointer to the bitmap data (row-major, MSB first).
 * @param  w       Bitmap width in pixels.
 * @param  h       Bitmap height in pixels.
 * @param  color   Drawing color (usually WHITE).
 */
void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char *bitmap,
                        uint8_t w, uint8_t h, SSD1306_COLOR color);

/**
 * @brief  Sets the display contrast (brightness).
 * @param  value  Contrast level (0x00 = lowest … 0xFF = highest).
 * @note   The hardware default / reset value is 0x7F.
 */
void ssd1306_SetContrast(const uint8_t value);

/**
 * @brief  Turns the display panel ON or puts it into sleep mode.
 * @param  on  1 to power ON, 0 to power OFF (sleep).
 */
void ssd1306_SetDisplayOn(const uint8_t on);

/**
 * @brief  Returns the current power state of the display.
 * @return 1 if the display is ON, 0 if it is OFF.
 */
uint8_t ssd1306_GetDisplayOn(void);

/**
 * @brief  Initializes a software scroll label structure.
 * @param  scroll            Pointer to an SSD1306_Scroll_t instance to initialize.
 * @param  x              X position of the label on the display (pixels).
 * @param  y              Y position of the label on the display (pixels).
 * @param  visible_chars  Number of characters that fit in the visible window.
 * @param  font           Font to use when rendering the scrolling text.
 */
void ssd1306_ScrollInit(SSD1306_Scroll_t *scroll, uint8_t x, uint8_t y,
                        uint8_t visible_chars, SSD1306_Font_t font);

/**
 * @brief  Loads new text into the scroll label.
 * @param  scroll   Pointer to an initialized SSD1306_Scroll_t instance.
 * @param  text  Null-terminated string to display.
 * @note   If the text fits within the visible window, it is shown statically.
 *         Otherwise, a circular scroll buffer is prepared and scrolling begins
 *         on the next ssd1306_ScrollTick() / ssd1306_ScrollTickAll() call.
 *         The display is updated immediately after this call.
 */
void ssd1306_ScrollWrite(SSD1306_Scroll_t *scroll, const char *text);

/**
 * @brief  Advances the scroll label by one step if the scroll interval has
 * elapsed.
 * @param  scroll  Pointer to an initialized and loaded SSD1306_Scroll_t instance.
 * @note   Call this function periodically (e.g. from your main loop or a timer
 *         callback). The scroll speed is controlled by SCROLL_INTERVAL_MS.
 */
void ssd1306_ScrollTick(SSD1306_Scroll_t *scroll);

/**
 * @brief  Registers a scroll label in the global registry.
 * @param  scroll  Pointer to the SSD1306_Scroll_t instance to register.
 * @note   Registered labels are updated automatically by
 * ssd1306_ScrollTickAll(). Up to SCROLL_REGISTRY_MAX labels may be registered.
 */
void ssd1306_ScrollRegister(SSD1306_Scroll_t *scroll);

/**
 * @brief  Calls ssd1306_ScrollTick() on every registered scroll label.
 * @note   Call this function periodically (e.g. from your main loop) to advance
 *         all registered scroll labels without managing them individually.
 */
void ssd1306_ScrollTickAll(void);

/**
 * @brief  Performs a software reset of the SSD1306 (sends 0x2E command).
 */
void ssd1306_Reset(void);

/**
 * @brief  Sends a single command byte to the display over I2C.
 * @param  byte  Command byte.
 */
void ssd1306_WriteCommand(uint8_t byte);

/**
 * @brief  Sends a sequence of command bytes to the display over I2C.
 * @param  cmds  Pointer to the command byte array.
 * @param  len   Number of commands in the array.
 */
void ssd1306_CommandList(const uint8_t *cmds, uint8_t len);

/**
 * @brief  Sends raw data bytes to the display GDDRAM over I2C.
 * @param  buffer     Pointer to the data.
 * @param  buff_size  Number of bytes to send.
 */
void ssd1306_WriteData(uint8_t *buffer, size_t buff_size);

/**
 * @brief  Copies an external buffer directly into the library's screen buffer.
 * @param  buf  Pointer to the source buffer.
 * @param  len  Number of bytes to copy.
 * @return SSD1306_OK on success, SSD1306_ERR if len exceeds
 * SSD1306_BUFFER_SIZE.
 */
SSD1306_Error_t ssd1306_FillBuffer(uint8_t *buf, uint32_t len);

_END_STD_C

#endif /* __SSD1306_H__ */