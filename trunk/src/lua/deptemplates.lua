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
--              belong to this this template.
-- full         Recommended usage. Default is true.
-- 
-- New structures should be created with the newtemplate() function

pkg.deptemplates = { }

local function newtemplate(n, d, l, f, no)
    if f == nil then
        f = true
    end
    local t = { name = n, description = d, full = f, notes = no }
    if type(l) == "table" then
        t.libs = l
    else
        t.check = l
    end
    table.insert(pkg.deptemplates, t)
end


-- C library
newtemplate("libc", "C system library.", function(l)
    return string.find(l, "^(libc.so.[%d])")
end, false, "Never create a full dependency for this library!")

-- C++ Library
newtemplate("libstdc++", "C++ system library.", function(l)
    return string.find(l, "^(libstdc%+%+.so.%d)")
end)

-- Core X Libraries (UNDONE)
newtemplate("X-core", "Core libraries from X.", function(l)
    return string.find(l, "libX11.so.%d")
end, false)

