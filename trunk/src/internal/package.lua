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

function installxdgentries()
    local function f(t, exec, cmd)
        if not t then
            return
        end
        
        for n, e in pairs(t) do
            local fname = string.format("%s/%s.desktop", curdir, n)
            local f = check(io.open(fname, "w"))
            check(f:write("[Desktop Entry]\n"))
            for i, v in pairs(e) do
                check(f:write(i .. "=" .. v .. "\n"))
            end
            f:close()
            checkcmd(exec, string.format("%s/xdg-utils/%s install --novendor %s", curdir, cmd, fname))
        end
    end
    
    f(pkg.menuentries, instcmd(), "xdg-desktop-menu")
    f(pkg.deskicons, install.execute, "xdg-desktop-icon")
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

packagers = { "deb", "pacman", "slack", "rpm", "generic" }

for _, p in ipairs(packagers) do
    require(p)
    pkg.packager = pkg.packager or (P[p].present() and P[p])
end

-- Check which package system user has
-- packager = (deb.present() and deb) or (pacman.present() and pacman) or (slack.present() and slack) or (rpm.present() and rpm) or generic
pkg.canregister = pkg.packager ~= generic -- Used by package toggle screen
-- pkg.missingtool = (pkg.canregister and ()) or nil

-- Defaults
OLDG.pkg.destdir = pkg.packager.getpkgpath() .. "/share"
OLDG.pkg.bindir = pkg.packager.getpkgpath() .. "/bin"

function pkg.gendesktopentry(b, i, c)
    -- Defaults
    t = {}
    t.Name = cfg.appname
    t.Type = "Application"
    t.Encoding = "UTF-8"
    t.Exec = pkg.bindir .. "/" .. string.gsub(b, "/*.+/", "")
    t.Icon = i
    t.Catagory = c
    return t
end

function pkg.addmenuentry(n, t)
    if not t.Exec then
        error("Need to specify executable for desktopentry")
--     elseif not t.Icon then
--         error("Need to specify icon for desktopentry")
    elseif not t.Catagory then
        error("Need to specify catagory for desktopentry")
    end
    
    pkg.menuentries = pkg.menuentries or {}
    pkg.menuentries[n] = t
end

function pkg.adddesktopicon(n, t)
    if not t.Exec then
        error("Need to specify executable for desktopentry")
--     elseif not t.Icon then
--         error("Need to specify icon for desktopentry")
    end
    
    pkg.deskicons = pkg.deskicons or {}
    pkg.deskicons[n] = t
end

-- Called from Install()
function install.generatepkg()
    if not install.createpkg then
        pkg.packager = generic
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
                version = pkg.packager.installedver()
            
                if not version and os.fileexists(getdestdir()) then
                    version = "unknown"
                end
                
                if version then
                    local msg
                    local myver = pkg.version .. "-" .. pkg.release
                    -- If type is string we don't know the version
                    if version == "unknown" or version == myver then
                        msg = "Package is already installed. Do you want to replace it?"
                    else
                        msg = string.format("Version %s is already installed, you're trying to install version %s.\nDo you want to replace the installed package?", version, myver)
                    end
                    
                    if msg and not gui.yesnobox(msg) then
                        os.exit(1)
                    end
                end
            end
            
            pkg.packager.create(dir)
            pkg.packager.install(dir)
            installxdgentries()
        end

        if pkg.packager == generic then
            f()
        else
            local g, msg2 = pcall(f)
            if not g then
                if type(msg2) == "table" and msg2[2] == "!check" then
                    pkg.packager.rollback(dir)
                    pkg.packager = generic
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
