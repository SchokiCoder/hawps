/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include "csi.h"

#include <unistd.h>

struct winsize
term_get_size()
{
	struct winsize ret;
	
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ret);

	return ret;
}
