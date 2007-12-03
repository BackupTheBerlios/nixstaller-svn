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

function check(g, msg, ...)
    if not g then
        -- Special string is used to identify errors thrown by this function.
        -- This is so install.generatepkg() can filter out other errors.
        error({msg, "!check"})
    end
    return g, msg, ...
end

-- f is a function that should execute the command and excepts a second argument
-- for not complaining when something went wrong
function checkcmd(f, cmd)
    if f(cmd, false) ~= 0 then
        error({"Failed to execute package shell command", "!check"})
    end
end

local needsroot
function needroot()
    if needsroot == nil then -- Result will be cached
        -- returns true if top most existing directory has write/read permissions for current user
        local function checkdirs(dir)
            local path = ""
            local ret = false
            for d in string.gmatch(dir, "/[^/]*") do
                path = path .. d
                if not os.fileexists(path) then
                    break
                end
                ret = (os.writeperm(path) and os.readperm(path))
            end
            return ret
        end
        
        needsroot = (install.createpkg or not checkdirs(pkg.destdir) or not checkdirs(pkg.bindir))
    end
    return needsroot
end

function instcmd()
    return (needroot() and install.executeasroot) or install.execute
end

-- -- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
-- OLDG = getfenv(1)
-- local P = {}
-- setmetatable(P, {__index = _G})
-- setfenv(1, P)


function genscript(file, dir)
    -- UNDONE: Check what other vars should be set (GTK, Qt, KDE etc)

    local filename = string.format("%s/%s", dir, string.gsub(file, "/*.+/", ""))
    
    script = check(io.open(filename, "w"))
    check(script:write(string.format([[
#!/bin/sh
LD_LIBRARY_PATH="%s/%s/lib:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH
KDEDIRS=%s/%s
export KDEDIRS
exec %s/%s/%s
]], pkg.destdir, pkg.name, pkg.destdir, pkg.name, pkg.destdir, pkg.name, file)))
    script:close() -- Flush
    os.chmod(filename, "0755")
end

--  UNDONE
function setpermissions(src)
    local dirlist = { "." }
    while #dirlist > 0 do
        local dir = table.remove(dirlist) -- Pop
        local dirpath = string.format("%s/files/%s", src, dir)
        for f in io.dir(dirpath) do
            local path = string.format("%s/%s", dirpath, f)
            if os.isdir(path) then
                table.insert(dirlist, path)
                os.chmod(path, "755")
            else
                os.chmod(path, "644")
            end
        end
    end
    
    -- Binaries
    if pkg.bins then
        for _, b in ipairs(pkg.bins) do
            os.chmod(b, "755")
        end
    end
end

dofile("groups.lua")

package.path = "?.lua"
package.cpath = ""

packagers = { "deb", "pacman", "slack", "rpm", "generic" }

for _, p in ipairs(packagers) do
    require(p)
    pkg.packager = pkg.packager or (_G[p].present() and _G[p])
end

pkg.canregister = pkg.packager ~= generic -- Used by package toggle screen

-- Defaults
pkg.destdir = pkg.packager.getpkgpath() .. "/share"
pkg.bindir = pkg.packager.getpkgpath() .. "/bin"

dofile("package-public.lua")
