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
    
    install.setstatus("Extracting files")
    
    install.extract(function (f, p)
        install.print(string.format("%s (%d%%)\n", f, p))
    end)
end

function install.setstepcount(n)
    verifyinstalling()
end

function install.setstatus(msg)
    verifyinstalling()
    
    install.print(string.format([[

**********************************
%s
**********************************
]], msg))
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

function install.executeasroot(cmd, req, path)
    install.askrootpw() -- Aborts if not running as root
    return install.execute(cmd, req, path)
end

function install.askrootpw()
    if os.geteuid() ~= 0 then
        abort([[
This installation requires root (administrator) privileges in order to continue.
Please restart the installer with admin privileges.
]])
    end
end

-- End Public functions
-----------------------------

if not utils.emptytable(cfg.unopts) and not utils.emptytable(args) then
    local short, long = "", { }
    for n, a in pairs(cfg.unopts) do
        if a.short then
            short = short .. a.short
            if a.opttype then
                short = short .. ":"
            end
        end
        table.insert(long, { n, (a.opttype ~= nil)})
    end
    
    -- args is set in unattended.cpp
    local opts = getopt(args, short, long, true)
    
    for _, o in ipairs(opts) do
        if cfg.unopts[o.name] then
            if cfg.unopts[o.name].opttype == "string" then
                cfg.unopts[o.name].value = o.val
            elseif cfg.unopts[o.name].opttype == "list" then
                cfg.unopts[o.name].value = optlisttotab(o.val)
            else
                cfg.unopts[o.name].value = true
            end
        end
    end
end

-- License options
local licensefname = string.format("%s/config/lang/%s/license", curdir, install.getlang())
if not os.fileexists(licensefname) then
    licensefname = string.format("%s/config/license", curdir)
end
    
local licensef = io.open(licensefname, "r")
if licensef then
    if haveunopt("show-license") then
        print("\nLicense Agreement:\n")
        print(licensef:read("*a"))
        licensef:close()
        os.exit(0)
    end
    
    if not haveunopt("accept-license") then
        licensef:close()
        abort([[
You need to accept to license agreement in order to install this software. Use the --show-license option to view it and --accept-license to accept the license.]])
    end
    
    licensef:close()
end

-- Handle args set by pkg.addpkgunopts()
if haveunopt("destdir") then
    install.destdir = cfg.unopts["destdir"].value
end
if haveunopt("datadir") then
    pkg.destdir = cfg.unopts["datadir"].value
    pkg.setdestdir = true
end
if haveunopt("bindir") then
    pkg.bindir = cfg.unopts["bindir"].value
    pkg.setbindir = true
end
if haveunopt("register") then
    pkg.register = true
end
if haveunopt("no-register") then
    pkg.register = false
end
checkunpkgman()

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

install.print(string.format("\n%s\n\n", tr("Installation of %s complete!", cfg.appname)))

if pkg.enable then
    -- Display installation summary
    install.print(string.format([[
GENERAL INFO
Name:           %s
Version:        %s-%s
Maintainer:     %s
]], cfg.appname, pkg.version, pkg.release, pkg.maintainer))
    
    if pkg.url then
        install.print(string.format("URL:            %s", pkg.url))
    end
    
    local uninstmsg
    if pkg.register and pkg.canregister then
        uninstmsg = string.format("Software can be uninstalled via system's package manager.")
    else
        uninstmsg = string.format("Software can be uninstalled by running the following script:\n%s/uninstall-%s", pkg.getbindir(), pkg.name)
    end
    
    install.print(string.format("\n\nUNINSTALLATION\n%s\n", uninstmsg))
    
    install.print("\n\nINSTALLED EXECUTABLES\n")
    for _, v in ipairs(pkg.bins) do
        install.print(string.format("%s/%s\n", pkg.getbindir(), v))
    end
    
    install.print("\n\nINSTALLED DATA FILES\n")
    install.print(string.format("Inside directory %s\n\n", pkg.getdatadir()))
end
