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

screen = install.newscreen("Installation summary")

function screen:canactivate()
    return pkg.enable
end

text = screen:addtextfield("The software has been installed to this system.\nAn installation summary is shown below.")

function screen:activate()
    text:clear()
    text:add(string.format([[
GENERAL INFO
Name            %s
Version         %s-%s
Maintainer      %s
]], cfg.appname, pkg.version, pkg.release, pkg.maintainer))

    if pkg.url then
        text:add(string.format("URL             %s\n", pkg.url))
    end
    
    local uninstmsg
    if pkg.register and pkg.canregister and not pkg.regfailed then
        text:add("Registrated    Yes\n")
        uninstmsg = string.format("Software can be uninstalled via system's package manager.")
    else
        text:add("Registrated     No")
        if pkg.regfailed then
            text:add(" (Failed)")
        end
        text:add("\n")
        uninstmsg = string.format("Software can be uninstalled by running the following script:\n%s/uninstall-%s", pkg.getbindir(), pkg.name)
    end
    
    text:add(string.format("\nUNINSTALLATION\n%s\n", uninstmsg))

    if pkg.bins then
        text:add("\n\nINSTALLED EXECUTABLES\n")
        for _, v in ipairs(pkg.bins) do
            text:add(string.format("%s/%s\n", pkg.getbindir(), v))
        end
    end
    
    text:add("\n\nINSTALLED DATA FILES\n")
    utils.recursivedir(pkg.getdatadir(), function (f)
        text:add(f .. "\n")
    end)
end

return screen
