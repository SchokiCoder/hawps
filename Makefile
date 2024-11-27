# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
LICENSE          :=LGPL-2.1-only
LICENSE_URL      :=https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html
REPOSITORY       :=https://github.com/SchokiCoder/hawps
VERSION          :=v0.3
GO_COMPILE_VARS  :=-ldflags "-X 'main.AppName=$(APP_NAME)' -X 'main.AppNameFormal=$(APP_NAME_FORMAL)' -X 'main.AppLicense=$(LICENSE)' -X 'main.AppLicenseUrl=$(LICENSE_URL)' -X 'main.AppRepository=$(REPOSITORY)' -X 'main.AppVersion=$(VERSION)'"
SRC              :=hawps.go

.PHONY: all build clean vet

all: vet build

build: $(APP_NAME)

clean:
	rm -f $(APP_NAME)

vet:
	go vet

$(APP_NAME): $(SRC)
	go build $(GO_COMPILE_VARS)
