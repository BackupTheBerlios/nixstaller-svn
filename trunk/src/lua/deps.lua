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


local lsymstat, loadedsyms = pcall(dofile, string.format("%s/config/symmap", curdir))

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

local progstep = 0
local curprogress = 0

function initprogress(bins)
    -- Rough progress indication
    
    curprogress = 0
    
    local count = 1 -- Checking deps which are always required
    count = count + #bins
    count = count + 1 -- Installing deps
    
    progstep = 100 / count
end

function incprogress(dialog, step)
    step = step or progstep
    curprogress = curprogress + step
    dialog:setprogress(curprogress)
end

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

function getdepfromlib(deps, lib)
    for _, d in ipairs(deps) do
        if utils.tablefind(pkg.depmap[d].libs, lib) then
            return pkg.depmap[d]
        end
    end
end

function getdeplibpath(dep, lib)
    local i = utils.tablefind(dep.libs, lib)
    if i then
        return pkg.getdepdir(dep, dep.libdir .. "/" .. dep.libs[i])
    end
end

local initdeps = { }
local ignorefaileddl = false
function initdep(d, dialog, wrongdeps)
    if initdeps[d] ~= nil then
        return initdeps[d]
    end
    
    if not d.full then
        initdeps[d] = true
        return true
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
        dialog:setcancelbutton(true)
        local fmap = dofile(dlfile)
        for f, sum in pairs(fmap) do
            if archfiles[f] then
                local path = string.format("%s/%s", src, f)
                while true do
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
                    
                    local dlsum = io.md5(path)
                    if not download or msg or dialog:cancelled() or dlsum ~= sum then -- Error
                        if dlsum ~= sum then
                            msg = msg or "file differs from server"
                        end
                        
                        local retry = false
                        if not ignorefaileddl and not dialog:cancelled() then
                            local guimsg = "Failed to download dependency"
                            if msg then
                                guimsg = guimsg .. ":\n" .. msg
                            end
                            local choice = gui.choicebox(guimsg, "Retry", "Ignore", "Ignore all")
                            retry = (choice == 1)
                            ignorefaileddl = (choice == 3)
                        end
                        
                        if not retry then
                            dialog:enablesecbar(false)
                            dialog:setcancelbutton(false)
                            initdeps[d] = false
                            print("Failed dep:", d.name, msg)
                            os.remove(path)
                            wrongdeps[d] = wrongdeps[d] or { }
                            wrongdeps[d].faileddl = true
                            return false
                        end
                    else
                        break
                    end
                end
            end
        end
        dialog:setcancelbutton(false)
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
            extrcmd = string.format("(%s/lzma-decode %s - 2>/dev/null | tar -C %s -xvf -)", bindir, f, dest)
        end
        
        local pipe = check(io.popen(extrcmd))
        for line in pipe:lines() do
            local file = string.gsub(line, "^x ", "")
            
            if os.osname == "sunos" then
                -- Solaris puts some extra info after filename, remove
                file = string.gsub(file, ", [%d]* bytes, [%d]* tape blocks", "")
            end
            
            if szmap[f][file] and totalsize > 0 then
                extractedsz = extractedsz + szmap[f][file]
                dialog:setsecprogress(extractedsz / totalsize * 100)
            end
        end
    end
    
    dialog:enablesecbar(false)
    initdeps[d] = true
    return true
end

function resetfaileddl(wrongdeps)
    for k, v in pairs(wrongdeps) do
        if v.faileddl then
            initdeps[k] = nil
            v.faileddl = nil
        end
    end
    ignorefaileddl = false
end

function verifysymverneeds(needs, path)
    local verdefs = getsymverdefs(path)
    if verdefs then
        for vn in pairs(needs) do
            if verdefs[vn] then
--                         print("Found symbol version:", vn)
                return true
            else
--                 print("Missing symbol version:", vn, path)
                return false
            end
        end
    end
    return true
end

function markdep(bininfo, lib, deps, parent, wronglibs, dialog)
    if parent then
        local p = getdeplibpath(parent, lib)
        if p then
            bininfo.found = true
            bininfo.path = p
            bininfo.dep = parent
            return true
        end
    end
    
    local dep = deps and getdepfromlib(deps, lib)
    
    if dep and dep.full then
        if initdep(dep, dialog, wrongdeps) then
            bininfo.found = true
            bininfo.path = getdeplibpath(dep, lib)
            bininfo.dep = dep
            return true
        end
    else
        wronglibs[lib] = wronglibs[lib] or { }
        if bininfo.native then
            wronglibs[lib].incompatlib = true
        else
            wronglibs[lib].missinglib = true
        end
        install.print(string.format("WARNING: Missing/incompatible library: %s\n", lib))
    end
    return false
end

function handleinvaliddep(infomap, incompatlib, lib, sym, wrongdeps, wronglibs, dialog)
    if infomap[incompatlib].native then
        local deps = infomap[incompatlib].dep and infomap[incompatlib].dep.deps
        if not markdep(infomap[incompatlib], incompatlib, deps, infomap[lib].dep, wronglibs, dialog) and not infomap[incompatlib].dep then
            wronglibs[incompatlib] = wronglibs[incompatlib] or { }
            wronglibs[incompatlib].incompatlib = true
        else
            local ok = true
            infomap[incompatlib].map = maplibs(infomap[incompatlib].path)
            infomap[incompatlib].symverneeds = getsymverneeds(infomap[incompatlib].path)
            
            if not infomap[incompatlib].map then
                ok = false
            else
                for l, p in pairs(infomap[incompatlib].map) do
                    if not p then
                        ok = false
                    elseif infomap[l] then
                        infomap[l].usedby[incompatlib] = true
                    end
                end
            end
            
            if ok then
                infomap[incompatlib].syms = getsyms(infomap[incompatlib].path)
                ok = infomap[incompatlib].syms and infomap[incompatlib].syms[sym] and infomap[incompatlib].syms[sym].version == v.version
            end
            
            if ok then
                for ul in pairs(infomap[incompatlib].usedby) do
                    if infomap[ul] and infomap[ul].found and infomap[ul].symverneeds then
                        ok = verifysymverneeds(infomap[ul].symverneeds, infomap[incompatlib].path)
                        if not ok then
                            break
                        end
                    end
                end
            end
            
            if not ok then
                ok = infomap[incompatlib].dep.HandleCompat and infomap[incompatlib].dep:HandleCompat(incompatlib)
            end
            
            if not ok then
                wrongdeps[infomap[incompatlib].dep] = wrongdeps[infomap[incompatlib].dep] or { }
                wrongdeps[infomap[incompatlib].dep].incompatdep = true
                install.print(string.format("Incompatible dependency: %s (%s, %s)\n", infomap[incompatlib].dep.name, incompatlib, sym))
            else
                install.print(string.format("Overrided dependency: %s (%s)\n", infomap[incompatlib].dep.name, incompatlib))
                return true
            end
        end
    elseif not infomap[incompatlib].dep.HandleCompat or not infomap[incompatlib].dep:HandleCompat(incompatlib) then
        wrongdeps[infomap[incompatlib].dep] = wrongdeps[infomap[incompatlib].dep] or { incompatdep = true }
        install.print(string.format("Incompatible dependency: %s (%s, %s)\n", infomap[incompatlib].dep.name, incompatlib, sym))
    end
    return false
end

function collectlibinfo(infomap, bin, deps, wrongdeps, wronglibs, dialog)
    if not infomap[bin].map then
        return
    end
    
    for lib, path in pairs(infomap[bin].map) do
        if not infomap[lib] then
            infomap[lib] = { found = false, native = false, usedby = { } }
            
            if path then -- Lib is already present
                infomap[lib].native = true
                infomap[lib].found = true
                infomap[lib].path = path
            else
                markdep(infomap[lib], lib, deps, infomap[bin].dep, wronglibs, dialog)
            end
            
            local ok = infomap[lib].found
            
            if ok then
                if infomap[bin].symverneeds and infomap[bin].symverneeds[lib] then
                    if not verifysymverneeds(infomap[bin].symverneeds[lib], infomap[lib].path) then
                        if infomap[lib].native then
                            if not markdep(infomap[lib], lib, deps, infomap[bin].dep, wronglibs, dialog) and not infomap[lib].dep then
                                wronglibs[lib] = wronglibs[lib] or { }
                                wronglibs[lib].incompatlib = true
                                ok = false
                            end
                        elseif not infomap[lib].dep.HandleCompat or not infomap[lib].dep:HandleCompat(lib) then
                            wrongdeps[infomap[lib].dep] = wrongdeps[infomap[lib].dep] or { }
                            wrongdeps[infomap[lib].dep].incompatdep = true
                            ok = false
                        end
                    end
                end
                
                if not wronglibs[lib] and (infomap[lib].native or not wrongdeps[infomap[lib].dep]) then
                    if lsymstat then
                        infomap[lib].syms = getsyms(infomap[lib].path)
                    end
                end
            end
            
            if ok then
                infomap[lib].map = maplibs(infomap[lib].path)
                infomap[lib].symverneeds = getsymverneeds(infomap[lib].path)
                if infomap[lib].map then
                    if infomap[lib].native then
                        collectlibinfo(infomap, lib, nil, wrongdeps, wronglibs, dialog)
                    else
                        collectlibinfo(infomap, lib, infomap[lib].dep.deps, wrongdeps, wronglibs, dialog)
                    end
                else
                    -- UNDONE?
                end
            end
        end
        infomap[lib].usedby[bin] = true
    end
end

-- UNDONE: Clear on restart
local indirectsyms = { }
local checkeddeps = { }
local tree = { }

function checkdeps(bins, bdir, dialog, wrongdeps, wronglibs)
    if lsymstat == false then
        install.print("WARNING: no symbol mapfile found, binary compatibility checking will be partly disabled.\n")
        lsymstat = nil -- Set to something else than true or false, so that the warning is only displayed once
    end
    
    if not pkg.deps then
        return
    end
    
    local ret = { }
    
    if bins then
        for _, bin in ipairs(bins) do
            local path = string.format("%s/%s", bdir, bin)
            if os.fileexists(path) then
                local infomap = { }
                infomap[bin] = { }
                infomap[bin].map = maplibs(path)
                infomap[bin].path = path
                infomap[bin].main = true
                infomap[bin].found = true
                infomap[bin].symverneeds = getsymverneeds(path)
                infomap[bin].syms = lsymstat and getsyms(path)
                infomap[bin].usedby = { }
                
                collectlibinfo(infomap, bin, pkg.deps, wrongdeps, wronglibs, dialog)
                
                if lsymstat then
                    local stack = { bin }
                    local checked = { }
                    
                    for lib, info in pairs(infomap) do
                        if not info.native and info.found then
                            table.insert(stack, lib)
                        end
                    end
                    
                    while not utils.emptytable(stack) do
                        local b = table.remove(stack)
                        local syms = getsyms(infomap[b].path)
                            
                        if syms then
                            for sym, v in pairs(syms) do
                                if v.undefined then
                                    local found = false
                                    local supposedlib = loadedsyms[b][sym]
                                    print("supposedlib:", supposedlib, b, sym)
--                                     assert(supposedlib)
                                    
                                    if supposedlib and infomap[supposedlib] and infomap[supposedlib].syms and
                                       infomap[supposedlib].syms[sym] and
                                       not infomap[supposedlib].syms[sym].undefined then
                                        infomap[supposedlib].usedby[b] = true
                                        found = true
                                    else
                                        for lib, info in pairs(infomap) do
                                            found = info.syms and info.syms[sym] and
                                                    not info.syms[sym].undefined and
                                                    info.syms[sym].version == v.version
                                            if found then
                                                infomap[lib].usedby[b] = true
                                                print(sym .. " --> " .. lib)
                                                break
                                            end
                                        end
                                    end
                                    
                                    if not found then
                                        if supposedlib and infomap[supposedlib] and infomap[supposedlib].found then
                                            if handleinvaliddep(infomap, supposedlib, b, sym, wrongdeps, wronglibs, dialog) then
                                                collectlibinfo(infomap, supposedlib, infomap[supposedlib].dep.deps, wrongdeps, wronglibs, dialog)
                                                
                                                -- Check for any new or affected dependencies
                                                for lib, info in pairs(infomap) do
                                                    if info.found and not utils.tablefind(stack, lib) then
                                                        if not checked[lib] or infomap[supposedlib].usedby[lib] then
                                                            table.insert(stack, lib)
                                                        end
                                                    end
                                                end
                                                
                                                found = true
                                            end
                                        else
                                            if not infomap[supposedlib] or infomap[supposedlib].native then
                                                wronglibs[supposedlib] = wronglibs[supposedlib] or { }
                                                wronglibs[supposedlib].incompatlib = true
                                            elseif infomap[supposedlib].dep and (not infomap[supposedlib].dep.HandleCompat or
                                                   not infomap[supposedlib].dep:HandleCompat(supposedlib)) then
                                                wrongdeps[infomap[supposedlib].dep] = wrongdeps[infomap[supposedlib].dep] or { }
                                                wrongdeps[infomap[supposedlib].dep].incompatdep = true
                                            end
                                        end
                                    end
                                end
                            end
                        end
                        checked[b] = true
                    end
                end
                
                -- Gather all deps
                for _, info in pairs(infomap) do
                    if info.found and not info.native and not info.main then
                        ret[info.dep] = true
                    end
                end
            end
        end
    end
    
    return ret
end

function instdeps(deps, installeddeps, wrongdeps, dialog)
    dialog:settitle("Installing dependencies.")
    
    incprogress(dialog)
    
    for dep in pairs(deps) do
        if not installeddeps[dep] and not wrongdeps[dep] and dep.full then
            if initdep(dep, dialog, wrongdeps) then
                -- Check if dep is usable
                if dep.CanInstall and not dep:CanInstall() then
                    wrongdeps[dep] = { failed = true }
                else
                    dep:Install()
                    installeddeps[dep] = true
                    install.print(string.format("Installed dependency: %s\n", dep.name))
                end
            end
        end
    end
end

dofile("deps-public.lua")
