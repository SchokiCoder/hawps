/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _CSI_H
#define _CSI_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/ioctl.h>

#define CSI_COLORSTRING_LEN 19

/* Single character strings, and characters.
 * This exists because C can't add chars to a string at compile time. Too bad.
 * Hit anyone who adds multichar strings here.
 */
#define CHAR_ESCAPE    '\x1b'
#define CSI_ESCAPE     "\x1b"
#define CHAR_BACKSPACE '\x7f'
#define CSI_BACKSPACE  "\x7f"

/* Sequences.
 */
#define CSI_FG_DEFAULT    CSI_ESCAPE "[39m"
#define CSI_BG_DEFAULT    CSI_ESCAPE "[49m"
#define CSI_CURSOR_HIDE   CSI_ESCAPE "[?25l"
#define CSI_CURSOR_SHOW   CSI_ESCAPE "[?25h"

#define CSI_KEY_UP         CSI_ESCAPE "[A"
#define CSI_KEY_DOWN       CSI_ESCAPE "[B"
#define CSI_KEY_RIGHT      CSI_ESCAPE "[C"
#define CSI_KEY_LEFT       CSI_ESCAPE "[D"
#define CSI_KEY_INSERT     CSI_ESCAPE "[2~"
#define CSI_KEY_DELETE     CSI_ESCAPE "[3~"
#define CSI_KEY_HOME       CSI_ESCAPE "[H"
#define CSI_KEY_END        CSI_ESCAPE "[F"
#define CSI_KEY_PGUP       CSI_ESCAPE "[5~"
#define CSI_KEY_PGDOWN     CSI_ESCAPE "[6~"
#define CSI_KEY_CTRLHOME   CSI_ESCAPE "[1;5H"
#define CSI_KEY_CTRLEND    CSI_ESCAPE "[1;5F"
#define CSI_KEY_CTRLPGUP   CSI_ESCAPE "[5;5~"
#define CSI_KEY_CTRLPGDOWN CSI_ESCAPE "[6;5~"

/* This is a double sequence for clear and cursor to top-left pos.
 * Without this, empty lines remain in the scrollback.
 */
#define CSI_CLEAR         CSI_ESCAPE "[2J" CSI_KEY_HOME

#define CSI_ENABLE_MOUSE  CSI_ESCAPE "[?1003h" CSI_ESCAPE "[?1006h"
#define CSI_DISABLE_MOUSE CSI_ESCAPE "[?1003l" CSI_ESCAPE "[?1006l"

enum MouseButton {
	CSI_MB_LEFT = 0,
	CSI_MB_LEFT_DRAG = 32,
	CSI_MB_MIDDLE = 1,
	CSI_MB_MIDDLE_DRAG = 33,
	CSI_MB_RIGHT = 2,
	CSI_MB_RIGHT_DRAG = 34,
	CSI_MB_HOVER = 35,
	CSI_MB_WHEELUP = 64,
	CSI_MB_WHEELDOWN = 65,
};

struct winsize
CSI_get_size(void);

/* @r: Red
 * @g: Green
 * @b: and Blue all in 0 to 255.
 * @is_fg: Return string for a foreground, otherwise background.
 * @str: Destination string.
 * @str_size: Destination string size, not length.
 *
 * Returns the amount of written bytes.
 */
size_t
CSI_color_to_string(const unsigned char r,
                    const unsigned char g,
                    const unsigned char b,
                    const bool          is_fg,
                    char               *str,
                    const size_t        str_size);

void
CSI_set_cursorpos(const int x,
                  const int y);

void
CSI_set_normal(void);

void
CSI_set_raw(void);

#endif /* _CSI_H */
