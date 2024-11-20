# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024  Andy Frank Schoknecht

APP_NAME        :=\"hawps\"
APP_NAME_FORMAL :="\"Half Assed Wannabe Physics Simulator\""
APP_VERSION     :=\"v0.3\"
APP_LICENSE     :=\"LGPL-2.1-only\"
APP_REPOSITORY  :=\"https://github.com/SchokiCoder/hawps\"
APP_LICENSE_URL :=\"https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html\"

CC       := cc
CFLAGS   := -Wall -Wextra -std=c99 -pedantic -fsanitize=address,undefined -g
INCLUDES := -I /usr/include/SDL2
LIBS     := -lSDL2
DEFINES  := -D _DEFAULT_SOURCE \
	-D APP_NAME=$(APP_NAME) \
	-D APP_NAME_FORMAL=$(APP_NAME_FORMAL) \
	-D APP_VERSION=$(APP_VERSION) \
	-D APP_LICENSE=$(APP_LICENSE) \
	-D APP_REPOSITORY=$(APP_REPOSITORY) \
	-D APP_LICENSE_URL=$(APP_LICENSE_URL)

.PHONY: clean run

run: clean all
	./hawps

all: hawps

hawps: hawps.c world.c
	$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(LIBS) $(DEFINES) $^

clean:
	rm -f hawps
