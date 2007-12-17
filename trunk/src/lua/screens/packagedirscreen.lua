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

module (..., package.seeall)

screen = install.newscreen("Software destination") -- UNDONE? (title)

pkgdest = screen:adddirselector("In this field you can select the destination directory for the software's data. The files are installed in a subdirectory below the specified directory.\n\nNote: if you don't have write access to this directory you need to grant the installer root access later on.\n\nIf unsure, just leave it as it is.")

function pkgdest:verify()
    if not pkgdest:get() then
        gui.msgbox("Please enter a valid path.")
        return false
    end
    pkg.destdir = pkgdest:get()
    bindest:set(string.gsub(pkg.destdir, "/[%w]*$", "/bin"))
    return true
end

screen:addscreenend()

bindest = screen:adddirselector("In this field you can select the destination directory for the executables. The files are directly installed to this directory.\n\nNote: if you don't have write access to this directory you need to grant the installer root access later on.\n\nWhen unsure, just leave it as it is.")

function bindest:verify()
    if not bindest:get() then
        gui.msgbox("Please enter a valid path.")
        return false
    end
    pkg.bindir = bindest:get()
    return true
end

function screen:activate()
    pkgdest:set(pkg.destdir)
    bindest:set(pkg.bindir)
end

return screen
