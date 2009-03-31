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

screen = install.newscreen("Please select a language")

menu = screen:addmenu("")
for l, v in pairs(install.langinfo) do
    menu:add(v.name)
end

function menu:verify()
    local choice = menu:get()
    for l, v in pairs(install.langinfo) do
        if v.name == choice then
            install.setlang(l)
            break
        end
    end
    return true
end

function screen:canactivate()
    return not install.didautolang and utils.mapsize(install.langinfo) > 1
end

function screen:activate()
    for l, v in pairs(install.langinfo) do
        if l == install.getlang() then
            menu:set(v.name)
            break
        end
    end
end

return screen
