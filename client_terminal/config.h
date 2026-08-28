/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include "types.h"

/* color settings
 */
#define SPAWNER_R 128
#define SPAWNER_G 0
#define SPAWNER_B 128
#define SPAWNER_A 125

#define THERMAL_VISION_R 100
#define THERMAL_VISION_G 0
#define THERMAL_VISION_B 0

#define TOOL_HOVER_R 80
#define TOOL_HOVER_G 120
#define TOOL_HOVER_B 120
#define TOOL_HOVER_A SPAWNER_A

/* looks settings
 */
#define CMDLINE_CURSOR      '<'
#define CMDLINE_INDICATOR   ':'
#define STATUSBAR_SEPARATOR " | "

static const char DOT_APPEARANCE[] = {
	'X',
	'X',
	'+',
	'-',
};

/* command settings
 */
#define CMD_BRUSH                   "brush"
#define CMD_BRUSH_SHORT             "b"
#define CMD_BRUSHMAT                "brushmat"
#define CMD_BRUSHMAT_SHORT          "bm"
#define CMD_BRUSHRADIUS             "brushradius"
#define CMD_BRUSHRADIUS_SHORT       "br"
#define CMD_CLEAR                   "clear"
#define CMD_CLEAR_SHORT             "cl"
#define CMD_CLEARALL                "clearall"
#define CMD_CLEARALL_SHORT          "cla"
#define CMD_COOLER                  "cooler"
#define CMD_COOLER_SHORT            "c"
#define CMD_DEFAULTS                "defaults"
#define CMD_DEFAULTS_SHORT          "def"
#define CMD_ERASER                  "eraser"
#define CMD_ERASER_SHORT            "e"
#define CMD_ERASERRADIUS            "eraserradius"
#define CMD_ERASERRADIUS_SHORT      "er"
#define CMD_FRAMERATE               "framerate"
#define CMD_FRAMERATE_SHORT         "fr"
#define CMD_GLOWCOLOR               "glowcolor"
#define CMD_GLOWCOLOR_SHORT         "gc"
#define CMD_HEATER                  "heater"
#define CMD_HEATER_SHORT            "h"
#define CMD_MAT                     "mat"
#define CMD_MAT_SHORT               "m"
#define CMD_NOGLOWCOLOR             "noglowcolor"
#define CMD_NOGLOWCOLOR_SHORT       "nogc"
#define CMD_NORMALVISION            "normalvision"
#define CMD_NORMALVISION_SHORT      "nv"
#define CMD_PAUSE                   "pause"
#define CMD_PAUSE_SHORT             "p"
#define CMD_QUIT                    "quit"
#define CMD_QUIT_SHORT              "q"
#define CMD_SPAWNER                 "spawner"
#define CMD_SPAWNER_SHORT           "s"
#define CMD_SPAWNERMAT              "spawnermat"
#define CMD_SPAWNERMAT_SHORT        "sm"
#define CMD_SPAWNTEMPERATURE        "spawntemperature"
#define CMD_SPAWNTEMPERATURE_SHORT  "st"
#define CMD_SPAWNTEMPERATUREK       "spawntemperaturek"
#define CMD_SPAWNTEMPERATUREK_SHORT "stk"
#define CMD_TEMPERATURE             "temperature"
#define CMD_TEMPERATURE_SHORT       "t"
#define CMD_TEMPERATUREK            "temperaturek"
#define CMD_TEMPERATUREK_SHORT      "tk"
#define CMD_THERMORADIUS            "thermoradius"
#define CMD_THERMORADIUS_SHORT      "thrd"
#define CMD_THERMORATE              "thermorate"
#define CMD_THERMORATE_SHORT        "thrt"
#define CMD_THERMOVISION            "thermovision"
#define CMD_THERMOVISION_SHORT      "thv"
#define CMD_TICKRATE                "tickrate"
#define CMD_TICKRATE_SHORT          "tr"

/* key binds
 */
#define KEY_QUIT          'q'
#define KEY_USE           'e'
#define KEY_SWITCH_VISION 't'
#define KEY_PREVIOUS_MAT  'u'
#define KEY_FIRST_MAT     'U'
#define KEY_NEXT_MAT      'i'
#define KEY_LAST_MAT      'I'
#define KEY_BRUSH         'a'
#define KEY_SPAWNER       's'
#define KEY_ERASER        'd'
#define KEY_HEATER        'f'
#define KEY_COOLER        'g'
#define KEY_LEFT          'h'
#define KEY_LEFT_MAX      'H'
#define KEY_DOWN          'j'
#define KEY_DOWN_MAX      'J'
#define KEY_UP            'k'
#define KEY_UP_MAX        'K'
#define KEY_RIGHT         'l'
#define KEY_RIGHT_MAX     'L'
#define KEY_RADIUS_DOWN   'c'
#define KEY_RADIUS_MIN    'C'
#define KEY_RADIUS_UP     'v'
#define KEY_RADIUS_MAX    'V'
#define KEY_SIMSPEED_DOWN 'b'
#define KEY_SIMSPEED_MIN  'B'
#define KEY_SIMSPEED_UP   'n'
#define KEY_SIMSPEED_MAX  'N'
#define KEY_CMD           ':'
#define KEY_PAUSE         ' '

/* statusbar settings
 * keep the length of both arrays equal or else...
 */
static enum StatusbarElement STATUSBAR_DISPLAY_ORDER[] = {
	SBE_WORLD_NAME, /* first */
	SBE_VIEW,
	SBE_SPEED,
	SBE_IP_ADDRESS,
	SBE_COORDS,
};

static enum StatusbarElement STATUSBAR_DISPLAY_PRIORITY[] = {
	SBE_SPEED,      /* most important */
	SBE_VIEW,
	SBE_IP_ADDRESS,
	SBE_WORLD_NAME,
	SBE_COORDS,
};

/* settings
 */
#define FEEDBACK_LIFETIME    3    /* in seconds */
#define MAX_RADIUS           16
#define MAX_TICKRATE         (STD_TICKRATE * 2.0 * 2.0)
#define MIN_TICKRATE         (STD_TICKRATE / 2.0 / 2.0 / 2.0 / 2.0 / 2.0)
#define SDL_FONT_SIZE        24
#define SDL_WIN_WIDTH        640
#define SDL_WIN_HEIGHT       480
#define SDL_WORLD_SCALE      (SDL_FONT_SIZE / 2)
#define THERMAL_VISION_MIN_T (-75.0 + CELSIUS_TO_KELVIN)

/* defaults for runtime settings
 */
#define STD_BRUSH_RADIUS      2
#define STD_ERASER_RADIUS     5
#define STD_FRAMERATE         60.0
#define STD_SELECTED_TOOL     TOOL_BRUSH
#define STD_SPAWN_TEMPERATURE (20.0 + CELSIUS_TO_KELVIN)
#define STD_THERMO_RATE       250.0
#define STD_THERMO_RADIUS     STD_BRUSH_RADIUS
#define STD_TICKRATE          24.0

/* size values
 * Consider only touching them when you have special hardware or problems.
 */
#define BUF_SIZE              64
#define CMDLINE_SIZE          81
#define DISPLAY_SIZE_MODIFIER 1.25
#define INPUT_SIZE            16




/* @User, DO NOT TOUCH, beyond this point!
 */
#define CELSIUS_TO_KELVIN 273.15

/* This constant is not meant to stick around,
 * if the configuration method changes.
 * This just a reminder for devs to remove a chapter in the help text.
 */
#define CONFIGURED_AT "compile time"

#endif /* _CONFIG_H */
