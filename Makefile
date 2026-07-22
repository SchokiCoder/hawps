# SPDX-License-Identifier: MPL-2.0
# Copyright (C) 2024 - 2026  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
APP_LICENSE      :=MPL-2.0
APP_LICENSE_URL  :=https://mozilla.org/MPL/2.0
APP_REPOSITORY   :=https://github.com/SchokiCoder/hawps
APP_VERSION      !=git describe --abbrev=0 --tags

# dest dir, uncomment below to install for only current user instead
BIN_DESTDIR:="/usr/local/bin"
#BIN_DESTDIR:="$(HOME)/.local/bin"

CC              :=cc
C_FLAGS_DEBUG   :=-std=c99 -pedantic -Wall -Wextra -Wvla -Wno-unused-variable -fsanitize=address,undefined -g
C_FLAGS_PROFILE :=-std=c99 -pedantic -Wall -Wextra -Wvla -Wno-unused-variable -g -O3 -pg
C_FLAGS_RELEASE :=-std=c99 -pedantic -Wall -Wextra -Wvla -Wno-unused-variable -O3
C_DEFINES       :=-D APP_NAME='"$(APP_NAME)"' -D APP_NAME_FORMAL='"$(APP_NAME_FORMAL)"' -D APP_LICENSE='"$(APP_LICENSE)"' -D APP_LICENSE_URL='"$(APP_LICENSE_URL)"' -D APP_REPOSITORY='"$(APP_REPOSITORY)"' -D APP_VERSION='"$(APP_VERSION)"'

CLIENT_TERMINAL_INCLUDE_DIRS :=-I lib_core -I lib_extra
CLIENT_TERMINAL_FILE_DEPS    :=client_terminal/* client_terminal/int_to_string.h lib_core/* lib_extra/*
CLIENT_TERMINAL_SRC_FILES    :=client_terminal/*.c lib_core/*.c lib_extra/*.c

DEFAULT_CLIENT :=$(APP_NAME)_terminal

GIT_HEAD !=git rev-parse --short HEAD

GO_DEFINES :=-ldflags "-X 'main.AppName=$(APP_NAME)' -X 'main.AppNameFormal=$(APP_NAME_FORMAL)' -X 'main.AppLicense=$(APP_LICENSE)' -X 'main.AppLicenseUrl=$(APP_LICENSE_URL)' -X 'main.AppRepository=$(APP_REPOSITORY)' -X 'main.AppVersion=$(APP_VERSION)'"

.PHONY: all build clean generate install profile remove run test vet

all: bin/$(DEFAULT_CLIENT)

clean:
	rm -f bin/*
	rm -f *.out

generate: client_terminal/int_to_string.h

install: bin/$(DEFAULT_CLIENT)
	mkdir -p $(BIN_DESTDIR)
	cp $< $(BIN_DESTDIR)/$(APP_NAME)

prerun:
	rm -f bin/$(DEFAULT_CLIENT)

preprofile:
	rm -f profiling/*$(GIT_HEAD)*

profile: preprofile profiling/$(DEFAULT_CLIENT)_$(GIT_HEAD)
	./profiling/$(DEFAULT_CLIENT)_$(GIT_HEAD)
	mv gmon.out profiling/gmon_$(GIT_HEAD).out
	gprof profiling/$(DEFAULT_CLIENT)_$(GIT_HEAD) \
		profiling/gmon_$(GIT_HEAD).out > \
		profiling/$(DEFAULT_CLIENT)_$(GIT_HEAD).txt

remove:
	rm -f $(BIN_DESTDIR)/$(APP_NAME)

run: prerun bin/$(DEFAULT_CLIENT)
	./bin/$(DEFAULT_CLIENT)

vet:
	go vet ./client_ebiten

bin/$(APP_NAME)_ebiten: client_ebiten/*.go client_ebiten/ui/*.go core/*.go extra/*.go core/mat/mat_string.go extra/tool_string.go
	go build $(GO_DEFINES) -o $@ ./client_ebiten

bin/$(APP_NAME)_terminal: $(CLIENT_TERMINAL_FILE_DEPS)
	$(CC) $(C_FLAGS_DEBUG) $(C_DEFINES) -o $@ \
		$(CLIENT_TERMINAL_INCLUDE_DIRS) \
		$(CLIENT_TERMINAL_SRC_FILES)

bin/$(APP_NAME)_tk: client_tk/* lib_core/* lib_extra/*
	$(CC) $(C_FLAGS) -o $@ -I lib_core -I lib_extra \
		$$(pkg-config --cflags tcl tk) \
		client_tk/*.c lib_core/*.c lib_extra/*.c \
		$$(pkg-config --libs tcl tk)

bin/gen_int_to_string_table:
	$(CC) $(C_FLAGS) $(C_DEFINES) -o $@ \
		client_terminal/gen/gen_int_to_string_table.c

client_terminal/int_to_string.h: bin/gen_int_to_string_table
	./$< $@

profiling/$(APP_NAME)_terminal_$(GIT_HEAD): $(CLIENT_TERMINAL_FILE_DEPS)
	$(CC) $(C_FLAGS_PROFILE) $(C_DEFINES) -o $@ \
		$(CLIENT_TERMINAL_INCLUDE_DIRS) \
		$(CLIENT_TERMINAL_SRC_FILES)
