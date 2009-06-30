--     Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)
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

screen = install.newscreen("Select destination directory")

dir = screen:adddirselector()

function screen:activate()
    dir:set(install.destdir)
end

function dir:datachanged()
    install.destdir = dir:get()
end

function dir:verify()
    -- Following function will check the destination directory:
    -- 1) If the directory exists, make sure it can be read and warn the user if it's not writable and needs root access.
    -- 2) If the directory does not exist, ask the user if it should be created.
    return internal.verifydestdir()
end

return screen
