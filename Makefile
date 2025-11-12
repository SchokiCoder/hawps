# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024 - 2025  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
LICENSE          :=LGPL-2.1-only
LICENSE_URL      :=https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html
REPOSITORY       :=https://github.com/SchokiCoder/hawps
VERSION          :=v0.6
GO_COMPILE_VARS  :=-ldflags "-X 'main.AppName=$(APP_NAME)' -X 'main.AppNameFormal=$(APP_NAME_FORMAL)' -X 'main.AppLicense=$(LICENSE)' -X 'main.AppLicenseUrl=$(LICENSE_URL)' -X 'main.AppRepository=$(REPOSITORY)' -X 'main.AppVersion=$(VERSION)'"

CC     :=cc
CFLAGS :=-std=c99 -pedantic -Wall -Wextra -Wvla -Wno-unused-variable -fsanitize=address,undefined -g

.PHONY: all build clean generate run test vet

all: $(APP_NAME)_ebiten

clean:
	rm -f $(APP_NAME)_*

run: clean $(APP_NAME)_ebiten
	./$(APP_NAME)_ebiten -window

test:
	go test ./core -cpuprofile cpu.prof -memprofile mem.prof -bench ./core

vet:
	go vet ./client_ebiten

$(APP_NAME)_ebiten: client_ebiten/*.go client_ebiten/ui/*.go core/*.go extra/*.go core/mat/mat_string.go extra/tool_string.go
	go build $(GO_COMPILE_VARS) -o $@ ./client_ebiten

core/mat/mat_string.go: core/mat/mat.go
	go generate ./core/mat

extra/tool_string.go: extra/extra.go
	go generate ./extra

$(APP_NAME)_tk: client_tk/* core/* extra/*
	$(CC) $(CFLAGS) -o $@ -I. $$(pkg-config --cflags tcl tk) \
		client_tk/*.c core/*.c extra/*.c \
		$$(pkg-config --libs tcl tk)
