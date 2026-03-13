/**
 * Private configuration file for the SSD1306 library.
 * This example is configured for STM32F4, I2C and including all fonts.
 */

#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

// Choose a microcontroller family
// #define STM32F0
// #define STM32F1
#define STM32F4
// #define STM32L0
// #define STM32L1
// #define STM32L4
// #define STM32F3
// #define STM32H7
// #define STM32F7
// #define STM32G0
// #define STM32C0
// #define STM32U5

// I2C Configuration
#define SSD1306_I2C_PORT hi2c1
#define SSD1306_I2C_ADDR (0x3C << 1)

// Mirror the screen if needed
// #define SSD1306_MIRROR_VERT
// #define SSD1306_MIRROR_HORIZ

// Set inverse color if needed
// #define SSD1306_INVERSE_COLOR

// Include only needed fonts
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18
#define SSD1306_INCLUDE_FONT_16x26
#define SSD1306_INCLUDE_FONT_16x24
#define SSD1306_INCLUDE_FONT_16x15

// The width of the screen can be set using this
// define. The default value is 128.
// #define SSD1306_WIDTH 64

// If your screen horizontal axis does not start
// in column 0 you can use this define to
// adjust the horizontal offset
// #define SSD1306_X_OFFSET

// The height can be changed as well if necessary.
// It can be 32, 64 or 128. The default value is 64.
#define SSD1306_HEIGHT 32

// ---------------------------------------------------------------------------
// Scroll — text buffer
// ---------------------------------------------------------------------------

/** Maximum character length of the scroll text buffer (including gap). */
#define SCROLL_BUF_MAX 128

/**
 * Number of space characters appended after the text before it wraps.
 * Increase for a longer pause between repetitions.
 */
#define SCROLL_GAP 2

/** Maximum number of scroll labels that can be registered globally. */
#define SCROLL_REGISTRY_MAX 8

// ---------------------------------------------------------------------------
// Scroll — pixel-based timing & speed
// ---------------------------------------------------------------------------

/**
 * Milliseconds between each scroll step.
 * Lower  = faster scrolling.
 * Recommended range: 15–40 ms for smooth pixel scrolling.
 */
#define SCROLL_INTERVAL_MS 15

/**
 * Pixels advanced per scroll step.
 * 1 = smoothest,  2–3 = faster but slightly coarser.
 */
#define SCROLL_SPEED_PX 1

// ---------------------------------------------------------------------------
// Scroll — pixel buffer sizing
// ---------------------------------------------------------------------------

/**
 * Maximum number of vertical pages (8 px each) the scroll font can span.
 * Default of 2 covers fonts up to 16 px tall (Font_7x10, Font_6x8 etc.).
 * Set to 3 for fonts up to 24 px, 4 for up to 32 px.
 *
 * RAM per SSD1306_Scroll_t = SCROLL_MAX_PAGES * SCROLL_BUF_MAX * SCROLL_MAX_FONT_W
 * With defaults: 2 * 128 * 8 = 2048 bytes — fine for STM32F4.
 */
#define SCROLL_MAX_PAGES 2

/**
 * Maximum glyph width (pixels) of any font used with scroll labels.
 * Font_6x8  → 6,  Font_7x10 → 7.  Set to the widest font you use.
 */
#define SCROLL_MAX_FONT_W 8

#endif /* __SSD1306_CONF_H__ */