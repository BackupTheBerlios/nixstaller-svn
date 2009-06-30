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

-- Check which file to use
function checkfile()
    filename = string.format("%s/lang/%s/finish", internal.configdir, install.getlang())
    
    if not os.fileexists(filename) then
        filename = string.format("%s/finish", internal.configdir)
    end
    
    if not os.fileexists(filename) then
        filename = nil
    end
end

screen = install.newscreen("Please read the following text")
group = screen:addgroup()

text = group:addtextfield()

function screen:canactivate()
    checkfile()
    return filename ~= nil
end

function screen:activate()
    text:clear()
    text:load(filename)
end

return screen
