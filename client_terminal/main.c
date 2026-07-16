/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#include <errno.h>
#include <hawps_core.h>
#include <hawps_extra.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "csi.h"
#include "config.h"
#include "int_to_string.h"
#include "str.h"
#include "types.h"

/* Macros
 */

#define DOT_RENDER_LOOP(COLOR) \
	for (y = 0; y < world_draw_h; y++) { \
		for (x = 0; x < world_draw_w; x++) { \
			written += render_dot(&out[written], \
			                      out_size - written, \
			                      COLOR, \
			                      world, \
			                      x, \
			                      y); \
		} \
	}

#define DOT_RENDER_LOOP_NO_COLOR \
	for (y = 0; y < world_draw_h; y++) { \
		for (x = 0; x < world_draw_w; x++) { \
			written += render_dot_no_color(&out[written], \
			                               world, \
			                               x, \
			                               y); \
		} \
	}

/* Constant defines
 */

#define CELSIUS_TO_KELVIN 273.15

#define FIRST_REAL_MAT MAT_SAND

#define FLAG_ABOUT                  "-about"
#define FLAG_ABOUT_SHORT            "-a"
#define FLAG_BRUSHRADIUS            "-brushradius"
#define FLAG_BRUSHRADIUS_SHORT      "-br"
#define FLAG_ERASERRADIUS           "-eraserradius"
#define FLAG_ERASERRADIUS_SHORT     "-er"
#define FLAG_HELP                   "-help"
#define FLAG_HELP_SHORT             "-h"
#define FLAG_NOCOLOR                "-nocolor"
#define FLAG_NOCOLOR_SHORT          "-noc"
#define FLAG_NOGLOWCOLOR            "-noglowcolor"
#define FLAG_NOGLOWCOLOR_SHORT      "-nogc"
#define FLAG_SIMSUBSAMPLE           "-simsubsample"
#define FLAG_SIMSUBSAMPLE_SHORT     "-sss"
#define FLAG_SPAWNTEMPERATURE       "-spawntemperature"
#define FLAG_SPAWNTEMPERATURE_SHORT "-st"
#define FLAG_THERMODELTA            "-thermodelta"
#define FLAG_THERMODELTA_SHORT      "-thd"
#define FLAG_THERMORADIUS           "-thermoradius"
#define FLAG_THERMORADIUS_SHORT     "-thr"
#define FLAG_TICKRATE               "-tickrate"
#define FLAG_TICKRATE_SHORT         "-tr"
#define FLAG_VERSION                "-version"
#define FLAG_VERSION_SHORT          "-v"

#define SIG_INT  '\003'
#define SIG_TSTP '\032'

/* Constants
 */

static const char APP_ABOUT[] = "The source code of \"" APP_NAME_FORMAL "\" "
"aka " APP_NAME " " APP_VERSION " is available,\n"
"licensed under the " APP_LICENSE " at:\n"
APP_REPOSITORY "\n"
"\n"
"If you did not receive a copy of the license, see below:\n"
APP_LICENSE_URL "\n";

static const char APP_HELP[] = "Usage: " APP_NAME " [OPTIONS]\n"
"\n"
"Silly program to simulate physics in *very* convincing ways.\n"
"It'll be great. Trust me.\n"
"\n";

static const char APP_HELP_COMMANDS[] = "Commands:\n"
"\n"
"    You can enter these commands into the internal command line.\n"
"    There is a short and a long variant for most commands.\n"
"    Some accept arguments.\n"
"\n"
"    " CMD_BRUSH_SHORT " " CMD_BRUSH "\n"
"        selects the brush as active tool\n"
"\n"
"    " CMD_BRUSHMAT_SHORT " " CMD_BRUSHMAT " TEXT\n"
"        selects the given material for the brush\n"
"\n"
"    " CMD_BRUSHRADIUS_SHORT " " CMD_BRUSHRADIUS " NUMBER\n"
"        sets the brush radius to the given number\n"
"\n"
"    " CMD_CLEAR_SHORT " " CMD_CLEAR "\n"
"        clears the world\n"
"\n"
"    " CMD_CLEARALL_SHORT " " CMD_CLEARALL "\n"
"        clears the world, including spawners\n"
"\n"
"    " CMD_COOLER_SHORT " " CMD_COOLER "\n"
"        selects the cooler as active tool\n"
"\n"
"    " CMD_DEFAULTS_SHORT " " CMD_DEFAULTS "\n"
"        sets everything back to default settings\n"
"\n"
"    " CMD_ERASER_SHORT " " CMD_ERASER "\n"
"        selects the eraser as active tool\n"
"\n"
"    " CMD_ERASERRADIUS_SHORT " " CMD_ERASERRADIUS " NUMBER\n"
"        sets the eraser radius to the given number\n"
"\n"
"    " CMD_GLOWCOLOR_SHORT " " CMD_GLOWCOLOR "\n"
"        enables dot glow coloring\n"
"\n"
"    " CMD_HEATER_SHORT " " CMD_HEATER "\n"
"        selects the heater as active tool\n"
"\n"
"    " CMD_MAT_SHORT " " CMD_MAT " TEXT\n"
"        sets the material of the currently active tool\n"
"\n"
"    " CMD_NOGLOWCOLOR_SHORT " " CMD_NOGLOWCOLOR "\n"
"        disables dot glow coloring\n"
"\n"
"    " CMD_NORMALVISION_SHORT " " CMD_NORMALVISION "\n"
"        disables thermal vision\n"
"\n"
"    " CMD_PAUSE_SHORT " " CMD_PAUSE "\n"
"        pauses the simulation\n"
"\n"
"    " CMD_QUIT_SHORT " " CMD_QUIT "\n"
"        quits and closes the application\n"
"\n"
"    " CMD_SPAWNER_SHORT " " CMD_SPAWNER "\n"
"        selects the spawner as active tool\n"
"\n"
"    " CMD_SPAWNERMAT_SHORT " " CMD_SPAWNERMAT "\n"
"        selects the given material for the spawner\n"
"\n"
"    " CMD_SPAWNTEMPERATURE_SHORT " " CMD_SPAWNTEMPERATURE " DECIMAL\n"
"        sets the temperature of newly spawned dots\n"
"\n"
"    " CMD_SPAWNTEMPERATUREK_SHORT " " CMD_SPAWNTEMPERATUREK " DECIMAL\n"
"        sets the temperature of newly spawned dots in degrees Kelvin\n"
"\n"
"    " CMD_TEMPERATURE_SHORT " " CMD_TEMPERATURE " DECIMAL\n"
"        sets the temperature of existing dots\n"
"\n"
"    " CMD_TEMPERATUREK_SHORT " " CMD_TEMPERATUREK " DECIMAL\n"
"        sets the temperature of existing dots in degrees Kelvin\n"
"\n"
"    " CMD_THERMODELTA_SHORT " " CMD_THERMODELTA " DECIMAL\n"
"        sets the delta of thermo tools, with which heating/cooling occurs\n"
"\n"
"    " CMD_THERMORADIUS_SHORT " " CMD_THERMORADIUS " NUMBER\n"
"        sets the radius of thermo tools to the given number\n"
"\n"
"    " CMD_THERMOVISION_SHORT " " CMD_THERMOVISION "\n"
"        enables thermal vision\n"
"\n"
"    " CMD_TICKRATE_SHORT " " CMD_TICKRATE " NUMBER\n"
"        sets the tickrate (ticks per second),\n"
"        which also affects simulation speed\n"
"\n";

static const char APP_HELP_FLAGS[] = "Options:\n"
"\n"
"    " FLAG_ABOUT_SHORT " " FLAG_ABOUT "\n"
"        prints program name, version, license and repository information then exits\n"
"\n"
"    " FLAG_BRUSHRADIUS_SHORT " " FLAG_BRUSHRADIUS " NUMBER\n"
"        sets the radius of the brush\n"
"        default: %i\n"
"\n"
"    " FLAG_ERASERRADIUS_SHORT " " FLAG_ERASERRADIUS " NUMBER\n"
"        sets the radius of the eraser\n"
"        default: %i\n"
"\n"
"    " FLAG_HELP_SHORT " " FLAG_HELP "\n"
"        prints this message then exits\n"
"\n"
"    " FLAG_NOCOLOR_SHORT " " FLAG_NOCOLOR "\n"
"        disables all world dot coloring\n"
"\n"
"    " FLAG_NOGLOWCOLOR_SHORT " " FLAG_NOGLOWCOLOR "\n"
"        disables dot glow coloring\n"
"\n"
"    " FLAG_SIMSUBSAMPLE_SHORT " " FLAG_SIMSUBSAMPLE " NUMBER\n"
"        sets the simulation subsample, which affects the simulation speed\n"
"        visible simulation speed roughly == tickrate / sim-subsample \n"
"        default: %i\n"
"\n"
"    " FLAG_SPAWNTEMPERATURE_SHORT " " FLAG_SPAWNTEMPERATURE " DECIMAL\n"
"        sets the temperature of every new dot in Kelvin\n"
"        0 °C == %.2f K\n"
"        default: %.2f\n"
"\n"
"    " FLAG_THERMODELTA_SHORT " " FLAG_THERMODELTA " DECIMAL\n"
"        sets the delta of thermo tools, with which heating/cooling occurs\n"
"        the delta is applied once per tick\n"
"        default: %.2f\n"
"\n"
"    " FLAG_THERMORADIUS_SHORT " " FLAG_THERMORADIUS " NUMBER\n"
"        sets the radius of thermo tools\n"
"        default: %i\n"
"\n"
"    " FLAG_TICKRATE_SHORT " " FLAG_TICKRATE " NUMBER\n"
"        sets the tickrate (ticks per second),\n"
"        which also affects simulation speed\n"
"        only use when otherwise performance problems occur\n"
"        default: %i\n"
"\n"
"    " FLAG_VERSION_SHORT " " FLAG_VERSION "\n"
"        prints version information then exits\n"
"\n";

static const char APP_HELP_MATERIALS[] = "Material list:\n"
"\n";

static const char APP_HELP_KEYBINDS[] = "Keybinds:\n"
"\n"
"    Currently, this program is configured at " CONFIGURED_AT ".\n"
"    It is static.\n"
"    Therefore you can't just change binds.\n"
"\n"
"    %c Escape\n"
"        quit the program\n"
"\n"
"    %c\n"
"        use currently active tool\n"
"\n"
"    %c\n"
"        toggle thermal vision\n"
"        the grayscale displays from %.0f to %.0f degrees Celsius\n"
"\n"
"    %c\n"
"        select an upper material from the material list, for current tool\n"
"\n"
"    %c\n"
"        select uppermost material from the material list, for current tool\n"
"\n"
"    %c\n"
"        select a lower material from the material list, for current tool\n"
"\n"
"    %c\n"
"        select a lowest material from the material list, for current tool\n"
"\n"
"    %c\n"
"        set brush as current tool\n"
"\n"
"    %c\n"
"        set spawner as current tool\n"
"\n"
"    %c\n"
"        set eraser as current tool\n"
"\n"
"    %c\n"
"        set heater as current tool\n"
"\n"
"    %c\n"
"        set cooler as current tool\n"
"\n"
"    %c Left\n"
"        move tool cursor left\n"
"\n"
"    %c Home\n"
"        move tool cursor leftmost\n"
"\n"
"    %c Down\n"
"        move tool cursor down\n"
"\n"
"    %c PgDn\n"
"        move tool cursor to bottom\n"
"\n"
"    %c Up\n"
"        move tool cursor up\n"
"\n"
"    %c PgUp\n"
"        move tool cursor to top\n"
"\n"
"    %c Right\n"
"        move tool cursor right\n"
"\n"
"    %c End\n"
"        move tool cursor rightmost\n"
"\n"
"    Ctrl+Home\n"
"        move tool cursor leftmost and top\n"
"\n"
"    Ctrl+End\n"
"        move tool cursor rightmost and bottom\n"
"\n"
"    %c\n"
"        decrease tool radius\n"
"\n"
"    %c\n"
"        set smallest tool radius\n"
"\n"
"    %c\n"
"        increase tool radius\n"
"\n"
"    %c\n"
"        set biggest tool radius\n"
"\n"
"    %c\n"
"        decrease the simulation speed\n"
"\n"
"    %c\n"
"        set to slowest simulation speed\n"
"\n"
"    %c\n"
"        increase the simulation speed\n"
"\n"
"    %c\n"
"        set to fastest simulation speed\n"
"\n"
"    %c\n"
"        enter the command line\n"
"\n"
"    %s\n"
"        pause world\n"
"\n";

static const char DOT_APPEARANCE[] = {
	'X',
	'X',
	'+',
	'-',
};

/* Function declarations
 */

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
     const int                    sim_subsample,
     const size_t                 statusbar_elems,
     const enum StatusbarElement *statusbar_elem,
     const int                    tickrate,
     const bool                   th_vision,
     const struct ToolOptions     tool_opts,
     const int                    win_w,
     const int                    win_h,
     const struct World           world,
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
handle_advanced_command(const char          *cmd,
                        const char          *arg,
                        char               **feedback,
                        clock_t             *feedback_expiration,
                        const clock_t        now,
                        int                 *tickrate,
                        struct ToolOptions  *tool_opts,
                        struct World        *world);

bool
handle_args(int                  argc,
            char               **argv,
            bool                *no_color,
            bool                *no_glowcolor,
            int                 *sim_subsample,
            int                 *tickrate,
            struct ToolOptions  *tool_opts);

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
               bool                *no_glowcolor,
               const clock_t        now,
               bool                *paused,
               int                 *sim_subsample,
               bool                *th_vision,
               int                 *tickrate,
               struct ToolOptions  *tool_opts,
               struct World        *world);

void
handle_command_input(const char          *in,
                     bool                *active,
                     char                *cmdline,
                     size_t              *cmdline_len,
                     size_t              *cmdline_shift,
                     char               **feedback,
                     clock_t             *feedback_expiration,
                     enum InputMode      *input_mode,
                     bool                *no_glowcolor,
                     clock_t              now,
                     bool                *paused,
                     int                 *sim_subsample,
                     bool                *th_vision,
                     int                 *tickrate,
                     struct ToolOptions  *tool_opts,
                     const int            win_w,
                     struct World        *world);

bool
handle_flag_float_arg(int    argc,
                      char **argv,
                      int   *idx,
                      float *out);

bool
handle_flag_int_arg(int    argc,
                    char **argv,
                    int   *idx,
                    int   *out);

void
handle_input(bool                *active,
             char                *cmdline,
             size_t              *cmdline_len,
             size_t              *cmdline_shift,
             char               **feedback,
             clock_t             *feedback_expiration,
             enum InputMode      *input_mode,
             bool                *mouse_pressed,
             bool                *no_glowcolor,
             clock_t              now,
             bool                *paused,
             int                 *sim_subsample,
             int                 *tickrate,
             bool                *th_vision,
             struct ToolOptions  *tool_opts,
             const int            win_w,
             struct World        *world);

void
handle_mouse_input(const char         *in,
                   bool               *mouse_pressed,
                   struct ToolOptions *tool_opts,
                   struct World       *world);

void
handle_normal_csi_input(const char         *in,
                        bool               *mouse_pressed,
                        struct ToolOptions *tool_opts,
                        struct World       *world);

void
handle_normal_input(const char         *in,
                    bool               *active,
                    enum InputMode     *input_mode,
                    bool               *mouse_pressed,
                    bool               *paused,
                    int                *sim_subsample,
                    const int           tickrate,
                    bool               *th_vision,
                    struct ToolOptions *tool_opts,
                    struct World       *world);

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
              const char             *world_name);

void
handle_simple_command(const char          *cmdline,
                      bool                *active,
                      char               **feedback,
                      clock_t             *feedback_expiration,
                      bool                *no_glowcolor,
                      clock_t              now,
                      bool                *paused,
                      int                 *sim_subsample,
                      bool                *th_vision,
                      int                 *tickrate,
                      struct ToolOptions  *tool_opts,
                      struct World        *world);

struct ToolOptions
new_tool_options();

void
tool_radius_add(const int           radius_change,
                struct ToolOptions *tool_opts);

size_t
render_dot(char               *out,
           const size_t        out_size,
           const struct Rgba   color,
           const struct World  world,
           const int           x,
           const int           y);

size_t
render_dot_no_color(char               *out,
                    const struct World  world,
                    const int           x,
                    const int           y);

size_t
render_statusbar_display(char                        *out,
                         const size_t                 out_size,
                         const char                  *ip_address,
                         const bool                   paused,
                         const enum StatusbarElement  sbe,
                         const int                    sim_subsample,
                         const int                    tickrate,
                         const bool                   th_vision,
                         struct ToolOptions           tool_opts,
                         const char                  *world_name);

size_t
render_tool_hint(char                     *out,
                 const size_t              out_size,
                 const struct ToolOptions  tool_opts);

size_t
render_world(char               *out,
             const size_t        out_size,
             const size_t        dot_depth,
             const bool          no_color,
             const bool          no_glowcolor,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world,
             const int           world_draw_w,
             const int           world_draw_h);

void
set_feedback(char          **feedback,
             clock_t        *feedback_expiration,
             const clock_t   now,
             char           *str);

void
use_tool(struct ToolOptions  tool_opts,
         struct World       *world);

/* Function definitions
 */

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
     const int                    sim_subsample,
     const size_t                 statusbar_elems,
     const enum StatusbarElement *statusbar_elem,
     const int                    tickrate,
     const bool                   th_vision,
     const struct ToolOptions     tool_opts,
     const int                    win_w,
     const int                    win_h,
     const struct World           world,
     const char                  *world_name)
{
	char   buf[BUF_SIZE];
	size_t buf_len = 0;
	size_t display_len = 0;
	size_t i;
	size_t space_len = 0;
	size_t st_bar_len = 0;
	int    world_draw_w = 0;
	int    world_draw_h = 0;

	display[0] = '\0';

	if (world.w > win_w) {
		world_draw_w = win_w;
	} else {
		world_draw_w = world.w;
	}
	if (world.h > win_h - 2) {
		world_draw_h = win_h - 2;
	} else {
		world_draw_h = world.h;
	}

	if (th_vision) {
		display_len += CSI_color_to_string(THERMAL_VISION_R,
		                                   THERMAL_VISION_G,
		                                   THERMAL_VISION_B,
		                                   false,
		                                   &display[display_len],
		                                   display_size - display_len);
	}

	display_len += render_world(&display[display_len],
	                            display_size - display_len,
	                            dot_depth,
	                            no_color,
	                            no_glowcolor,
	                            th_vision,
	                            tool_opts,
	                            world,
	                            world_draw_w,
	                            world_draw_h);

	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          CSI_FG_DEFAULT);
	display_len += string_cat(display,
	                          display_size,
	                          display_len,
	                          CSI_BG_DEFAULT);

	st_bar_len = display_len;

	i = 0;
	while (1) {
		display_len += render_statusbar_display(&display[display_len],
		                                        display_size - display_len,
		                                        ip_address,
		                                        paused,
		                                        statusbar_elem[i],
		                                        sim_subsample,
		                                        tickrate,
		                                        th_vision,
		                                        tool_opts,
		                                        world_name);

		i++;
		if (i >= statusbar_elems) {
			break;
		}

		display_len += string_cat(display,
		                          display_size,
		                          display_len,
		                          STATUSBAR_SEPARATOR);
	}

	st_bar_len = display_len - st_bar_len;

	space_len = win_w - st_bar_len;
	memset(&display[display_len], ' ', space_len);
	display_len += space_len;

	switch (input_mode) {
	case IM_NORMAL:
		if (feedback != NULL) {
			display_len += string_cat(display,
			                          display_size,
			                          display_len,
			                          feedback);
			space_len = win_w - strlen(feedback);
			break;
		}

		buf[0] = '\0';
		buf_len = 0;
		buf_len = render_tool_hint(buf, BUF_SIZE, tool_opts);

		if (buf_len > (size_t) win_w) {
			buf_len -= buf_len - win_w;
			buf[buf_len] = '\0';
		}

		display_len += string_cat(display, display_size, display_len, buf);

		space_len = win_w - buf_len;
		break;

	case IM_COMMAND:
		display[display_len] = CMDLINE_INDICATOR;
		display_len += 1;

		display_len += string_cat(display,
		                          display_size,
		                          display_len,
		                          &cmdline[cmdline_shift]);

		display[display_len] = CMDLINE_CURSOR;
		display_len += 1;

		space_len = win_w - 1 - cmdline_len + cmdline_shift - 1;
		break;
	}

	memset(&display[display_len], ' ', space_len);
	display_len += space_len;

	display[display_len] = '\0';

	CSI_set_cursorpos(0, 0);
	buf_len = 0;
	while (buf_len < display_len) {
		buf_len += fwrite(&display[buf_len],
		                  1,
		                  display_len - buf_len,
		                  stdout);
	}
}

struct Rgba
get_normal_dot_color(const struct World world,
                     const int          x,
                     const int          y)
{
	struct Rgba a, b;

	a = thermo_to_color(world.thermo[x][y]);

	b.r = MAT_R[world.dot[x][y]];
	b.g = MAT_G[world.dot[x][y]];
	b.b = MAT_B[world.dot[x][y]];
	b.a = 255;

	return rgba_blend(a, b);
}

struct Rgba
get_normal_dot_color_simple(const struct World world,
                            const int          x,
                            const int          y)
{
	struct Rgba ret;

	ret.r = MAT_R[world.dot[x][y]];
	ret.g = MAT_G[world.dot[x][y]];
	ret.b = MAT_B[world.dot[x][y]];
	ret.a = 255;

	return ret;
}

struct Rgba
get_thermal_dot_color(const struct World world,
                      const int          x,
                      const int          y)
{
	struct Rgba ret;
	unsigned char vis_t;

	if (world.thermo[x][y] > (THERMAL_VISION_MIN_T + 255)) {
		vis_t = 255;
	} else if (world.thermo[x][y] < THERMAL_VISION_MIN_T) {
		vis_t = 0;
	} else {
		vis_t = world.thermo[x][y] - THERMAL_VISION_MIN_T;
	}

	ret.r = vis_t;
	ret.g = vis_t;
	ret.b = vis_t;
	ret.a = 255;

	return ret;
}

void
handle_advanced_command(const char          *cmd,
                        const char          *arg,
                        char               **feedback,
                        clock_t             *feedback_expiration,
                        const clock_t        now,
                        int                 *tickrate,
                        struct ToolOptions  *tool_opts,
                        struct World        *world)
{
	float f = 0.0;
	long l;
	int x, y;

	if (strcmp(cmd, CMD_BRUSHMAT) == 0 ||
	    strcmp(cmd, CMD_BRUSHMAT_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_BRUSH;
		if (mat_from_string(arg, &tool_opts->brush_mat)) {
			return;
		}

		set_feedback(feedback, feedback_expiration, now,
		             "Material not recognized.");
	} else if (strcmp(cmd, CMD_BRUSHRADIUS) == 0 ||
	           strcmp(cmd, CMD_BRUSHRADIUS_SHORT) == 0) {
		errno = 0;
		l = strtol(arg, NULL, 10);

		if (errno != 0 ||
		    l < 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->brush_radius = l;
		}
	} else if (strcmp(cmd, CMD_ERASERRADIUS) == 0 ||
	           strcmp(cmd, CMD_ERASERRADIUS_SHORT) == 0) {
		errno = 0;
		l = strtol(arg, NULL, 10);

		if (errno != 0 ||
		    l < 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->eraser_radius = l;
		}
	} else if (strcmp(cmd, CMD_MAT) == 0 ||
	           strcmp(cmd, CMD_MAT_SHORT) == 0) {
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			if (mat_from_string(arg, &tool_opts->brush_mat)) {
				return;
			}

			set_feedback(feedback, feedback_expiration, now,
			             "Material not recognized.");
			break;

		case TOOL_SPAWNER:
			if (mat_from_string(arg, &tool_opts->spawner_mat)) {
				return;
			}

			set_feedback(feedback, feedback_expiration, now,
			             "Material not recognized.");
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			set_feedback(feedback, feedback_expiration, now,
			             "Unsupported tool selected.");
			break;
		}
	} else if (strcmp(cmd, CMD_SPAWNERMAT) == 0 ||
	           strcmp(cmd, CMD_SPAWNERMAT_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_SPAWNER;
		if (mat_from_string(arg, &tool_opts->spawner_mat)) {
			return;
		}

		set_feedback(feedback, feedback_expiration, now,
		             "Material not recognized.");
	} else if (strcmp(cmd, CMD_SPAWNTEMPERATURE) == 0 ||
	           strcmp(cmd, CMD_SPAWNTEMPERATURE_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->spawn_temperature = f + CELSIUS_TO_KELVIN;
		}
	} else if (strcmp(cmd, CMD_SPAWNTEMPERATUREK) == 0 ||
	           strcmp(cmd, CMD_SPAWNTEMPERATUREK_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->spawn_temperature = f;
		}
	} else if (strcmp(cmd, CMD_TEMPERATURE) == 0 ||
	           strcmp(cmd, CMD_TEMPERATURE_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			for (x = 0; x < world->w; x++) {
				for (y = 0; y < world->h; y++) {
					world->thermo[x][y] = f +
					                      CELSIUS_TO_KELVIN;
				}
			}
		}
	} else if (strcmp(cmd, CMD_TEMPERATUREK) == 0 ||
	           strcmp(cmd, CMD_TEMPERATUREK_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			for (x = 0; x < world->w; x++) {
				for (y = 0; y < world->h; y++) {
					world->thermo[x][y] = f;
				}
			}
		}
	} else if (strcmp(cmd, CMD_THERMODELTA) == 0 ||
	           strcmp(cmd, CMD_THERMODELTA_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
			return;
		}

		if (f == 0.0) {
			set_feedback(feedback, feedback_expiration, now,
			             "That's dumb, but okay.");
		}

		tool_opts->thermo_delta = f;
	} else if (strcmp(cmd, CMD_THERMORADIUS) == 0 ||
	           strcmp(cmd, CMD_THERMORADIUS_SHORT) == 0) {
		errno = 0;
		l = strtol(arg, NULL, 10);

		if (errno != 0 ||
		    l < 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			tool_opts->thermo_radius = l;
		}
	} else if (strcmp(cmd, CMD_TICKRATE) == 0 ||
	           strcmp(cmd, CMD_TICKRATE_SHORT) == 0) {
		errno = 0;
		l = strtol(arg, NULL, 10);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
			return;
		}

		if (l <= 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "No.");
			return;
		}

		*tickrate = l;
	} else {
		set_feedback(feedback, feedback_expiration, now,
		             "Command not recognized.");
	}
}

bool
handle_args(int                  argc,
            char               **argv,
            bool                *no_color,
            bool                *no_glowcolor,
            int                 *sim_subsample,
            int                 *tickrate,
            struct ToolOptions  *tool_opts)
{
	float flagargf;
	int   flagargi;
	int   i;
	char  key_pause[8] = "Space";

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], FLAG_ABOUT_SHORT) == 0 ||
		    strcmp(argv[i], FLAG_ABOUT) == 0) {
			printf(APP_ABOUT);
			return false;
		} else if (strcmp(argv[i], FLAG_BRUSHRADIUS) == 0 ||
		           strcmp(argv[i], FLAG_BRUSHRADIUS_SHORT) == 0) {
			if (!handle_flag_int_arg(argc, argv, &i, &flagargi)) {
				return false;
			}
			tool_opts->brush_radius = flagargi;
			if (tool_opts->brush_radius < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_ERASERRADIUS) == 0 ||
		           strcmp(argv[i], FLAG_ERASERRADIUS_SHORT) == 0) {
			if (!handle_flag_int_arg(argc, argv, &i, &flagargi)) {
				return false;
			}
			tool_opts->eraser_radius = flagargi;
			if (tool_opts->eraser_radius < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_HELP) == 0 ||
		           strcmp(argv[i], FLAG_HELP_SHORT) == 0) {
			printf(APP_HELP);

			printf(APP_HELP_FLAGS,
			       STD_BRUSH_RADIUS,
			       STD_ERASER_RADIUS,
			       STD_SIM_SUBSAMPLE,
			       CELSIUS_TO_KELVIN,
			       STD_SPAWN_TEMPERATURE,
			       STD_THERMO_DELTA,
			       STD_THERMO_RADIUS,
			       STD_TICKRATE);

			if (KEY_PAUSE != ' ') {
				key_pause[0] = KEY_PAUSE;
				key_pause[1] = '\0';
			}
			printf(APP_HELP_KEYBINDS,
			       KEY_QUIT,
			       KEY_USE,
			       KEY_SWITCH_VISION,
			       THERMAL_VISION_MIN_T - CELSIUS_TO_KELVIN,
			       THERMAL_VISION_MIN_T - CELSIUS_TO_KELVIN + 255,
			       KEY_PREVIOUS_MAT,
			       KEY_FIRST_MAT,
			       KEY_NEXT_MAT,
			       KEY_LAST_MAT,
			       KEY_BRUSH,
			       KEY_SPAWNER,
			       KEY_ERASER,
			       KEY_HEATER,
			       KEY_COOLER,
			       KEY_LEFT,
			       KEY_LEFT_MAX,
			       KEY_DOWN,
			       KEY_DOWN_MAX,
			       KEY_UP,
			       KEY_UP_MAX,
			       KEY_RIGHT,
			       KEY_RIGHT_MAX,
			       KEY_RADIUS_DOWN,
			       KEY_RADIUS_MIN,
			       KEY_RADIUS_UP,
			       KEY_RADIUS_MAX,
			       KEY_SIMSPEED_DOWN,
			       KEY_SIMSPEED_MIN,
			       KEY_SIMSPEED_UP,
			       KEY_SIMSPEED_MAX,
			       KEY_CMD,
			       key_pause);

			printf(APP_HELP_COMMANDS);

			printf(APP_HELP_MATERIALS);
			for (i = 0; i < MAT_COUNT; i++) {
				printf("    %s\n", MAT_NAME[i]);
			}
			printf("\n");

			return false;
		} else if (strcmp(argv[i], FLAG_NOCOLOR) == 0 ||
		           strcmp(argv[i], FLAG_NOCOLOR_SHORT) == 0) {
			*no_color = true;
		} else if (strcmp(argv[i], FLAG_NOGLOWCOLOR) == 0 ||
		           strcmp(argv[i], FLAG_NOGLOWCOLOR_SHORT) == 0) {
			*no_glowcolor = true;
		} else if (strcmp(argv[i], FLAG_SIMSUBSAMPLE) == 0 ||
		           strcmp(argv[i], FLAG_SIMSUBSAMPLE_SHORT) == 0) {
			if (!handle_flag_int_arg(argc, argv, &i, &flagargi)) {
				return false;
			}
			*sim_subsample = flagargi;
			if (*sim_subsample <= 0) {
				fprintf(stderr,
				        "The value for \"%s\" must be positive\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_SPAWNTEMPERATURE) == 0 ||
		           strcmp(argv[i], FLAG_SPAWNTEMPERATURE_SHORT) == 0) {
			if (!handle_flag_float_arg(argc, argv, &i, &flagargf)) {
				return false;
			}
			tool_opts->spawn_temperature = flagargf;
			if (tool_opts->spawn_temperature < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_THERMODELTA) == 0 ||
		           strcmp(argv[i], FLAG_THERMODELTA_SHORT) == 0) {
			if (!handle_flag_float_arg(argc, argv, &i, &flagargf)) {
				return false;
			}
			tool_opts->thermo_delta = flagargf;
			if (tool_opts->thermo_delta < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_THERMORADIUS) == 0 ||
		           strcmp(argv[i], FLAG_THERMORADIUS_SHORT) == 0) {
			if (!handle_flag_int_arg(argc, argv, &i, &flagargi)) {
				return false;
			}
			tool_opts->thermo_radius = flagargi;
			if (tool_opts->thermo_radius < 0) {
				fprintf(stderr,
				        "The value for \"%s\" must not be negative\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_TICKRATE) == 0 ||
		           strcmp(argv[i], FLAG_TICKRATE_SHORT) == 0) {
			if (!handle_flag_int_arg(argc, argv, &i, &flagargi)) {
				return false;
			}
			*tickrate = flagargi;
			if (*tickrate <= 0) {
				fprintf(stderr,
				        "The value for \"%s\" must be positive\n",
				        argv[i]);
				return false;
			}
			i++;
		} else if (strcmp(argv[i], FLAG_VERSION_SHORT) == 0 ||
		           strcmp(argv[i], FLAG_VERSION) == 0) {
			printf("%s: version %s\n", APP_NAME, APP_VERSION);
			return false;
		} else {
			fprintf(stderr,
			        "Argument \"%s\" is not recognized\n",
			        argv[i]);
			return false;
		}
	}

	return true;
}

void
handle_cmdline_shift(const size_t          cmdline_len,
                     size_t               *cmdline_shift,
                     const int             win_w)
{
	if (1 + cmdline_len + 1 > (size_t) win_w) {
		*cmdline_shift = 1 + cmdline_len + 1 - win_w;
	} else {
		*cmdline_shift = 0;
	}
}

void
handle_command(char                *cmdline,
               const size_t         cmdline_len,
               bool                *active,
               char               **feedback,
               clock_t             *feedback_expiration,
               bool                *no_glowcolor,
               const clock_t        now,
               bool                *paused,
               int                 *sim_subsample,
               bool                *th_vision,
               int                 *tickrate,
               struct ToolOptions  *tool_opts,
               struct World        *world)
{
	char buf1[BUF_SIZE];
	char buf2[BUF_SIZE];
	size_t i;

	buf1[0] = '\0';
	buf2[0] = '\0';

	for (i = 0; i < cmdline_len; i++) {
		switch (cmdline[i]) {
		case ' ':
			cmdline[i] = '\0';
			string_cat(buf1, BUF_SIZE, 0, cmdline);
			cmdline[i] = ' ';
			string_cat(buf2, BUF_SIZE, 0, &cmdline[i + 1]);

			handle_advanced_command(buf1, buf2,
			                        feedback,
			                        feedback_expiration,
			                        now,
			                        tickrate,
			                        tool_opts,
			                        world);
			return;
			break;

		case '\n':
		case '\r':
		case '\0':
			i = cmdline_len;
			break;
		}
	}

	handle_simple_command(cmdline,
	                      active,
	                      feedback,
	                      feedback_expiration,
	                      no_glowcolor,
	                      now,
	                      paused,
	                      sim_subsample,
	                      th_vision,
	                      tickrate,
	                      tool_opts,
	                      world);
}

void
handle_command_input(const char          *in,
                     bool                *active,
                     char                *cmdline,
                     size_t              *cmdline_len,
                     size_t              *cmdline_shift,
                     char               **feedback,
                     clock_t             *feedback_expiration,
                     enum InputMode      *input_mode,
                     bool                *no_glowcolor,
                     clock_t              now,
                     bool                *paused,
                     int                 *sim_subsample,
                     bool                *th_vision,
                     int                 *tickrate,
                     struct ToolOptions  *tool_opts,
                     const int            win_w,
                     struct World        *world)
{
	switch (in[0]) {
	case CHAR_BACKSPACE:
		if (*cmdline_len > 0) {
			cmdline[*cmdline_len - 1] = '\0';
			*cmdline_len -= 1;
			handle_cmdline_shift(*cmdline_len,
			                     cmdline_shift,
			                     win_w);
		}
		break;

	case '\n':
		handle_command(cmdline,
		               *cmdline_len,
		               active,
		               feedback,
		               feedback_expiration,
		               no_glowcolor,
		               now,
		               paused,
		               sim_subsample,
		               th_vision,
		               tickrate,
		               tool_opts,
		               world);
		/* fallthrough */
	case SIG_INT:
	case SIG_TSTP:
		cmdline[0] = '\0';
		*cmdline_len = 0;
		*input_mode = IM_NORMAL;
		handle_cmdline_shift(*cmdline_len,
		                     cmdline_shift,
		                     win_w);
		break;

	default:
		if (*cmdline_len < CMDLINE_SIZE - 1) {
			cmdline[*cmdline_len] = in[0];
			cmdline[*cmdline_len + 1] = '\0';
			*cmdline_len += 1;
			handle_cmdline_shift(*cmdline_len,
			                     cmdline_shift,
			                     win_w);
		}
	}
}

bool
handle_flag_float_arg(int    argc,
                      char **argv,
                      int   *idx,
                      float *out)
{
	if (argc <= *idx + 1) {
		fprintf(stderr,
		        "The argument \"%s\" needs to be followed by a value\n",
		        argv[*idx]);
		return false;
	}
	*idx += 1;

	errno = 0;
	*out = strtof(argv[*idx], NULL);
	if (errno != 0) {
		fprintf(stderr,
		        "\"%s\" could not be converted to a float\n",
		        argv[*idx - 1]);
		return false;
	}

	return true;
}

bool
handle_flag_int_arg(int    argc,
                    char **argv,
                    int   *idx,
                    int   *out)
{
	if (argc <= *idx + 1) {
		fprintf(stderr,
		        "The argument \"%s\" needs to be followed by a value\n",
		        argv[*idx]);
		return false;
	}
	*idx += 1;

	errno = 0;
	*out = strtol(argv[*idx], NULL, 10);
	if (errno != 0) {
		fprintf(stderr,
		        "\"%s\" could not be converted to an int\n",
		        argv[*idx - 1]);
		return false;
	}

	return true;
}

void
handle_input(bool                *active,
             char                *cmdline,
             size_t              *cmdline_len,
             size_t              *cmdline_shift,
             char               **feedback,
             clock_t             *feedback_expiration,
             enum InputMode      *input_mode,
             bool                *mouse_pressed,
             bool                *no_glowcolor,
             clock_t              now,
             bool                *paused,
             int                 *sim_subsample,
             int                 *tickrate,
             bool                *th_vision,
             struct ToolOptions  *tool_opts,
             const int            win_w,
             struct World        *world)
{
	ssize_t input_len = 0;
	char    input[INPUT_SIZE];

	input_len = read(STDIN_FILENO, &input, INPUT_SIZE);

	switch (*input_mode) {
	case IM_NORMAL:
		if (input_len > 0 &&
		    input_len < INPUT_SIZE) {
			input[input_len] = '\0';
			handle_normal_input(input,
			                    active,
			                    input_mode,
			                    mouse_pressed,
			                    paused,
			                    sim_subsample,
			                    *tickrate,
			                    th_vision,
			                    tool_opts,
			                    world);
		}
		break;

	case IM_COMMAND:
		if (input_len == 1) {
			handle_command_input(input,
			                     active,
			                     cmdline,
			                     cmdline_len,
			                     cmdline_shift,
			                     feedback,
			                     feedback_expiration,
			                     input_mode,
			                     no_glowcolor,
			                     now,
			                     paused,
			                     sim_subsample,
			                     th_vision,
			                     tickrate,
			                     tool_opts,
			                     win_w,
			                     world);
		}
		break;
	}
}

void
handle_mouse_input(const char         *in,
                   bool               *mouse_pressed,
                   struct ToolOptions *tool_opts,
                   struct World       *world)
{
	unsigned int  b;
	size_t        i;
	size_t        l_start = 3;
	char          pressed;
	unsigned int  report_vals[3];
	unsigned int  x;
	unsigned int  y;

	for (i = 0; i < 3; i++) {
		l_start += string_to_uint(&in[l_start], &report_vals[i]) + 1;
	}
	b = report_vals[0];
	x = report_vals[1];
	y = report_vals[2];
	x -= 1;
	y -= 1;

	l_start -= 1;
	pressed = in[l_start];

	switch (b) {
	case CSI_MB_LEFT:
	case CSI_MB_LEFT_DRAG:
		tool_opts->x = x;
		tool_opts->y = y;
		use_tool(*tool_opts, world);

		if ('M' == pressed) {
			*mouse_pressed = true;
		} else {
			*mouse_pressed = false;
		}
		break;

	case CSI_MB_HOVER:
		tool_opts->x = x;
		tool_opts->y = y;
		*mouse_pressed = false;
		break;

	case CSI_MB_MIDDLE:
	case CSI_MB_MIDDLE_DRAG:
	case CSI_MB_RIGHT:
	case CSI_MB_RIGHT_DRAG:
		break;

	case CSI_MB_WHEELUP:
		tool_radius_add(1, tool_opts);
		break;

	case CSI_MB_WHEELDOWN:
		tool_radius_add(-1, tool_opts);
		break;
	}
}

void
handle_normal_csi_input(const char         *in,
                        bool               *mouse_pressed,
                        struct ToolOptions *tool_opts,
                        struct World       *world)
{
	if (strcmp(in, CSI_KEY_LEFT) == 0) {
		if (tool_opts->x > 0) {
			tool_opts->x -= 1;
		}
	} else if (strcmp(in, CSI_KEY_DOWN) == 0) {
		if (tool_opts->y < world->h - 1) {
			tool_opts->y += 1;
		}
	} else if (strcmp(in, CSI_KEY_UP) == 0) {
		if (tool_opts->y > 0) {
			tool_opts->y -= 1;
		}
	} else if (strcmp(in, CSI_KEY_RIGHT) == 0) {
		if (tool_opts->x < world->w - 1) {
			tool_opts->x += 1;
		}
	} else if (strcmp(in, CSI_KEY_HOME) == 0) {
		tool_opts->x = 0;
	} else if (strcmp(in, CSI_KEY_END) == 0) {
		tool_opts->x = world->w - 1;
	} else if (strcmp(in, CSI_KEY_PGUP) == 0) {
		tool_opts->y = 0;
	} else if (strcmp(in, CSI_KEY_PGDOWN) == 0) {
		tool_opts->y = world->h - 1;
	} else if (strcmp(in, CSI_KEY_CTRLHOME) == 0) {
		tool_opts->x = 0;
		tool_opts->y = 0;
	} else if (strcmp(in, CSI_KEY_CTRLEND) == 0) {
		tool_opts->x = world->w - 1;
		tool_opts->y = world->h - 1;
	} else if (in[1] == '[' &&
	           in[2] == '<') {
		handle_mouse_input(in, mouse_pressed, tool_opts, world);
	}
}

void
handle_normal_input(const char         *in,
                    bool               *active,
                    enum InputMode     *input_mode,
                    bool               *mouse_pressed,
                    bool               *paused,
                    int                *sim_subsample,
                    const int           tickrate,
                    bool               *th_vision,
                    struct ToolOptions *tool_opts,
                    struct World       *world)
{
	switch (in[0]) {
	case KEY_QUIT:
		*active = false;
		break;

	case KEY_USE:
		use_tool(*tool_opts, world);
		break;

	case KEY_SWITCH_VISION:
		if (*th_vision)
			*th_vision = false;
		else
			*th_vision = true;
		break;

	case KEY_PREVIOUS_MAT:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			if (tool_opts->brush_mat > FIRST_REAL_MAT) {
				tool_opts->brush_mat -= 1;
			}
			break;

		case TOOL_SPAWNER:
			if (tool_opts->spawner_mat > MAT_NONE) {
				tool_opts->spawner_mat -= 1;
			}
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_FIRST_MAT:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_mat = FIRST_REAL_MAT;
			break;

		case TOOL_SPAWNER:
			tool_opts->spawner_mat = MAT_NONE;
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_NEXT_MAT:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			if (tool_opts->brush_mat < MAT_COUNT - 1) {
				tool_opts->brush_mat += 1;
			}
			break;

		case TOOL_SPAWNER:
			if (tool_opts->spawner_mat < MAT_COUNT - 1) {
				tool_opts->spawner_mat += 1;
			}
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_LAST_MAT:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_mat = MAT_COUNT - 1;
			break;

		case TOOL_SPAWNER:
			tool_opts->spawner_mat = MAT_COUNT - 1;
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_BRUSH:
		tool_opts->sel_tool = TOOL_BRUSH;
		break;

	case KEY_SPAWNER:
		tool_opts->sel_tool = TOOL_SPAWNER;
		break;

	case KEY_ERASER:
		tool_opts->sel_tool = TOOL_ERASER;
		break;

	case KEY_HEATER:
		tool_opts->sel_tool = TOOL_HEATER;
		break;

	case KEY_COOLER:
		tool_opts->sel_tool = TOOL_COOLER;
		break;

	case KEY_LEFT:
		if (tool_opts->x > 0)
			tool_opts->x -= 1;
		break;

	case KEY_LEFT_MAX:
		tool_opts->x = 0;
		break;

	case KEY_DOWN:
		if (tool_opts->y < world->h - 1)
			tool_opts->y += 1;
		break;

	case KEY_DOWN_MAX:
		tool_opts->y = world->h - 1;
		break;

	case KEY_UP:
		if (tool_opts->y > 0)
			tool_opts->y -= 1;
		break;

	case KEY_UP_MAX:
		tool_opts->y = 0;
		break;

	case KEY_RIGHT:
		if (tool_opts->x < world->w - 1)
			tool_opts->x += 1;
		break;

	case KEY_RIGHT_MAX:
		tool_opts->x = world->w - 1;
		break;

	case KEY_RADIUS_DOWN:
		tool_radius_add(-1, tool_opts);
		break;

	case KEY_RADIUS_MIN:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_radius = 0;
			break;

		case TOOL_SPAWNER:
			break;

		case TOOL_ERASER:
			tool_opts->eraser_radius = 0;
			break;

		case TOOL_HEATER:
		case TOOL_COOLER:
			tool_opts->thermo_radius = 0;
			break;

		case TOOL_COUNT:
			break;
		}
		break;

	case KEY_RADIUS_UP:
		tool_radius_add(1, tool_opts);
		break;

	case KEY_RADIUS_MAX:
		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_radius = MAX_RADIUS;
			break;

		case TOOL_SPAWNER:
			break;

		case TOOL_ERASER:
			tool_opts->eraser_radius = MAX_RADIUS;
			break;

		case TOOL_HEATER:
		case TOOL_COOLER:
			tool_opts->thermo_radius = MAX_RADIUS;
			break;

		case TOOL_COUNT:
			break;
		}
		break;

	case '-':
	case KEY_SIMSPEED_DOWN:
		if (*sim_subsample < tickrate) {
			*sim_subsample *= 2;
		}
		break;

	case KEY_SIMSPEED_MIN:
		while (*sim_subsample < tickrate) {
			*sim_subsample *= 2;
		}
		break;

	case '+':
	case KEY_SIMSPEED_UP:
		if (*sim_subsample > 1) {
			*sim_subsample /= 2;
		}
		break;

	case KEY_SIMSPEED_MAX:
		*sim_subsample = 1;
		break;

	case KEY_CMD:
		*input_mode = IM_COMMAND;
		break;

	case KEY_PAUSE:
		if (*paused)
			*paused = false;
		else
			*paused = true;
		break;

	case SIG_INT:
	case SIG_TSTP:
		*active = false;
		break;

	case CHAR_ESCAPE:
		handle_normal_csi_input(in, mouse_pressed, tool_opts, world);
		break;
	}
}

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
              const char             *world_name)
{
	size_t             a, b;
	char               buf[BUF_SIZE];
	size_t             buf_len;
	struct ToolOptions maxcoords_to = {
		.x = 999,
		.y = 999,
	};
	size_t             statusbar_len = 0;
	size_t             statusbar_max_elems = 0;
	struct winsize     ws;

	ws = CSI_get_size();
	if (*win_w != ws.ws_col ||
	    *win_h != ws.ws_row) {
		*win_w = ws.ws_col;
		*win_h = ws.ws_row;

		*display_size = (size_t) ((float) *win_w *
		                          (float) *win_h *
		                          (float) DISPLAY_SIZE_MODIFIER) *
		                dot_depth;
		*display = realloc(*display, *display_size);

		for (a = 0; a < SBE_COUNT; a++) {
			buf[0] = '\0';
			/* Here it is important to render the biggest possible
			 * thing, unless it's not expected to change.
			 * Only in that case use real data.
			 */
			statusbar_len += render_statusbar_display(buf,
		                                                  BUF_SIZE,
		                                                  ip_address,
		                                                  false,
		                                                  STATUSBAR_DISPLAY_PRIORITY[a],
		                                                  1,
		                                                  120,
		                                                  true,
		                                                  maxcoords_to,
		                                                  world_name);

			if (statusbar_len > (size_t) *win_w) {
				break;
			}

			statusbar_len += strlen(STATUSBAR_SEPARATOR);
		}
		statusbar_max_elems = a;

		for (a = 0; a < SBE_COUNT; a++) {
			for (b = 0; b < statusbar_max_elems; b++) {
				if (STATUSBAR_DISPLAY_ORDER[a] == STATUSBAR_DISPLAY_PRIORITY[b]) {
					statusbar_elem[*statusbar_elems] = STATUSBAR_DISPLAY_ORDER[a];
					*statusbar_elems += 1;
				}
			}
		}

		if (input_mode == IM_COMMAND) {
			handle_cmdline_shift(cmdline_len,
			                     cmdline_shift,
			                     *win_w);
		}
	}
}

void
handle_simple_command(const char          *cmdline,
                      bool                *active,
                      char               **feedback,
                      clock_t             *feedback_expiration,
                      bool                *no_glowcolor,
                      clock_t              now,
                      bool                *paused,
                      int                 *sim_subsample,
                      bool                *th_vision,
                      int                 *tickrate,
                      struct ToolOptions  *tool_opts,
                      struct World        *world)
{
	int x, y;

	*feedback = NULL;

	if (strcmp(cmdline, CMD_BRUSH) == 0 ||
	    strcmp(cmdline, CMD_BRUSH_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_BRUSH;
	} else if (strcmp(cmdline, CMD_CLEAR) == 0 ||
	           strcmp(cmdline, CMD_CLEAR_SHORT) == 0) {
		for (x = 0; x < world->w; x++) {
			for (y = 0; y < world->h; y++) {
				world_clear_dot(world, x, y);
			}
		}
	} else if (strcmp(cmdline, CMD_CLEARALL) == 0 ||
	           strcmp(cmdline, CMD_CLEARALL_SHORT) == 0) {
		for (x = 0; x < world->w; x++) {
			for (y = 0; y < world->h; y++) {
				world_clear_dot(world, x, y);
				world->spawner[x][y] = false;
			}
		}
	} else if (strcmp(cmdline, CMD_COOLER) == 0 ||
	           strcmp(cmdline, CMD_COOLER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_COOLER;
	} else if (strcmp(cmdline, CMD_DEFAULTS) == 0 ||
	           strcmp(cmdline, CMD_DEFAULTS_SHORT) == 0) {
		tool_opts->brush_radius = STD_BRUSH_RADIUS;
		tool_opts->eraser_radius = STD_ERASER_RADIUS;
		tool_opts->sel_tool = STD_SELECTED_TOOL;
		tool_opts->thermo_delta = STD_THERMO_DELTA;
		tool_opts->thermo_radius = STD_THERMO_RADIUS;
		*sim_subsample = STD_SIM_SUBSAMPLE;
		tool_opts->spawn_temperature = STD_SPAWN_TEMPERATURE;
		*tickrate = STD_TICKRATE;
	} else if (strcmp(cmdline, CMD_ERASER) == 0 ||
	           strcmp(cmdline, CMD_ERASER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_ERASER;
	} else if (strcmp(cmdline, CMD_GLOWCOLOR) == 0 ||
	           strcmp(cmdline, CMD_GLOWCOLOR_SHORT) == 0) {
		*no_glowcolor = false;
	} else if (strcmp(cmdline, CMD_HEATER) == 0 ||
	           strcmp(cmdline, CMD_HEATER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_HEATER;
	} else if (strcmp(cmdline, CMD_NOGLOWCOLOR) == 0 ||
	           strcmp(cmdline, CMD_NOGLOWCOLOR_SHORT) == 0) {
		*no_glowcolor = true;
	} else if (strcmp(cmdline, CMD_NORMALVISION) == 0 ||
	           strcmp(cmdline, CMD_NORMALVISION_SHORT) == 0) {
		*th_vision = false;
	} else if (strcmp(cmdline, CMD_PAUSE) == 0 ||
	           strcmp(cmdline, CMD_PAUSE_SHORT) == 0) {
		if (*paused) {
			*paused = false;
		} else {
			*paused = true;
		}
	} else if (strcmp(cmdline, CMD_QUIT) == 0 ||
	           strcmp(cmdline, CMD_QUIT_SHORT) == 0 ||
	           strcmp(cmdline, "exit") == 0) {
		*active = false;
	} else if (strcmp(cmdline, CMD_SPAWNER) == 0 ||
	           strcmp(cmdline, CMD_SPAWNER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_SPAWNER;
	} else if (strcmp(cmdline, CMD_THERMOVISION) == 0 ||
	           strcmp(cmdline, CMD_THERMOVISION_SHORT) == 0) {
		*th_vision = true;
	} else {
		set_feedback(feedback, feedback_expiration, now,
		             "Command not recognized.");
	}
}

struct ToolOptions
new_tool_options()
{
	struct ToolOptions ret = {
		.brush_mat = FIRST_REAL_MAT,
		.brush_radius = STD_BRUSH_RADIUS,
		.eraser_radius = STD_ERASER_RADIUS,
		.sel_tool = STD_SELECTED_TOOL,
		.spawn_temperature = STD_SPAWN_TEMPERATURE,
		.spawner_mat = FIRST_REAL_MAT,
		.thermo_delta = STD_THERMO_DELTA,
		.thermo_radius = STD_THERMO_RADIUS,
		.x = 0,
		.y = 0,
		.x1 = 0,
		.y1 = 0,
		.x2 = 0,
		.y2 = 0,
	};
	return ret;
}

void
tool_radius_add(const int           radius_change,
                struct ToolOptions *tool_opts)
{
	int  *target = NULL;

	switch (tool_opts->sel_tool) {
	case TOOL_BRUSH:
		target = &tool_opts->brush_radius;
		break;

	case TOOL_SPAWNER:
		return;
		break;

	case TOOL_ERASER:
		target = &tool_opts->eraser_radius;
		break;

	case TOOL_HEATER:
	case TOOL_COOLER:
		target = &tool_opts->thermo_radius;
		break;

	case TOOL_COUNT:
		break;
	}

	*target += radius_change;
	if (*target < 0) {
		*target = 0;
	} else if (*target > MAX_RADIUS) {
		*target = MAX_RADIUS;
	}
}

size_t
render_dot(char               *out,
           const size_t        out_size,
           const struct Rgba   color,
           const struct World  world,
           const int           x,
           const int           y)
{
	size_t written = 0;

	if (world.spawner[x][y] == true) {
		written += CSI_color_to_string(SPAWNER_R,
		                               SPAWNER_G,
		                               SPAWNER_B,
		                               true,
		                               &out[written],
		                               out_size - written);
		out[written] = 'O';
		written += 1;
	} else if (world.dot[x][y] == MAT_NONE) {
		written += CSI_color_to_string(255, 255, 255,
		                               true,
		                               &out[written],
		                               out_size - written);
		out[written] = ' ';
		written += 1;
	} else {
		written += CSI_color_to_string(color.r,
		                               color.g,
		                               color.b,
		                               true,
		                               &out[written],
		                               out_size - written);
		out[written] = DOT_APPEARANCE[world.state[x][y]];
		written += 1;
	}

	out[written] = '\0';
	return written;
}

size_t
render_dot_no_color(char               *out,
                    const struct World  world,
                    const int           x,
                    const int           y)
{
	size_t written = 0;

	if (world.spawner[x][y] == true) {
		out[written] = 'O';
		written += 1;
	} else if (world.dot[x][y] == MAT_NONE) {
		out[written] = ' ';
		written += 1;
	} else {
		out[written] = DOT_APPEARANCE[world.state[x][y]];
		written += 1;
	}

	out[written] = '\0';
	return written;
}

size_t
render_statusbar_display(char                        *out,
                         const size_t                 out_size,
                         const char                  *ip_address,
                         const bool                   paused,
                         const enum StatusbarElement  sbe,
                         const int                    sim_subsample,
                         const int                    tickrate,
                         const bool                   th_vision,
                         struct ToolOptions           tool_opts,
                         const char                  *world_name)
{
	int    sim_speed = 0;
	char  *vision = NULL;
	size_t written = 0;

	switch (sbe) {
	case SBE_WORLD_NAME:
		written += string_cat(out,
		                      out_size,
		                      written,
		                      world_name);
		break;

	case SBE_COORDS:
		written += string_cat(out,
		                      out_size,
		                      written,
		                      NUMBERSTRING[tool_opts.x]);
		out[written] = ',';
		written += 1;
		written += string_cat(out,
		                      out_size,
		                      written,
		                      NUMBERSTRING[tool_opts.y]);
		break;

	case SBE_VIEW:
		if (th_vision) {
			vision = "Thermal";
		} else {
			vision = "Normal";
		}

		written += string_cat(out,
		                      out_size,
		                      written,
		                      "View:");
		written += string_cat(out,
		                      out_size,
		                      written,
		                      vision);
		break;

	case SBE_SPEED:
		written += string_cat(out,
		                      out_size,
		                      written,
		                      "Speed:");
		if (paused) {
			written += string_cat(out,
			                      out_size,
			                      written,
			                      "None");
		} else {
			sim_speed = (float) tickrate /
			            (float) sim_subsample;
			written += string_cat(out,
			                      out_size,
			                      written,
			                      NUMBERSTRING[sim_speed]);
			written += string_cat(out,
			                      out_size,
			                      written,
			                      "/s");
		}
		break;

	case SBE_IP_ADDRESS:
		written += string_cat(out,
		                      out_size,
		                      written,
		                      ip_address);
		break;

	case SBE_COUNT:
		break;
	}

	return written;
}

size_t
render_tool_hint(char                     *out,
                 const size_t              out_size,
                 const struct ToolOptions  tool_opts)
{
	size_t written = 0;

	written += string_cat(out,
	                      out_size,
	                      written,
	                      TOOL_NAME[tool_opts.sel_tool]);

	if (tool_opts.sel_tool == TOOL_BRUSH) {
		written += string_cat(out, out_size, written, " ");
		written += string_cat(out,
		                      out_size,
		                      written,
		                      MAT_NAME[tool_opts.brush_mat]);
	} else if (tool_opts.sel_tool == TOOL_SPAWNER) {
		written += string_cat(out, out_size, written, " ");
		written += string_cat(out,
		                      out_size,
		                      written,
		                      MAT_NAME[tool_opts.spawner_mat]);
	}

	return written;
}

size_t
render_world(char               *out,
             const size_t        out_size,
             const size_t        dot_depth,
             const bool          no_color,
             const bool          no_glowcolor,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world,
             const int           world_draw_w,
             const int           world_draw_h)
{
	size_t written = 0;
	int    x, y;

	if (th_vision) {
		if (no_color) {
			DOT_RENDER_LOOP_NO_COLOR
		} else {
			DOT_RENDER_LOOP(get_thermal_dot_color(world, x, y))
		}
	} else if (no_glowcolor) {
		if (no_color) {
			DOT_RENDER_LOOP_NO_COLOR
		} else {
			DOT_RENDER_LOOP(get_normal_dot_color_simple(world, x, y))
		}
	} else {
		if (no_color) {
			DOT_RENDER_LOOP_NO_COLOR
		} else {
			DOT_RENDER_LOOP(get_normal_dot_color(world, x, y))
		}
	}

	for (x = tool_opts.x1; x < tool_opts.x2; x++) {
		for (y = tool_opts.y1; y < tool_opts.y2; y++) {
			out[((y * world.w) + x + 1) * dot_depth - 1] = '^';
		}
	}

	return written;
}

void
set_feedback(char          **feedback,
             clock_t        *feedback_expiration,
             const clock_t   now,
             char           *str)
{
	*feedback = str;
	*feedback_expiration = now + (CLOCKS_PER_SEC * FEEDBACK_LIFETIME);
}

void
use_tool(struct ToolOptions  tool_opts,
         struct World       *world)
{
	switch (tool_opts.sel_tool) {
	case TOOL_BRUSH:
		world_use_brush(world,
		                tool_opts.brush_mat,
		                tool_opts.spawn_temperature,
		                tool_opts.x,
		                tool_opts.y,
		                tool_opts.brush_radius);
		break;

	case TOOL_SPAWNER:
		world->spawner[tool_opts.x][tool_opts.y] = true;
		world->spawner_mat[tool_opts.x][tool_opts.y] = tool_opts.spawner_mat;
		break;

	case TOOL_ERASER:
		world_use_eraser(world,
		                 tool_opts.x,
		                 tool_opts.y,
		                 tool_opts.eraser_radius);
		break;

	case TOOL_HEATER:
		world_use_heater(world,
		                 tool_opts.thermo_delta,
		                 tool_opts.x,
		                 tool_opts.y,
		                 tool_opts.thermo_radius);
		break;

	case TOOL_COOLER:
		world_use_cooler(world,
		                 tool_opts.thermo_delta,
		                 tool_opts.x,
		                 tool_opts.y,
		                 tool_opts.thermo_radius);
		break;

	case TOOL_COUNT:
		break;
	}
}

int
main(int    argc,
     char **argv)
{
	bool                   active = true;
	char                   cmdline[CMDLINE_SIZE];
	size_t                 cmdline_len = 0;
	size_t                 cmdline_shift = 0;
	char                  *display = NULL;
	size_t                 display_size = 0;
	size_t                 dot_depth = 0;
	char                  *feedback = NULL;
	clock_t                feedback_expiration = 0;
	enum InputMode         input_mode = IM_NORMAL;
	char                  *ip_address = "localhost";
	bool                   paused = false;
	clock_t                last_tick = 0;
	bool                   mouse_pressed = false;
	bool                   no_color = false;
	bool                   no_glowcolor = false;
	clock_t                now = 0;
	int                    sim_subsample = STD_SIM_SUBSAMPLE;
	size_t                 statusbar_elems = 0;
	enum StatusbarElement  statusbar_elem[SBE_COUNT];
	bool                   th_vision = false;
	int                    tickrate = STD_TICKRATE;
	int                    ts_since_sim = 9001; /* ticks since last simulation */
	struct ToolOptions     tool_opts;
	int                    tool_radius = STD_BRUSH_RADIUS;
	int                    win_w = 0;
	int                    win_h = 0;
	struct World           world;
	char                  *world_name = "worldname";

	if (!handle_args(argc, argv,
	                 &no_color,
	                 &no_glowcolor,
	                 &sim_subsample,
	                 &tickrate,
	                 &tool_opts)) {
		return 0;
	}

	hawps_core_init();
	hawps_extra_init();

	tool_opts = new_tool_options();

	CSI_set_raw();
	fputs(CSI_CLEAR, stdout);

	cmdline[0] = '\0';
	if (no_color) {
		dot_depth = 1;
	} else {
		dot_depth = CSI_COLORSTRING_LEN + 1;
	}

	display = malloc(1); /* we love hacks (next function ONLY reallocs) */

	handle_resize(cmdline_len,
	              &cmdline_shift,
	              &display,
	              &display_size,
	              dot_depth,
	              input_mode,
	              ip_address,
	              &statusbar_elems,
	              statusbar_elem,
	              &win_w,
	              &win_h,
	              world_name);

	world = world_new(win_w, win_h - 2, tool_opts.spawn_temperature);

	while (active) {
		handle_input(&active,
		             cmdline,
		             &cmdline_len,
		             &cmdline_shift,
		             &feedback,
		             &feedback_expiration,
		             &input_mode,
		             &mouse_pressed,
		             &no_glowcolor,
		             now,
		             &paused,
		             &sim_subsample,
		             &tickrate,
		             &th_vision,
		             &tool_opts,
		             win_w,
		             &world);

		switch (tool_opts.sel_tool) {
		case TOOL_BRUSH:
			tool_radius = tool_opts.brush_radius;
			break;

		case TOOL_SPAWNER:
			tool_radius = 0;
			break;

		case TOOL_ERASER:
			tool_radius = tool_opts.eraser_radius;
			break;

		case TOOL_HEATER:
		case TOOL_COOLER:
			tool_radius = tool_opts.thermo_radius;
			break;

		case TOOL_COUNT:
			break;
		}

		tool_opts.x1 = tool_opts.x - tool_radius;
		if (tool_opts.x1 < 0)
			tool_opts.x1 = 0;

		tool_opts.y1 = tool_opts.y - tool_radius;
		if (tool_opts.y1 < 0)
			tool_opts.y1 = 0;

		tool_opts.x2 = tool_opts.x + tool_radius + 1;
		if (tool_opts.x2 >= world.w)
			tool_opts.x2 = world.w;

		tool_opts.y2 = tool_opts.y + tool_radius + 1;
		if (tool_opts.y2 >= world.h)
			tool_opts.y2 = world.h;

		now = clock();
		if (now - last_tick >= (long) (CLOCKS_PER_SEC / tickrate)) {
			last_tick = now;

			if (now > feedback_expiration) {
				feedback = NULL;
			}

			if (mouse_pressed) {
				use_tool(tool_opts, &world);
			}

			world_update(&world, tool_opts.spawn_temperature);
			if (!paused) {
				if (ts_since_sim >= sim_subsample) {
					world_sim(&world);
					ts_since_sim = 0;
				} else {
					ts_since_sim += 1;
				}
			}

			handle_resize(cmdline_len,
			              &cmdline_shift,
			              &display,
			              &display_size,
			              dot_depth,
			              input_mode,
			              ip_address,
			              &statusbar_elems,
			              statusbar_elem,
			              &win_w,
			              &win_h,
			              world_name);

			draw(cmdline,
			     cmdline_len,
			     cmdline_shift,
			     display,
			     display_size,
			     dot_depth,
			     feedback,
			     input_mode,
			     ip_address,
			     no_color,
			     no_glowcolor,
			     paused,
			     sim_subsample,
			     statusbar_elems,
			     statusbar_elem,
			     tickrate,
			     th_vision,
			     tool_opts,
			     win_w,
			     win_h,
			     world,
			     world_name);
		}
	}

	CSI_set_normal();
	fputs(CSI_CLEAR, stdout);
	fputs(CSI_FG_DEFAULT, stdout);
	fputs(CSI_BG_DEFAULT, stdout);
	world_free(&world);
	if (display != NULL) {
		free(display);
	}

	return 0;
}
