# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024 - 2025  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
LICENSE          :=LGPL-2.1-only
LICENSE_URL      :=https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html
REPOSITORY       :=https://github.com/SchokiCoder/hawps
VERSION          :=v0.6
GO_COMPILE_VARS  :=-ldflags "-X 'cross_platform.AppName=$(APP_NAME)' -X 'cross_platform.AppNameFormal=$(APP_NAME_FORMAL)' -X 'cross_platform.AppLicense=$(LICENSE)' -X 'cross_platform.AppLicenseUrl=$(LICENSE_URL)' -X 'cross_platform.AppRepository=$(REPOSITORY)' -X 'cross_platform.AppVersion=$(VERSION)'"

.PHONY: all build clean generate run test vet

all: generate vet build

build: $(APP_NAME)

clean:
	rm -f $(APP_NAME) cross_platform/tool_string.go core/mat_string.go

generate: cross_platform/tool_string.go core/mat_string.go

run: clean all
	./$(APP_NAME) -window

test:
	go test ./core -cpuprofile cpu.prof -memprofile mem.prof -bench ./core

vet:
	go vet ./cross_platform

$(APP_NAME): cross_platform/main.go cross_platform/tool_string.go cross_platform/ui/ui.go core/mat.go core/mat_string.go extra/glowcolor.go
	go build $(GO_COMPILE_VARS) -o $(APP_NAME) ./cross_platform

cross_platform/tool_string.go: cross_platform/main.go
	go generate ./cross_platform

core/mat_string.go: core/mat.go
	go generate ./core
