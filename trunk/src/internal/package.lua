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

-- Prefix in <prefix> directory (ie in /usr/local)
pkgprefix = string.format("share/%s", pkg.name)

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
    print("Executing:", cmd)
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

function getdestdir()
    return pkg.destdir .. "/" .. pkg.name
end

-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)


function genscript(file, dir)
    -- UNDONE: Check what other vars should be set (GTK, Qt, KDE etc)

    local fullpkgpath = string.format("%s/%s", packager.getpkgpath(), pkgprefix)
    local filename = string.format("%s/%s", dir, string.gsub(file, "/*.+/", ""))
    
    script = check(io.open(filename, "w"))
    check(script:write(string.format([[
#!/bin/sh
LD_LIBRARY_PATH="%s/files/lib:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH
exec %s/%s
]], fullpkgpath, fullpkgpath, file)))
    script:close() -- Flush
    os.chmod(filename, "0755")
end

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
require "generic"
require "deb"
require "rpm"

-- Check which package system user has
packager = (deb.present() and deb) or (rpm.present() and rpm) or generic

OLDG.pkg.destdir = packager.getpkgpath() .. "/share"
OLDG.pkg.bindir = packager.getpkgpath() .. "/bin"


-- Called from Install()
function install.generatepkg()
    if not install.createpkg then
        packager = generic
    end

    local success, msg = pcall(function ()
        local dir = curdir .. "/pkg"
        
        check(os.mkdirrec(dir .. "/bins"))
        if pkg.bins then
            install.print("Generating executable scripts\n")
            for _, f in ipairs(pkg.bins) do
                genscript(f, dir .. "/bins")
            end
        end
    
        install.setstatus("Installing package")
        
        setpermissions(dir) -- UNDONE
        
        local version
        
        local f = function()
            if not version then
                version = packager.installedver()
            
                if version then
                    local msg
                    -- If type is string we don't know the version
                    if type(version) == "string" or version == pkg.version then
                        msg = "Package is already installed. Do you want to replace it?"
                    else
                        msg = string.format("Version %s is already installed, you're trying to install version %s.\nDo you want to replace the installed package?", version, pkg.version)
                    end
                    
                    if msg and not gui.yesnobox(msg) then
                        os.exit(1)
                    end
                end
            end
            
            packager.create(dir)
            packager.install(dir)
        end

        if packager == generic then
            f()
        else
            local g, msg2 = pcall(f)
            if not g then
                if type(msg2) == "table" and msg2[2] == "!check" then
                    packager.rollback(dir)
                    packager = generic
                    install.print("WARNING: Failed to create system native package.\nReverting to generic package.")
                    f()
                else
                    error((type(msg2) == "table" and msg2[1]) or msg2, 0) -- Rethrow
                end
            end
        end
    end)
    
    if not success then
        if type(msg) == "table" then
            msg = "Failed to create package: " .. msg[1]
        end
        error(msg, 0) -- Rethrow (use level '0' to avoid adding extra info to msg)
    end
end
