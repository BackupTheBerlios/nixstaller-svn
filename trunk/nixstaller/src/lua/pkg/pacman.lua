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
    return "pacman (eg. Arch Linux)"
end

function getpkgpath()
    return "/usr" -- UNDONE?
end

function installedver()
    local cmd = check(io.popen(string.format("(pacman -Qi %s | grep Version) 2>/dev/null", pkg.name, pkg.name)))
    local ver = cmd:read("*a")
    cmd:close()
    
    if not ver or ver == "" then
        return nil
    end
    
    ver = string.gsub(ver, "Version.*: ", "")
    ver = string.gsub(ver, "\n$", "")
    
    return (ver ~= "" and ver) or nil
end

function missingtool()
end

function present()
    -- Extra check, incase a pacman game is launched ... :-)
    if not os.isdir("/var/lib/pacman") then
        return false
    end
    return os.exitstatus(os.execute("(pacman --version) >/dev/null 2>&1")) == 2
end

function genfilelist(src)
    local flist = check(io.open(src .. "/.FILELIST", "w"))
    utils.recursivedir(src, function (_, d)
                         if d == ".FILELIST" then
                             return
                         end
                         check(flist:write(d .. "\n"))
                      end)
    flist:close()
end

function pkgname()
    return string.format("%s-%s-%s-%s.pkg.tar.gz", pkg.name, pkg.version, pkg.release, os.arch)
end

function canxdg()
    return false
end

function verifylock()
    if os.fileexists("/tmp/pacman.lck") then
        return false, "Another program seems to be using pacman. Please close this program."
    end
    return true
end

function create(src)
    local pkgdir = gettmpinterndir("pac")
    pkgbindir = string.format("%s/%s", pkgdir, pkg.bindir)
    instfiles = string.format("%s/%s", pkgdir, pkg.getdatadir())

    check(os.mkdirrec(pkgdir))

    -- Create directory structure
    check(os.mkdirrec(instfiles))
    check(os.mkdirrec(pkgbindir))
    checkmvrec(src .. "/files", instfiles)
    checkmvrec(src .. "/bins", pkgbindir)

    -- Copy XDG desktop files
    copydeskfiles(pkgdir)

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
arch = %s
]], pkg.name, pkg.version, pkg.release, pkg.maintainer, pkg.url, os.date("%a %b %d %H:%M:%S %Y"), size, pkg.summary, os.arch)))
    pkginfo:close() -- important, otherwise data may still be buffered and not in the file
    
    -- Create the package
    checkcmd(OLDG.install.execute, string.format("cd %s && tar czf %s --owner=root --group=root * .FILELIST .PKGINFO", pkgdir, gettmpinterndir(pkgname())))
    
    -- Move install files back
    checkmvrec(instfiles, src .. "/files")
    checkmvrec(pkgbindir, src .. "/bins")
end

function install(src)
    checkcmd(OLDG.install.executeasroot, string.format("pacman --force --upgrade %s", gettmpinterndir(pkgname())))
end

function rollback(src)
    if instfiles and os.fileexists(instfiles) then
        -- Move install files back
        checkmvrec(instfiles, src .. "/files")
        checkmvrec(pkgbindir, src .. "/bins")
    end
end
