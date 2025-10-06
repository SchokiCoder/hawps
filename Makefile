# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024 - 2025  Andy Frank Schoknecht

APP_NAME         :=hawps
APP_NAME_FORMAL  :=Half Assed Wannabe Physics Simulator
LICENSE          :=LGPL-2.1-only
LICENSE_URL      :=https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html
REPOSITORY       :=https://github.com/SchokiCoder/hawps
VERSION          :=v0.6

CC     :=cc
CFLAGS :=-std=c99 -pedantic -Wall -Wextra -Wvla -Wno-unused-variable -fsanitize=address,undefined

.PHONY: all build clean run

all: $(APP_NAME)_tk

clean:
	rm -f $(APP_NAME)_*

run: clean $(APP_NAME)_tk
	./$(APP_NAME)_tk

# core/* extra/*
$(APP_NAME)_tk: client_tk/*.c
	$(CC) $$(pkg-config --cflags tcl tk) -o $@ $< $$(pkg-config --libs tcl tk)

desktop/embedded_glade.h: desktop/desktop.glade
	cat $< > EMBEDDED_GLADE
	printf "\0" >> EMBEDDED_GLADE
	xxd -i EMBEDDED_GLADE > $@
	rm -f EMBEDDED_GLADE
