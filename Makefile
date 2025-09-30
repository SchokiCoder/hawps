# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024 - 2025  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
LICENSE          :=LGPL-2.1-only
LICENSE_URL      :=https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html
REPOSITORY       :=https://github.com/SchokiCoder/hawps
VERSION          :=v0.6
GO_COMPILE_VARS  :=-ldflags "-X 'cross_platform.AppName=$(APP_NAME)' -X 'cross_platform.AppNameFormal=$(APP_NAME_FORMAL)' -X 'cross_platform.AppLicense=$(LICENSE)' -X 'cross_platform.AppLicenseUrl=$(LICENSE_URL)' -X 'cross_platform.AppRepository=$(REPOSITORY)' -X 'cross_platform.AppVersion=$(VERSION)'"

CC     :=cc
CFLAGS :=-std=c99 -pedantic -Wall -Wextra -Wvla -Wno-unused-variable -fsanitize=address,undefined

.PHONY: all build clean generate run test vet

all: $(APP_NAME)_desktop

build: $(APP_NAME)

clean:
	rm -f $(APP_NAME)_* cross_platform/tool_string.go core/mat_string.go desktop/embedded_glade.h

generate: cross_platform/tool_string.go core/mat_string.go

run: clean $(APP_NAME)_desktop
	./$(APP_NAME)_desktop

test:
	go test ./core -cpuprofile cpu.prof -memprofile mem.prof -bench ./core

vet:
	go vet ./cross_platform

$(APP_NAME)_cross: cross_platform/*.go cross_platform/ui/*.go core/*.go extra/*.go
	go build $(GO_COMPILE_VARS) -o $@ ./cross_platform

cross_platform/tool_string.go: cross_platform/main.go
	go generate ./cross_platform

core/mat_string.go: core/mat.go
	go generate ./core

$(APP_NAME)_desktop: desktop/embedded_glade.h desktop/* core/* extra/*
	$(CC) $$(pkg-config --cflags gtk+-3.0) $(CFLAGS) -o $@ \
		-I . \
		./desktop/*.c $$(pkg-config --libs gtk+-3.0) \
		core/mat.c extra/glowcolor.c

desktop/embedded_glade.h: desktop/desktop.glade
	cat $< > EMBEDDED_GLADE
	printf "\0" >> EMBEDDED_GLADE
	xxd -i EMBEDDED_GLADE > $@
	rm -f EMBEDDED_GLADE
