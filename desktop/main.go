// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

package main

import (
	_ "embed"
	"os"

	"github.com/gotk3/gotk3/gdk"
	"github.com/gotk3/gotk3/glib"
	"github.com/gotk3/gotk3/gtk"
)

//go:embed desktop.glade
var glade string

func main(
) {
	var (
		builder *gtk.Builder
		err     error
		obj     glib.IObject
		ok      bool
		win     *gtk.Window
	)

	gtk.Init(&os.Args)

	_, err = gdk.DisplayGetDefault()
	if err != nil {
		panic(err)
	}

	builder, err = gtk.BuilderNewFromString(glade)
	if err != nil {
		panic(err)
	}

	obj, err = builder.GetObject("main")
	if err != nil {
		panic("No window in glade file")
	}
	win, ok = obj.(*gtk.Window)
	if !ok {
		panic("Invalid window in glade file")
	}

	win.Connect("destroy", func() {
		gtk.MainQuit()
	})

	win.ShowAll()
	gtk.Main()
}
