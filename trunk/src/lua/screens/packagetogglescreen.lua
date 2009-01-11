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

function checktool()
    local mtool = pkg.packager.missingtool()
    if mtool then
        while gui.choicebox(tr("In order to let the installer register the software, the '%s' package needs to be available.\nPlease install this package now and hit continue or press ignore to continue without software registration.", mtool), "Continue", "Ignore") == 1 do
            mtool = pkg.packager.missingtool()
            if not mtool then
                break
            end
        end
    end
    
    pkg.register = not pkg.packager.missingtool()
    if not pkg.register then
        -- Update to generic
        pkg.packager = pkg.packagers["generic"]
        pkg.updatepackager()
    end
end

screen = install.newscreen("Software registration")

function screen:canactivate()
    return pkg.enable and pkg.canregister
end

genpkg = screen:addcheckbox("By enabling this box, the installer will (try to) register the software in the system's package manager. This allows easy removal or upgrading.\n\nNote: When enabled, you need to grant the installer root access later on.", {"Register software"})

screen:addscreenend()

local pkgchoice = screen:addradiobutton("Multiple package managers were found, please choose which one should be used.")
pkgchoice:enable(false)

function screen:activate()
    genpkg:set(1, pkg.register)
end

function genpkg:verify()
    pkg.register = genpkg:get(1)
    
    if pkg.register then
        if utils.mapsize(pkg.packagers) > 2 then
            pkgchoice:enable(true)
            pkgchoice:clear()
            for p in pairs(pkg.packagers) do
                if p ~= "generic" then
                    pkgchoice:add(pkg.packagers[p].name())
                end
            end
        else
            checktool()
        end
    end
    
    return true
end

function pkgchoice:verify()
    local choice = self:get()
    for n, p in pairs(pkg.packagers) do
        if n ~= "generic" and p.name() == choice then
            pkg.packager = p
            break
        end
    end
    pkg.updatepackager()
    checktool()
    return true
end

return screen
