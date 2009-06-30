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

-- Called by install.lua for attended installations

local oldf = install.newscreen
    
function install.newscreen(t) -- Overide function
    local ret = oldf(t)
    local wrapfuncs = { "addinput", "addcheckbox", "addradiobutton", "adddirselector", "addcfgmenu", "addmenu", "addimage",
                        "addprogressbar", "addtextfield", "addlabel" }
    
    -- Create wrappers for widget creation functions from luagroups: the functions given in 'wrapfuncs'
    -- will be added to the installscreen. These will create a new luagroup and call the function
    -- with the same name from that group.
    for i,v in pairs(wrapfuncs) do
        ret[v] = function(self, ...)
                    local group = self:addgroup()
                    return group[v](group, ...)
                    end
    end

    return ret
end

-- Initialize default screens
LangScreen = require "langscreen"
OLDG.WelcomeScreen = require "welcomescreen"
OLDG.LicenseScreen = require "licensescreen"
OLDG.SelectDirScreen = require "selectdirscreen"
OLDG.PackageDirScreen = require "packagedirscreen"
OLDG.PackageToggleScreen = require "packagetogglescreen"
OLDG.InstallScreen = require "installscreen"
OLDG.SummaryScreen = require "summaryscreen"
OLDG.FinishScreen = require "finishscreen"
OLDG.install.screenlist = { WelcomeScreen, LicenseScreen, SelectDirScreen, InstallScreen, FinishScreen }

if os.fileexists(internal.configdir .. "/run.lua") then
    loadrun(internal.configdir)
    if Init then
        Init()
    end
end

-- Add any screens
internal.addscreen(LangScreen)

if (install.screenlist ~= nil and #install.screenlist > 0) then
    for _, s in ipairs(install.screenlist) do
        internal.addscreen(s)
    end
else
    internal.addscreen(WelcomeScreen)
    internal.addscreen(LicenseScreen)
    internal.addscreen(SelectDirScreen)
    internal.addscreen(InstallScreen)
    internal.addscreen(FinishScreen)
end
