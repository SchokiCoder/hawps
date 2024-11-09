/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2024  Andy Frank Schoknecht
 */

#ifndef _COMMON_H
#define _COMMON_H

#define _DUMB_MAGIC(arg) #arg
#define DEF_TO_STRING(name) _DUMB_MAGIC(name)

enum ClientEvent {
	CE_request_pause,
};

enum ServerEvent {
	SE_update
};

#endif /* _COMMON_H */
