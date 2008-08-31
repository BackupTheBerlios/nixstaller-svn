--     Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)
-- 
--     This file is part of Nixstaller.
-- 
--     This program is free software; you can redistribute it and/or modify it under
--     the terms of the GNU General Public License as published by the Free Software
--     Foundation; either version 2 of the License, or (at your option) any later
--     version. 
-- 
--     This program is distributed in the hope that it will be useful, but WITHOUT ANY
--     WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
--     PARTICULAR PURPOSE. See the GNU General Public License for more details. 
-- 
--     You should have received a copy of the GNU General Public License along with
--     this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
--     St, Fifth Floor, Boston, MA 02110-1301 USA


-- Default dependency templates. New submissions are very welcome...
-- 
-- NOTE: Template searching is order dependent!
-- 
-- Table structure fields:
-- name         Dependency name.
-- description  Dependency description.
-- check        Function that is used to determine if a library is part of this template.
-- libs         As an alternative to 'check', this field can be used to list libraries which
--              belong to this this template. It should contain a table (array) containing strings
--              of the library name, without any starting "lib" and ending ".so.<version> part".
--              So if you want to look for "libfoo.so.<version>" simply specify "foo".
-- full         Recommended usage. Default is true.
-- notes        Notes for this dependency.
-- 
-- New structures should be created with the newtemplate() function

pkg.deptemplates = { }

local function newtemplate(n, d, l, f, no, inst)
    if f == nil then
        f = true
    end
    local t = { name = n, description = d, full = f, notes = no, install = inst }
    if type(l) == "table" then
        t.libs = l
    else
        t.check = l
    end
    table.insert(pkg.deptemplates, t)
end


-- C library
newtemplate("libc", "C system library.", {"c"}, false, "Never create a full dependency for this library!")

-- C++ Library
newtemplate("libstdc++", "C++ system library.", {"stdc++"})

-- Core X Libraries (UNDONE)
newtemplate("X-core", "Core libraries from X.", {"X11"}, false)

-- ATK
newtemplate("ATK", "Accessibility toolkit.", {"atk-1.0"}, false)

-- GLib
newtemplate("GLib2", "Library of useful routines for C programming.", {"glib-2.0", "gio-2.0", "gmodule-2.0", "gobject-2.0", "gthread-2.0"}, false)

-- GTK2
newtemplate("GTK2", "Used to build graphical interfaces.", {"gtk-x11-2.0", "gdk-x11-2.0", "gdk_pixbuf-2.0", "gdk_pixbuf_xlib-2.0", "gthread-2.0"}, false)

-- Pango
newtemplate("Pango", "System for Layout and Rendering of Internationalised Text.", {"pango-1.0", "pangocairo-1.0", "pangoft2-1.0", "pangox-1.0", "pangoxft-1.0"}, false)

-- Cairo
newtemplate("Cairo", "Vector Graphics Library with Cross-Device Output Support.", {"cairo"}, false)
