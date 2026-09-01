/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _BACKEND_TERMINAL_H
#define _BACKEND_TERMINAL_H

#include <hawps_core.h>
#include <hawps_extra.h>
#include <stddef.h>
#include <time.h>

#include "config.h"
#include "csi.h"
#include "types.h"

void
draw(const char                  *cmdline,
     const size_t                 cmdline_len,
     const size_t                 cmdline_shift,
     char                        *display,
     const size_t                 display_size,
     const size_t                 dot_depth,
     const char                  *feedback,
     const enum InputMode         input_mode,
     const char                  *ip_address,
     const bool                   no_color,
     const bool                   no_glowcolor,
     const bool                   paused,
     const size_t                 statusbar_elems,
     const enum StatusbarElement *statusbar_elem,
     const float                  tickrate,
     const bool                   th_vision,
     const struct ToolOptions     tool_opts,
     const int                    win_w,
     const struct World           world,
     const struct Rect            world_draw,
     const int                    world_draw_space_w,
     const int                    world_draw_space_h,
     const char                  *world_name);

void
handle_command_input(const char          *in,
                     bool                *active,
                     char                *cmdline,
                     size_t              *cmdline_len,
                     size_t              *cmdline_shift,
                     char               **feedback,
                     clock_t             *feedback_expiration,
                     float               *framerate,
                     enum InputMode      *input_mode,
                     bool                *no_glowcolor,
                     clock_t              now,
                     bool                *paused,
                     bool                *th_vision,
                     float               *tickrate,
                     struct ToolOptions  *tool_opts,
                     const int            win_w,
                     struct World        *world);

void
handle_mouse_input(const char         *in,
                   const float         delta,
                   int                *drag_start_x,
                   int                *drag_start_y,
                   bool               *lmb_pressed,
                   struct ToolOptions *tool_opts,
                   struct World       *world,
                   struct Rect        *world_draw);

void
handle_normal_csi_input(const char         *in,
                        const float         delta,
                        int                *drag_start_x,
                        int                *drag_start_y,
                        bool               *lmb_pressed,
                        struct ToolOptions *tool_opts,
                        struct World       *world,
                        struct Rect        *world_draw);

void
handle_resize(const size_t            cmdline_len,
              size_t                 *cmdline_shift,
              char                  **display,
              size_t                 *display_size,
              const size_t            dot_depth,
              const enum InputMode    input_mode,
              const char             *ip_address,
              size_t                 *statusbar_elems,
              enum StatusbarElement  *statusbar_elem,
              int                    *win_w,
              int                    *win_h,
              const struct World      world,
              struct Rect            *world_draw,
              int                    *world_draw_space_w,
              int                    *world_draw_space_h,
              const char             *world_name);

#endif /* _BACKEND_TERMINAL_H */
