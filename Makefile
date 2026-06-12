# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024 - 2026  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
LICENSE          :=LGPL-2.1-only
LICENSE_URL      :=https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html
REPOSITORY       :=https://github.com/SchokiCoder/hawps
VERSION          :=v0.6
GO_DEFINES       :=-ldflags "-X 'main.AppName=$(APP_NAME)' -X 'main.AppNameFormal=$(APP_NAME_FORMAL)' -X 'main.AppLicense=$(LICENSE)' -X 'main.AppLicenseUrl=$(LICENSE_URL)' -X 'main.AppRepository=$(REPOSITORY)' -X 'main.AppVersion=$(VERSION)'"

CC        :=cc
C_FLAGS   :=-std=c99 -pedantic -Wall -Wextra -Wvla -Wno-unused-variable -fsanitize=address,undefined -g
C_DEFINES :=-D APP_NAME='"$(APP_NAME)"' -D APP_NAME_FORMAL='"$(APP_NAME_FORMAL)"' -D APP_LICENSE='"$(LICENSE)"' -D APP_LICENSE_URL='"$(LICENSE_URL)"' -D APP_REPOSITORY='"$(REPOSITORY)"' -D APP_VERSION='"$(VERSION)"'

.PHONY: all build clean generate run test vet

all: $(APP_NAME)_terminal

clean:
	rm -f $(APP_NAME)_*

run: clean $(APP_NAME)_terminal
	./$(APP_NAME)_terminal

vet:
	go vet ./client_ebiten

$(APP_NAME)_ebiten: client_ebiten/*.go client_ebiten/ui/*.go core/*.go extra/*.go core/mat/mat_string.go extra/tool_string.go
	go build $(GO_DEFINES) -o $@ ./client_ebiten

$(APP_NAME)_terminal: client_terminal/* client_terminal/csi/* core/* extra/*
	$(CC) $(C_FLAGS) $(C_DEFINES) -o $@ -I . \
		client_terminal/*.c client_terminal/csi/*.c \
		core/*.c extra/*.c

core/mat/mat_string.go: core/mat/mat.go
	go generate ./core/mat

extra/tool_string.go: extra/extra.go
	go generate ./extra

$(APP_NAME)_tk: client_tk/* core/* extra/*
	$(CC) $(C_FLAGS) -o $@ -I. $$(pkg-config --cflags tcl tk) \
		client_tk/*.c core/*.c extra/*.c \
		$$(pkg-config --libs tcl tk)
