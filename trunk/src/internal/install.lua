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

do
    local oldf = install.newscreen
    
    -- This function takes a string argument specifying a function name. It will return a function that creates a new group and
    -- call the specified function from that group.
    local function wrapper(f)
        return function (self, ...)
            local group = self:addgroup()
            return group[f](group, ...)
        end
    end

    function install.newscreen(t) -- Overide function
        local ret = oldf(t)
        
        -- Wrap some functions: this will make widget adding possible directly from calling a function from the installscreen and
        -- without manually creating a group. This is handy because in many cases groups only contain one widget.
        ret.addinput = wrapper("addinput")
        ret.addprogressbar = wrapper("addprogressbar")
        ret.addcheckbox = wrapper("addcheckbox")
        ret.addradiobutton = wrapper("addradiobutton")
        ret.adddirselector = wrapper("adddirselector")
        ret.addcfgmenu = wrapper("addcfgmenu")
        ret.addmenu = wrapper("addmenu")
        ret.addimage = wrapper("addimage")
        ret.addtextfield = wrapper("addtextfield")
        ret.addlabel = wrapper("addlabel")
        
        return ret
    end
end


-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)

function GenerateDefaultScreens()
    package.path = "?.lua"
    package.cpath = ""
    
    OLDG.WelcomeScreen = require "welcomescreen"
    OLDG.InstallScreen = require "installscreen"
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

