--  ***************************************************************************
--  *   Copyright (C) 2009 by Rick Helmus / InternetNightmare                 *
--  *   rhelmus_AT_gmail.com / internetnightmare_AT_gmail.com                 *
--  *                                                                         *
--  *   This program is free software; you can redistribute it and/or modify  *
--  *   it under the terms of the GNU General Public License as published by  *
--  *   the Free Software Foundation; either version 2 of the License, or     *
--  *   (at your option) any later version.                                   *
--  *                                                                         *
--  *   This program is distributed in the hope that it will be useful,       *
--  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
--  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
--  *   GNU General Public License for more details.                          *
--  *                                                                         *
--  *   You should have received a copy of the GNU General Public License     *
--  *   along with this program; if not, write to the                         *
--  *   Free Software Foundation, Inc.,                                       *
--  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
--  ***************************************************************************

local validOSs = { "linux", "freebsd", "netbsd", "openbsd", "sunos" }
local validCPUArchs = { "x86", "x86_64" }

function getFriendlyOS(os)
    local osmap = {
        linux = "Linux",
        freebsd = "FreeBSD",
        netbsd = "NetBSD",
        openbsd = "OpenBSD",
        sunos = "Solaris"
    }

    return osmap[os] or os
end

function getFriendlyArch(arch)
    return arch
end

function getFriendlyFilesName(dir)
    if dir == "files_all" then
        return "Generic Files"
    elseif dir == "files_extra" then
        return "Extra (runtime) Files"
    end
    
    for _, os in ipairs(validOSs) do
        if dir == string.format("files_%s_all", os) then
            return string.format("Files for %s", getFriendlyOS(os))
        end
        
        for _, arch in ipairs(validCPUArchs) do
            if dir == string.format("files_%s_%s", os, arch) then
                return string.format("Files for %s (%s only)",
                                     getFriendlyOS(os), getFriendlyArch(arch))
            end
        end
    end

    for _, arch in ipairs(validCPUArchs) do
        if dir == string.format("files_all_%s", arch) then
            return string.format("Generic Files (%s only)", getFriendlyArch(arch))
        end
    end
end

function getFilesOSs()
    return validOSs
end

function getFilesCPUArchs()
    return validCPUArchs
end

