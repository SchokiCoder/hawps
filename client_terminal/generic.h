/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _GENERIC_H
#define _GENERIC_H

#include <hawps_core.h>
#include <stddef.h>
#include <time.h>

#include "types.h"

#define SIG_INT  '\003'
#define SIG_TSTP '\032'

void
command_temperature(const float   new_temperature,
                    struct World *world);

size_t
write_statusbar_elem(char                        *out,
                     const size_t                 out_size,
                     const char                  *ip_address,
                     const bool                   paused,
                     const enum StatusbarElement  sbe,
                     const bool                   th_vision,
                     const float                  tickrate,
                     struct ToolOptions           tool_opts,
                     const char                  *world_name);

size_t
write_tool_hint(char                     *out,
                const size_t              out_size,
                const struct ToolOptions  tool_opts);

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
handle_advanced_command(const char          *cmd,
                        const char          *arg,
                        char               **feedback,
                        clock_t             *feedback_expiration,
                        float               *framerate,
                        const clock_t        now,
                        float               *tickrate,
                        struct ToolOptions  *tool_opts,
                        struct World        *world);

void
handle_cmdline_shift(const size_t          cmdline_len,
                     size_t               *cmdline_shift,
                     const int             win_w);

void
handle_command(char                *cmdline,
               const size_t         cmdline_len,
               bool                *active,
               char               **feedback,
               clock_t             *feedback_expiration,
               float               *framerate,
               bool                *no_glowcolor,
               const clock_t        now,
               bool                *paused,
               bool                *th_vision,
               float               *tickrate,
               struct ToolOptions  *tool_opts,
               struct World        *world);

void
handle_simple_command(const char          *cmdline,
                      bool                *active,
                      char               **feedback,
                      clock_t             *feedback_expiration,
                      float               *framerate,
                      bool                *no_glowcolor,
                      clock_t              now,
                      bool                *paused,
                      bool                *th_vision,
                      float               *tickrate,
                      struct ToolOptions  *tool_opts,
                      struct World        *world);

void
set_feedback(char          **feedback,
             clock_t        *feedback_expiration,
             const clock_t   now,
             char           *str);

void
tool_radius_add(const int           radius_change,
                struct ToolOptions *tool_opts);

void
use_tool(const float         delta,
         struct ToolOptions  tool_opts,
         struct World       *world);

#endif /* _GENERIC_H */
