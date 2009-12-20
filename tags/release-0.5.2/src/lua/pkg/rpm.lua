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

local OLDG = _G
module (..., package.seeall)

function name()
    return "rpm (eg. Fedora, Mandriva)"
end

function getpkgpath()
    return "/usr/local" -- UNDONE?
end

function present()
    return os.osname == "linux" and (os.execute("(rpm --version) >/dev/null 2>&1") == 0)
end

function missingtool()
    if (os.execute("(rpmbuild --version) >/dev/null 2>&1") ~= 0) then
        return "rpmbuild"
    end
end

function installedver()
    if os.execute(string.format("rpm -q %s >/dev/null 2>&1", pkg.name)) ~= 0 then
        return nil -- Not installed
    end
    local cmd = check(io.popen(string.format("rpm -q --queryformat '%%{VERSION}-%%{RELEASE}' %s", pkg.name)))
    local ver = cmd:read("*a")
    cmd:close()
    return ver
end

function canxdg()
    return true
end

function verifylock()
    local locked = (OLDG.install.executeasroot(string.format("%s/lock /var/lib/rpm/Packages", bindir)) == 0)
    
    if locked then
        return false, [[
Another program seems to be using the RPM database.
Please close all applications that may use this database (eg. Smart or YaST).]]
    end
    
    return true
end

function create(src)
    local specdir = src .. "/SPECS"
    check(os.mkdir(specdir))
    check(os.mkdir(src .. "/RPMS"))
    
    spec = check(io.open(specdir .. "/pkg.spec", "w"))
    check(spec:write(string.format([[
%%define _topdir %s
%%define _tmppath %s/tmp

Prefix: %s/files
Prefix: %s/bins
Name: %s
Version: %s
Release: %s
Packager: %s
Summary: %s
Group: %s
License: %s
URL: %s
AutoReqProv: 0

%%description
%s

%%files
%%defattr(-,root,root,755)
%s/files
%s/bins
]], src, src, src, src, pkg.name, pkg.version, pkg.release, pkg.maintainer, pkg.summary, pkg.grouplist[pkg.group]["rpm"], pkg.license, pkg.url, pkg.description, src, src)))

    local script = xdguninstscript()
    if script then
        check(spec:write("\n%preun\n"))
        check(spec:write(script))
        check(spec:write("exit 0\n")) -- rpm wants clean exit status
    end
    
    spec:close()
    checkcmd(OLDG.install.execute, string.format("rpmbuild -bb %s/pkg.spec", specdir))
end

function install(src)
    local arch = os.arch
    if os.arch == "x86" then
        -- Figure out which arch rpmbuild used (i386, i586 etc)
        for d in io.dir(string.format("%s/RPMS", src)) do
            if string.match(d, "i?86") then
                arch = d
                break
            end
        end
    end
    
    checkcmd(OLDG.install.executeasroot, string.format("rpm --relocate %s/files=%s --relocate %s/bins=%s --force -U %s/RPMS/%s/%s-%s-%s.%s.rpm", src, pkg.getdatadir(), src, pkg.bindir, src, arch, pkg.name, pkg.version, pkg.release, arch))
end

function rollback(src)
end
