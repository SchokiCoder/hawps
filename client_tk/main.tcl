#!/usr/bin/tclsh

# SPDX-License-Identifier: LGPL-2.1-only
# Copyright (C) 2024 - 2025  Andy Frank Schoknecht

package require Tk

set brush_thermo ""
set spawner_thermo ""

proc validate {var} {
	if {[string is integer $var]} {
		return 1
	}
	return 0
}

menu .menubar
menu .menubar.files -tearoff 0
.menubar.files add command -label "New" -underline 0 -accelerator "Ctrl+N"
.menubar.files add command -label "Open" -underline 0 -accelerator "Ctrl+O"
.menubar.files add separator
.menubar.files add command -label "Save" -underline 0 -accelerator "Ctrl+S"
.menubar.files add command -label "Save as" -underline 5 -accelerator "Shift+Ctrl+S"
.menubar.files add separator
.menubar.files add command -label "Quit" -underline 0 -accelerator "Ctrl+Q" \
	-command {exit}
.menubar add cascade -label "File" -underline 0 -menu .menubar.files

menu .menubar.view -tearoff 0
.menubar.view add command -label "Thermoview" -underline 0 -accelerator "T"
.menubar add cascade -label "View" -underline 0 -menu .menubar.view

menu .menubar.help -tearoff 0
.menubar.help add command -label "About" -underline 0
.menubar add cascade -label "Help" -underline 0 -menu .menubar.help

. configure -menu .menubar

ttk::labelframe .frmleft -text "Settings"

ttk::label .lblbrshtmprtr -text "Brush Temperature" -underline 0
ttk::entry .txtbrshtmprtr -textvariable brush_thermo \
	-validate all -validatecommand {validate %P}

ttk::label .lblspwnrtmprtr -text "Spawner Temperature" -underline 0
ttk::entry .txtspwnrtmprtr -textvariable spawner_thermo \
	-validate all -validatecommand {validate %P}

ttk::label .lbltl -text "Tool" -underline 3
ttk::combobox .cbotl

ttk::label .lblmtrl -text "Material" -underline 0
ttk::combobox .cbomtrl

ttk::label .lblthrmvw -text "Thermoview" -underline 7
ttk::checkbutton .chkthrmvw

ttk::label .lblsmltnpsd -text "Simulation Paused" -underline 11
ttk::checkbutton .chksmltnpsd

ttk::label .lblsmltnspd -text "Simulation Speed" -underline 8
ttk::scale .sclsmltnspd -from 0 -to 48 -value 24

ttk::labelframe .frmright -text "World"

canvas .world

bind all <Alt-b> {focus .txtbrshtmprtr}
bind all <Alt-i> {.chkthrmvw invoke}
bind all <Alt-l> {focus .cbotl}
bind all <Alt-m> {focus .cbomtrl}
bind all <Alt-o> {focus .sclsmltnspd}
bind all <Alt-p> {.chksmltnpsd invoke}
bind all <Alt-s> {focus .txtspwnrtmprtr}

bind all <Control-q> {exit}

grid .lblbrshtmprtr .txtbrshtmprtr -in .frmleft
grid .lblspwnrtmprtr .txtspwnrtmprtr -in .frmleft
grid .lbltl .cbotl -in .frmleft
grid .lblmtrl .cbomtrl -in .frmleft
grid .lblthrmvw .chkthrmvw -in .frmleft
grid .lblsmltnpsd .chksmltnpsd -in .frmleft
grid .lblsmltnspd .sclsmltnspd -in .frmleft

grid .world -in .frmright

grid .frmleft .frmright
