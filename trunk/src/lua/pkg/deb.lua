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

local OLDG = _G
module (..., package.seeall)

function getpkgpath()
    return "/usr/local" -- UNDONE?
end

function installedver()
    local cmd = check(io.popen(string.format("dpkg-query -W -f '${VERSION}' %s", pkg.name)))
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
    
    local fname = string.format("%s/prerm", dest)
    local f = check(io.open(fname, "w"))
    
    check(f:write(string.format([[
#!/bin/sh
EXEC=""
(xdg-desktop-menu --version >/dev/null 2>&1) && EXEC="xdg-desktop-menu"
[ -z $EXEC ] && ("%s/xdg-utils/xdg-desktop-menu" --version 2>&1 >/dev/null) && EXEC="%s/xdg-utils/xdg-desktop-menu"
if [ -z "$EXEC" ]; then
    exit 0
fi
]], pkg.getdatadir(), pkg.getdatadir(), pkg.getdatadir())))
    
    for n, _ in pairs(OLDG.install.menuentries) do
        check(f:write(string.format("$EXEC uninstall --novendor %s.desktop\n", n)))
    end
    
    f:close()
    os.chmod(fname, "555")
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
    moverec(src .. "/files", instfiles)
    moverec(src .. "/bins", debbin)

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
    checkcmd(OLDG.install.execute, string.format("dpkg-deb -z1 -b %s/ %s/pkg.deb", debdir, curdir))
    
    -- Move install files back
    checkcmd(OLDG.install.executeasroot, string.format("chown -R %s %s/ %s", os.getenv("USER"), debbin, instfiles))
    moverec(instfiles, src .. "/files")
    moverec(debbin, src .. "/bins")
end

function install(src)
    local locked = OLDG.install.executeasroot("lsof /var/lib/dpkg/lock >/dev/null")
    while locked == 0 do
        if gui.choicebox("Another program seems to be using the DEB database. \
Please close all applications that may use the database (ie Synaptic) \
and hit continue.", "Continue", "Abort") == 2 then
            os.exit(1)
        end
        locked = OLDG.install.executeasroot("lsof /var/lib/dpkg/lock >/dev/null")
    end
    
    checkcmd(OLDG.install.executeasroot, string.format("dpkg -i %s/pkg.deb", curdir))
end

function rollback(src)
    if instfiles and os.fileexists(instfiles) then
        if not os.writeperm(instfiles) then
            checkcmd(OLDG.install.executeasroot, string.format("chown -R %s %s/ %s", os.getenv("USER"), debbin, instfiles))
        end
        -- Move install files back
        moverec(instfiles, src .. "/files")
        moverec(debbin, src .. "/bins")
    end
end
