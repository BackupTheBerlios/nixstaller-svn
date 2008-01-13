--     Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)
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

screen = install.newscreen("Software registration")

function screen:canactivate()
    return pkg.enable and pkg.canregister
end

genpkg = screen:addcheckbox("By enabling this box, the installer will (try to) register the software in the system's package manager. This allows easy removal or upgrading.\n\nNote: When enabled, you need to grant the installer root access later on.\n\nWhen unsure, just leave it enabled.", {"Register software"})

function screen:activate()
    genpkg:set(1, pkg.register)
end

function genpkg:verify()
    pkg.register = genpkg:get(1)
    
    if (pkg.register) then
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
    end
end

return screen
