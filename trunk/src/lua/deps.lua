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


local lsymstat, loadedsyms = pcall(dofile, string.format("%s/config/symmap.lua", curdir))

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

depprocess = {
progstep = 0,
curprogress = 0,
wrongdeps = { },
wronglibs = { },
installeddeps = { },
notifier = nil
}

function initprogress(bins, mainlibs)
    -- Rough progress indication
    
    depprocess.curprogress = 0
    local count = 0
    
    if bins then
        count = count + #bins
    end
    
    if mainlibs then
        count = count + #mainlibs
    end
    
    count = count + utils.mapsize(pkg.depmap)
    
    count = count + 1 -- Installing deps
    
    depprocess.progstep = 100 / count
end

function incprogress(step)
    if not install.unattended then
        step = step or depprocess.progstep
        depprocess.curprogress = depprocess.curprogress + step
        depprocess.notifier:setprogress(depprocess.curprogress)
    end
end

function setstatus(s)
    if install.unattended then
        install.print("\n*  " .. tr(s) .. "\n\n")
    else
        depprocess.notifier:settitle(s)
    end
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

function getnativedeps(deps, lib)
    local dep = getdepfromlib(deps, lib)
    if dep then
        return dep.deps
    end
end

local initdeps = { }
local ignorefaileddl = false
function initdep(d)
    if initdeps[d] ~= nil then
        return initdeps[d]
    end
    
    if not d.full then
        initdeps[d] = true
        return true
    end
    
    local function enablesecbar(e)
        if not install.unattended then
            depprocess.notifier:enablesecbar(e)
        end
    end

    local printedprog = 0
    local function setsecprogress(p)
        if install.unattended then
            if printedprog >= 100 then
                return
            end
            
            while (p - printedprog) >= 10 do
                printedprog = printedprog + 10
                install.print(string.format("%d%% ", printedprog))
            end
        else
            depprocess.notifier:setsecprogress(p)
        end
    end

    local function notifydownload()
        if install.unattended then
            install.print("Downloading dependency " .. d.name .. ": ")
        else
            depprocess.notifier:setsectitle("Downloading dependency " .. d.name)
            depprocess.notifier:setcancelbutton(true)
        end
    end
    
    local function notifyenddownload()
        if install.unattended then
            install.print("\n")
            printedprog = 0
        else
            depprocess.notifier:setcancelbutton(true)
        end
    end
    
    local function notifyextract()
        if install.unattended then
            install.print("Extracting dependency " .. d.name .. ": ")
        else
            depprocess.notifier:setsectitle("Extracting dependency " .. d.name)
        end
    end
    
    local function notifyendextract()
        if install.unattended then
            install.print("\n")
            printedprog = 0
        end
    end
    
    local function cancelled()
        if install.unattended then
            return false
        else
            return depprocess.notifier:cancelled()
        end
    end
    
    enablesecbar(true)
    
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
        notifydownload()
        local fmap = dofile(dlfile)
        for f, sum in pairs(fmap) do
            if archfiles[f] then
                local path = string.format("%s/%s", src, f)
                
                -- For unattended installs
                local retries, maxretries = 0, (cfg.unopts["dlretries"] and
                    cfg.unopts["dlretries"].value and
                    cfg.unopts["dlretries"].internal) or 3
                
                while true do
                    local download, msg = os.initdownload(string.format("%s/%s", d.baseurl, f), path)
                    
                    if download then
                        function download:updateprogress(t, d)
                            if d > 0 then
                                setsecprogress(d/t*100)
                            end
                        end
        
                        local stat
                        stat, msg = download:process()
                        while stat and not cancelled() do
                            stat, msg = download:process()
                        end
                        download:close()
                    end
                    
                    local dlsum = io.md5(path)
                    if not download or msg or cancelled() or dlsum ~= sum then -- Error
                        if dlsum ~= sum then
                            msg = msg or "file differs from server"
                        end
                        
                        local retry = false
                        if not ignorefaileddl and not cancelled() then
                            local usermsg = "Failed to download dependency"
                            if msg then
                                if install.unattended then
                                    usermsg = usermsg .. ":" .. msg
                                else
                                    usermsg = usermsg .. ":\n" .. msg
                                end
                            end
                            
                            if install.unattended then
                                retries = retries + 1
                                retry = (retries <= maxretries)
                                install.print(usermsg .. "\n")
                                if retry then
                                    install.print(string.format("Retrying ... (%d/%d)\n", retries, maxretries))
                                end
                            else
                                local choice = gui.choicebox(usermsg, "Retry", "Ignore", "Ignore all")
                                retry = (choice == 1)
                                ignorefaileddl = (choice == 3)
                            end
                        end
                        
                        if not retry then
                            enablesecbar(false)
                            notifyenddownload()
                            initdeps[d] = false
                            os.remove(path)
                            depprocess.wrongdeps[d] = depprocess.wrongdeps[d] or { }
                            depprocess.wrongdeps[d].faileddl = true
                            return false
                        end
                    else
                        break
                    end
                end
            end
        end
        setsecprogress(100)
        notifyenddownload()
    end
    
    notifyextract()
    
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
            szfile:close()
        end
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
            file = string.gsub(file, "\n$", "")
            
            if os.osname == "sunos" then
                -- Solaris puts some extra info after filename, remove
                file = string.gsub(file, ", [%d]* bytes, [%d]* tape blocks", "")
            end
            
            if szmap[f] and szmap[f][file] and totalsize > 0 then
                extractedsz = extractedsz + szmap[f][file]
                setsecprogress(extractedsz / totalsize * 100)
            end
        end
    end
    
    enablesecbar(false)
    notifyendextract()
    initdeps[d] = true
    return true
end

function resetfaileddl()
    for k, v in pairs(depprocess.wrongdeps) do
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
                return true
            else
                return false
            end
        end
    end
    return true
end

function markdepfromlib(bininfo, lib, deps, parent)
    assert(not bininfo.main)

    if parent then
        local p = getdeplibpath(parent, lib)
        if p then
            bininfo.found = true
            bininfo.native = false
            bininfo.path = p
            bininfo.dep = parent
            bininfo.deps = parent.deps
            return true
        end
    end

    local dep = deps and getdepfromlib(deps, lib)
    return markdep(bininfo, lib, dep, deps)
end

function markdep(bininfo, lib, dep, deps)
    assert(not bininfo.main)
    
    if dep and dep.full then
        if initdep(dep) then
            bininfo.found = true
            bininfo.native = false
            bininfo.path = getdeplibpath(dep, lib)
            bininfo.dep = dep
            bininfo.deps = dep.deps
            return true
        end
    else
        if dep then
            depprocess.wrongdeps[dep] = depprocess.wrongdeps[dep] or { }
            depprocess.wrongdeps[dep].incompatdep = true
            install.print(string.format("WARNING: Missing/incompatible dependency: %s\n", dep.name))
        else
            depprocess.wronglibs[lib] = depprocess.wronglibs[lib] or { }
            if bininfo.native then
                depprocess.wronglibs[lib].incompatlib = true
            else
                depprocess.wronglibs[lib].missinglib = true
            end
            install.print(string.format("WARNING: Missing/incompatible library: %s\n", lib))
        end
    end
    return false
end

function collectlibinfo(infomap, bin, mainlibs, deps)
    if not infomap[bin].map then
        return
    end
    
    for lib, path in pairs(infomap[bin].map) do
        if not infomap[lib] then
            infomap[lib] = { found = false, native = false, usedby = { } }
            
            if mainlibs[lib] then -- Lib is provided from main
                infomap[lib].found = true
                infomap[lib].main = true
                infomap[lib].native = false
                infomap[lib].path = mainlibs[lib]
                infomap[lib].deps = pkg.deps
            else
                local dep = deps and getdepfromlib(deps, lib)
                if (dep and dep.Required and dep:Required() and dep.full) or not path then
                    markdepfromlib(infomap[lib], lib, deps, infomap[bin].dep)
                elseif path then -- Lib is already present
                    infomap[lib].native = true
                    infomap[lib].found = true
                    infomap[lib].path = path
                    infomap[lib].deps = deps and getnativedeps(deps, lib)
                end
            end
            
            if infomap[lib].found then
                -- Main libs aren't checked, since we and the user can't fix them (UNDONE?)
                if not infomap[lib].main and infomap[bin].symverneeds and infomap[bin].symverneeds[lib] then
                    if not verifysymverneeds(infomap[bin].symverneeds[lib], infomap[lib].path) then
                        if infomap[lib].native then
                            markdepfromlib(infomap[lib], lib, deps, infomap[bin].dep)
                        elseif not infomap[lib].dep.HandleCompat or not infomap[lib].dep:HandleCompat(lib) then
                            depprocess.wrongdeps[infomap[lib].dep] = depprocess.wrongdeps[infomap[lib].dep] or { }
                            depprocess.wrongdeps[infomap[lib].dep].incompatdep = true
                        end
                    end
                end
                
                if lsymstat then
                    infomap[lib].syms = getsyms(infomap[lib].path)
                end
            end
            
            if infomap[lib].found then
                infomap[lib].map = maplibs(infomap[lib].path)
                infomap[lib].symverneeds = getsymverneeds(infomap[lib].path)
                if infomap[lib].map then
                    collectlibinfo(infomap, lib, mainlibs, infomap[lib].deps)
                else
                    -- UNDONE?
                end
            end
        end
        infomap[lib].usedby[bin] = true
    end
end

function handleinvaliddep(infomap, incompatlib, lib, sym, symver)
    assert(not infomap[incompatlib].main)
    
    if infomap[incompatlib].native then
        if markdepfromlib(infomap[incompatlib], incompatlib, infomap[lib].deps, infomap[lib].dep) then
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
                ok = infomap[incompatlib].syms and infomap[incompatlib].syms[sym] and infomap[incompatlib].syms[sym].version == symver
            end
            
            if ok then
                for ul in pairs(infomap[incompatlib].usedby) do
                    if infomap[ul] and infomap[ul].found and infomap[ul].symverneeds and infomap[ul].symverneeds[incompatlib] then
                        ok = verifysymverneeds(infomap[ul].symverneeds[incompatlib], infomap[incompatlib].path)
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
                depprocess.wrongdeps[infomap[incompatlib].dep] = depprocess.wrongdeps[infomap[incompatlib].dep] or { }
                depprocess.wrongdeps[infomap[incompatlib].dep].incompatdep = true
                install.print(string.format("Incompatible dependency: %s (%s, %s)\n", infomap[incompatlib].dep.name, incompatlib, sym))
            else
                install.print(string.format("Overrided dependency: %s (%s)\n", infomap[incompatlib].dep.name, incompatlib))
                return true
            end
        end
    elseif not infomap[incompatlib].dep.HandleCompat or not infomap[incompatlib].dep:HandleCompat(incompatlib) then
        depprocess.wrongdeps[infomap[incompatlib].dep] = depprocess.wrongdeps[infomap[incompatlib].dep] or { incompatdep = true }
        install.print(string.format("Incompatible dependency: %s (%s, %s)\n", infomap[incompatlib].dep.name, incompatlib, sym))
    end
    return false
end

function verifysyms(infomap, bin)
    local stack = { }
    local checked = { }
    
    for lib, info in pairs(infomap) do
        if info.found then
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
                    
                    if supposedlib and infomap[supposedlib] and infomap[supposedlib].syms and
                       infomap[supposedlib].syms[sym] and not infomap[supposedlib].syms[sym].undefined then
                        infomap[supposedlib].usedby[b] = true
                        found = true
                    else
                        for lib, info in pairs(infomap) do
                            found = info.syms and info.syms[sym] and
                                    not info.syms[sym].undefined and
                                    info.syms[sym].version == v.version
                            if found then
                                infomap[lib].usedby[b] = true
                                break
                            end
                        end
                    end
                    
                    if not found then
                        -- Main libs aren't checked, since we and the user can't fix them (UNDONE?)
                        if supposedlib and infomap[supposedlib] and infomap[supposedlib].found and
                           not infomap[supposedlib].main then
                            if handleinvaliddep(infomap, supposedlib, b, sym, v.version) then
                                collectlibinfo(infomap, supposedlib, mainlibs, infomap[supposedlib].deps)
                                
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
                                depprocess.wronglibs[supposedlib] = depprocess.wronglibs[supposedlib] or { }
                                depprocess.wronglibs[supposedlib].incompatlib = true
                            elseif infomap[supposedlib].dep and (not infomap[supposedlib].dep.HandleCompat or
                                    not infomap[supposedlib].dep:HandleCompat(supposedlib)) then
                                depprocess.wrongdeps[infomap[supposedlib].dep] = depprocess.wrongdeps[infomap[supposedlib].dep] or { }
                                depprocess.wrongdeps[infomap[supposedlib].dep].incompatdep = true
                            end
                        end
                    end
                end
            end
        end
        checked[b] = true
    end
end

function checkdeps(bins, libs, bdir)
    if lsymstat == false then
        install.print("WARNING: no symbol mapfile found, binary compatibility checking will be partly disabled.\n")
        lsymstat = nil -- Set to something else than true or false, so that the warning is only displayed once
    end
    
    if not pkg.deps then
        return
    end
    
    initprogress(bins, libs)
    
    local mainlibs = { }
    if libs then
        for _, l in ipairs(libs) do
            mainlibs[l] = string.format("%s/%s", bdir, l)
        end
    end

    local needs = { }
    
    local function checkbins(bins, dep, deps)
        for _, bin in ipairs(bins) do
            if not dep then -- HACK: Only update progress for main bins/libs
                incprogress()
            end
            
            local path = string.format("%s/%s", (dep and pkg.getdepdir(dep, dep.libdir)) or bdir, bin)
            if os.fileexists(path) then
                local infomap = { }
                infomap[bin] = { }
                infomap[bin].map = maplibs(path)
                infomap[bin].symverneeds = getsymverneeds(path)
                infomap[bin].syms = lsymstat and getsyms(path)
                infomap[bin].usedby = { }

                if dep then
                    markdep(infomap[bin], bin, dep, deps, nil)
                else
                    infomap[bin].main = true
                    infomap[bin].path = path
                    infomap[bin].native = false
                    infomap[bin].found = true
                    infomap[bin].deps = deps
                end
                
                collectlibinfo(infomap, bin, mainlibs, deps)
                
                if lsymstat then
                    verifysyms(infomap, bin)
                end
                
                -- Gather all deps to be installed
                for _, info in pairs(infomap) do
                    if info.found and not info.native and not info.main then
                        needs[info.dep] = true
                    end
                end
            end
        end
    end
    
    if bins then
        -- Check main binaries
        setstatus("Gathering main binary dependencies.")
        checkbins(bins, nil, pkg.deps)
    end
    
    if libs then
        -- Check main libraries
        setstatus("Gathering main library dependencies.")
        checkbins(libs, nil, pkg.deps)
    end
    
    -- Check deps which say they are required
    setstatus("Gathering mandatory dependencies.")
    for _, dep in pairs(pkg.depmap) do
        incprogress()
        if not needs[dep] and dep.Required and dep:Required() and dep.full then
            checkbins(dep.libs, dep, dep.deps)
            needs[dep] = true
        end
    end
    
    return needs
end

function instdeps(deps)
    setstatus("Installing dependencies.")
    
    incprogress()
    
    for dep in pairs(deps) do
        if not depprocess.installeddeps[dep] and not depprocess.wrongdeps[dep] and dep.full then
            if initdep(dep) then
                -- Check if dep is usable
                if dep.CanInstall and not dep:CanInstall() then
                    depprocess.wrongdeps[dep] = { failed = true }
                else
                    dep:Install()
                    depprocess.installeddeps[dep] = true
                    install.print(string.format("Installed dependency: %s\n", dep.name))
                end
            end
        end
    end
end

function verifydeps(bins, libs)
    depprocess.wrongdeps, depprocess.wronglibs = { }, { }

    if install.unattended then
        local deps = checkdeps(bins, libs, install.getpkgdir())
        instdeps(deps, self)
    else
        gui.newprogressdialog(function(self)
            depprocess.notifier = self
            local deps = checkdeps(bins, libs, install.getpkgdir())
            instdeps(deps, self)
        end)
    end
    
    local ret = { }
    for k, v in pairs(depprocess.wrongdeps) do
        ret[k.name] = { }
        ret[k.name].desc = k.description
        if v.failed then
            ret[k.name].problem = "Failed to install."
        elseif v.faileddl then
            ret[k.name].problem = "Failed to download."
        elseif v.incompatdep then
            ret[k.name].problem = "(Binary) incompatible."
        end
    end
    
    resetfaileddl()
    
    for k, v in pairs(depprocess.wronglibs) do
        ret[k] = { }
        ret[k].desc = ""
        if v.missinglib then
            ret[k].problem = "File missing."
        elseif v.incompatlib then
            ret[k].problem = "(Binary) incompatible."
        end
    end

    return ret
end

dofile("deps-public.lua")
