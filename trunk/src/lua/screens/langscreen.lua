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

menu = screen:addmenu("", cfg.languages or {})

function menu:verify()
    install.setlang(menu:get())
    return true
end

function screen:canactivate()
    return #cfg.languages > 1
end

function screen:activate()
    for i, v in ipairs(cfg.languages) do
        if v == install.getlang() then
            menu:set(i)
            break
        end
    end
end

return screen
