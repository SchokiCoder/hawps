# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2024  Andy Frank Schoknecht

CC       := cc
CFLAGS   := -Wall -Wextra -std=c99 -pedantic -fsanitize=address,undefined -g
INCLUDES := -I /usr/include/SDL2
LIBS     := -lSDL2

APP_NAME := hawps

.PHONY: clean

$(APP_NAME): main.c
	$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(LIBS) $^

clean:
	rm -f $(APP_NAME)
