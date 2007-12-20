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
    return nil -- UNDONE
end

function present()
    return os.exitstatus(os.execute("(/sbin/upgradepkg) >/dev/null 2>&1")) == 1
end

function missingtool()
end

function makedesc(src)
    check(os.mkdirrec(src .. "/install"))
    local descfile = check(io.open(src .. "/install/slack-desc", "w"))
    
    local desc, line = "", ""
    local maxlen, lines = 72, 11
    local n = 0
    
    -- Make sure that lines won't be longer than maxlen
    -- UNDONE?
    for char in string.gmatch(pkg.summary .. "\n\n" .. pkg.description, ".") do
        if char == '\n' or n == maxlen then
            if #desc > 0 then
                desc = desc .. '\n' .. line
            else
                desc = line
            end
            line = ""
            n = 0
        end
        if char ~= '\n' then
            line = line .. char
        end
        n = n + 1
    end

    -- Add empty lines when necessary
    local nl = 0
    for line in string.gmatch(desc, "([^\n]*)\n") do
        nl = nl + 1
    end
    
    if nl < lines then
        for n=1,lines - nl do
            desc = desc .. "\n"
        end
    end

    local l = 0
    for line in string.gmatch(desc, "([^\n]*)\n") do
        descfile:write(pkg.name .. ": " .. line .. "\n")
        l = l + 1
        if l >= lines then
            break
        end
    end

    descfile:close()
end

function pkgname()
    return string.format("%s-%s-%s-%s.tgz", pkg.name, pkg.version, pkg.release, os.arch)
end

function canxdg()
    return false
end

function create(src)
    local pkgdir = curdir .. "/slack"
    pkgbindir = string.format("%s/%s", pkgdir, pkg.bindir)
    instfiles = string.format("%s/%s", pkgdir, pkg.getdatadir())

    check(os.mkdirrec(pkgdir))

    -- Create directory structure
    check(os.mkdirrec(instfiles))
    check(os.mkdirrec(pkgbindir))
    utils.moverec(src .. "/files", instfiles)
    utils.moverec(src .. "/bins", pkgbindir)

    -- Make description
    makedesc(pkgdir)
      
    -- Copy XDG desktop files
    copydeskfiles(pkgdir)

    -- Create the package
    checkcmd(OLDG.install.execute, string.format("cd %s && /sbin/makepkg -l y -c n %s/%s", pkgdir, curdir, pkgname()))
    
    -- Move install files back
    utils.moverec(instfiles, src .. "/files")
    utils.moverec(pkgbindir, src .. "/bins")
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
    
    checkcmd(OLDG.install.executeasroot, string.format("/sbin/upgradepkg --reinstall --install-new %s/%s", curdir, pkgname()))
end

function rollback(src)
    if instfiles and os.fileexists(instfiles) then
        -- Move install files back
        utils.moverec(instfiles, src .. "/files")
        utils.moverec(pkgbindir, src .. "/bins")
    end
end
