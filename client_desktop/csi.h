/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _CSI_H
#define _CSI_H

#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>

/* Macros
 */

/* Constant defines
 */

#define COLOR_BUF_SIZE 4

#define CSI_COLORSTRING_LEN 19

/* Constant defines: Single character strings, and characters.
 * This exists because C can't add chars to a string at compile time. Too bad.
 * Hit anyone who adds multichar strings here.
 */
#define CHAR_DELETE    '\x7f'
#define CSI_DELETE     "\x7f"
#define CHAR_ESCAPE    '\x1b'
#define CSI_ESCAPE     "\x1b"

/* Constant defines: Sequences
 */
#define CSI_FG_DEFAULT    CSI_ESCAPE "[39m"
#define CSI_BG_DEFAULT    CSI_ESCAPE "[49m"
#define CSI_CURSOR_HIDE   CSI_ESCAPE "[?25l"
#define CSI_CURSOR_SHOW   CSI_ESCAPE "[?25h"

#define CSI_KEY_F5         CSI_ESCAPE "[15~"
#define CSI_KEY_F6         CSI_ESCAPE "[17~"
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

/* Types
 */

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

/* Global variables
 */
static bool           term_raw = false;
static struct termios term_initial_settings;
static int            term_stdin_initial_flags;

/* Function declarations
 */

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

struct winsize
CSI_get_size(void);

void
CSI_set_cursorpos(const int x,
                  const int y);

void
CSI_set_normal(void);

void
CSI_set_raw(void);

/* Function definitions
 */

#ifdef HAWPS_IMPL

size_t
CSI_color_to_string(const unsigned char r,
                    const unsigned char g,
                    const unsigned char b,
                    const bool          is_fg,
                    char               *str,
                    const size_t        str_size)
{
	char    buf[COLOR_BUF_SIZE];
	char   *color_type;
	size_t  str_len = 0;

	str[0] = '\0';

	if (is_fg)
		color_type = "\x1b[38";
	else
		color_type = "\x1b[48";

	str_len += string_cat(str, str_size, str_len, color_type);
	str_len += string_cat(str, str_size, str_len, ";2;");
	snprintf(buf, COLOR_BUF_SIZE, "%.3i", r);
	str_len += string_cat(str, str_size, str_len, buf);
	str_len += string_cat(str, str_size, str_len, ";");
	snprintf(buf, COLOR_BUF_SIZE, "%.3i", g);
	str_len += string_cat(str, str_size, str_len, buf);
	str_len += string_cat(str, str_size, str_len, ";");
	snprintf(buf, COLOR_BUF_SIZE, "%.3i", b);
	str_len += string_cat(str, str_size, str_len, buf);
	str_len += string_cat(str, str_size, str_len, "m");

	return str_len;
}

struct winsize
CSI_get_size(void)
{
	struct winsize ret;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ret);

	return ret;
}

void
CSI_set_cursorpos(const int x,
                  const int y)
{
	printf("\033[%i;%iH", y, x);
}

void
CSI_set_normal(void)
{
	if (!term_raw) {
		return;
	}

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_initial_settings);
	fcntl(STDIN_FILENO, F_SETFL, term_stdin_initial_flags);
	fputs(CSI_DISABLE_MOUSE, stdout);
	fputs(CSI_CURSOR_SHOW, stdout);
	fputs(CSI_FG_DEFAULT, stdout);
	fputs(CSI_BG_DEFAULT, stdout);
	term_raw = false;
}

void
CSI_set_raw(void)
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
	term_stdin_initial_flags = fcntl(STDIN_FILENO, F_GETFL);
	fcntl(STDIN_FILENO, F_SETFL, term_stdin_initial_flags | O_NONBLOCK);
	fputs(CSI_ENABLE_MOUSE, stdout);
	fputs(CSI_CURSOR_HIDE, stdout);
	term_raw = true;
}

#endif /* HAWPS_IMPL */

#endif /* _CSI_H */
