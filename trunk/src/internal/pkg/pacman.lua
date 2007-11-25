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
    return "/usr" -- UNDONE?
end

function installedver()
    local cmd = check(io.popen(string.format("pacman -Qi %s | grep Version 2>/dev/null", pkg.name, pkg.name)))
    local ver = cmd:read("*a")
    cmd:close()
    
    if not ver or ver == "" then
        return nil
    end
    
    ver = string.gsub(ver, "Version.*: ", "")
    
    return (ver ~= "" and ver) or nil
end

function missingtool()
end

function present()
    -- UNDONE: Works?
    return os.execute("(pacman --version) >/dev/null 2>&1") == 2
end

function genfilelist(src)
    local flist = check(io.open(src .. "/.FILELIST", "w"))
    recursivedir(src, function (_, d)
                         if d == ".FILELIST" then
                             return
                         end
                         check(flist:write(d .. "\n"))
                      end)
    flist:close()
end

function create(src)
    local pkgdir = curdir .. "/pac"
    pkgbindir = string.format("%s/%s", pkgdir, pkg.bindir)
    instfiles = string.format("%s/%s/%s", pkgdir, pkg.destdir, pkg.name)

    check(os.mkdirrec(pkgdir))

    -- Create directory structure
    check(os.mkdirrec(instfiles))
    check(os.mkdirrec(pkgbindir))
    moverec(src .. "/files", instfiles)
    moverec(src .. "/bins", pkgbindir)

    genfilelist(pkgdir)

    local size = pkgsize(instfiles) + pkgsize(pkgbindir)
    
    -- Generate PKGINFO file
    local pkginfo = check(io.open(pkgdir .. "/.PKGINFO", "w"))
    check(pkginfo:write(string.format([[
pkgname = %s
pkgver = %s-%s
packager = %s
url = %s
builddate = %s
size = %d
pkgdesc = %s
]], pkg.name, pkg.version, pkg.release, pkg.maintainer, pkg.url, os.date(), size, pkg.summary)))
    pkginfo:close() -- important, otherwise data may still be buffered and not in the file
    
    -- Create the package
    checkcmd(OLDG.install.execute, string.format("tar czf %s/pkg.tar.gz -C %s/ --owner=root --group=root .", curdir, pkgdir))
    
    -- Move install files back
    moverec(instfiles, src .. "/files")
    moverec(pkgbindir, src .. "/bins")
end

function install(src)
    -- UNDONE
--     local locked = OLDG.install.executeasroot("lsof /var/lib/dpkg/lock >/dev/null")
--     while locked == 0 do
--         if gui.choicebox("Another program seems to be using the DEB database. \
-- Please close all applications that may use the database (ie Synaptic) \
-- and hit continue.", "Continue", "Abort") == 2 then
--             os.exit(1)
--         end
--         locked = OLDG.install.executeasroot("lsof /var/lib/dpkg/lock >/dev/null")
--     end
    
    checkcmd(OLDG.install.executeasroot, string.format("pacman --upgrade %s/pkg.tar.gz", curdir))
end

function rollback(src)
    if instfiles and os.fileexists(instfiles) then
        -- Move install files back
        moverec(instfiles, src .. "/files")
        moverec(pkgbindir, src .. "/bins")
    end
end
