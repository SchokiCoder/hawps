# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024 - 2025  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
LICENSE          :=LGPL-2.1-only
LICENSE_URL      :=https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html
REPOSITORY       :=https://github.com/SchokiCoder/hawps
VERSION          :=v0.6

CC     :=cc
CFLAGS :=-std=c99 -pedantic -Wall -Wextra -Wvla -Wno-unused-variable -fsanitize=address,undefined -g

.PHONY: all build clean run

all: $(APP_NAME)_tk

clean:
	rm -f $(APP_NAME)_*

run: clean $(APP_NAME)_tk
	LSAN_OPTIONS=suppressions=client_tk/lsan.supp ./$(APP_NAME)_tk

$(APP_NAME)_tk: client_tk/* core/* extra/*
	$(CC) $(CFLAGS) -o $@ -I. $$(pkg-config --cflags tcl tk) \
		client_tk/*.c core/*.c extra/*.c \
		$$(pkg-config --libs tcl tk)
