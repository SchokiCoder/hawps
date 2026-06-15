/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include "csi.h"

#include <stdbool.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static bool           term_raw = false;
static struct termios term_initial_settings;

struct winsize
CSI_get_size()
{
	struct winsize ret;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ret);

	return ret;
}

void
CSI_set_normal()
{
	if (!term_raw) {
		return;
	}

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_initial_settings);
	fputs(CSI_CURSOR_SHOW, stdout);
	fputs(CSI_FG_DEFAULT, stdout);
	fputs(CSI_BG_DEFAULT, stdout);
	term_raw = false;
}

void
CSI_set_raw()
{
	struct termios raw;

	if (term_raw) {
		return;
	}

	setbuf(stdout, NULL);
	tcgetattr(STDIN_FILENO, &term_initial_settings);
	raw = term_initial_settings;
	raw.c_lflag &= ~(ECHO | ICANON | ISIG);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
	fputs(CSI_CURSOR_HIDE, stdout);
	term_raw = true;
}
