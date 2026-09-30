/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _STR_H
#define _STR_H

#include <stddef.h>
#include <string.h>

/* Macros
 */

/* Constant defines
 */

/* Function declarations
 */

/* @dst: Destination string.
 * @dst_size: Destination string size, not length.
 * @cat_pos: Position at which to concatenate, overwriting everything behind.
 * @src: Source string
 *
 * Returns the amount of written bytes.
 */
size_t
string_cat(char         *restrict dst,
           const size_t           dst_size,
           const size_t           cat_pos,
           const char   *restrict src);

/* @dst: Destination string.
 * @dst_size: Destination string size, not length.
 * @src: Source string
 *
 * Returns the amount of written bytes.
 */
size_t
string_copy(char         *restrict dst,
            const size_t           dst_size,
            const char   *restrict src);

/* @str: String to be converted.
 * @out: Resulting number.
 *
 * Returns the amount of read bytes.
 */
size_t
string_to_uint(const char   *str,
               unsigned int *out);

/* Function definitions
 */

#ifdef HAWPS_IMPL

size_t
string_cat(char         *restrict dst,
           const size_t           dst_size,
           const size_t           cat_pos,
           const char   *restrict src)
{
	size_t copy_len;
	size_t src_len;

	src_len = strlen(src);

	copy_len = dst_size - cat_pos - 1;
	if (src_len < copy_len) {
		copy_len = src_len;
	}

	memcpy(&dst[cat_pos], src, copy_len);
	dst[cat_pos + copy_len] = '\0';

	return copy_len;
}

size_t
string_copy(char         *restrict dst,
            const size_t           dst_size,
            const char   *restrict src)
{
	return string_cat(dst, dst_size, 0, src);
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

#endif /* HAWPS_IMPL */

#endif /* _STR_H */
