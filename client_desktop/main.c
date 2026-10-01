/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#define HAWPS_IMPL
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

#include "config.h"
#include "int_to_string.h"
#include "str.h"
#include "types.h"

#ifdef SDL_BACKEND
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#else
#include "csi.h"
#endif

/* Macros
 */

#ifdef SDL_BACKEND
#else

#define DOT_EMPTY_LINES_RENDER_LOOP \
	for (y = 0; y < world_draw_space_h; y++) { \
		memset(&out[written], ' ', world_draw.w + world_draw_space_w); \
		written += world_draw.w + world_draw_space_w;\
	}

#define DOT_RENDER_LOOP(COLOR_FN_CALL) \
	for (y = world_draw.y; y < world_draw.h + world_draw.y; y++) { \
		for (x = world_draw.x; x < world_draw.w + world_draw.x; x++) { \
			written += render_dot(&out[written], \
			                      out_size - written, \
			                      COLOR_FN_CALL, \
			                      world, \
			                      x, \
			                      y); \
		} \
		memset(&out[written], ' ', world_draw_space_w); \
		written += world_draw_space_w;\
	} \
	DOT_EMPTY_LINES_RENDER_LOOP

#define DOT_RENDER_LOOP_NO_COLOR \
	for (y = world_draw.y; y < world_draw.h + world_draw.y; y++) { \
		for (x = world_draw.x; x < world_draw.w + world_draw.x; x++) { \
			written += render_dot_no_color(&out[written], \
			                               world, \
			                               x, \
			                               y); \
		} \
		memset(&out[written], ' ', world_draw_space_w); \
		written += world_draw_space_w;\
	} \
	DOT_EMPTY_LINES_RENDER_LOOP

#endif /* SDL_BACKEND */

/* Constant defines
 */

#ifdef SDL_BACKEND
#define CMDL_SIZE (CMDLINE_SIZE + 2)
#endif

#define FLAG_ABOUT                  "-about"
#define FLAG_ABOUT_SHORT            "-a"
#define FLAG_AUTOSAVE_ALL           "-autosave-all"
#define FLAG_AUTOSAVE_ALL_SHORT     "-asva"
#define FLAG_AUTOSAVE_NONE          "-autosave-none"
#define FLAG_AUTOSAVE_NONE_SHORT    "-asvn"
#define FLAG_BRUSHRADIUS            "-brushradius"
#define FLAG_BRUSHRADIUS_SHORT      "-br"
#define FLAG_ERASERRADIUS           "-eraserradius"
#define FLAG_ERASERRADIUS_SHORT     "-er"
#define FLAG_FRAMERATE              "-framerate"
#define FLAG_FRAMERATE_SHORT        "-fr"
#define FLAG_HELP                   "-help"
#define FLAG_HELP_SHORT             "-h"
#define FLAG_NOGLOWCOLOR            "-noglowcolor"
#define FLAG_NOGLOWCOLOR_SHORT      "-nogc"
#define FLAG_SPAWNTEMPERATURE       "-spawntemperature"
#define FLAG_SPAWNTEMPERATURE_SHORT "-st"
#define FLAG_THERMORADIUS           "-thermoradius"
#define FLAG_THERMORADIUS_SHORT     "-thrd"
#define FLAG_THERMORATE             "-thermorate"
#define FLAG_THERMORATE_SHORT       "-thrt"
#define FLAG_TICKRATE               "-tickrate"
#define FLAG_TICKRATE_SHORT         "-tr"
#define FLAG_VERSION                "-version"
#define FLAG_VERSION_SHORT          "-v"

#ifdef SDL_BACKEND
#define FLAG_FONT_PATH              "-fontpath"
#define FLAG_FONT_PATH_SHORT        "-fp"
#define FLAG_FONT_SIZE              "-fontsize"
#define FLAG_FONT_SIZE_SHORT        "-fs"
#define FLAG_WORLD_SCALE            "-worldscale"
#define FLAG_WORLD_SCALE_SHORT      "-wldsc"
#else
#define FLAG_NOCOLOR                "-nocolor"
#define FLAG_NOCOLOR_SHORT          "-noc"
#endif

#ifdef SDL_BACKEND
#if defined(__linux__)
static const char *FONTPATH[] = {
	"/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
	"/usr/share/fonts/open-sans/OpenSans-Regular.ttf",
	"/usr/share/fonts/adwaita-sans-fonts/AdwaitaSans-Regular.ttf",
	"/usr/share/fonts/google-noto-sans-cjk-vf-fonts/NotoSansCJK-VF.ttc",
	"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
	"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
	"/usr/share/fonts/truetype/ubuntu/Ubuntu-Th.ttf",
	"/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
};
#elif defined(_WIN32)
static const char *FONTPATH[] = {
	"C:\\Windows\\Fonts\\segoeui.ttf",
	"C:\\Windows\\Fonts\\arial.ttf",
	"C:\\Windows\\Fonts\\cour.ttf",
};
#elif defined(__APPLE__)
static const char *FONTPATH[] = {
	"/System/Library/Fonts/SFNS.ttf",
};
#else
static const char *FONTPATH[] = {
	"/usr/share/fonts/truetype/freefont/FreeSans.ttf",
};
#endif
#endif /* SDL_BACKEND */

#define KEY_USE_THERMO_CONTINUE_LIMIT 0.5

#define MAX_FONT_SIZE 128

#ifdef _WIN32
#define PATH_DELIM "\\"
#else
#define PATH_DELIM "/"
#endif

#define SIG_INT  '\003'
#define SIG_TSTP '\032'

#define WORLDNAME_NEW    "new"
#define WORLDNAME_SIZE   32
#define WORLDNAME_MAXLEN (WORLDNAME_SIZE - 2)
#define WORLDNAME_TYPE   ".wld"

/* Types
 */

struct ToolOptions {
	enum Mat  brush_mat;
	int       brush_radius;
	int       eraser_radius;
	enum Tool sel_tool;
	float     spawn_temperature;
	enum Mat  spawner_mat;
	int       thermo_radius;
	float     thermo_rate;
	int       x;
	int       y;
};

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
"    " CMD_FRAMERATE_SHORT " " CMD_FRAMERATE " DECIMAL\n"
"        sets the rate-limit for frames per second\n"
"\n"
"    " CMD_GLOWCOLOR_SHORT " " CMD_GLOWCOLOR "\n"
"        enables dot glow coloring\n"
"\n"
"    " CMD_HEATER_SHORT " " CMD_HEATER "\n"
"        selects the heater as active tool\n"
"\n"
"    " CMD_LOAD_SHORT " " CMD_LOAD " [TEXT]\n"
"        loads the world with given name or with current name\n"
"        max name length: %i\n"
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
"    " CMD_SAVE_SHORT " " CMD_SAVE " [TEXT]\n"
"        saves the world with given name or with current name\n"
"        max name length: %i\n"
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
"    " CMD_THERMORADIUS_SHORT " " CMD_THERMORADIUS " NUMBER\n"
"        sets the radius of thermo tools to the given number\n"
"\n"
"    " CMD_THERMORATE_SHORT " " CMD_THERMORATE " DECIMAL\n"
"        sets the rate of thermo tools, with which heating/cooling occurs\n"
"\n"
"    " CMD_THERMOVISION_SHORT " " CMD_THERMOVISION "\n"
"        enables thermal vision\n"
"\n"
"    " CMD_TICKRATE_SHORT " " CMD_TICKRATE " DECIMAL\n"
"        sets the rate for ticks (simulations) per second\n"
"\n";

static const char APP_HELP_FLAGS[] = "Options:\n"
"\n"
"    " FLAG_ABOUT_SHORT " " FLAG_ABOUT "\n"
"        prints program name, version, license and repository information then exits\n"
"\n"
"    " FLAG_AUTOSAVE_ALL_SHORT " " FLAG_AUTOSAVE_ALL "\n"
"        enables autosave for all worlds\n"
"        which is by default only enabled for world \"" WORLDNAME_NEW "\"\n"
"\n"
"    " FLAG_AUTOSAVE_NONE_SHORT " " FLAG_AUTOSAVE_NONE "\n"
"        disables autosave for all worlds\n"
"\n"
"    " FLAG_BRUSHRADIUS_SHORT " " FLAG_BRUSHRADIUS " NUMBER\n"
"        sets the radius of the brush\n"
"        default: %i\n"
"\n"
"    " FLAG_ERASERRADIUS_SHORT " " FLAG_ERASERRADIUS " NUMBER\n"
"        sets the radius of the eraser\n"
"        default: %i\n"
"\n"
"    " FLAG_FRAMERATE_SHORT " " FLAG_FRAMERATE " DECIMAL\n"
"        sets the rate-limit for frames per second\n"
"        default: %.2f\n"
"\n"
"    " FLAG_HELP_SHORT " " FLAG_HELP "\n"
"        prints this message then exits\n"
"\n"
"    " FLAG_NOGLOWCOLOR_SHORT " " FLAG_NOGLOWCOLOR "\n"
"        disables dot glow coloring\n"
"\n"
"    " FLAG_SPAWNTEMPERATURE_SHORT " " FLAG_SPAWNTEMPERATURE " DECIMAL\n"
"        sets the temperature of every new dot in Kelvin\n"
"        0 °C == %.2f K\n"
"        default: %.2f\n"
"\n"
"    " FLAG_THERMORADIUS_SHORT " " FLAG_THERMORADIUS " NUMBER\n"
"        sets the radius of thermo tools\n"
"        default: %i\n"
"\n"
"    " FLAG_THERMORATE_SHORT " " FLAG_THERMORATE " DECIMAL\n"
"        sets the rate of thermo tools, with which heating/cooling occurs\n"
"        default: %.2f\n"
"\n"
"    " FLAG_TICKRATE_SHORT " " FLAG_TICKRATE " DECIMAL\n"
"        sets the rate for ticks (simulations) per second\n"
"        default: %.2f\n"
"\n"
"    " FLAG_VERSION_SHORT " " FLAG_VERSION "\n"
"        prints version information then exits\n"
"\n";

#ifdef SDL_BACKEND
static const char APP_HELP_FLAGS_SDL[] = "SDL backend options:\n"
"\n"
"    " FLAG_FONT_PATH_SHORT " " FLAG_FONT_PATH " TEXT\n"
"        sets the path for the globally used font\n"
"\n"
"    " FLAG_FONT_SIZE_SHORT " " FLAG_FONT_SIZE " NUMBER\n"
"        sets the size of the globally used font\n"
"        default: %i\n"
"        max:     %i\n"
"\n"
"    " FLAG_WORLD_SCALE_SHORT " " FLAG_WORLD_SCALE " NUMBER\n"
"        sets the size of a single dot in the world\n"
"        default: %i\n"
"\n";
#else
static const char APP_HELP_FLAGS_TERMINAL[] = "Terminal backend options:\n"
"\n"
"    " FLAG_NOCOLOR_SHORT " " FLAG_NOCOLOR "\n"
"        disables all world dot coloring\n"
"\n";
#endif

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
"    F5\n"
"        take a screenshot\n"
"\n"
"    F6\n"
"        quicksave (save under current worldname)\n"
"\n"
"    F7\n"
"        quickload (load from current worldname)\n"
"\n"
"    %c Left-Mouse\n"
"        use currently active tool\n"
"\n"
"    Middle-Mouse\n"
"        pick material for currently active tool\n"
"\n"
"    Right-Mouse\n"
"        drag world view\n"
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
"    %c Mouse-Wheel-Down\n"
"        decrease tool radius\n"
"\n"
"    %c\n"
"        set smallest tool radius\n"
"\n"
"    %c Mouse-Wheel-Up\n"
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

/* Function declarations
 */

bool
check_flag_arg(int         argc,
               char      **argv,
               const int   idx);

void
command_load_core(const char   *cwd,
                  struct World *world,
                  const char   *world_name);

void
command_save_core(const char         *cwd,
                  const struct World  world,
                  const char         *world_name);

void
command_screenshot(
#ifdef SDL_BACKEND
                   SDL_Renderer             *r,
                   const size_t              world_area_w,
                   const size_t              world_area_h,
#else
                   const size_t              display_size,
                   const size_t              dot_depth,
                   const bool                no_color,
                   const bool                no_glowcolor,
                   const struct ToolOptions  tool_opts,
                   const struct World        world,
                   const struct Rect         world_draw,
#endif
                   const char               *cwd,
                   char                    **feedback,
                   clock_t                  *feedback_expiration,
                   const clock_t             now,
                   const bool                th_vision);

void
command_temperature(const float   new_temperature,
                    struct World *world);

#ifdef SDL_BACKEND
void
draw(const char                  *cmdline,
     const char                  *feedback,
     TTF_Font                    *font,
     const size_t                 font_size,
     const enum InputMode         input_mode,
     const char                  *ip_address,
     const bool                   no_glowcolor,
     const bool                   paused,
     const size_t                 statusbar_elems,
     const enum StatusbarElement *statusbar_elem,
     const bool                   th_vision,
     const float                  tickrate,
     const struct ToolOptions     tool_opts,
     SDL_Renderer                *r,
     const struct World           world,
     const SDL_FRect              world_dst,
     const char                  *world_name,
     SDL_Texture                 *world_tx);

#else

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
#endif /* SDL_BACKEND */

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
handle_advanced_command(const char            *cmd,
                        const char            *arg,
#ifdef SDL_BACKEND
                        TTF_Font              *font,
                        SDL_Renderer          *renderer,
                        SDL_FRect             *world_dst,
                        const size_t           world_scale,
                        SDL_Texture          **world_tx,
#else
                        const int              win_h,
                        struct Rect           *world_draw,
                        int                   *world_draw_space_w,
                        int                   *world_draw_space_h,
#endif
                        const char            *cwd,
                        char                 **feedback,
                        clock_t               *feedback_expiration,
                        float                 *framerate,
                        const char            *ip_address,
                        const clock_t          now,
                        size_t                *statusbar_elems,
                        enum StatusbarElement *statusbar_elem,
                        float                 *tickrate,
                        struct ToolOptions    *tool_opts,
                        const int              win_w,
                        struct World          *world,
                        char                  *world_name);

bool
handle_args(int                  argc,
            char               **argv,
#ifdef SDL_BACKEND
            char               **font_path,
            size_t              *font_size,
            size_t              *world_scale,
#else
            bool                *no_color,
#endif
            bool                *autosave_all,
            bool                *autosave_none,
            float               *framerate,
            bool                *no_glowcolor,
            float               *tickrate,
            struct ToolOptions  *tool_opts);

void
handle_autosave(const bool          autosave_all,
                const bool          autosave_none,
                const char         *cwd,
                clock_t            *last_autosave,
                const clock_t       now,
                const struct World  world,
                const char         *world_name);

void
handle_cmdline_shift(const size_t          cmdline_len,
                     size_t               *cmdline_shift,
                     const int             win_w);

void
handle_command(char                  *cmdline,
               const size_t           cmdline_len,
#ifdef SDL_BACKEND
               TTF_Font              *font,
               SDL_Renderer          *renderer,
               SDL_FRect             *world_dst,
               const size_t           world_scale,
               SDL_Texture          **world_tx,
#else
               const int              win_h,
               struct Rect           *world_draw,
               int                   *world_draw_space_w,
               int                   *world_draw_space_h,
#endif
               bool                  *active,
               const char            *cwd,
               char                 **feedback,
               clock_t               *feedback_expiration,
               float                 *framerate,
               const char            *ip_address,
               bool                  *no_glowcolor,
               const clock_t          now,
               bool                  *paused,
               size_t                *statusbar_elems,
               enum StatusbarElement *statusbar_elem,
               bool                  *th_vision,
               float                 *tickrate,
               struct ToolOptions    *tool_opts,
               const int              win_w,
               struct World          *world,
               char                  *world_name);

#ifdef SDL_BACKEND
#else
void
handle_command_input(const char            *in,
                     bool                  *active,
                     char                  *cmdline,
                     size_t                *cmdline_len,
                     size_t                *cmdline_shift,
                     const char            *cwd,
                     char                 **feedback,
                     clock_t               *feedback_expiration,
                     float                 *framerate,
                     enum InputMode        *input_mode,
                     const char            *ip_address,
                     bool                  *no_glowcolor,
                     clock_t                now,
                     bool                  *paused,
                     size_t                *statusbar_elems,
                     enum StatusbarElement *statusbar_elem,
                     bool                  *th_vision,
                     float                 *tickrate,
                     struct ToolOptions    *tool_opts,
                     const int              win_w,
                     const int              win_h,
                     struct World          *world,
                     struct Rect           *world_draw,
                     int                   *world_draw_space_w,
                     int                   *world_draw_space_h,
                     char                  *world_name);
#endif /* SDL_BACKEND */

bool
handle_flag_number_arg(int                            argc,
                       char                         **argv,
                       int                           *idx,
                       float                         *out,
                       const enum NumberRequirement   requirement);

void
handle_input(
#ifdef SDL_BACKEND
             TTF_Font              *font,
             const size_t           font_size,
             SDL_Renderer          *renderer,
             SDL_Window            *win,
             int                   *win_w,
             int                   *win_h,
             size_t                *world_area_w,
             size_t                *world_area_h,
             SDL_FRect             *world_dst,
             const size_t           world_scale,
             SDL_Texture          **world_tx,
#else
             size_t                *cmdline_shift,
             const size_t           display_size,
             const size_t           dot_depth,
             bool                  *lmb_pressed,
             const bool             no_color,
             const int              win_w,
             const int              win_h,
             struct Rect           *world_draw,
             int                   *world_draw_space_w,
             int                   *world_draw_space_h,
#endif /* SDL_BACKEND */
             bool                  *active,
             char                  *cmdline,
             size_t                *cmdline_len,
             const char            *cwd,
             const float            delta,
             int                   *drag_start_x,
             int                   *drag_start_y,
             char                 **feedback,
             clock_t               *feedback_expiration,
             float                 *framerate,
             enum InputMode        *input_mode,
             const char            *ip_address,
             clock_t               *last_key_use,
             bool                  *no_glowcolor,
             const clock_t          now,
             bool                  *paused,
             size_t                *statusbar_elems,
             enum StatusbarElement *statusbar_elem,
             float                 *tickrate,
             bool                  *th_vision,
             struct ToolOptions    *tool_opts,
             struct World          *world,
             char                  *world_name);

#ifdef SDL_BACKEND
void
handle_mouse_state(const float           delta,
                   int                  *drag_start_x,
                   int                  *drag_start_y,
                   struct ToolOptions   *tool_opts,
                   SDL_Window           *win,
                   struct World         *world,
                   const size_t          world_area_w,
                   const size_t          world_area_h,
                   SDL_FRect            *world_dst);

#else

void
handle_mouse_input(const char         *in,
                   const float         delta,
                   int                *drag_start_x,
                   int                *drag_start_y,
                   bool               *lmb_pressed,
                   struct ToolOptions *tool_opts,
                   struct World       *world,
                   struct Rect        *world_draw);
#endif /* SDL_BACKEND */

#ifdef SDL_BACKEND
#else
void
handle_normal_csi_input(const char         *in,
                        const char         *cwd,
                        const float         delta,
                        const size_t        display_size,
                        const size_t        dot_depth,
                        int                *drag_start_x,
                        int                *drag_start_y,
                        char              **feedback,
                        clock_t            *feedback_expiration,
                        bool               *lmb_pressed,
                        const bool          no_color,
                        const bool          no_glowcolor,
                        const clock_t       now,
                        const bool          th_vision,
                        struct ToolOptions *tool_opts,
                        struct World       *world,
                        struct Rect        *world_draw,
                        const char         *world_name);
#endif /* SDL_BACKEND */

/* @in: Input.
 * @world_draw: Runtime data.
 * @active: Runtime data.
 * @delta: Runtime data.
 * @input_mode: Runtime data.
 * @last_key_use: Runtime data.
 * @now: Runtime data.
 * @paused: Runtime data.
 * @tickrate: Runtime data.
 * @th_vision: Runtime data.
 * @tool_opts: Runtime data.
 * @world: Runtime data.
 *
 * Returns true if the input had been fully handled.
 */
bool
handle_normal_input(const char         *in,
#ifdef SDL_BACKEND
                    const int           win_w,
                    const int           win_h,
                    const size_t        world_area_w,
                    const size_t        world_area_h,
                    SDL_FRect          *world_dst,
                    const size_t        world_scale,
#else
                    struct Rect        *world_draw,
#endif
                    bool               *active,
                    const float         delta,
                    enum InputMode     *input_mode,
                    clock_t            *last_key_use,
                    const clock_t       now,
                    bool               *paused,
                    float              *tickrate,
                    bool               *th_vision,
                    struct ToolOptions *tool_opts,
                    struct World       *world);

#ifdef SDL_BACKEND
void
handle_resize(const size_t        font_size,
              SDL_Window         *win,
              int                *win_w,
              int                *win_h,
              size_t             *world_area_w,
              size_t             *world_area_h,
              SDL_FRect          *world_dst);

#else

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
              struct ToolOptions     *tool_opts,
              int                    *win_w,
              int                    *win_h,
              const struct World      world,
              struct Rect            *world_draw,
              int                    *world_draw_space_w,
              int                    *world_draw_space_h,
              const char             *world_name);
#endif /* SDL_BACKEND */

void
handle_simple_command(const char          *cmdline,
                      bool                *active,
                      const char          *cwd,
                      char               **feedback,
                      clock_t             *feedback_expiration,
                      float               *framerate,
                      bool                *no_glowcolor,
                      const clock_t        now,
                      bool                *paused,
                      bool                *th_vision,
                      float               *tickrate,
                      struct ToolOptions  *tool_opts,
                      struct World        *world,
                      const char          *world_name);

void
handle_statusbar_resize(
#ifdef SDL_BACKEND
                        TTF_Font              *font,
#endif
                        const char            *ip_address,
                        size_t                *statusbar_elems,
                        enum StatusbarElement *statusbar_elem,
                        const size_t           win_w,
                        const char            *world_name);

#ifdef SDL_BACKEND
void
handle_world_dst_clamp_x(const int     win_w,
                         const size_t  world_area_w,
                         SDL_FRect    *world_dst);
#endif /* SDL_BACKEND */

#ifdef SDL_BACKEND
void
handle_world_dst_clamp_y(const int     win_h,
                         const size_t  world_area_h,
                         SDL_FRect    *world_dst);
#endif /* SDL_BACKEND */

void
handle_world_resize(
#ifdef SDL_BACKEND
                    SDL_Renderer        *renderer,
                    SDL_FRect           *world_dst,
                    const size_t         world_scale,
                    SDL_Texture        **world_tx,
#else
                    const int            win_w,
                    const int            win_h,
                    struct Rect         *world_draw,
                    int                 *world_draw_space_w,
                    int                 *world_draw_space_h,
#endif
                    struct ToolOptions  *tool_opts,
                    const struct World   world);

void
quickload(const char     *cwd,
          char          **feedback,
          clock_t        *feedback_expiration,
          const clock_t	  now,
          struct World   *world,
          const char     *world_name);

#ifdef SDL_BACKEND
#else
size_t
render_dot(char               *out,
           const size_t        out_size,
           const struct Rgba   color,
           const struct World  world,
           const int           x,
           const int           y);
#endif /* SDL_BACKEND */

#ifdef SDL_BACKEND
#else
size_t
render_dot_no_color(char               *out,
                    const struct World  world,
                    const int           x,
                    const int           y);
#endif /* SDL_BACKEND */

#ifdef SDL_BACKEND
void
render_world(const bool          no_glowcolor,
             SDL_Renderer       *r,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world);

#else

size_t
render_world(char               *out,
             const size_t        out_size,
             const size_t        dot_depth,
             const bool          no_color,
             const bool          no_glowcolor,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world,
             const struct Rect   world_draw,
             const int           world_draw_space_w,
             const int           world_draw_space_h);
#endif /* SDL_BACKEND */

void
set_feedback(char          **feedback,
             clock_t        *feedback_expiration,
             const clock_t   now,
             char           *str);

void
tool_radius_add(const int           radius_change,
                struct ToolOptions *tool_opts);

struct ToolOptions
ToolOptions_new(void);

void
use_tool(const float         delta,
         struct ToolOptions  tool_opts,
         struct World       *world);

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

/* Function definitions
 */

bool
check_flag_arg(int         argc,
               char      **argv,
               const int   idx)
{
	if (argc <= idx + 1) {
		fprintf(stderr,
		        "The argument \"%s\" needs to be followed by a value\n",
		        argv[idx]);
		return false;
	}

	return true;
}

void
command_load_core(const char   *cwd,
                  struct World *world,
                  const char   *world_name)
{
	FILE  *file;
	char   path[PATH_SIZE];
	size_t path_len = 0;

	path_len += string_copy(path, PATH_SIZE, cwd);
	path_len += string_cat(path, PATH_SIZE, path_len, PATH_DELIM);
	path_len += string_cat(path, PATH_SIZE, path_len, world_name);
	path_len += string_cat(path, PATH_SIZE, path_len, WORLDNAME_TYPE);

	file = fopen(path, "r");
	if (NULL == file) {
		world->w = 0;
		world->h = 0;
		return;
	}

	*world = world_load(file);
	fclose(file);
}

void
command_save_core(const char         *cwd,
                  const struct World  world,
                  const char         *world_name)
{
	FILE   *file;
	char    path[PATH_SIZE];
	size_t  path_len = 0;

	path_len += string_copy(path, PATH_SIZE, cwd);
	path_len += string_cat(path, PATH_SIZE, path_len, PATH_DELIM);
	path_len += string_cat(path, PATH_SIZE, path_len, world_name);
	path_len += string_cat(path, PATH_SIZE, path_len, WORLDNAME_TYPE);

	file = fopen(path, "w");
	world_save(world, file);
	fclose(file);
}

void
command_screenshot(
#ifdef SDL_BACKEND
                   SDL_Renderer             *r,
                   const size_t              world_area_w,
                   const size_t              world_area_h,
#else
                   const size_t              display_size,
                   const size_t              dot_depth,
                   const bool                no_color,
                   const bool                no_glowcolor,
                   const struct ToolOptions  tool_opts,
                   const struct World        world,
                   const struct Rect         world_draw,
#endif
                   const char               *cwd,
                   char                    **feedback,
                   clock_t                  *feedback_expiration,
                   const clock_t             now,
                   const bool                th_vision)
{
	char    datetime[BUF_SIZE];
	time_t  epoch_now;
	char    path[BUF_SIZE];
	size_t  path_len = 0;

#ifdef SDL_BACKEND
	SDL_Rect     rect;
	SDL_Surface *world_sf = NULL;
#else
	FILE   *f = NULL;
	int     i;
	char   *world_print = NULL;

	datetime[0] = '\0';
	path[0] = '\0';
#endif

	path_len = string_copy(path, BUF_SIZE, cwd);
	path_len += string_cat(path, BUF_SIZE, path_len, PATH_DELIM);
	path_len += string_cat(path, BUF_SIZE, path_len, APP_NAME);
	path_len += string_cat(path,
	                       BUF_SIZE,
	                       path_len,
	                       (th_vision ? "_thermal_" : "_normal_"));
	epoch_now = time(NULL);
	strftime(datetime, BUF_SIZE, "%F_%H-%M-%S", localtime(&epoch_now));
	path_len += string_cat(path,
	                       BUF_SIZE,
	                       path_len,
	                       datetime);

#ifdef SDL_BACKEND
	path_len += string_cat(path, BUF_SIZE, path_len, ".png");
#else
	path_len += string_cat(path, BUF_SIZE, path_len, ".txt");
#endif

#ifdef SDL_BACKEND
	rect.x = 0;
	rect.y = 0;
	rect.w = world_area_w;
	rect.h = world_area_h;

	world_sf = SDL_RenderReadPixels(r, &rect);

	if (!SDL_SavePNG(world_sf, path)) {
		set_feedback(feedback, feedback_expiration, now,
		             "Couldn't save the screenshot");
		return;
	}

	SDL_DestroySurface(world_sf);
#else
	world_print = malloc(display_size);
	render_world(world_print,
	             display_size,
	             dot_depth,
	             no_color,
	             no_glowcolor,
	             th_vision,
	             tool_opts,
	             world,
	             world_draw,
	             0,
	             0);

	f = fopen(path, "w");
	if (NULL == f) {
		set_feedback(feedback, feedback_expiration, now,
		             "Couldn't save the screenshot");
		return;
	}

	for (i = 0; i < world_draw.h; i++) {
		fwrite(&world_print[i * ((dot_depth * world_draw.w))],
		       1,
		       dot_depth * world_draw.w,
		       f);
		fwrite("\n", 1, 1, f);
	}

	fclose(f);
	free(world_print);
#endif /* SDL_BACKEND */

	set_feedback(feedback, feedback_expiration, now,
	             "Screenshot saved");
}

void
command_temperature(const float   new_temperature,
                    struct World *world)
{
	int x, y;

	for (x = 0; x < world->w; x++) {
		for (y = 0; y < world->h; y++) {
			world->thermo[x][y] = new_temperature;

			if (world->thermo[x][y] >= MAT_BOIL_P[world->dot[x][y]]) {
				if (MAT_MELT_DECOMP[world->dot[x][y]]) {
					world->dot[x][y] = mat_melt_prdct(world->dot[x][y]);
				}
			}
		}
	}
}

#ifdef SDL_BACKEND
void
draw(const char                  *cmdline,
     const char                  *feedback,
     TTF_Font                    *font,
     const size_t                 font_size,
     const enum InputMode         input_mode,
     const char                  *ip_address,
     const bool                   no_glowcolor,
     const bool                   paused,
     const size_t                 statusbar_elems,
     const enum StatusbarElement *statusbar_elem,
     const bool                   th_vision,
     const float                  tickrate,
     const struct ToolOptions     tool_opts,
     SDL_Renderer                *r,
     const struct World           world,
     const SDL_FRect              world_dst,
     const char                  *world_name,
     SDL_Texture                 *world_tx)
{
	SDL_Color    bg;
	char         cmdl[CMDL_SIZE];
	size_t       cmdl_len = 0;
	SDL_FRect    cmdlr;
	SDL_Surface *cmdls;
	SDL_Texture *cmdlt;
	SDL_Color    fg;
	size_t       i;
	char         sb[CMDLINE_SIZE];
	size_t       sb_len = 0;
	SDL_FRect    sbr;
	SDL_Surface *sbs;
	SDL_Texture *sbt;
	SDL_Window  *win;
	int          win_w;
	int          win_h;

	cmdl[0] = '\0';
	sb[0] = '\0';
	win = SDL_GetRenderWindow(r);
	SDL_GetWindowSize(win, &win_w, &win_h);
	sbr = (SDL_FRect) {
		.x = 0,
		.y = win_h - (font_size * 2),
		.w = 0,
		.h = font_size,
	};
	cmdlr = (SDL_FRect) {
		.x = 0,
		.y = sbr.y + sbr.h,
		.w = 0,
		.h = font_size,
	};

	SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(r);

	if (th_vision) {
		bg.r = THERMAL_VISION_R;
		bg.g = THERMAL_VISION_G;
		bg.b = THERMAL_VISION_B;
		bg.a = SDL_ALPHA_OPAQUE;
	} else {
		bg.r = 30;
		bg.g = 30;
		bg.b = 30;
		bg.a = SDL_ALPHA_OPAQUE;
	}
	fg.r = 255 - bg.r;
	fg.g = 255 - bg.g;
	fg.b = 255 - bg.b;
	fg.a = SDL_ALPHA_OPAQUE;

	SDL_SetRenderDrawColor(r, bg.r, bg.g, bg.b, bg.a);
	SDL_RenderClear(r);

	SDL_SetRenderTarget(r, world_tx);
	SDL_RenderClear(r);
	render_world(no_glowcolor, r, th_vision, tool_opts, world);
	SDL_SetRenderTarget(r, NULL);

	SDL_RenderTexture(r, world_tx, NULL, &world_dst);

	i = 0;
	while (1) {
		sb_len += write_statusbar_elem(&sb[sb_len],
		                               CMDLINE_SIZE - sb_len,
		                               ip_address,
		                               paused,
		                               statusbar_elem[i],
		                               th_vision,
		                               tickrate,
		                               tool_opts,
		                               world_name);

		i++;
		if (i >= statusbar_elems) {
			break;
		}

		sb_len += string_cat(sb,
		                     CMDLINE_SIZE,
		                     sb_len,
		                     STATUSBAR_SEPARATOR);
	}
	sbs = TTF_RenderText_LCD(font, sb, sb_len, fg, bg);
	sbt = SDL_CreateTextureFromSurface(r, sbs);
	sbr.w = sbt->w;
	SDL_RenderTexture(r, sbt, NULL, &sbr);

	switch (input_mode) {
	case IM_NORMAL:
		if (feedback != NULL) {
			cmdl_len += string_cat(cmdl,
			                       CMDL_SIZE,
			                       cmdl_len,
			                       feedback);
			break;
		}

		cmdl_len += write_tool_hint(&cmdl[cmdl_len],
		                            CMDL_SIZE - cmdl_len,
		                            tool_opts);
		break;

	case IM_COMMAND:
		cmdl[0] = CMDLINE_INDICATOR;
		cmdl_len += 1;
		cmdl[1] = '\0';

		cmdl_len += string_cat(cmdl,
		                       CMDL_SIZE,
		                       cmdl_len,
		                       cmdline);

		cmdl[cmdl_len] = CMDLINE_CURSOR;
		cmdl_len += 1;
		cmdl[cmdl_len] = '\0';
		break;
	}
	cmdls = TTF_RenderText_LCD(font, cmdl, cmdl_len, fg, bg);
	cmdlt = SDL_CreateTextureFromSurface(r, cmdls);
	cmdlr.w = cmdlt->w;
	if (cmdlr.w > win_w) {
		cmdlr.x = win_w - cmdlr.w;
	}
	SDL_RenderTexture(r, cmdlt, NULL, &cmdlr);

	SDL_RenderPresent(r);

	SDL_DestroySurface(cmdls);
	SDL_DestroyTexture(cmdlt);
	SDL_DestroySurface(sbs);
	SDL_DestroyTexture(sbt);
}

#else

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
     const char                  *world_name)
{
	char   buf[BUF_SIZE];
	size_t buf_len = 0;
	size_t display_len = 0;
	size_t feedback_len;
	size_t i;
	size_t space_len = 0;
	size_t st_bar_len = 0;

	display[0] = '\0';

	if (th_vision) {
		buf[0] = '\0';
		buf_len = 0;
		buf_len += CSI_color_to_string(THERMAL_VISION_R,
		                               THERMAL_VISION_G,
		                               THERMAL_VISION_B,
		                               false,
		                               buf,
		                               BUF_SIZE);
		fwrite(buf, 1, buf_len, stdout);
	}

	display_len += render_world(&display[display_len],
	                            display_size - display_len,
	                            dot_depth,
	                            no_color,
	                            no_glowcolor,
	                            th_vision,
	                            tool_opts,
	                            world,
	                            world_draw,
	                            world_draw_space_w,
	                            world_draw_space_h);

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
		display_len += write_statusbar_elem(&display[display_len],
		                                    display_size - display_len,
		                                    ip_address,
		                                    paused,
		                                    statusbar_elem[i],
		                                    th_vision,
		                                    tickrate,
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
			feedback_len = strlen(feedback);
			if (feedback_len > (size_t) win_w) {
				feedback_len -= feedback_len - win_w;
			}
			display_len += string_cat(display,
			                          /* hack: */
			                          display_len + feedback_len + 1,
			                          display_len,
			                          feedback);
			space_len = win_w - feedback_len;
			break;
		}

		buf[0] = '\0';
		buf_len = 0;
		buf_len = write_tool_hint(buf, BUF_SIZE, tool_opts);

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
#endif /* SDL_BACKEND */

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

	switch (world.state[x][y]) {
	case MS_STATIC:
	case MS_GRAIN:
		b.a = 255;
		break;

	case MS_LIQUID:
		b.a = 255 - APLHA_LOSS_PER_STATE;
		break;

	case MS_GAS:
		b.a = 255 - (APLHA_LOSS_PER_STATE * 2);
		break;

	case MS_COUNT:
		break;
	}

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
handle_advanced_command(const char            *cmd,
                        const char            *arg,
#ifdef SDL_BACKEND
                        TTF_Font              *font,
                        SDL_Renderer          *renderer,
                        SDL_FRect             *world_dst,
                        const size_t           world_scale,
                        SDL_Texture          **world_tx,
#else
                        const int              win_h,
                        struct Rect           *world_draw,
                        int                   *world_draw_space_w,
                        int                   *world_draw_space_h,
#endif
                        const char            *cwd,
                        char                 **feedback,
                        clock_t               *feedback_expiration,
                        float                 *framerate,
                        const char            *ip_address,
                        const clock_t          now,
                        size_t                *statusbar_elems,
                        enum StatusbarElement *statusbar_elem,
                        float                 *tickrate,
                        struct ToolOptions    *tool_opts,
                        const int              win_w,
                        struct World          *world,
                        char                  *world_name)
{
	float         f = 0.0;
	FILE         *file;
	long          l;
	struct World  tempworld;
	int           x, y;

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
	} else if (strcmp(cmd, CMD_FRAMERATE) == 0 ||
	           strcmp(cmd, CMD_FRAMERATE_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
			return;
		}

		if (f <= 0.0) {
			set_feedback(feedback, feedback_expiration, now,
			             "No.");
			return;
		}

		*framerate = f;
	} else if (strcmp(cmd, CMD_LOAD) == 0 ||
	           strcmp(cmd, CMD_LOAD_SHORT) == 0) {
		if (strlen(arg) > WORLDNAME_MAXLEN) {
			set_feedback(feedback, feedback_expiration, now,
			             "World name too long.");
			return;
		}

		command_load_core(cwd, &tempworld, arg);
		if (0 == tempworld.w ||
		    0 == tempworld.h) {
			set_feedback(feedback, feedback_expiration, now,
			             "World could not be loaded.");
			return;
		}
		world_free(world);
		*world = tempworld;

		string_copy(world_name, WORLDNAME_SIZE, arg);

		handle_statusbar_resize(
#ifdef SDL_BACKEND
		                        font,
#endif
		                        ip_address,
		                        statusbar_elems,
		                        statusbar_elem,
		                        win_w,
		                        world_name);

		handle_world_resize(
#ifdef SDL_BACKEND
		                    renderer,
		                    world_dst,
		                    world_scale,
		                    world_tx,
#else
		                    win_w,
		                    win_h,
		                    world_draw,
		                    world_draw_space_w,
		                    world_draw_space_h,
#endif
		                    tool_opts,
		                    *world);
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
	} else if (strcmp(cmd, CMD_SAVE) == 0 ||
	           strcmp(cmd, CMD_SAVE_SHORT) == 0) {
		if (strlen(arg) > WORLDNAME_MAXLEN) {
			set_feedback(feedback, feedback_expiration, now,
			             "World name too long.");
			return;
		}

		command_save_core(cwd, *world, arg);
		set_feedback(feedback, feedback_expiration, now, "World saved");

		string_copy(world_name, WORLDNAME_SIZE, arg);
		handle_statusbar_resize(
#ifdef SDL_BACKEND
		                        font,
#endif
		                        ip_address,
		                        statusbar_elems,
		                        statusbar_elem,
		                        win_w,
		                        world_name);
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
			command_temperature(f + CELSIUS_TO_KELVIN, world);
		}
	} else if (strcmp(cmd, CMD_TEMPERATUREK) == 0 ||
	           strcmp(cmd, CMD_TEMPERATUREK_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
		} else {
			command_temperature(f, world);
		}
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
	} else if (strcmp(cmd, CMD_THERMORATE) == 0 ||
	           strcmp(cmd, CMD_THERMORATE_SHORT) == 0) {
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

		tool_opts->thermo_rate = f;
	} else if (strcmp(cmd, CMD_TICKRATE) == 0 ||
	           strcmp(cmd, CMD_TICKRATE_SHORT) == 0) {
		errno = 0;
		f = strtof(arg, NULL);

		if (errno != 0) {
			set_feedback(feedback, feedback_expiration, now,
			             "Number is invalid.");
			return;
		}

		if (f <= 0.0) {
			set_feedback(feedback, feedback_expiration, now,
			             "No.");
			return;
		}

		*tickrate = f;
	} else {
		set_feedback(feedback, feedback_expiration, now,
		             "Command not recognized.");
	}
}

bool
handle_args(int                  argc,
            char               **argv,
#ifdef SDL_BACKEND
            char               **font_path,
            size_t              *font_size,
            size_t              *world_scale,
#else
            bool                *no_color,
#endif
            bool                *autosave_all,
            bool                *autosave_none,
            float               *framerate,
            bool                *no_glowcolor,
            float               *tickrate,
            struct ToolOptions  *tool_opts)
{
	float f;
	int   i;
	char  key_pause[8] = "Space";

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], FLAG_ABOUT_SHORT) == 0 ||
		    strcmp(argv[i], FLAG_ABOUT) == 0) {
			printf(APP_ABOUT);
			return false;
		} else if (strcmp(argv[i], FLAG_AUTOSAVE_ALL) == 0 ||
		           strcmp(argv[i], FLAG_AUTOSAVE_ALL_SHORT) == 0) {
			*autosave_all = true;
		} else if (strcmp(argv[i], FLAG_AUTOSAVE_NONE) == 0 ||
		           strcmp(argv[i], FLAG_AUTOSAVE_NONE_SHORT) == 0) {
			*autosave_none = true;
		} else if (strcmp(argv[i], FLAG_BRUSHRADIUS) == 0 ||
		           strcmp(argv[i], FLAG_BRUSHRADIUS_SHORT) == 0) {
			if (!handle_flag_number_arg(argc, argv,
			                            &i,
			                            &f,
			                            NR_NOT_NEGATIVE)) {
				return false;
			}
			tool_opts->brush_radius = f;
		} else if (strcmp(argv[i], FLAG_ERASERRADIUS) == 0 ||
		           strcmp(argv[i], FLAG_ERASERRADIUS_SHORT) == 0) {
			if (!handle_flag_number_arg(argc, argv,
			                            &i,
			                            &f,
			                            NR_NOT_NEGATIVE)) {
				return false;
			}
			tool_opts->eraser_radius = f;
		} else if (strcmp(argv[i], FLAG_FRAMERATE) == 0 ||
		           strcmp(argv[i], FLAG_FRAMERATE_SHORT) == 0) {
			if (!handle_flag_number_arg(argc, argv,
			                            &i,
			                            &f,
			                            NR_POSITIVE)) {
				return false;
			}
			*framerate = f;
		} else if (strcmp(argv[i], FLAG_HELP) == 0 ||
		           strcmp(argv[i], FLAG_HELP_SHORT) == 0) {
			printf(APP_HELP);

			printf(APP_HELP_FLAGS,
			       STD_BRUSH_RADIUS,
			       STD_ERASER_RADIUS,
			       STD_FRAMERATE,
			       CELSIUS_TO_KELVIN,
			       STD_SPAWN_TEMPERATURE,
			       STD_THERMO_RADIUS,
			       STD_THERMO_RATE,
			       STD_TICKRATE);

#ifdef SDL_BACKEND
			printf(APP_HELP_FLAGS_SDL,
			       STD_FONT_SIZE,
			       MAX_FONT_SIZE,
			       STD_WORLD_SCALE);
#else
			printf(APP_HELP_FLAGS_TERMINAL);
#endif

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

			printf(APP_HELP_COMMANDS,
			       WORLDNAME_MAXLEN,
			       WORLDNAME_MAXLEN);

			printf(APP_HELP_MATERIALS);
			for (i = 0; i < MAT_COUNT; i++) {
				printf("    %s\n", MAT_NAME[i]);
			}
			printf("\n");

			return false;
		} else if (strcmp(argv[i], FLAG_NOGLOWCOLOR) == 0 ||
		           strcmp(argv[i], FLAG_NOGLOWCOLOR_SHORT) == 0) {
			*no_glowcolor = true;
		} else if (strcmp(argv[i], FLAG_SPAWNTEMPERATURE) == 0 ||
		           strcmp(argv[i], FLAG_SPAWNTEMPERATURE_SHORT) == 0) {
			if (!handle_flag_number_arg(argc, argv,
			                            &i,
			                            &tool_opts->spawn_temperature,
			                            NR_NOT_NEGATIVE)) {
				return false;
			}
		} else if (strcmp(argv[i], FLAG_THERMORADIUS) == 0 ||
		           strcmp(argv[i], FLAG_THERMORADIUS_SHORT) == 0) {
			if (!handle_flag_number_arg(argc, argv,
			                            &i,
			                            &f,
			                            NR_NOT_NEGATIVE)) {
				return false;
			}
			tool_opts->thermo_radius = f;
		} else if (strcmp(argv[i], FLAG_THERMORATE) == 0 ||
		           strcmp(argv[i], FLAG_THERMORATE_SHORT) == 0) {
			if (!handle_flag_number_arg(argc, argv,
			                            &i,
			                            &tool_opts->thermo_rate,
			                            NR_NOT_NEGATIVE)) {
				return false;
			}
		} else if (strcmp(argv[i], FLAG_TICKRATE) == 0 ||
		           strcmp(argv[i], FLAG_TICKRATE_SHORT) == 0) {
			if (!handle_flag_number_arg(argc, argv,
			                            &i,
			                            tickrate,
			                            NR_POSITIVE)) {
				return false;
			}
		} else if (strcmp(argv[i], FLAG_VERSION_SHORT) == 0 ||
		           strcmp(argv[i], FLAG_VERSION) == 0) {
			printf("%s: version %s\n", APP_NAME, APP_VERSION);
			return false;
#ifdef SDL_BACKEND
		} else if (strcmp(argv[i], FLAG_FONT_PATH_SHORT) == 0 ||
		           strcmp(argv[i], FLAG_FONT_PATH) == 0) {
			if (!check_flag_arg(argc, argv, i)) {
				return false;
			}
			i++;
			*font_path = argv[i];
		} else if (strcmp(argv[i], FLAG_FONT_SIZE_SHORT) == 0 ||
		           strcmp(argv[i], FLAG_FONT_SIZE) == 0) {
			if (!handle_flag_number_arg(argc, argv,
			                            &i,
			                            &f,
			                            NR_POSITIVE)) {
				return false;
			}
			if (f > MAX_FONT_SIZE) {
				printf("The given font size is too high and was capped\n");
				f = MAX_FONT_SIZE;
			}
			*font_size = f;
		} else if (strcmp(argv[i], FLAG_WORLD_SCALE_SHORT) == 0 ||
		           strcmp(argv[i], FLAG_WORLD_SCALE) == 0) {
			if (!handle_flag_number_arg(argc, argv,
			                            &i,
			                            &f,
			                            NR_POSITIVE)) {
				return false;
			}
			*world_scale = f;
#else
		} else if (strcmp(argv[i], FLAG_NOCOLOR) == 0 ||
		           strcmp(argv[i], FLAG_NOCOLOR_SHORT) == 0) {
			*no_color = true;
#endif
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
handle_autosave(const bool          autosave_all,
                const bool          autosave_none,
                const char         *cwd,
                clock_t            *last_autosave,
                const clock_t       now,
                const struct World  world,
                const char         *world_name)
{
	if (!autosave_none &&
	    (autosave_all ||
	     strcmp(world_name, WORLDNAME_NEW) == 0)) {
		*last_autosave = now;
		command_save_core(cwd, world, world_name);
	}
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
handle_command(char                  *cmdline,
               const size_t           cmdline_len,
#ifdef SDL_BACKEND
               TTF_Font              *font,
               SDL_Renderer          *renderer,
               SDL_FRect             *world_dst,
               const size_t           world_scale,
               SDL_Texture          **world_tx,
#else
               const int              win_h,
               struct Rect           *world_draw,
               int                   *world_draw_space_w,
               int                   *world_draw_space_h,
#endif
               bool                  *active,
               const char            *cwd,
               char                 **feedback,
               clock_t               *feedback_expiration,
               float                 *framerate,
               const char            *ip_address,
               bool                  *no_glowcolor,
               const clock_t          now,
               bool                  *paused,
               size_t                *statusbar_elems,
               enum StatusbarElement *statusbar_elem,
               bool                  *th_vision,
               float                 *tickrate,
               struct ToolOptions    *tool_opts,
               const int              win_w,
               struct World          *world,
               char                  *world_name)
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
#ifdef SDL_BACKEND
			                        font,
			                        renderer,
			                        world_dst,
			                        world_scale,
			                        world_tx,
#else
			                        win_h,
			                        world_draw,
			                        world_draw_space_w,
			                        world_draw_space_h,
#endif
			                        cwd,
			                        feedback,
			                        feedback_expiration,
			                        framerate,
			                        ip_address,
			                        now,
			                        statusbar_elems,
			                        statusbar_elem,
			                        tickrate,
			                        tool_opts,
			                        win_w,
			                        world,
			                        world_name);
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
	                      cwd,
	                      feedback,
	                      feedback_expiration,
	                      framerate,
	                      no_glowcolor,
	                      now,
	                      paused,
	                      th_vision,
	                      tickrate,
	                      tool_opts,
	                      world,
	                      world_name);
}

#ifdef SDL_BACKEND
#else
void
handle_command_input(const char            *in,
                     bool                  *active,
                     char                  *cmdline,
                     size_t                *cmdline_len,
                     size_t                *cmdline_shift,
                     const char            *cwd,
                     char                 **feedback,
                     clock_t               *feedback_expiration,
                     float                 *framerate,
                     enum InputMode        *input_mode,
                     const char            *ip_address,
                     bool                  *no_glowcolor,
                     clock_t                now,
                     bool                  *paused,
                     size_t                *statusbar_elems,
                     enum StatusbarElement *statusbar_elem,
                     bool                  *th_vision,
                     float                 *tickrate,
                     struct ToolOptions    *tool_opts,
                     const int              win_w,
                     const int              win_h,
                     struct World          *world,
                     struct Rect           *world_draw,
                     int                   *world_draw_space_w,
                     int                   *world_draw_space_h,
                     char                  *world_name)
{
	switch (in[0]) {
	case '\b':
	case CHAR_DELETE:
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
		               win_h,
		               world_draw,
		               world_draw_space_w,
		               world_draw_space_h,
		               active,
		               cwd,
		               feedback,
		               feedback_expiration,
		               framerate,
		               ip_address,
		               no_glowcolor,
		               now,
		               paused,
		               statusbar_elems,
		               statusbar_elem,
		               th_vision,
		               tickrate,
		               tool_opts,
		               win_w,
		               world,
		               world_name);
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
#endif /* SDL_BACKEND */

bool
handle_flag_number_arg(int                            argc,
                       char                         **argv,
                       int                           *idx,
                       float                         *out,
                       const enum NumberRequirement   requirement)
{
	float val;

	if (!check_flag_arg(argc, argv, *idx)) {
		return false;
	}
	*idx += 1;

	errno = 0;
	val = strtof(argv[*idx], NULL);
	if (errno != 0) {
		fprintf(stderr,
		        "The value for \"%s\" is malformed\n",
		        argv[*idx - 1]);
		return false;
	}

	switch (requirement) {
	case NR_NONE:
		break;

	case NR_NOT_NEGATIVE:
		if (val < 0.0) {
			fprintf(stderr,
			        "The value for \"%s\" must not be negative\n",
			        argv[*idx - 1]);
				return false;
		}
		break;

	case NR_POSITIVE:
		if (val <= 0.0) {
			fprintf(stderr,
			        "The value for \"%s\" must be positive\n",
			        argv[*idx - 1]);
				return false;
		}
		break;
	}

	*out = val;
	return true;
}

void
handle_input(
#ifdef SDL_BACKEND
             TTF_Font              *font,
             const size_t           font_size,
             SDL_Renderer          *renderer,
             SDL_Window            *win,
             int                   *win_w,
             int                   *win_h,
             size_t                *world_area_w,
             size_t                *world_area_h,
             SDL_FRect             *world_dst,
             const size_t           world_scale,
             SDL_Texture          **world_tx,
#else
             size_t                *cmdline_shift,
             const size_t           display_size,
             const size_t           dot_depth,
             bool                  *lmb_pressed,
             const bool             no_color,
             const int              win_w,
             const int              win_h,
             struct Rect           *world_draw,
             int                   *world_draw_space_w,
             int                   *world_draw_space_h,
#endif /* SDL_BACKEND */
             bool                  *active,
             char                  *cmdline,
             size_t                *cmdline_len,
             const char            *cwd,
             const float            delta,
             int                   *drag_start_x,
             int                   *drag_start_y,
             char                 **feedback,
             clock_t               *feedback_expiration,
             float                 *framerate,
             enum InputMode        *input_mode,
             const char            *ip_address,
             clock_t               *last_key_use,
             bool                  *no_glowcolor,
             const clock_t          now,
             bool                  *paused,
             size_t                *statusbar_elems,
             enum StatusbarElement *statusbar_elem,
             float                 *tickrate,
             bool                  *th_vision,
             struct ToolOptions    *tool_opts,
             struct World          *world,
             char                  *world_name)
{
#ifdef SDL_BACKEND
	SDL_Event e;
	int mx, my;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (SDL_BUTTON_RIGHT == e.button.button) {
				*drag_start_x = e.button.x - world_dst->x;
				*drag_start_y = e.button.y - world_dst->y;
			}
			break;

		case SDL_EVENT_MOUSE_MOTION:
			mx = e.motion.x;
			my = e.motion.y;

			if (mx < 0.0) {
				mx  = 0.0;
			}
			else if (mx > *win_w) {
				mx  = *win_w;
			}
			if (my < 0.0) {
				my  = 0.0;
			}
			else if (my > *win_h) {
				my  = *win_h;
			}

			tool_opts->x = (mx - world_dst->x) / world_scale;
			tool_opts->y = (my - world_dst->y) / world_scale;
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			tool_radius_add(e.wheel.y, tool_opts);
			break;

		case SDL_EVENT_KEY_DOWN:
			switch (e.key.key) {
			case SDLK_F5:
				command_screenshot(renderer,
				                   *world_area_w,
				                   *world_area_h,
				                   cwd,
				                   feedback,
				                   feedback_expiration,
				                   now,
				                   *th_vision);
				break;

			case SDLK_F6:
				command_save_core(cwd, *world, world_name);
				set_feedback(feedback, feedback_expiration, now,
				             "World saved");
				break;

			case SDLK_F7:
				quickload(cwd,
				          feedback,
				          feedback_expiration,
				          now,
				          world,
				          world_name);
				break;

			case SDLK_BACKSPACE:
				if (*cmdline_len > 0) {
					cmdline[*cmdline_len - 1] = '\0';
					*cmdline_len -= 1;
				}
				break;

			case SDLK_RETURN:
				handle_command(cmdline,
				               *cmdline_len,
				               font,
				               renderer,
				               world_dst,
				               world_scale,
				               world_tx,
				               active,
				               cwd,
				               feedback,
				               feedback_expiration,
				               framerate,
				               ip_address,
				               no_glowcolor,
				               now,
				               paused,
				               statusbar_elems,
				               statusbar_elem,
				               th_vision,
				               tickrate,
				               tool_opts,
				               *win_w,
				               world,
				               world_name);
				cmdline[0] = '\0';
				*cmdline_len = 0;
				*input_mode = IM_NORMAL;
				break;
			}
			break;

		case SDL_EVENT_TEXT_INPUT:
			switch (*input_mode) {
			case IM_COMMAND:
				if (*cmdline_len < CMDLINE_SIZE - 1) {
					cmdline[*cmdline_len] = e.text.text[0];
					cmdline[*cmdline_len + 1] = '\0';
					*cmdline_len += 1;
				}
				break;

			case IM_NORMAL:
				handle_normal_input(e.text.text,
				                    *win_w,
				                    *win_h,
				                    *world_area_w,
				                    *world_area_h,
				                    world_dst,
				                    world_scale,
				                    active,
				                    delta,
				                    input_mode,
				                    last_key_use,
				                    now,
				                    paused,
				                    tickrate,
				                    th_vision,
				                    tool_opts,
				                    world);
				break;
			}
			break;

		case SDL_EVENT_QUIT:
			*active = false;
			break;

		case SDL_EVENT_WINDOW_RESIZED:
			handle_resize(font_size,
			              win,
			              win_w,
			              win_h,
			              world_area_w,
			              world_area_h,
			              world_dst);
			handle_statusbar_resize(font,
			                        ip_address,
			                        statusbar_elems,
			                        statusbar_elem,
			                        *win_w,
			                        world_name);
			break;
		}
	}

	handle_mouse_state(delta,
	                   drag_start_x,
	                   drag_start_y,
	                   tool_opts,
	                   win,
	                   world,
	                   *world_area_w,
	                   *world_area_h,
	                   world_dst);

#else /* SDL_BACKEND */

	ssize_t input_len = 0;
	char    input[INPUT_SIZE];

	input_len = read(STDIN_FILENO, &input, INPUT_SIZE);

	switch (*input_mode) {
	case IM_NORMAL:
		if (input_len > 0 &&
		    input_len < INPUT_SIZE) {
			input[input_len] = '\0';
			if (!handle_normal_input(input,
			                         world_draw,
			                         active,
			                         delta,
			                         input_mode,
			                         last_key_use,
			                         now,
			                         paused,
			                         tickrate,
			                         th_vision,
			                         tool_opts,
			                         world)) {
				handle_normal_csi_input(input,
				                        cwd,
				                        delta,
				                        display_size,
				                        dot_depth,
				                        drag_start_x,
				                        drag_start_y,
				                        feedback,
				                        feedback_expiration,
				                        lmb_pressed,
				                        no_color,
				                        no_glowcolor,
				                        now,
				                        *th_vision,
				                        tool_opts,
				                        world,
				                        world_draw,
				                        world_name);
			}
		}
		break;

	case IM_COMMAND:
		if (input_len == 1) {
			handle_command_input(input,
			                     active,
			                     cmdline,
			                     cmdline_len,
			                     cmdline_shift,
			                     cwd,
			                     feedback,
			                     feedback_expiration,
			                     framerate,
			                     input_mode,
			                     ip_address,
			                     no_glowcolor,
			                     now,
			                     paused,
			                     statusbar_elems,
			                     statusbar_elem,
			                     th_vision,
			                     tickrate,
			                     tool_opts,
			                     win_w,
			                     win_h,
			                     world,
			                     world_draw,
			                     world_draw_space_w,
			                     world_draw_space_h,
			                     world_name);
		}
		break;
	}
#endif /* SDL_BACKEND */
}

#ifdef SDL_BACKEND
void
handle_mouse_state(const float           delta,
                   int                  *drag_start_x,
                   int                  *drag_start_y,
                   struct ToolOptions   *tool_opts,
                   SDL_Window           *win,
                   struct World         *world,
                   const size_t          world_area_w,
                   const size_t          world_area_h,
                   SDL_FRect            *world_dst)
{
	SDL_MouseButtonFlags mbf;
	int win_w, win_h;
	int dx = tool_opts->x;
	int dy = tool_opts->y;
	float x;
	float y;

	mbf = SDL_GetMouseState(&x, &y);
	SDL_GetWindowSize(win, &win_w, &win_h);

	switch (mbf) {
	case SDL_BUTTON_LMASK:
		use_tool(delta, *tool_opts, world);
		break;

	case SDL_BUTTON_MMASK:
		if (dx >= world->w ||
		    dy >= world->h) {
			break;
		}

		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_mat = world->dot[dx][dy];
			break;

		case TOOL_SPAWNER:
			tool_opts->spawner_mat = world->dot[dx][dy];
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case SDL_BUTTON_RMASK:
		world_dst->x = x - *drag_start_x;
		world_dst->y = y - *drag_start_y;

		handle_world_dst_clamp_x(win_w, world_area_w, world_dst);
		handle_world_dst_clamp_y(win_h, world_area_h, world_dst);
		break;

	case SDL_BUTTON_X1MASK:
	case SDL_BUTTON_X2MASK:
		break;

	default:
		break;
	}
}

#else

void
handle_mouse_input(const char         *in,
                   const float         delta,
                   int                *drag_start_x,
                   int                *drag_start_y,
                   bool               *lmb_pressed,
                   struct ToolOptions *tool_opts,
                   struct World       *world,
                   struct Rect        *world_draw)
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
		tool_opts->x = x + world_draw->x;
		tool_opts->y = y + world_draw->y;
		use_tool(delta, *tool_opts, world);

		if ('M' == pressed) {
			*lmb_pressed = true;
		} else {
			*lmb_pressed = false;
		}
		break;

	case CSI_MB_HOVER:
		tool_opts->x = x + world_draw->x;
		tool_opts->y = y + world_draw->y;
		*lmb_pressed = false;
		break;

	case CSI_MB_MIDDLE:
	case CSI_MB_MIDDLE_DRAG:
		if (x >= (unsigned int) world->w ||
		    y >= (unsigned int) world->h) {
			break;
		}

		switch (tool_opts->sel_tool) {
		case TOOL_BRUSH:
			tool_opts->brush_mat = world->dot[x][y];
			break;

		case TOOL_SPAWNER:
			tool_opts->spawner_mat = world->dot[x][y];
			break;

		case TOOL_ERASER:
		case TOOL_HEATER:
		case TOOL_COOLER:
		case TOOL_COUNT:
			break;
		}
		break;

	case CSI_MB_RIGHT:
		*drag_start_x = x + world_draw->x;
		*drag_start_y = y + world_draw->y;
		break;

	case CSI_MB_RIGHT_DRAG:
		world_draw->x = *drag_start_x - x;
		world_draw->y = *drag_start_y - y;

		if (world_draw->x < 0) {
			world_draw->x = 0;
		}
		if (world_draw->y < 0) {
			world_draw->y = 0;
		}
		if (world_draw->x > world->w - world_draw->w) {
			world_draw->x = world->w - world_draw->w;
		}
		if (world_draw->y > world->h - world_draw->h) {
			world_draw->y = world->h - world_draw->h;
		}
		break;

	case CSI_MB_WHEELUP:
		tool_radius_add(1, tool_opts);
		break;

	case CSI_MB_WHEELDOWN:
		tool_radius_add(-1, tool_opts);
		break;
	}
}
#endif /* SDL_BACKEND */

#ifdef SDL_BACKEND
#else
void
handle_normal_csi_input(const char         *in,
                        const char         *cwd,
                        const float         delta,
                        const size_t        display_size,
                        const size_t        dot_depth,
                        int                *drag_start_x,
                        int                *drag_start_y,
                        char              **feedback,
                        clock_t            *feedback_expiration,
                        bool               *lmb_pressed,
                        const bool          no_color,
                        const bool          no_glowcolor,
                        const clock_t       now,
                        const bool          th_vision,
                        struct ToolOptions *tool_opts,
                        struct World       *world,
                        struct Rect        *world_draw,
                        const char         *world_name)
{
	if (strcmp(in, CSI_KEY_LEFT) == 0) {
		if (tool_opts->x > 0) {
			tool_opts->x -= 1;
			if (tool_opts->x < world_draw->x) {
				world_draw->x -= 1;
			}
		}
	} else if (strcmp(in, CSI_KEY_DOWN) == 0) {
		if (tool_opts->y < world->h - 1) {
			tool_opts->y += 1;
			if (tool_opts->y >= world_draw->y + world_draw->h) {
				world_draw->y += 1;
			}
		}
	} else if (strcmp(in, CSI_KEY_UP) == 0) {
		if (tool_opts->y > 0) {
			tool_opts->y -= 1;
			if (tool_opts->y < world_draw->y) {
				world_draw->y -= 1;
			}
		}
	} else if (strcmp(in, CSI_KEY_RIGHT) == 0) {
		if (tool_opts->x < world->w - 1) {
			tool_opts->x += 1;
			if (tool_opts->x >= world_draw->x + world_draw->w) {
				world_draw->x += 1;
			}
		}
	} else if (strcmp(in, CSI_KEY_F5) == 0) {
		command_screenshot(display_size,
		                   dot_depth,
		                   no_color,
		                   no_glowcolor,
		                   *tool_opts,
		                   *world,
		                   *world_draw,
		                   cwd,
		                   feedback,
		                   feedback_expiration,
		                   now,
		                   th_vision);
	} else if (strcmp(in, CSI_KEY_F6) == 0) {
		command_save_core(cwd, *world, world_name);
		set_feedback(feedback, feedback_expiration, now, "World saved");
	} else if (strcmp(in, CSI_KEY_F7) == 0) {
		quickload(cwd,
		          feedback,
		          feedback_expiration,
		          now,
		          world,
		          world_name);
	} else if (strcmp(in, CSI_KEY_HOME) == 0) {
		tool_opts->x = 0;
		world_draw->x = 0;
	} else if (strcmp(in, CSI_KEY_END) == 0) {
		tool_opts->x = world->w - 1;
		world_draw->x = world->w - world_draw->w;
	} else if (strcmp(in, CSI_KEY_PGUP) == 0) {
		tool_opts->y = 0;
		world_draw->y = 0;
	} else if (strcmp(in, CSI_KEY_PGDOWN) == 0) {
		tool_opts->y = world->h - 1;
		world_draw->y = world->h - world_draw->h;
	} else if (strcmp(in, CSI_KEY_CTRLHOME) == 0) {
		tool_opts->x = 0;
		tool_opts->y = 0;
		world_draw->x = 0;
		world_draw->y = 0;
	} else if (strcmp(in, CSI_KEY_CTRLEND) == 0) {
		tool_opts->x = world->w - 1;
		tool_opts->y = world->h - 1;
		world_draw->x = world->w - world_draw->w;
		world_draw->y = world->h - world_draw->h;
	} else if (in[1] == '[' &&
	           in[2] == '<') {
		handle_mouse_input(in,
		                   delta,
		                   drag_start_x,
		                   drag_start_y,
		                   lmb_pressed,
		                   tool_opts,
		                   world,
		                   world_draw);
	}
}
#endif /* SDL_BACKEND */

bool
handle_normal_input(const char         *in,
#ifdef SDL_BACKEND
                    const int           win_w,
                    const int           win_h,
                    const size_t        world_area_w,
                    const size_t        world_area_h,
                    SDL_FRect          *world_dst,
                    const size_t        world_scale,
#else
                    struct Rect        *world_draw,
#endif
                    bool               *active,
                    const float         delta,
                    enum InputMode     *input_mode,
                    clock_t            *last_key_use,
                    const clock_t       now,
                    bool               *paused,
                    float              *tickrate,
                    bool               *th_vision,
                    struct ToolOptions *tool_opts,
                    struct World       *world)
{
	float use_tool_delta;

	switch (in[0]) {
	case KEY_QUIT:
		*active = false;
		break;

	case KEY_USE:
		use_tool_delta = (float) (now - *last_key_use) /
		                 (float) CLOCKS_PER_SEC;
		if (use_tool_delta > KEY_USE_THERMO_CONTINUE_LIMIT) {
			use_tool_delta = delta;
		}
		use_tool(use_tool_delta, *tool_opts, world);
		*last_key_use = now;
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
		if (tool_opts->x > 0) {
			tool_opts->x -= 1;
#ifdef SDL_BACKEND
			if (tool_opts->x * -1 > world_dst->x / world_scale) {
				world_dst->x += world_scale;
			}
			handle_world_dst_clamp_x(win_w, world_area_w, world_dst);
#else
			if (tool_opts->x < world_draw->x) {
				world_draw->x -= 1;
			}
#endif
		}
		break;

	case KEY_LEFT_MAX:
		tool_opts->x = 0;

#ifdef SDL_BACKEND
		world_dst->x = 0;
#else
		world_draw->x = 0;
#endif
		break;

	case KEY_DOWN:
		if (tool_opts->y < world->h - 1) {
			tool_opts->y += 1;
#ifdef SDL_BACKEND
			if (tool_opts->y >= (int) ((world_area_h - world_dst->y) / world_scale)) {
				world_dst->y -= world_scale;
			}
			handle_world_dst_clamp_y(win_h, world_area_h, world_dst);
#else
			if (tool_opts->y >= world_draw->y + world_draw->h) {
				world_draw->y += 1;
			}
#endif
		}
		break;

	case KEY_DOWN_MAX:
		tool_opts->y = world->h - 1;

#ifdef SDL_BACKEND
		world_dst->y = world_area_h - world_dst->h;
		handle_world_dst_clamp_y(win_h, world_area_h, world_dst);
#else
		world_draw->y = world->h - world_draw->h;
#endif
		break;

	case KEY_UP:
		if (tool_opts->y > 0) {
			tool_opts->y -= 1;
#ifdef SDL_BACKEND
			if (tool_opts->y * -1 > world_dst->y / world_scale) {
				world_dst->y += world_scale;
			}
			handle_world_dst_clamp_y(win_h, world_area_h, world_dst);
#else
			if (tool_opts->y < world_draw->y) {
				world_draw->y -= 1;
			}
#endif
		}
		break;

	case KEY_UP_MAX:
		tool_opts->y = 0;

#ifdef SDL_BACKEND
		world_dst->y = 0;
#else
		world_draw->y = 0;
#endif
		break;

	case KEY_RIGHT:
		if (tool_opts->x < world->w - 1) {
			tool_opts->x += 1;
#ifdef SDL_BACKEND
			if (tool_opts->x >= (int) ((world_area_w - world_dst->x) / world_scale)) {
				world_dst->x -= world_scale;
			}
			handle_world_dst_clamp_x(win_w, world_area_w, world_dst);
#else
			if (tool_opts->x >= world_draw->x + world_draw->w) {
				world_draw->x += 1;
			}
#endif
		}
		break;

	case KEY_RIGHT_MAX:
		tool_opts->x = world->w - 1;

#ifdef SDL_BACKEND
		world_dst->x = world_area_w - world_dst->w;
		handle_world_dst_clamp_x(win_w, world_area_w, world_dst);
#else
		world_draw->x = world->w - world_draw->w;
#endif
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
		if (*tickrate > MIN_TICKRATE) {
			*tickrate /= 2;
		}
		break;

	case KEY_SIMSPEED_MIN:
		*tickrate = MIN_TICKRATE;
		break;

	case '+':
	case KEY_SIMSPEED_UP:
		if (*tickrate < MAX_TICKRATE) {
			*tickrate *= 2;
		}
		break;

	case KEY_SIMSPEED_MAX:
		*tickrate = MAX_TICKRATE;
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

	default:
		return false;
		break;
	}

	return true;
}

#ifdef SDL_BACKEND
void
handle_resize(const size_t        font_size,
              SDL_Window         *win,
              int                *win_w,
              int                *win_h,
              size_t             *world_area_w,
              size_t             *world_area_h,
              SDL_FRect          *world_dst)
{
	SDL_GetWindowSize(win, win_w, win_h);
	*world_area_w = *win_w;
	*world_area_h = (*win_h - (font_size * 2));
	world_dst->x = 0;
	world_dst->y = 0;
}

#else

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
              struct ToolOptions     *tool_opts,
              int                    *win_w,
              int                    *win_h,
              const struct World      world,
              struct Rect            *world_draw,
              int                    *world_draw_space_w,
              int                    *world_draw_space_h,
              const char             *world_name)
{
	size_t             a, b;
	char               buf[BUF_SIZE];
	size_t             buf_len;
	size_t             new_display_size;
	size_t             statusbar_len = 0;
	size_t             statusbar_max_elems = 0;
	struct winsize     ws;

	ws = CSI_get_size();
	if (*win_w != ws.ws_col ||
	    *win_h != ws.ws_row) {
		*win_w = ws.ws_col;
		*win_h = ws.ws_row;

		handle_world_resize(*win_w,
		                    *win_h,
		                    world_draw,
		                    world_draw_space_w,
		                    world_draw_space_h,
		                    tool_opts,
		                    world);

		new_display_size = (size_t) ((float) *win_w *
		                             (float) *win_h *
		                             (float) DISPLAY_SIZE_MODIFIER) *
		                   dot_depth;
		if (new_display_size > *display_size) {
			*display_size = new_display_size;
			*display = realloc(*display, *display_size);
		}

		handle_statusbar_resize(ip_address,
		                        statusbar_elems,
		                        statusbar_elem,
		                        *win_w,
		                        world_name);

		if (input_mode == IM_COMMAND) {
			handle_cmdline_shift(cmdline_len,
			                     cmdline_shift,
			                     *win_w);
		}
	}
}
#endif /* SDL_BACKEND */

void
handle_simple_command(const char          *cmdline,
                      bool                *active,
                      const char          *cwd,
                      char               **feedback,
                      clock_t             *feedback_expiration,
                      float               *framerate,
                      bool                *no_glowcolor,
                      const clock_t        now,
                      bool                *paused,
                      bool                *th_vision,
                      float               *tickrate,
                      struct ToolOptions  *tool_opts,
                      struct World        *world,
                      const char          *world_name)
{
	int           x, y;

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
		world_clear(world);
	} else if (strcmp(cmdline, CMD_COOLER) == 0 ||
	           strcmp(cmdline, CMD_COOLER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_COOLER;
	} else if (strcmp(cmdline, CMD_DEFAULTS) == 0 ||
	           strcmp(cmdline, CMD_DEFAULTS_SHORT) == 0) {
		*framerate = STD_FRAMERATE;
		*tickrate = STD_TICKRATE;
		tool_opts->brush_radius = STD_BRUSH_RADIUS;
		tool_opts->eraser_radius = STD_ERASER_RADIUS;
		tool_opts->sel_tool = STD_SELECTED_TOOL;
		tool_opts->thermo_radius = STD_THERMO_RADIUS;
		tool_opts->thermo_rate = STD_THERMO_RATE;
		tool_opts->spawn_temperature = STD_SPAWN_TEMPERATURE;
	} else if (strcmp(cmdline, CMD_ERASER) == 0 ||
	           strcmp(cmdline, CMD_ERASER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_ERASER;
	} else if (strcmp(cmdline, CMD_GLOWCOLOR) == 0 ||
	           strcmp(cmdline, CMD_GLOWCOLOR_SHORT) == 0) {
		*no_glowcolor = false;
	} else if (strcmp(cmdline, CMD_HEATER) == 0 ||
	           strcmp(cmdline, CMD_HEATER_SHORT) == 0) {
		tool_opts->sel_tool = TOOL_HEATER;
	} else if (strcmp(cmdline, CMD_LOAD) == 0 ||
	           strcmp(cmdline, CMD_LOAD_SHORT) == 0) {
		quickload(cwd,
		          feedback,
		          feedback_expiration,
		          now,
		          world,
		          world_name);
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
	} else if (strcmp(cmdline, CMD_SAVE) == 0 ||
	           strcmp(cmdline, CMD_SAVE_SHORT) == 0) {
		command_save_core(cwd, *world, world_name);
		set_feedback(feedback, feedback_expiration, now, "World saved");
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

void
handle_statusbar_resize(
#ifdef SDL_BACKEND
                        TTF_Font              *font,
#endif
                        const char            *ip_address,
                        size_t                *statusbar_elems,
                        enum StatusbarElement *statusbar_elem,
                        const size_t           win_w,
                        const char            *world_name)
{
	size_t             a, b;
	char               buf[BUF_SIZE];
	size_t             buf_len = 0;
	struct ToolOptions maxcoords_to = {
		.x = 999,
		.y = 999,
	};
	size_t             sb_max_elems = 0;
	size_t             sb_w = 0;

#ifdef SDL_BACKEND
	SDL_Surface *text = NULL;
#endif

	buf[0] = '\0';

	for (a = 0; a < ARRLEN(STATUSBAR_DISPLAY_PRIORITY); a++) {
		/* Here it is important to render the biggest possible
		 * thing, unless it's not expected to change.
		 * Only in that case use real data.
		 */
		buf_len += write_statusbar_elem(&buf[buf_len],
	                                        BUF_SIZE - buf_len,
	                                        ip_address,
	                                        false,
	                                        STATUSBAR_DISPLAY_PRIORITY[a],
	                                        true,
	                                        120.0,
	                                        maxcoords_to,
	                                        world_name);

#ifdef SDL_BACKEND
		text = TTF_RenderText_LCD(font,
		                          buf,
		                          buf_len,
		                          (SDL_Color) {0},
		                          (SDL_Color) {0});
		sb_w = text->w;
		SDL_DestroySurface(text);
#else
		sb_w = buf_len;
#endif

		if (sb_w > win_w) {
			break;
		}

		buf_len += string_cat(buf, BUF_SIZE, buf_len, STATUSBAR_SEPARATOR);
	}
	sb_max_elems = a;
	*statusbar_elems = 0;

	for (a = 0; a < ARRLEN(STATUSBAR_DISPLAY_PRIORITY); a++) {
		for (b = 0; b < sb_max_elems; b++) {
			if (STATUSBAR_DISPLAY_ORDER[a] == STATUSBAR_DISPLAY_PRIORITY[b]) {
				statusbar_elem[*statusbar_elems] = STATUSBAR_DISPLAY_ORDER[a];
				*statusbar_elems += 1;
			}
		}
	}
}

#ifdef SDL_BACKEND
void
handle_world_dst_clamp_x(const int     win_w,
                         const size_t  world_area_w,
                         SDL_FRect    *world_dst)
{
	if (world_dst->x > 0 ||
	    win_w > world_dst->w) {
		world_dst->x = 0;
	} else if (world_dst->x < world_area_w - world_dst->w) {
		world_dst->x = world_area_w - world_dst->w;
	}
}
#endif /* SDL_BACKEND */

#ifdef SDL_BACKEND
void
handle_world_dst_clamp_y(const int     win_h,
                         const size_t  world_area_h,
                         SDL_FRect    *world_dst)
{
	if (world_dst->y > 0 ||
	    win_h > world_dst->h) {
		world_dst->y = 0;
	} else if (world_dst->y < world_area_h - world_dst->h) {
		world_dst->y = world_area_h - world_dst->h;
	}
}
#endif /* SDL_BACKEND */

void
handle_world_resize(
#ifdef SDL_BACKEND
                    SDL_Renderer        *renderer,
                    SDL_FRect           *world_dst,
                    const size_t         world_scale,
                    SDL_Texture        **world_tx,
#else
                    const int            win_w,
                    const int            win_h,
                    struct Rect         *world_draw,
                    int                 *world_draw_space_w,
                    int                 *world_draw_space_h,
#endif
                    struct ToolOptions  *tool_opts,
                    const struct World   world)
{
	tool_opts->x = 0;
	tool_opts->y = 0;

#ifdef SDL_BACKEND
	world_dst->x = 0;
	world_dst->y = 0;
	world_dst->w = world.w * world_scale;
	world_dst->h = world.h * world_scale;
	SDL_DestroyTexture(*world_tx);
	*world_tx = SDL_CreateTexture(renderer,
	                              SDL_PIXELFORMAT_RGBA8888,
	                              SDL_TEXTUREACCESS_TARGET,
	                              world.w, world.h);
	SDL_SetTextureScaleMode(*world_tx, SDL_SCALEMODE_PIXELART);
#else
	world_draw->x = 0;
	world_draw->y = 0;
	world_draw->w = win_w;
	world_draw->h = win_h - 2;

	if (world.w <= win_w) {
		world_draw->w = world.w;
		*world_draw_space_w = win_w - world.w;
	}

	if (world.h <= win_h - 2) {
		world_draw->h = world.h;
		*world_draw_space_h = win_h - 2 - world.h;
	}
#endif /* SDL_BACKEND */
}

void
quickload(const char     *cwd,
          char          **feedback,
          clock_t        *feedback_expiration,
          const clock_t	  now,
          struct World   *world,
          const char     *world_name)
{
	struct World tempworld;

	command_load_core(cwd, &tempworld, world_name);
	if (0 == tempworld.w ||
	    0 == tempworld.h) {
		set_feedback(feedback, feedback_expiration, now,
		             "World could not be loaded.");
		return;
	}
	world_free(world);
	*world = tempworld;
}

#ifdef SDL_BACKEND
#else
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
#endif /* SDL_BACKEND */

#ifdef SDL_BACKEND
#else
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
#endif /* SDL_BACKEND */

#ifdef SDL_BACKEND
void
render_world(const bool          no_glowcolor,
             SDL_Renderer       *r,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world)
{
	int         alpha;
	struct Rgba dc;
	int         tool_radius = 0;
	SDL_FRect   tool;
	int         x, y;

	if (th_vision) {
		for (x = 0; x < world.w; x++) {
			for (y = 0; y < world.h; y++) {
				/* the following hack is sponsored
				 * by optimization (we avoid an if)
				 */
				alpha = 255 * world.dot[x][y];

				dc = get_thermal_dot_color(world, x, y);
				dc.a = alpha;
				SDL_SetRenderDrawColor(r, dc.r, dc.g, dc.b, dc.a);
				SDL_RenderPoint(r, x, y);
			}
		}
	} else if (no_glowcolor) {
		for (x = 0; x < world.w; x++) {
			for (y = 0; y < world.h; y++) {
				dc = get_normal_dot_color_simple(world, x, y);
				SDL_SetRenderDrawColor(r, dc.r, dc.g, dc.b, dc.a);
				SDL_RenderPoint(r, x, y);
			}
		}
	} else {
		for (x = 0; x < world.w; x++) {
			for (y = 0; y < world.h; y++) {
				if (MAT_NONE == world.dot[x][y]) {
					dc = (struct Rgba) {
						.r = 0,
						.g = 0,
						.b = 0,
						.a = SDL_ALPHA_OPAQUE,
					};
				} else {
					dc = get_normal_dot_color(world, x, y);
				}
				SDL_SetRenderDrawColor(r, dc.r, dc.g, dc.b, dc.a);
				SDL_RenderPoint(r, x, y);
			}
		}
	}

	SDL_SetRenderDrawColor(r, SPAWNER_R, SPAWNER_G, SPAWNER_B, SPAWNER_A);
	for (x = 0; x < world.w; x++) {
		for (y = 0; y < world.h; y++) {
			if (world.spawner[x][y]) {
				SDL_RenderPoint(r, x, y);
			}
		}
	}

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

	tool.x = tool_opts.x - tool_radius;
	tool.y = tool_opts.y - tool_radius;
	tool.w = tool_radius * 2 + 1;
	tool.h = tool_radius * 2 + 1;

	SDL_SetRenderDrawColor(r,
	                       TOOL_HOVER_R,
	                       TOOL_HOVER_G,
	                       TOOL_HOVER_B,
	                       TOOL_HOVER_A);
	SDL_RenderFillRect(r, &tool);
}

#else

size_t
render_world(char               *out,
             const size_t        out_size,
             const size_t        dot_depth,
             const bool          no_color,
             const bool          no_glowcolor,
             const bool          th_vision,
             struct ToolOptions  tool_opts,
             const struct World  world,
             const struct Rect   world_draw,
             const int           world_draw_space_w,
             const int           world_draw_space_h)
{
	int    tool_radius = 0;
	int    tool_x1 = 0;
	int    tool_y1 = 0;
	int    tool_x2 = 0;
	int    tool_y2 = 0;
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

	tool_x1 = tool_opts.x - tool_radius - world_draw.x;
	if (tool_x1 < 0)
		tool_x1 = 0;

	tool_y1 = tool_opts.y - tool_radius - world_draw.y;
	if (tool_y1 < 0)
		tool_y1 = 0;

	tool_x2 = tool_opts.x + tool_radius + 1 - world_draw.x;
	if (tool_x2 >= world_draw.w)
		tool_x2 = world_draw.w;

	tool_y2 = tool_opts.y + tool_radius + 1 - world_draw.y;
	if (tool_y2 >= world_draw.h)
		tool_y2 = world_draw.h;

	for (x = tool_x1; x < tool_x2; x++) {
		for (y = tool_y1; y < tool_y2; y++) {
			out[((y * world_draw.w) + x + 1) * dot_depth +
			    (y * world_draw_space_w) -
			    1] = '^';
		}
	}

	return written;
}
#endif /* SDL_BACKEND */

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

struct ToolOptions
ToolOptions_new(void)
{
	struct ToolOptions ret = {
		.brush_mat = FIRST_REAL_MAT,
		.brush_radius = STD_BRUSH_RADIUS,
		.eraser_radius = STD_ERASER_RADIUS,
		.sel_tool = STD_SELECTED_TOOL,
		.spawn_temperature = STD_SPAWN_TEMPERATURE,
		.spawner_mat = FIRST_REAL_MAT,
		.thermo_radius = STD_THERMO_RADIUS,
		.thermo_rate = STD_THERMO_RATE,
		.x = 0,
		.y = 0,
	};
	return ret;
}

void
use_tool(const float         delta,
         struct ToolOptions  tool_opts,
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
		                 tool_opts.thermo_rate * delta,
		                 tool_opts.x,
		                 tool_opts.y,
		                 tool_opts.thermo_radius);
		break;

	case TOOL_COOLER:
		world_use_cooler(world,
		                 tool_opts.thermo_rate * delta,
		                 tool_opts.x,
		                 tool_opts.y,
		                 tool_opts.thermo_radius);
		break;

	case TOOL_COUNT:
		break;
	}
}

size_t
write_statusbar_elem(char                        *out,
                     const size_t                 out_size,
                     const char                  *ip_address,
                     const bool                   paused,
                     const enum StatusbarElement  sbe,
                     const bool                   th_vision,
                     const float                  tickrate,
                     struct ToolOptions           tool_opts,
                     const char                  *world_name)
{
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
			written += string_cat(out,
			                      out_size,
			                      written,
			                      NUMBERSTRING[(int) tickrate]);
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
write_tool_hint(char                     *out,
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

int
main(int    argc,
     char **argv)
{
	bool                   active = true;
	bool                   autosave_all = false;
	bool                   autosave_none = false;
	char                   cmdline[CMDLINE_SIZE];
	size_t                 cmdline_len = 0;
	char                   cwd[PATH_SIZE];
	float                  delta = 0.0;
	int                    drag_start_x = 0;
	int                    drag_start_y = 0;
	char                  *feedback = NULL;
	clock_t                feedback_expiration = 0;
	float                  framerate = STD_FRAMERATE;
	enum InputMode         input_mode = IM_NORMAL;
	char                  *ip_address = "localhost";
	bool                   paused = false;
	clock_t                last_autosave = 0;
	clock_t                last_input = 0;
	clock_t                last_frame = 0;
	clock_t                last_key_use = 0;
	clock_t                last_tick = 0;
	int                    new_world_w = 0;
	int                    new_world_h = 0;
	bool                   no_glowcolor = false;
	clock_t                now = 0;
	size_t                 statusbar_elems = 0;
	enum StatusbarElement  statusbar_elem[ARRLEN(STATUSBAR_DISPLAY_PRIORITY)];
	bool                   th_vision = false;
	float                  tickrate = STD_TICKRATE;
	struct ToolOptions     tool_opts;
	int                    win_w = 0;
	int                    win_h = 0;
	struct World           world;
	char                   world_name[WORLDNAME_SIZE];

#ifdef SDL_BACKEND
	SDL_Renderer *renderer = NULL;
	TTF_Font     *font = NULL;
	char         *font_path = NULL;
	size_t        font_size = STD_FONT_SIZE;
	size_t        i;
	SDL_Window   *win = NULL;
	size_t        world_area_w = 0; /* how much space is allocated to display the world */
	size_t        world_area_h = 0; /* how much space is allocated to display the world */
	SDL_FRect     world_dst = {
		.x = 0, /* scroll x */
		.y = 0, /* scroll y */
		.w = 0, /* entire world, scaled up */
		.h = 0, /* entire world, scaled up */
	};
	size_t        world_scale = STD_WORLD_SCALE;
	SDL_Texture  *world_tx = NULL;
#else
	size_t                 cmdline_shift = 0;
	char                  *display = NULL;
	size_t                 display_size = 0;
	size_t                 dot_depth = 0;
	bool                   lmb_pressed = false;
	bool                   no_color = false;
	struct winsize         ws;
	struct Rect            world_draw = {
		.x = 0,
		.y = 0,
		.w = 0,
		.h = 0,
	};
	int                    world_draw_space_w = 0; /* how much space comes after the world display */
	int                    world_draw_space_h = 0; /* how much space comes after the world display */
#endif

	cmdline[0] = '\0';
	tool_opts = ToolOptions_new();

	if (!handle_args(argc, argv,
#ifdef SDL_BACKEND
			 &font_path,
			 &font_size,
			 &world_scale,
#else
	                 &no_color,
#endif
	                 &autosave_all,
	                 &autosave_none,
	                 &framerate,
	                 &no_glowcolor,
	                 &tickrate,
	                 &tool_opts)) {
		return 0;
	}

	getcwd(cwd, PATH_SIZE);

	hawps_core_init();
	hawps_extra_init();

	string_copy(world_name, WORLDNAME_SIZE, WORLDNAME_NEW);

#ifdef SDL_BACKEND
	// TODO add proper identifier
	SDL_SetAppMetadata(APP_NAME, APP_VERSION, "lol.69." APP_NAME);

	if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
		fprintf(stderr, "%s\n", SDL_GetError());
		goto cleanup;
	}

	if (!TTF_Init()) {
		fprintf(stderr, "%s\n", SDL_GetError());
		goto cleanup;
	}

	if (NULL != font_path) {
		font = TTF_OpenFont(font_path, font_size);
	} else {
		for (i = 0; i < ARRLEN(FONTPATH); i++) {
			font = TTF_OpenFont(FONTPATH[i], font_size);
			if (NULL != font) {
				break;
			}
		}
	}

	if (NULL == font) {
		fprintf(stderr, "Font couldn't be opened\n%s\n", SDL_GetError());
		goto cleanup;
	}
	TTF_SetFontDirection(font, TTF_DIRECTION_LTR);
	TTF_SetFontLanguage(font, "en");

	if (!SDL_CreateWindowAndRenderer(APP_NAME_FORMAL,
	                                 SDL_WIN_WIDTH, SDL_WIN_HEIGHT,
	                                 SDL_WINDOW_RESIZABLE,
	                                 &win, &renderer)) {
		fprintf(stderr, "%s\n", SDL_GetError());
		goto cleanup;
	}
	win_w = SDL_WIN_WIDTH;
	win_h = SDL_WIN_HEIGHT;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_StartTextInput(win);

	handle_resize(font_size,
	              win,
	              &win_w,
	              &win_h,
	              &world_area_w,
	              &world_area_h,
	              &world_dst);
	handle_statusbar_resize(font,
	                        ip_address,
	                        &statusbar_elems,
	                        statusbar_elem,
	                        win_w,
	                        world_name);

	new_world_w = world_area_w / world_scale;
	new_world_h = world_area_h / world_scale;
#else
	CSI_set_raw();
	fputs(CSI_CLEAR, stdout);

	if (no_color) {
		dot_depth = 1;
	} else {
		dot_depth = CSI_COLORSTRING_LEN + 1;
	}

	ws = CSI_get_size();
	new_world_w = ws.ws_col;
	new_world_h = ws.ws_row - 2;

	/* we love hacks
	 * handle_resize ONLY reallocs for performance */
	display = malloc(1);
	if (NULL == display) {
		fprintf(stderr, "Could not allocate memory\n");
		goto cleanup;
	}
#endif /* SDL_BACKEND */

	command_load_core(cwd, &world, WORLDNAME_NEW);

	if (0 == world.w ||
	    0 == world.h) {
		world = world_new(new_world_w, new_world_h);
	}

#ifdef SDL_BACKEND
	handle_world_resize(renderer,
	                    &world_dst,
	                    world_scale,
	                    &world_tx,
	                    &tool_opts,
	                    world);
#else
	handle_resize(cmdline_len,
		      &cmdline_shift,
		      &display,
		      &display_size,
		      dot_depth,
		      input_mode,
		      ip_address,
		      &statusbar_elems,
		      statusbar_elem,
		      &tool_opts,
		      &win_w,
		      &win_h,
		      world,
		      &world_draw,
		      &world_draw_space_w,
		      &world_draw_space_h,
		      world_name);
#endif /* SDL_BACKEND */

	while (active) {
		now = clock();
		delta = (float) (now - last_input) / (float) CLOCKS_PER_SEC;

		last_input = now;

		handle_input(
#ifdef SDL_BACKEND
		             font,
		             font_size,
		             renderer,
		             win,
		             &win_w,
		             &win_h,
		             &world_area_w,
		             &world_area_h,
		             &world_dst,
		             world_scale,
		             &world_tx,
#else
		             &cmdline_shift,
		             display_size,
		             dot_depth,
		             &lmb_pressed,
		             no_color,
		             win_w,
		             win_h,
		             &world_draw,
		             &world_draw_space_w,
		             &world_draw_space_h,
#endif
		             &active,
		             cmdline,
		             &cmdline_len,
		             cwd,
		             delta,
		             &drag_start_x,
		             &drag_start_y,
		             &feedback,
		             &feedback_expiration,
		             &framerate,
		             &input_mode,
		             ip_address,
		             &last_key_use,
		             &no_glowcolor,
		             now,
		             &paused,
		             &statusbar_elems,
		             statusbar_elem,
		             &tickrate,
		             &th_vision,
		             &tool_opts,
		             &world,
		             world_name);

#ifdef SDL_BACKEND
#else
		if (lmb_pressed) {
			use_tool(delta, tool_opts, &world);
		}
#endif
		if (now - last_tick >= (long) (CLOCKS_PER_SEC / tickrate)) {
			last_tick = now;

			world_update(&world, tool_opts.spawn_temperature);

			if (!paused) {
				world_sim(&world);
			}

			if (now - last_autosave >=
			    CLOCKS_PER_SEC * AUTOSAVE_INTERVAL) {
				handle_autosave(autosave_all,
					        autosave_none,
					        cwd,
					        &last_autosave,
					        now,
					        world,
					        world_name);
			}
		}

		if (now - last_frame >= (long) (CLOCKS_PER_SEC / framerate)) {
			last_frame = now;

			if (now > feedback_expiration) {
				feedback = NULL;
			}

#ifdef SDL_BACKEND
			draw(cmdline,
			     feedback,
			     font,
			     font_size,
			     input_mode,
			     ip_address,
			     no_glowcolor,
			     paused,
			     statusbar_elems,
			     statusbar_elem,
			     th_vision,
			     tickrate,
			     tool_opts,
			     renderer,
			     world,
			     world_dst,
			     world_name,
			     world_tx);
#else
			handle_resize(cmdline_len,
				      &cmdline_shift,
				      &display,
				      &display_size,
				      dot_depth,
				      input_mode,
				      ip_address,
				      &statusbar_elems,
				      statusbar_elem,
				      &tool_opts,
				      &win_w,
				      &win_h,
				      world,
				      &world_draw,
				      &world_draw_space_w,
				      &world_draw_space_h,
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
			     statusbar_elems,
			     statusbar_elem,
			     tickrate,
			     th_vision,
			     tool_opts,
			     win_w,
			     world,
			     world_draw,
			     world_draw_space_w,
			     world_draw_space_h,
			     world_name);
#endif /* SDL_BACKEND */
		}
	}

	handle_autosave(autosave_all,
	                autosave_none,
	                cwd,
	                &last_autosave,
	                now,
	                world,
	                world_name);

cleanup:
#ifdef SDL_BACKEND
	SDL_DestroyTexture(world_tx);
	SDL_StopTextInput(win);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
#else
	CSI_set_normal();
	fputs(CSI_CLEAR, stdout);
	fputs(CSI_FG_DEFAULT, stdout);
	fputs(CSI_BG_DEFAULT, stdout);

	if (display != NULL) {
		free(display);
	}
#endif

	world_free(&world);

	return 0;
}
