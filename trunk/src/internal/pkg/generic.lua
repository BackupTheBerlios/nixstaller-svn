--     Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)
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

local OLDG = _G
module (..., package.seeall)

function getpkgpath()
    return "/usr/local" -- Seems like a good default prefix on many systems
end

function installedver()
    if os.fileexists(getdestdir()) then
        return "unknown" -- No way to get version (yet)
    end
    return nil
end

function create(src)
end

function install(src)
    -- UNDONE: Check for conflicting files/directories
    checkcmd(instcmd(), string.format("mkdir -p %s/ %s/ && cp -R %s/files/* %s/ && cp -R %s/bins/* %s", getdestdir(), pkg.bindir, src, getdestdir(), src, pkg.bindir))
end
