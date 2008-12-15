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

-- Called by install.lua for unattended installations

local installing = false
local function verifyinstalling() -- Not really necessary, but there for consistency
    if not installing then
        error("Function called when install is not in progress.")
    end
end


function install.extractfiles()
    verifyinstalling()
    install.extract(function (f, p)
        install.print(string.format("%s (%d%%)\n", f, p))
    end)
end

function install.setstepcount(n)
    verifyinstalling()
end

function install.setstatus(msg)
    verifyinstalling()
end

function install.print(msg)
    verifyinstalling()
    io.write(msg)
    io.flush()
end

function install.execute(cmd, req, path)
    verifyinstalling()
    
    install.print(string.format("\n=============================\nExecute: %s\n\n", cmd))
    local ret = install.executecmd(cmd, function (s)
                                            install.print(s)
                                        end, req, path)
    install.print("=============================\n")
    return ret
end

function install.askrootpw()
    abort([[
This installation requires root (administrator) privileges in order to continue.
Please restart the installer with admin privileges.
]])
end

-- End Public functions
-----------------------------

if os.fileexists("config/run.lua") then
    loadrun("config")
    if UnattendedInit then
        UnattendedInit()
    end
end

if os.fileexists(install.destdir) then
    if not os.readperm(install.destdir) then
        abort("Cannot read destination directory, restart installer with as a user who can (eg. root)")
    elseif not os.writeperm(install.destdir) then
        abort("Cannot write to destination directory, restart installer with as a user who can (eg. root)")
    end
else
    if not utils.mkdirperm(install.destdir) then
        abort("Cannot create destination directory, restart installer with as a user who can (eg. root)")
    end
    os.mkdirrec(install.destdir)
end

os.chdir(install.destdir)

-- Install right away

installing = true

if Install then
    Install()
else
    install.extractfiles()
end

install.print(string.format("\n%s\n", tr("Installation of %s complete!", cfg.appname)))

