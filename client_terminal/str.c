/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include "str.h"

#include <string.h>

size_t
string_cat(char *restrict       dst,
           const size_t         dst_size,
           const size_t         cat_pos,
           const char *restrict src)
{
	size_t copy_len;
	size_t src_len;

	src_len = strlen(src);

	copy_len = dst_size - cat_pos - 1;
	if (src_len < copy_len) {
		copy_len = src_len;
	}

	strncpy(&dst[cat_pos], src, copy_len);
	dst[cat_pos + copy_len] = '\0';

	return copy_len;
}

size_t
string_to_uint(const char   *str,
               unsigned int *out)
{
	size_t i;

	*out = 0;
	for (i = 0; str[i] >= '0' && str[i] <= '9'; i++) {
		*out = *out * 10 + (str[i] - '0');
	}

	return i;
}
