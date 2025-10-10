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

all: $(APP_NAME)_gtk

clean:
	rm -f $(APP_NAME)_* client_gtk/embedded_glade.h

run: clean $(APP_NAME)_gtk
	./$(APP_NAME)_gtk

$(APP_NAME)_gtk: client_gtk/embedded_glade.h client_gtk/* core/* extra/*
	$(CC) $$(pkg-config --cflags gtk+-3.0) $(CFLAGS) -o $@ \
		-I . \
		client_gtk/*.c $$(pkg-config --libs gtk+-3.0) \
		core/*.c extra/*.c

client_gtk/embedded_glade.h: client_gtk/desktop.glade
	cat $< > EMBEDDED_GLADE
	printf "\0" >> EMBEDDED_GLADE
	xxd -i EMBEDDED_GLADE > $@
	rm -f EMBEDDED_GLADE
