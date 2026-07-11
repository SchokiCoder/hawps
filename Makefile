# SPDX-License-Identifier: MPL-2.0
# Copyright (C) 2024 - 2026  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
LICENSE          :=MPL-2.0
LICENSE_URL      :=https://mozilla.org/MPL/2.0
REPOSITORY       :=https://github.com/SchokiCoder/hawps
VERSION          :=v0.6

DEFAULT_CLIENT :=$(APP_NAME)_terminal

GO_DEFINES :=-ldflags "-X 'main.AppName=$(APP_NAME)' -X 'main.AppNameFormal=$(APP_NAME_FORMAL)' -X 'main.AppLicense=$(LICENSE)' -X 'main.AppLicenseUrl=$(LICENSE_URL)' -X 'main.AppRepository=$(REPOSITORY)' -X 'main.AppVersion=$(VERSION)'"

CC        :=cc
C_FLAGS   :=-std=c99 -pedantic -Wall -Wextra -Wvla -Wno-unused-variable -fsanitize=address,undefined -g -pg
C_DEFINES :=-D APP_NAME='"$(APP_NAME)"' -D APP_NAME_FORMAL='"$(APP_NAME_FORMAL)"' -D APP_LICENSE='"$(LICENSE)"' -D APP_LICENSE_URL='"$(LICENSE_URL)"' -D APP_REPOSITORY='"$(REPOSITORY)"' -D APP_VERSION='"$(VERSION)"'

.PHONY: all build clean generate run test vet

all: bin/$(DEFAULT_CLIENT)

clean:
	rm -f bin/*
	rm -f *.out

prerun:
	rm -f bin/$(DEFAULT_CLIENT)

run: prerun bin/$(DEFAULT_CLIENT)
	./bin/$(DEFAULT_CLIENT)

vet:
	go vet ./client_ebiten

bin/$(APP_NAME)_ebiten: client_ebiten/*.go client_ebiten/ui/*.go core/*.go extra/*.go core/mat/mat_string.go extra/tool_string.go
	go build $(GO_DEFINES) -o $@ ./client_ebiten

bin/$(APP_NAME)_terminal: client_terminal/* lib_core/* lib_extra/*
	$(CC) $(C_FLAGS) $(C_DEFINES) -o $@ -I lib_core -I lib_extra \
		client_terminal/*.c lib_core/*.c lib_extra/*.c

bin/$(APP_NAME)_tk: client_tk/* lib_core/* lib_extra/*
	$(CC) $(C_FLAGS) -o $@ -I lib_core -I lib_extra \
		$$(pkg-config --cflags tcl tk) \
		client_tk/*.c lib_core/*.c lib_extra/*.c \
		$$(pkg-config --libs tcl tk)
