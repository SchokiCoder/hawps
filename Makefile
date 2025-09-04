# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024 - 2025  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
LICENSE          :=LGPL-2.1-only
LICENSE_URL      :=https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html
REPOSITORY       :=https://github.com/SchokiCoder/hawps
VERSION          :=v0.6
GO_COMPILE_VARS  :=-ldflags "-X 'main.AppName=$(APP_NAME)' -X 'main.AppNameFormal=$(APP_NAME_FORMAL)' -X 'main.AppLicense=$(LICENSE)' -X 'main.AppLicenseUrl=$(LICENSE_URL)' -X 'main.AppRepository=$(REPOSITORY)' -X 'main.AppVersion=$(VERSION)'"

.PHONY: all build clean generate run test vet

all: generate vet build

build: $(APP_NAME)

clean:
	rm -f $(APP_NAME) main/tool_string.go mat/mat_string.go

generate: main/tool_string.go mat/mat_string.go

run: clean all
	./$(APP_NAME) -window

test:
	go test ./mat -cpuprofile cpu.prof -memprofile mem.prof -bench ./mat

vet:
	go vet ./main

$(APP_NAME): main/main.go main/tool_string.go extra/glowcolor.go mat/mat.go mat/mat_string.go ui/ui.go
	go build $(GO_COMPILE_VARS) ./main

main/tool_string.go: main/main.go
	go generate ./main

mat/mat_string.go: mat/mat.go
	go generate ./mat
