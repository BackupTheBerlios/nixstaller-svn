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

local OLDG = _G
module (..., package.seeall)

function name()
    return "dpkg (eg. Ubuntu, Debian)"
end

function getpkgpath()
    return "/usr/local" -- UNDONE?
end

function installedver()
    local cmd = check(io.popen(string.format("(dpkg-query -W -f '${VERSION}' %s) 2>/dev/null", pkg.name)))
    local ver = cmd:read("*a")
    cmd:close()
    return (ver ~= "" and ver) or nil
end

function pkgdesc()
    -- Each line should start with a space. Empty lines need a single dot.
    -- No more than one empty line at the end is allowed (not checked atm, just filling in dots at the end seems to work)
    
    local ret = ""
    for line in string.gmatch(pkg.description, "([^\n]*)\n") do
        if #line == 0 then -- Empty line?
            line = "."
        end
        ret = ret .. " " .. line .. "\n"
    end
    
    return ret
end

function present()
    return os.execute("(dpkg --version && dpkg-deb --version && dpkg-query --version) >/dev/null 2>&1") == 0
end

function missingtool()
end

function canxdg()
    return true
end

function genxdgscript(dest)
    if not OLDG.install.menuentries then
        return
    end
    
    local script = xdguninstscript()
    
    if not script then
        return
    end
    
    local fname = string.format("%s/prerm", dest)
    local f = check(io.open(fname, "w"))
    
    check(f:write("#!/bin/sh\n"))
    check(f:write(script))
    check(f:write("exit 0\n")) -- dpkg wants clean exit status
    
    f:close()
    os.chmod(fname, "555")
end

function verifylock()
    local locked = checklock("/var/lib/dpkg/lock", false)
    
    if locked then
        return false, [[
Another program seems to be using the dpkg database.
Please close all applications that may use the database (ie Synaptic).]]
    end
    
    return true
end

function create(src)
    local debdir = curdir .. "/deb"
    debbin = string.format("%s/%s", debdir, pkg.bindir)
    instfiles = string.format("%s/%s", debdir, pkg.getdatadir())

    -- Set up deb work directory
    check(os.mkdirrec(debdir .. "/DEBIAN"))

    -- Create directory structure
    check(os.mkdirrec(instfiles))
    check(os.mkdirrec(debbin))
    checkmvrec(src .. "/files", instfiles)
    checkmvrec(src .. "/bins", debbin)

    local size = math.ceil((pkgsize(instfiles) + pkgsize(debbin)) / 1024)
    
    -- Generate control file
    local control = check(io.open(debdir .. "/DEBIAN/control", "w"))
    check(control:write(string.format([[
Package: %s
Version: %s-%s
Architecture: all
Maintainer: %s
Priority: optional
Installed-Size: %d
Section: %s
Description: %s
%s
]], pkg.name, pkg.version, pkg.release, pkg.maintainer, size, pkg.grouplist[pkg.group]["deb"], pkg.summary, pkgdesc())))
    control:close() -- important, otherwise data may still be buffered and not in the file

    genxdgscript(debdir .. "/DEBIAN")
    
    -- Fix permissions
    checkcmd(OLDG.install.executeasroot, string.format("chown -R root %s/ %s", debbin, instfiles))

    -- Create the package (use low compression level for extra speed)
    checkcmd(OLDG.install.executeasroot, string.format("dpkg-deb -z1 -b %s/ %s/pkg.deb", debdir, curdir))
    
    -- Move install files back
    checkcmd(OLDG.install.executeasroot, string.format("chown -R %s %s/ %s", os.getenv("USER"), debbin, instfiles))
    checkmvrec(instfiles, src .. "/files")
    checkmvrec(debbin, src .. "/bins")
end

function install(src)
    checkcmd(OLDG.install.executeasroot, string.format("dpkg -i %s/pkg.deb", curdir))
end

function rollback(src)
    if instfiles and os.fileexists(instfiles) then
        if not os.writeperm(instfiles) then
            checkcmd(OLDG.install.executeasroot, string.format("chown -R %s %s/ %s", os.getenv("USER"), debbin, instfiles))
        end
        -- Move install files back
        checkmvrec(instfiles, src .. "/files")
        checkmvrec(debbin, src .. "/bins")
    end
end
