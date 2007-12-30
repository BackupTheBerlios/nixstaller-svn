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

function pkg.addbinenv(var, val, bin)
    bin = bin or "All"
    pkg.binenv = pkg.binenv or {}
    pkg.binenv[var] = pkg.binenv[var] or {}
    pkg.binenv[var][bin] = val
end

function pkg.addbinopts(bin, opts)
    pkg.binopts = pkg.binopts or {}
    pkg.binopts[bin] = opts
end

function pkg.getdatadir(f)
    return pkg.destdir .. "/" .. pkg.name .. ((f and ("/" .. f)) or "")
end

function pkg.getbindir(f)
    return pkg.bindir .. ((f and ("/" .. string.gsub(f, "/*.+/", ""))) or "")
end

local needsroot
function pkg.needroot()
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
        
        needsroot = (pkg.register or not checkdirs(pkg.destdir) or not checkdirs(pkg.bindir))
    end
    return needsroot
end

function pkg.setpermissions()
    local prefix = curdir .. "/pkg/files"
    local dirlist = { "." }
    while #dirlist > 0 do
        local dir = table.remove(dirlist) -- Pop
        local dirpath = prefix .. "/dir"
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
            os.chmod(prefix .. "/" .. b, "755")
        end
    end
    
    -- XDG utilities
    for f in io.dir(prefix .. "/xdg-utils") do
        os.chmod(prefix .. "/xdg-utils/" .. f, "755")
    end
end

function install.generatepkg()
    if not pkg.enable then
        error("Called generatepkg when pkg.enable is false.")
    end
    
    if not pkg.register then
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
    
        copyxdgutils(dir .. "/files")
        
        install.setstatus("Installing package")
        
        local version
        
        local f = function()
            if not version then
                version = pkg.packager.installedver()
            
                if not version and os.fileexists(pkg.getdatadir()) then
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
        
        if pkg.packager.canxdg() then
            -- This should be done after the package is installed, because if a package is replaced
            -- it may uninstall any entries.
            for n, _ in pairs(install.menuentries) do
                instxdgentries(needroot(), xdgentryfname(n))
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