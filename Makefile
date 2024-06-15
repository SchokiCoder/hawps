# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2024  Andy Frank Schoknecht

APP_NAME="hawps"

.PHONY: all build clean vet

all: vet build

build:
	go build

clean:
	rm -f $(APP_NAME)

vet:
	go vet
