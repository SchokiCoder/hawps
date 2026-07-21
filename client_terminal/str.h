/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _STR_H
#define _STR_H

#include <stddef.h>

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

/* @str: String to be converted.
 * @out: Resulting number.
 *
 * Returns the amount of read bytes.
 */
size_t
string_to_uint(const char   *str,
               unsigned int *out);

#endif /* _STR_H */
