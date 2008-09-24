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
-- post         Function that is called after the dependency is generated. Can be used for example to
--              copy additional files.
-- install      String that contains Lua code used in the Install() function inside the generated config.lua.
--              Default is to call self:CopyFiles().
-- require      As the 'install' field, but for the Require() function. Default is to simply return false.
-- compat       As the 'install' field, but for the HandleCompat() function. Default is to simply return false.
--
-- New structures should be created with the newtemplate() function

pkg.deptemplates = { }

function newtemplate(t)
    local function default(field, val)
        if t[field] == nil then
            t[field] = val
        end
    end
    
    default("full", true)
    default("install", "    self:CopyFiles()")
    default("require", "    return false")
    default("compat", "    return false")
    
    table.insert(pkg.deptemplates, t)
end


-- C library
newtemplate{
name = "libc",
description = "C system library.",
libs = { "c" },
full = false,
notes = "Never create a full dependency for this library!"
}

-- C++ Library
newtemplate{
name = "libstdc++",
description = "C++ system library.",
libs = { "stdc++" }
}

-- Core X Libraries (UNDONE)
newtemplate{
name = "X-core",
description = "Core libraries from X.",
libs = { "X11" },
full = false
}

-- ATK
newtemplate{
name = "ATK",
description = "Accessibility toolkit.",
libs = { "atk-1.0" },
full = false
}

-- GLib
newtemplate{
name = "GLib2",
description = "Library of useful routines for C programming.",
libs = { "glib-2.0", "gio-2.0", "gmodule-2.0", "gobject-2.0", "gthread-2.0" },
full = false
}

-- Cairo
newtemplate{
name = "Cairo",
description = "Vector Graphics Library with Cross-Device Output Support.",
libs = { "cairo" },
full = false
}


-- Bigger templates, split in other files.
dofile(ndir .. "/src/lua/deptemplates/gtk.lua")
dofile(ndir .. "/src/lua/deptemplates/pango.lua")
