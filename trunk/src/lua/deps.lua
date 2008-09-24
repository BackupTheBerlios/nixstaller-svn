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
        print("Collect:", lib)
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

-- UNDONE: Move up
local lsymstat, loadedsyms = pcall(dofile, string.format("%s/config/symmap", curdir))
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
                                    assert(supposedlib)
                                    
                                    if supposedlib and infomap[supposedlib] and infomap[supposedlib].syms and
                                       infomap[supposedlib].syms[sym] then
                                        infomap[supposedlib].usedby[b] = true
                                        found = true
                                    else
                                        for lib, info in pairs(infomap) do
                                            found = info.syms and info.syms[sym] and
                                                    info.syms[sym].version == v.version
                                            if found then
                                                infomap[lib].usedby[b] = true
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

--[[
function checkdeps(bins, bdir, deps, dialog, wrongdeps, wronglibs, mydep)
    if lsymstat == false then
        install.print("WARNING: no symbol mapfile found, binary compatibility checking will be partly disabled.\n")
        lsymstat = nil -- Set to something else than true or false, so that the warning is only displayed once
    end
    
    if not deps then
        return
    end
    
    if mydep and checkeddeps[mydep] then
        print("Skipping already checked dependency " .. mydep.name)
--         return checkeddeps[mydep]
    end
    
    local needs = { }
    
    if bins then
        for i, b in ipairs(bins) do
            --     UNDONE: Remove
            if mydep then
                table.insert(tree, mydep.name)
                io.write("Checking: ")
                for _, t in ipairs(tree) do
                    io.write(t .. " --> ")
                end
                print("")
            end
            
            local bprog = progstep
            local path = string.format("%s/%s", bdir, b)
            if os.fileexists(path) then
                local function setstat(msg)
                    local s = (mydep and string.format("dependency %s", mydep.name)) or string.format("binary %s", b)
                    dialog:settitle(string.format("Checking dependencies for %s: %s (%d/%d)", s, msg, i, #bins))
                end
                
                local bininfo = { }
                local map = maplibs(path)
                local symverneeds = getsymverneeds(path)
                
                local function markdep(bi, l)
                    if mydep then
                        local mp = getdeplibpath(mydep, l)
                        if mp then
                            bi.found = true
                            bi.path = mp
                            bi.dep = mydep
                            return true
                        end
                    end
                    
                    local dep = getdepfromlib(deps, l)
                    
                    if dep and dep.full then
                        if initdep(dep, dialog, wrongdeps) then
                            bi.found = true
                            bi.path = getdeplibpath(dep, l)
                            bi.dep = dep
                            return true
                        end
                    else
                        wronglibs[l] = wronglibs[l] or { }
                        if bi.native then
                            wronglibs[l].incompatlib = true
                        else
                            wronglibs[l].missinglib = true
                        end
                        install.print(string.format("WARNING: Missing/incompatible library: %s\n", l))
                    end
                    return false
                end
                
                local function getbininfo(lib, path)
                    local ret = { found = false, native = false }
                    
                    if path then -- Lib is already present
                        ret.native = true
                        ret.found = true
                        ret.path = path
                    else
                        markdep(ret, lib)
                    end
                    
                    if ret.found then
                        if symverneeds and symverneeds[lib] then
                            if not verifysymverneeds(symverneeds[lib], lib, ret.path) then
                                if ret.native then
                                    if not markdep(ret, lib) and not ret.dep then
                                        wronglibs[lib] = wronglibs[lib] or { }
                                        wronglibs[lib].incompatlib = true
                                    end
                                elseif not ret.dep.HandleCompat or
                                    not ret.dep:HandleCompat(lib) then
                                    wrongdeps[ret.dep] = wrongdeps[ret.dep] or { }
                                    wrongdeps[ret.dep].incompatdep = true
                                end
                            end
                        end
                        
                        if not wronglibs[lib] and (ret.native or not wrongdeps[ret.dep]) then
                            if lsymstat then
                                ret.syms = getsyms(ret.path)
                            end
                        end
                    end
                    return ret
                end
                
                local function handleinvaliddep(bi, lib, sym)
                    if bi.native then
                        if not markdep(bi, lib) and not bi.dep then
                            wronglibs[lib] = wronglibs[lib] or { }
                            wronglibs[lib].incompatlib = true
                        elseif symverneeds and symverneeds[lib] and
                            not verifysymverneeds(symverneeds[lib], lib, bi.path) and
                            (not bi.dep.HandleCompat or not bi.dep:HandleCompat(lib)) then
                            wrongdeps[bi.dep] = wrongdeps[bi.dep] or { }
                            wrongdeps[bi.dep].incompatdep = true
                        else
                            local syms = getsyms(bi.path)
                            if not syms or not syms[sym] or syms[sym].version ~= v.version then
                                wrongdeps[bi.dep] = wrongdeps[bi.dep] or { }
                                wrongdeps[bi.dep].incompatdep = true
                                install.print(string.format("Incompatible dependency: %s (%s, %s)\n", bi.dep.name, lib, sym))
                            else
                                install.print(string.format("Overrided dependency: %s (%s)\n", bi.dep.name, lib))
                            end
                        end
                    else
                        wrongdeps[bi.dep] = wrongdeps[bi.dep] or { incompatdep = true }
                        install.print(string.format("Incompatible dependency: %s (%s, %s)\n", bi.dep.name, lib, sym))
                    end
                end
                
                setstat("Find missing")
                
                if map then
                    for l, p in pairs(map) do
                        bininfo[l] = getbininfo(l, p)
                        
                        if bininfo[l].found then
--                             -- Check for any previous collected indirect symbol dependencies and try to solve them here
--                             for ib, is in pairs(indirectsyms) do
--                                 for s, sinfo in pairs(is) do
--                                     print("Route:", tabtostr(sinfo.route))
--                                     if sinfo.route[1] == l then
--                                         print("Found ind lib", l)
--                                         table.remove(sinfo.route, 1)
--                                         if utils.emptytable(sinfo.route) then
--                                             -- Found destination library
--                                             if bininfo[l].syms and bininfo[l].syms[s] and bininfo[l].syms[s].version == sinfo.version then
--                                                 print(string.format("Solved indirect symbol %s from %s with %s via %s (%s)", s, ib, l, b, (mydep and mydep.name) or ""))
--                                             else
--                                                 -- Symbol was not found, try to override dep
--                                                 handleinvaliddep(bininfo[l], l, s)
--                                             end
--                                             sinfo.found = true
--                                         elseif bininfo[l].native then
--                                             -- Find final lib through maps
--                                         else
--                                             -- lib will be seached in a next cycle
--                                         end
--                                     end
--                                 end
--                             end
                            --[=[
                            for ib, il in pairs(indirectsyms) do
                                print("Checking:", ib, l)
                                print("Contains:")
                                table.foreach(il, print)
                                if il[l] then
                                    print("Found ind lib", l)
                                    for is, iv in pairs(il[l]) do
                                        if bininfo[l].syms and bininfo[l].syms[is] and bininfo[l].syms[is].version == iv.version then
                                            print(string.format("Solved indirect symbol %s from %s with %s via %s (%s)", is, ib, l, b, (mydep and mydep.name) or ""))
                                        else
                                            -- Symbol was not found, try to override dep
                                            handleinvaliddep(bininfo[l], l, is)
                                        end
                                        il[l][is] = nil
                                    end
                                end
                            end
                            --]=]
                        end
                    end
                end
                
                if lsymstat then
                    local binsyms = getsyms(path)
                    if binsyms then
                        setstat("Verifying compatibility")
                        for s, v in pairs(binsyms) do
                            if v.undefined then
                                local found = false
                                for _, bi in pairs(bininfo) do
                                    found = bi.found and (bi.syms and bi.syms[s] and
                                            bi.syms[s].version == v.version)
                                    if found then
--                                         print(string.format("Found symbol %s in %s", s, bi.path))
                                        break
                                    end
                                end
                                
                                if not found then
                                    local lib = loadedsyms[utils.basename(b)][s]

                                    assert(lib)
                                    assert(type(lib) ~= "string" or lib ~= b)
                                    assert(type(lib) == "string" or lib.lib ~= b)

                                    -- This may happen incase a symbol is resolved indirectly
                                    -- In this case 'lib' points to a 'route' to the lib which
                                    -- contains the symbol.
                                    if not bininfo[lib] then
                                        assert(type(lib) == "table")
                                        indirectsyms[b] = indirectsyms[b] or { }
                                        indirectsyms[b][s] = { version = v.version, route = lib }
                                        print(string.format("Found indirect symbol %s from %s", s, b))
                                    end
                                    
                                    if bininfo[lib] and bininfo[lib].found then
                                        handleinvaliddep(bininfo[lib], lib, s)
                                    end
                                end
                            end
                        end
                    end
                end
                
                local subinc = (not mydep and (progstep / utils.mapsize(bininfo)))
                bprog = 0
                for l, i in pairs(bininfo) do
                    -- Check for any previous collected indirect symbol dependencies and try to solve them here
                    if i.found then
                        for ib, is in pairs(indirectsyms) do
                            for s, sinfo in pairs(is) do
                                print("Route:", tabtostr(sinfo.route))
                                if sinfo.route[1] == l then
                                    print("Found ind lib", l)
                                    table.remove(sinfo.route, 1)
                                    if utils.emptytable(sinfo.route) then
                                        -- Found destination library
                                        if i.syms and i.syms[s] and i.syms[s].version == sinfo.version then
                                            print(string.format("Solved indirect symbol %s from %s with %s via %s (%s)", s, ib, l, b, (mydep and mydep.name) or ""))
                                        else
                                            -- Symbol was not found, try to override dep
                                            handleinvaliddep(i, l, s)
                                        end
                                        sinfo.found = true
                                    elseif i.native then
                                        -- Find final lib through maps
                                    else
                                        -- lib will be searched in a next cycle
                                    end
                                end
                            end
                        end
                        
                        if not i.native and i.dep and i.dep ~= mydep then
                            if not needs[i.dep] and not wrongdeps[i.dep] then
                                if initdep(i.dep, dialog, wrongdeps) then
                                    needs[i.dep] = checkdeps(i.dep.libs, pkg.getdepdir(i.dep, i.dep.libdir), i.dep.deps, dialog, wrongdeps, wronglibs, i.dep) or { }
                                end
                            end
                        end
                    end
                    
                    if not mydep then
                        incprogress(dialog, subinc)
                    end
                end
            end
            
            if not mydep then
                incprogress(dialog, bprog)
            end
            
            --     UNDONE: Remove
            if mydep then
                table.remove(tree)
            end
        end
        
        -- Check if all unresolved indirect syms are now solved. If not, no dependency was
        -- found that has the needed lib, then mark it as missing.
        for _, b in ipairs(bins) do
            if indirectsyms[b] then
                for is, sinfo in pairs(indirectsyms[b]) do
                    if not sinfo.found then
                        print("Symbol still missing:", is, b)
--                         wronglibs[il] = wronglibs[il] or { }
--                         wronglibs[il].missinglib = true
                    end
                end
            end
        end
    end
    
    dialog:settitle("Gathering mandatory dependencies.")
    if not mydep then
        incprogress(dialog)
    end
    
    for _, d in ipairs(deps) do
        local dep = pkg.depmap[d]
        if dep and not needs[dep] and not wrongdeps[dep] and dep.Required and dep:Required() then
            -- Add deps which are always required
            if initdep(dep, dialog, wrongdeps) then
                needs[dep] = checkdeps(dep.libs, pkg.getdepdir(dep, dep.libdir), dep.deps, dialog, wrongdeps, wronglibs, dep) or { }
            end
        end
    end

    if mydep then
        checkeddeps[mydep] = needs
    end
    
    return needs
end

function instdeps(deps, installeddeps, wrongdeps, dialog, rec)
    dialog:settitle("Installing dependencies.")
    
    if not rec then
        incprogress(dialog)
    end
    
    for d, rd in pairs(deps) do
        if not installeddeps[d] and not wrongdeps[d] and d.full then
            -- if rd is a table, the dep needs other dependencies
            if type(rd) == "table" and not utils.emptytable(rd) then
                instdeps(rd, installeddeps, wrongdeps, dialog, true)
            end

            if initdep(d, dialog, wrongdeps) then
                -- Check if dep is usable
                if d.CanInstall and not d:CanInstall() then
                    wrongdeps[d] = { failed = true }
                else
                    d:Install()
                    installeddeps[d] = true
                    install.print(string.format("Installed dependency: %s\n", d.name))
                end
            end
        end
    end
end
--]]

dofile("deps-public.lua")
