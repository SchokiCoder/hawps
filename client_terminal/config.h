/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* key binds
 */
#define KEY_QUIT          'q'
#define KEY_USE           'e'
#define KEY_SWITCH_VISION 't'
#define KEY_BRUSH         'a'
#define KEY_SPAWNER       's'
#define KEY_ERASER        'd'
#define KEY_HEATER        'f'
#define KEY_COOLER        'g'
#define KEY_LEFT          'h'
#define KEY_DOWN          'j'
#define KEY_UP            'k'
#define KEY_RIGHT         'l'
#define KEY_CMD           ':'
#define KEY_PAUSE         ' '

/* settings
 */
#define MAX_RADIUS           16
#define THERMAL_VISION_MIN_T (-75.0 + CELSIUS_TO_KELVIN)
#define THERMAL_VISION_R 100
#define THERMAL_VISION_G 0
#define THERMAL_VISION_B 0

/* startup settings
 */
#define STD_BRUSH_RADIUS  2
#define STD_ERASER_RADIUS 5
#define STD_SELECTED_TOOL TOOL_BRUSH
#define STD_SIM_SUBSAMPLE 4
#define STD_TEMPERATURE   (20.0 + CELSIUS_TO_KELVIN)
#define STD_THERMO_DELTA  5.0
#define STD_THERMO_RADIUS STD_BRUSH_RADIUS
#define STD_TICKRATE      120

/* size values
 * Consider only touching them when you have special hardware or problems.
 */
// TODO allow for dynamic size via display string being on heap
#define BUF_SIZE     64
#define INPUT_SIZE   16

#endif /* _CONFIG_H */
