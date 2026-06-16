/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _CSI_H
#define _CSI_H

#include <sys/ioctl.h>

/* This is a double sequence for clear and cursor to top-left pos.
 * Without this, empty lines remain in the scrollback.
 */
#define CSI_CLEAR        "\x1b[2J\033[H"

#define CSI_FG_DEFAULT   "\x1b[39m"
#define CSI_BG_DEFAULT   "\x1b[49m"
#define CSI_CURSOR_HIDE  "\033[?25l"
#define CSI_CURSOR_SHOW  "\033[?25h"
#define CSI_CURSOR_UP    "\033[A"
#define CSI_CURSOR_DOWN  "\033[B"
#define CSI_CURSOR_RIGHT "\033[C"
#define CSI_CURSOR_LEFT  "\033[D"

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
