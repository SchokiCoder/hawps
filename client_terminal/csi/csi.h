/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _CSI_H
#define _CSI_H

#include <sys/ioctl.h>

/* This is a double sequence for clear and cursor to top-left pos.
 * Without this, empty lines remain in the scrollback.
 */
#define CSI_CLEAR         "\x1b[2J\033[H"

#define CSI_ENABLE_MOUSE  "\033[?1003h\033[?1006h"
#define CSI_DISABLE_MOUSE "\033[?1003l\033[?1006l"
#define CSI_FG_DEFAULT    "\x1b[39m"
#define CSI_BG_DEFAULT    "\x1b[49m"
#define CSI_CURSOR_HIDE   "\033[?25l"
#define CSI_CURSOR_SHOW   "\033[?25h"
#define CSI_CURSOR_UP     "\033[A"
#define CSI_CURSOR_DOWN   "\033[B"
#define CSI_CURSOR_RIGHT  "\033[C"
#define CSI_CURSOR_LEFT   "\033[D"

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
CSI_get_size();

void
CSI_set_cursorpos(const int x,
                  const int y);

void
CSI_set_normal();

void
CSI_set_raw();

#endif /* _CSI_H */
