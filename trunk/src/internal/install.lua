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

-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)

function GenerateDefaultScreens()
    package.path = "?.lua"
    package.cpath = ""
    
    OLDG.WelcomeScreen = require "welcome"
end

function LoadConfig()
    local file = "config/run.lua"
    
    if (os.fileexists(file)) then
        dofile("config/run.lua")
        Init()
    end
end

function AddScreens()
    if (install.screenlist ~= nil and #install.screenlist > 0) then
        for _, s in pairs(install.screenlist) do
            install.addscreen(s)
        end
    else
        install.addscreen(WelcomeScreen)
        -- UNDONE
    end
end

GenerateDefaultScreens()
LoadConfig()
AddScreens()

