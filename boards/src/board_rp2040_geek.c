/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Johannes Winter
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "board_ui.h"

#include "get_serial.h"

/* Waveshare RP2040-GEEK "01-LCD" demo (1.14" LCD display) */
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "Debug.h"
#include "LCD_1in14_V2.h"

typedef struct rp2040_geek_console
{
    sFONT* font;
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint16_t width;
    uint16_t height;
    uint16_t forecolor;
    uint16_t backcolor;
    UWORD framebuffer[];
} rp2040_geek_console_t;

static rp2040_geek_console_t* g_console;

static void rp2040_geek_console_clear(void)
{
    if (g_console)
    {
        Paint_Clear(BLACK);
    }
}

static void rp2040_geek_console_flush(void)
{
    if (g_console)
    {
        LCD_1IN14_V2_Display(g_console->framebuffer);
    }
}

static bool rp2040_geek_console_init(void)
{
    if (!g_console)
    {
        g_console = malloc(sizeof(rp2040_geek_console_t) + (LCD_1IN14_V2_HEIGHT * LCD_1IN14_V2_WIDTH * 2u));
        if (!g_console)
        {
            return false;
        }

        DEV_Delay_ms(100);
        if (DEV_Module_Init() != 0)
        {
            free(g_console);
            g_console = NULL;
            return false;
        }

        DEV_SET_PWM(0);

        /* LCD Init */
        LCD_1IN14_V2_Init(HORIZONTAL);
        LCD_1IN14_V2_Clear(BLACK);
        DEV_SET_PWM(75);

        /* Init the console structure (metrics are available after LCD init) */
        g_console->font      = &Font16;
        g_console->cursor_x  = 0u;
        g_console->cursor_y  = 0u;
        g_console->width     = LCD_1IN14_V2.WIDTH  / g_console->font->Width;
        g_console->height    = LCD_1IN14_V2.HEIGHT / g_console->font->Height;
        g_console->forecolor = WHITE;
        g_console->backcolor = BLACK;

        /* Create a black background image (in the frame buffer) */
        Paint_NewImage((UBYTE *) g_console->framebuffer, LCD_1IN14_V2.WIDTH, LCD_1IN14_V2.HEIGHT, 0, BLACK);
        Paint_SetScale(65);
        Paint_Clear(BLACK);
        Paint_SetRotate(ROTATE_0);
        Paint_Clear(BLACK);

        /* Flush the console */
        LCD_1IN14_V2_Display(g_console->framebuffer);
    }

    return true;
}

static void rp2040_geek_console_setcolor(uint16_t forecolor, uint16_t backcolor)
{
    if (g_console)
    {
        g_console->forecolor = forecolor;
        g_console->backcolor = backcolor;
    }
}

static void rp2040_geek_console_putchar(char ch)
{
    if (g_console)
    {
        uint16_t old_x = g_console->cursor_x;
        uint16_t old_y = g_console->cursor_y;

        uint16_t new_x = old_x;
        uint16_t new_y = old_y;

        if (ch == '\r')
        {
            // Carriag-Return (CR)
            new_x = 0u;
        }
        else if (ch == '\n')
        {
            // Line-feed (LF)
            new_y += 1u;
        }
        else if (ch == '\t')
        {
            // Horizontal Tab (\t)
            new_x += 8u;
        }
        else if (ch == '\v')
        {
            // Vertical Tab (handled like a line feed)
            new_y += 1u;
        }
        else
        {
            // Some (printable?) character
            ch = (ch >= 0x20 && ch <= 0x7E) ? ch : ' ';

            sFONT *font = g_console->font;
            Paint_DrawChar(old_x * font->Width, old_y * font->Height,
                ch, g_console->font, g_console->backcolor, g_console->forecolor);

            new_x += 1u;
        }

        // Handle wrapping (after output)
        if (new_x >= g_console->width)
        {
            // Line break
            new_x  = 0u;
            new_y += 1u;
        }

        if (new_y >= g_console->height)
        {
            new_x = 0u;
            new_y = 0u;
        }

        g_console->cursor_x = new_x;
        g_console->cursor_y = new_y;
    }
}

static void rp2040_geek_console_puts(const char *str)
{
    char c;

    while ((c = *str++) != '\0')
    {
        rp2040_geek_console_putchar(c);
    }
}

void board_ui_init(void)
{
    (void) rp2040_geek_console_init();

    // Show same basic information
    rp2040_geek_console_setcolor(WHITE, BLACK);
    rp2040_geek_console_puts("debugprobe\r\n");

    rp2040_geek_console_setcolor(GRAY, BLACK);
    rp2040_geek_console_puts(usb_serial);
    rp2040_geek_console_puts("\r\n");

    rp2040_geek_console_flush();
}
