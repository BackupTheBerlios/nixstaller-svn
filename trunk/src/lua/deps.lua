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

function getsymverneeds(bin)
    local elf = os.openelf(bin)
    
    if not elf then
        return
    end
    
    local ret = { }
    local n = 1
    local ver = elf:getsymneed(n)
    while ver do
        if ver.flags ~= "BASE" and ver.flags ~= "WEAK" then
            ret[ver.lib] = ret[ver.lib] or { }
            ret[ver.lib][ver.name] = true
        end
        n = n + 1
        ver = elf:getsymneed(n)
    end
    
    elf:close()
    
    return ret
end

function getsymverdefs(bin)
    local elf = os.openelf(bin)
    
    if not elf then
        return
    end
    
    local ret = { }
    local n = 1
    local ver = elf:getsymdef(n)
    while ver do
        ret[ver] = true
        n = n + 1
        ver = elf:getsymdef(n)
    end
    
    elf:close()
    
    return ret
end

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

local initdeps = { }
function initdep(d, dialog)
    if initdeps[d] ~= nil then
        return initdeps[d]
    end
    
    dialog:enablesecbar(true)
    
    local src = string.format("%s/deps/%s", curdir, d.name)
    local dest = string.format("%s/files", src)
    os.mkdirrec(dest)
    
    local archfiles = { }
    archfiles[string.format("%s_files_all", d.name)] = true
    archfiles[string.format("%s_files_%s_all", d.name, os.osname)] = true
    archfiles[string.format("%s_files_all_%s", d.name, os.arch)] = true
    archfiles[string.format("%s_files_%s_%s", d.name, os.osname, os.arch)] = true
    
    local dlfile = src .. "/dlfiles"
    if os.fileexists(dlfile) and d.baseurl then
        dialog:setsectitle("Downloading dependency " .. d.name)
        local fmap = dofile(dlfile)
        for f, sum in pairs(fmap) do
            if archfiles[f] then
                local path = string.format("%s/%s", src, f)
                local download, msg = os.initdownload(string.format("%s/%s", d.baseurl, f), path)
                
                if download then
                    function download:updateprogress(t, d)
                        dialog:setsecprogress(d/t*100)
                    end
    
                    local stat
                    stat, msg = download:process()
                    while stat and not dialog:cancelled() do
                        stat, msg = download:process()
                    end
                    download:close()
                end
                
                if not download or msg or dialog:cancelled() or io.md5(path) ~= sum then -- Error
                    dialog:enablesecbar(false)
                    initdeps[d] = false
                    print("Failed dep:", d.name, msg)
                    os.remove(path)
                    return false
                end
            end
        end
    end
    
    dialog:setsectitle("Extracting dependency " .. d.name)
    
    local archives = { }
    local function checkarch(a)
        if os.fileexists(a) then
            table.insert(archives, a)
        end
    end
    
    for a in pairs(archfiles) do
        checkarch(src .. "/" .. a)
    end

    local szmap = { }
    local totalsize = 0
    for _, f in ipairs(archives) do
        local szfile = io.open(f .. ".sizes", "r")
        if szfile then
            szmap[f] = { }
            for line in szfile:lines() do
                local sz = tonumber(string.match(line, "^[^%s]*"))
                szmap[f][string.gsub(line, "^[^%s]*%s*", "")] = sz
                totalsize = totalsize + sz
            end
        end
        szfile:close()
    end
    
    local extractedsz = 0
    for _, f in ipairs(archives) do
        local extrcmd
        if cfg.archivetype == "gzip" then
            extrcmd = string.format("cat %s | gzip -cd | tar -C %s -xvf -", f, dest)
        elseif cfg.archivetype == "bzip2" then
            extrcmd = string.format("cat %s | bzip -d | tar -C %s -xvf -", f, dest)
        else
            extrcmd = string.format("(%s/lzma-decode %s dep.tar >/dev/null 2>&1 && tar -C %s -xvf dep.tar && rm -f dep.tar)", bindir, f, dest)
        end
        
        local pipe = check(io.popen(extrcmd))
        for line in pipe:lines() do
            local file = string.gsub(line, "^x ", "")
            
            if os.osname == "sunos" then
                -- Solaris put some extra info after filename, remove
                file = string.gsub(file, ", [%d]* bytes, [%d]* tape blocks", "")
            end
            
            if szmap[f][file] and totalsize > 0 then
                extractedsz = extractedsz + szmap[f][file]
                dialog:setsecprogress(extractedsz / totalsize * 100)
            end
            for n=1,1000 do install.updateui() end
        end
    end
    
    dialog:enablesecbar(false)
    initdeps[d] = true
    return true
end

--[[
function extractdeps(dialog)
    local depsize = #pkg.deps
    local count = 0
    
    dialog:enablesecbar(true)

    for n in pairs(pkg.depmap) do
        dialog:setsectitle("Dependency: " .. n)
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

        local szmap = { }
        local totalsize = 0
        for _, f in ipairs(archives) do
            local szfile = io.open(f .. ".sizes", "r")
            if szfile then
                szmap[f] = { }
                for line in szfile:lines() do
                    local sz = tonumber(string.match(line, "^[^%s]*"))
                    szmap[f][string.gsub(line, "^[^%s]*%s*", "")] = sz
                    totalsize = totalsize + sz
                end
            end
            szfile:close()
        end
        
        local extractedsz = 0
        for _, f in ipairs(archives) do
            local extrcmd
            if cfg.archivetype == "gzip" then
                extrcmd = string.format("cat %s | gzip -cd | tar -C %s -xvf -", f, dest)
            elseif cfg.archivetype == "bzip2" then
                extrcmd = string.format("cat %s | bzip -d | tar -C %s -xvf -", f, dest)
            else
                extrcmd = string.format("(%s/lzma-decode %s dep.tar >/dev/null 2>&1 && tar -C %s -xvf dep.tar && rm -f dep.tar)", bindir, f, dest)
            end
            
            local pipe = check(io.popen(extrcmd))
            for line in pipe:lines() do
                local file = string.gsub(line, "^x ", "")
                
                if os.osname == "sunos" then
                    -- Solaris put some extra info after filename, remove
                    file = string.gsub(file, ", [%d]* bytes, [%d]* tape blocks", "")
                end
                
                if szmap[f][file] and totalsize > 0 then
                    extractedsz = extractedsz + szmap[f][file]
                    dialog:setsecprogress(extractedsz / totalsize * 100)
                end
                for n=1,1000 do install.updateui() end
            end
        end
        count = count + 1
        dialog:setprogress(count / depsize * 100)
    end
    dialog:enablesecbar(false)
end
--]]

function checkdeps(bins, bdir, deps, dialog)
    local needs, failed = { }, { }
    
    if bins then
        for _, b in ipairs(bins) do
            local path = string.format("%s/%s", bdir, b)
            if os.fileexists(path) then
                libs = getmissinglibs(path) -- Collect any dep libs which are not found
                for rd in pairs(getdepsfromlibs(libs)) do -- Check which deps provide missing libs
                    if not needs[rd] and not failed[rd] then
                        if initdep(rd, dialog) then
                            needs[rd] = checkdeps(rd.libs, pkg.getdepdir(rd), rd.deps, dialog) or { }
                        else
                            failed[rd] = true
                        end
                    end
                end
            end
        end
    end
    
    for _, d in ipairs(deps) do
        if not needs[d] and not failed[d] and d.required and d:required() then
            -- Add deps which are always required
            if initdep(d, dialog) then
                needs[d] = checkdeps(d.libs, pkg.getdepdir(d), d.deps, dialog) or { }
            else
                failed[d] = true
            end
        end
    end
    
    return needs, failed
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
    
    local wronglibs = { }
    local foundsyms = { }
    local symverneeds = getsymverneeds(bin)
    
    for l, v in pairs(map) do
        if v and os.fileexists(v) then
            foundsyms[l] = getsyms(v)
            
            if symverneeds and symverneeds[l] then
                local verdefs = getsymverdefs(v)
                if verdefs then
                    for vn in pairs(symverneeds[l]) do
                        if verdefs[vn] then
    --                         print("Found symbol version:", vn)
                        else
                            wronglibs[l] = true
                        end
                    end
                end
            end
        end
    end
    
    for s, v in pairs(binsyms) do
        if v.undefined then
            local found = false
            for l, ls in pairs(foundsyms) do
                found = (ls[s] and ls[s].version == v.version)
                if found then
--                     print(string.format("Found symbol %s in %s", s, l))
                    break
                end
            end
            
            if not found then
                local lib = loadedsyms[utils.basename(bin)][s]
                assert(lib and lib ~= bin)
                wronglibs[lib] = true
                local dep = getdepsfromlibs{[lib] = true}
            end
        end
    end
    
    local incompatlibs = { }
    local incompatdeps = { }
    local overridedeps = { }

    for l in pairs(wronglibs) do
        local dep = getdepsfromlibs{[l] = true}

        if not utils.emptytable(dep) then
            if not os.fileexists(string.format("%s/%s", libpath, l)) then
                utils.tablemerge(overridedeps, dep)
            else
                utils.tablemerge(incompatdeps, dep)
            end
        else
            incompatlibs[l] = true
        end
    end
    
    local incompat = (not utils.emptytable(overridedeps) or not utils.emptytable(incompatdeps) or not utils.emptytable(incompatlibs))
    return incompat, overridedeps, incompatdeps, incompatlibs
end
    
dofile("deps-public.lua")
