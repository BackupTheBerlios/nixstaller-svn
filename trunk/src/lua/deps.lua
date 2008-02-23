--     Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)
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


depclass = { }
depclass.__index = depclass

function depclass:CopyFiles()
    utils.recursivedir(pkg.getdepdir(self), function (f, rf)
        local dest = string.format("%s/%s", install.getpkgdir(), rf)
        if os.isdir(f) then
            check(os.mkdirrec(dest))
        else
            check(os.copy(f, dest))
        end
    end)
end

function depclass:Install()
    self:CopyFiles()
end
-- UNDONE


function getmissinglibs(bin)
    local neededlibs = { }
    local map = maplibs(bin)
    
    if not map then
        return
    end
    
    for l, v in pairs(map) do
        if not v then
            neededlibs[l] = true
        end
    end
    
    return neededlibs
end

function getdepsfromlibs(libs) -- libs is a map, not a list
    local ret = { }
    for l in pairs(libs) do
        local found = false
        for _, d in pairs(pkg.depmap) do
            for _, dl in ipairs(d.libs) do
                if utils.basename(dl) == l then
                    ret[d] = true
                    found = true
                    break
                end
            end
            if found then
                break
            end
        end
    end
    return ret
end

function extractdeps()
    for n in pairs(pkg.depmap) do
        local src = string.format("%s/deps/%s", curdir, n)
        local dest = string.format("%s/files", src)
        os.mkdirrec(dest)
        
        local archives = { }
        local function checkarch(a)
            if os.fileexists(a) then
                table.insert(archives, a)
            end
        end
        
        checkarch(string.format("%s/files_all", src))
        checkarch(string.format("%s/files_%s_all", src, os.osname))
        checkarch(string.format("%s/files_all_%s", src, os.arch))
        checkarch(string.format("%s/files_%s_%s", src, os.osname, os.arch))

        for _, f in ipairs(archives) do
            local extrcmd
            if cfg.archivetype == "gzip" then
                extrcmd = string.format("cat %s | gzip -cd | tar -C %s -xvf -", f, dest)
            elseif cfg.archivetype == "bzip2" then
                extrcmd = string.format("cat %s | bzip -d | tar -C %s -xvf -", f, dest)
            else
                extrcmd = string.format("(%s/lzma-decode %s dep.tar >/dev/null 2>&1 && tar -C %s -xvf dep.tar && rm -f dep.tar)", bindir, f, dest)
            end
            
            install.execute(extrcmd)
        end
    end
end

function checkdeps(bins, bdir, deps)
    local ret = { }
    
    if bins then
        for _, b in ipairs(bins) do
            local path = string.format("%s/%s", bdir, b)
            libs = getmissinglibs(path) -- Collect any dep libs which are not found
            for rd in pairs(getdepsfromlibs(libs)) do -- Check which deps provide missing libs
                if not ret[rd] then
                    ret[rd] = checkdeps(rd.libs, pkg.getdepdir(rd), rd.deps) or { }
                end
            end
        end
    end
    
    for _, d in ipairs(deps) do
        if not ret[d] and d.required and d:required() then
            -- Add deps which are always required
            ret[d] = checkdeps(d.libs, pkg.getdepdir(d), d.deps) or { }
        end
    end
    
    return ret
end

local lsymstat, loadedsyms = pcall(dofile, string.format("%s/config/symmap", curdir))
function checkelf(bin)
    if lsymstat == false then
        install.print("Warning: no symbol mapfile found, binary compatibility checking will be disabled.")
        lsymstat = nil -- Set to something else than true or false, so that the warning is only displayed once
    end
    
    if not lsymstat then
        return false -- Ignore what we can't check
    end
    
    local libpath = install.getpkgdir("lib")
    local map = maplibs(bin, { libpath })
    
    if not map then
        return false
    end
    
    local binsyms = getsyms(bin)
    
    if not binsyms then
        return false
    end
    
    local foundsyms = { }
    for l, v in pairs(map) do
        print("v:", v, l)
--         assert(v and os.fileexists(v))
        
        if not v or not os.fileexists(v) then
            return false -- UNDONE
        end
        
        foundsyms[l] = getsyms(v)
    end
    
    local incompatlibs = { }
    local incompatdeps = { }
    local overridedeps = { }
    for s, v in pairs(binsyms) do
        if not v then
            local found = false
            for l, ls in pairs(foundsyms) do
                found = ls[s]
                if found then
                    print(string.format("Found symbol %s in %s", s, l))
                    break
                end
            end
            
            if not found then
                local lib = loadedsyms[bin][s]
                assert(lib and lib ~= bin)
                
                local dep = getdepsfromlibs{[lib] = true}

                if not utils.emptytable(dep) then
                    if not os.fileexists(string.format("%s/%s", libpath, lib)) then
                        utils.tablemerge(overridedeps, dep)
                    else
                        utils.tablemerge(incompatdeps, dep)
                    end
                else
                    incompatlibs[lib] = true
                end
            end
        end
    end
    
    local incompat = (not utils.emptytable(overridedeps) or not utils.emptytable(incompatdeps) or not utils.emptytable(incompatlibs))
    return incompat, overridedeps, incompatdeps, incompatlibs
end
    
dofile("deps-public.lua")
