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

screen = install.newscreen("Heh")

progressbar = screen:addprogressbar("Progress")
statlabel = screen:addlabel("")
output = screen:addtextfield()
output:follow(true)

function screen:activate()
    install.lockscreen(true, true) -- Disable Back and Next buttons
    install.startinstall(function (s)
                            statlabel:set(s)
                         end,
                         function (s)
                            progressbar:set(s)
                         end,
                         function (s)
                            output:add(s)
                         end)
    install.lockscreen(true, false) -- Re-enable Next button, keep Back button locked
end

return screen
