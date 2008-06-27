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
        for _, dl in ipairs(pkg.depmap[d].libs) do
            if utils.basename(dl) == lib then
                return pkg.depmap[d]
            end
        end
    end
end

function getdeplibpath(dep, lib)
    for _, l in ipairs(dep.libs) do
        if utils.basename(l) == lib then
            return pkg.getdepdir(dep, l)
        end
    end
end

local initdeps = { }
local ignorefaileddl = false
function initdep(d, dialog, wrongdeps)
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

function verifysymverneeds(needs, lib, path)
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
end

local lsymstat, loadedsyms = pcall(dofile, string.format("%s/config/symmap", curdir))
function checkdeps(bins, bdir, deps, dialog, wrongdeps, wronglibs, mydep)
    if lsymstat == false then
        install.print("WARNING: no symbol mapfile found, binary compatibility checking will be partly disabled.\n")
        lsymstat = nil -- Set to something else than true or false, so that the warning is only displayed once
    end
    
    local needs = { }
    
    if bins then
        for _, b in ipairs(bins) do
            local bprog = progstep
            local path = string.format("%s/%s", bdir, b)
            if os.fileexists(path) then
                local function setstat(msg)
                    local s = (mydep and string.format("dependency %s", mydep.name)) or string.format("binary %s", b)
                    dialog:settitle(string.format("Checking dependencies for %s: %s", s, msg))
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
                    
                    if dep and initdep(dep, dialog, wrongdeps) then
                        bi.found = true
                        bi.path = getdeplibpath(dep, l)
                        bi.dep = dep
                        return true
                    end
                    return false
                end
                
                setstat("Find missing")
                
                for l, p in pairs(map) do
                    bininfo[l] = { found = false, native = false }
                    
                    if p then -- Lib is already present
                        bininfo[l].native = true
                        bininfo[l].found = true
                        bininfo[l].path = p
                    else
                        markdep(bininfo[l], l)
                    end
                    
                    if symverneeds and symverneeds[l] and bininfo[l].found then
                        if not verifysymverneeds(symverneeds[l], l, p) then
                            if bininfo[l].native then
                                if not markdep(bininfo[l], l) then
                                    wronglibs[l] = wrongdeps[l] or { }
                                    wronglibs[l].incompatlib = true
                                end
                            elseif not bininfo[l].dep.HandleCompat or
                                   not bininfo[l].dep:HandleCompat() then
                                wrongdeps[bininfo[l].dep] = wrongdeps[bininfo[l].dep] or { }
                                wrongdeps[bininfo[l].dep].incompatdep = true
                            end
                        end
                    end
                    
                    if bininfo[l].found then
                        if not wronglibs[l] and (not bininfo[l].native or
                            not wrongdeps[bininfo[l].dep]) then
                            if lsymstat then
                                bininfo[l].syms = getsyms(bininfo[l].path)
                            end
                        end
                    elseif bininfo[l].native then
                        wronglibs[l] = wronglibs[l] or { missinglib = true }
                        install.print(string.format("WARNING: Missing library: %s\n", l))
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
                                    found = (bi.syms and bi.syms[s] and
                                            bi.syms[s].version == v.version)
                                    if found then
                    --                     print(string.format("Found symbol %s in %s", s, l))
                                        break
                                    end
                                end
                                
                                if not found then
                                    local lib = loadedsyms[utils.basename(b)][s]
                                    assert(lib and lib ~= b and bininfo[lib])
                                    if bininfo[lib].found then
                                        if bininfo[lib].native then
                                            if not markdep(bininfo[lib], lib) then
                                                wronglibs[lib] = wronglibs[lib] or { }
                                                wronglibs[lib].incompatlib = true
                                            elseif symverneeds and symverneeds[lib] and
                                                not verifysymverneeds(symverneeds[lib], lib,
                                                                    bininfo[lib].path) and
                                                (not bininfo[lib].dep.HandleCompat or
                                                    not bininfo[lib].dep:HandleCompat()) then
                                                wrongdeps[bininfo[lib].dep] = wrongdeps[bininfo[lib].dep] or { }
                                                wrongdeps[bininfo[lib].dep].incompatdep = true
                                            else
                                                local syms = getsyms(bininfo[lib].path)
                                                if not syms or not syms[s] or
                                                syms[s].version ~= v.version then
                                                    wrongdeps[bininfo[lib].dep] = wrongdeps[bininfo[lib].dep] or { }
                                                    wrongdeps[bininfo[lib].dep].incompatdep = true
                                                    install.print(string.format("Incompatible dependency: %s (%s, %s)\n", bininfo[lib].dep.name, lib, s))
                                                else
                                                    install.print(string.format("Overrided dependency: %s (%s)\n", bininfo[lib].dep.name, lib))
                                                end
                                            end
                                        else
                                            wrongdeps[bininfo[lib].dep] = wrongdeps[bininfo[lib].dep] or { incompatdep = true }
                                            install.print(string.format("Incompatible dependency: %s (%s, %s)\n", bininfo[lib].dep.name, lib, s))
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
                
                local subinc = (not mydep and (progstep / utils.mapsize(bininfo)))
                bprog = 0
                for l, i in pairs(bininfo) do
                    if not i.native and i.dep and i.dep ~= mydep then
                        if not needs[i.dep] and not wrongdeps[i.dep] then
                            if initdep(i.dep, dialog, wrongdeps) then
                                needs[i.dep] = checkdeps(i.dep.libs, pkg.getdepdir(i.dep), i.dep.deps, dialog, wrongdeps, wronglibs, i.dep) or { }
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
        end        
    end
    
    dialog:settitle("Gathering mandatory dependencies.")
    if not mydep then
        incprogress(dialog)
    end
    
    for _, d in ipairs(deps) do
        if not needs[d] and not wrongdeps[d] and d.required and d:required() then
            -- Add deps which are always required
            if initdep(d, dialog, wrongdeps) then
                needs[d] = checkdeps(d.libs, pkg.getdepdir(d), d.deps, dialog, wrongdeps, wronglibs, d) or { }
            end
        end
    end
    
    return needs
end

function instdeps(deps, installeddeps, wrongdeps, dialog, rec)
    dialog:settitle("Installing dependencies.")
    
    if not rec then
        incprogress(dialog)
    end
    
    for d, rd in pairs(deps) do
        if not installeddeps[d] and not wrongdeps[d] then
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

dofile("deps-public.lua")
