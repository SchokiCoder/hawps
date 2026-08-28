/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _GENERIC_H
#define _GENERIC_H

#include <hawps_core.h>
#include <stddef.h>

#include "types.h"

size_t
generate_statusbar_elem(char                        *out,
                        const size_t                 out_size,
                        const char                  *ip_address,
                        const bool                   paused,
                        const enum StatusbarElement  sbe,
                        const bool                   th_vision,
                        const float                  tickrate,
                        struct ToolOptions           tool_opts,
                        const char                  *world_name);

struct Rgba
get_normal_dot_color(const struct World world,
                     const int          x,
                     const int          y);

struct Rgba
get_normal_dot_color_simple(const struct World world,
                            const int          x,
                            const int          y);

struct Rgba
get_thermal_dot_color(const struct World world,
                      const int          x,
                      const int          y);

void
handle_cmdline_shift(const size_t          cmdline_len,
                     size_t               *cmdline_shift,
                     const int             win_w);

void
tool_radius_add(const int           radius_change,
                struct ToolOptions *tool_opts);

void
use_tool(const float         delta,
         struct ToolOptions  tool_opts,
         struct World       *world);

#endif /* _GENERIC_H */
