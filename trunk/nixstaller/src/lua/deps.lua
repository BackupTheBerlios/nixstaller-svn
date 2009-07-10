--     Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)
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


local lsymstat, loadedsyms = pcall(dofile, internal.configdir .. "/symmap.lua")

depclass = { }
depclass.__index = depclass

function depclass:copyfiles()
    utils.recursivedir(pkg.getdepdir(self), function (f, rf)
        local dest = string.format("%s/%s", install.getpkgdir(), rf)
        if os.isdir(f) then
            check(os.mkdirrec(dest))
        else
            check(os.copy(f, dest))
        end
    end)
end

function depclass:install()
    self:copyfiles()
end

depprocess = {
progstep = 0,
curprogress = 0,
wrongdeps = { },
wronglibs = { },
installeddeps = { },
notifier = nil,
libpaths = { }
}

function adddepprob(dep, field)
    depprocess.wrongdeps[dep] = depprocess.wrongdeps[dep] or { }
    depprocess.wrongdeps[dep][field] = depprocess.wrongdeps[dep][field] or { }
    depprocess.wrongdeps[dep][field] = true
end

function adddeplibprob(dep, field, lib, other)
    other = other or true
    depprocess.wrongdeps[dep] = depprocess.wrongdeps[dep] or { }
    depprocess.wrongdeps[dep][field] = depprocess.wrongdeps[dep][field] or { }
    if not depprocess.wrongdeps[dep][field][lib] then
        depprocess.wrongdeps[dep][field][lib] = other
    end
end

function addlibprob(lib, field, val)
    depprocess.wronglibs[lib] = depprocess.wronglibs[lib] or { }
    depprocess.wronglibs[lib][field] = val
end

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
    local elf = internal.openelf(bin)
    
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
    local elf = internal.openelf(bin)
    
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

-- Used by fastrun mode
function copydep(dep, setprogress)
    if install.unattended then
        install.print(tr("Extracting dependency (fast mode) %s", dep.name) .. ": ")
    else
        depprocess.notifier:setsectitle(tr("Extracting dependency %s", dep.name))
    end
    
    local filemap = { }
    local totalsize = 0

    local function getinstfiles(dir)
        if not os.fileexists(dir) then
            return
        end
        utils.recursivedir(dir, function(path, relpath)
            filemap[relpath] = { }
            filemap[relpath].size = os.filesize(path)
            filemap[relpath].path = path
            totalsize = totalsize + filemap[relpath].size
        end)
    end

    local basesrc = string.format("%s/deps/%s", internal.configdir, dep.name)
    getinstfiles(string.format("%s/files_all", basesrc))
    getinstfiles(string.format("%s/files_%s_all", basesrc, os.osname))
    getinstfiles(string.format("%s/files_all_%s", basesrc, os.arch))
    getinstfiles(string.format("%s/files_%s_%s", basesrc, os.osname, os.arch))

    local copiedsz = 0
    for rp in pairs(filemap) do
        local destdir
        local isdir = os.isdir(filemap[rp].path)
        if not isdir then
            destdir = utils.dirname(rp)
        else
            destdir = rp
        end

        destdir = pkg.getdepdir(dep, destdir)

        os.mkdirrec(destdir)

        copiedsz = copiedsz + filemap[rp].size

        local stat, msg
        if not isdir then -- only copy files as dir should already be created
            stat, msg = os.copy(filemap[rp].path, destdir)
            if not stat then
                install.print(string.format("WARNING: Failed to copy file '%s': %s\n", rp, msg))
            end
        end
        
        if stat and totalsize > 0 then
            setprogress(copiedsz / totalsize * 100)
        end
    end

    setprogress(100)
    if install.unattended then
        install.print("\n")
    end
end

function downloaddep(dep, archfiles, dlfile, setprogress)
    local function enddownload()
        if install.unattended then
            install.print("\n")
        else
            depprocess.notifier:setcancelbutton(false)
        end
    end

    if install.unattended then
        install.print("Downloading dependency " .. d.name .. ": ")
    else
        depprocess.notifier:setsectitle("Downloading dependency " .. d.name)
        depprocess.notifier:setcancelbutton(true)
    end

    
    local fmap = dofile(dlfile)
    for f, sum in pairs(fmap) do
        if archfiles[f] then
            local path = string.format("%s/deps/%s/%s", internal.rundir, dep.name, f)
            
            -- For unattended installs
            local retries, maxretries = 0, (cfg.unopts["dlretries"] and
                cfg.unopts["dlretries"].value and
                cfg.unopts["dlretries"].internal) or 3
            
            while true do
                local download, msg = internal.initdownload(string.format("%s/%s", dep.baseurl, f), path)
                
                if download then
                    function download:updateprogress(t, d)
                        if d > 0 then
                            setprogress(d/t*100)
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
                    local cancelled = (install.unattended and depprocess.notifier:cancelled()) or false
                    if not ignorefaileddl and not cancelled then
                        local usermsg = "Failed to download dependency"
                        
                        if install.unattended then
                            retries = retries + 1
                            retry = (retries <= maxretries)
                            
                            if msg then
                                install.print(string.format("%s: %s\n", usermsg, msg))
                            else
                                install.print(usermsg .. "\n")
                            end
                            
                            if retry then
                                install.print(string.format("Retrying ... (%d/%d)\n", retries, maxretries))
                            end
                        else
                            if msg then
                                usermsg = usermsg .. ":\n" .. msg
                            end
                            local choice = gui.choicebox(usermsg, "Retry", "Ignore", "Ignore all")
                            retry = (choice == 1)
                            ignorefaileddl = (choice == 3)
                        end
                    end
                    
                    if not retry then
                        --enablesecbar(false)
                        enddownload()
                        os.remove(path)
                        adddepprob(d, "faileddl")
                        return false
                    end
                else
                    break
                end
            end
        end
    end
    
    setprogress(100)
    enddownload()
end

function extractdep(dep, dest, archfiles, setprogress)
    if install.unattended then
        install.print(tr("Extracting dependency %s", dep.name) .. ": ")
    else
        depprocess.notifier:setsectitle(tr("Extracting dependency %s", dep.name))
    end
    
    local archives = { }
    for a in pairs(archfiles) do
        local apath = string.format("%s/deps/%s/%s", internal.rundir, dep.name, a)
        if os.fileexists(apath) then
            table.insert(archives, apath)
        end
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
            extrcmd = string.format("cat %s | gzip -cd | tar -xvf - 2>&1", f)
        elseif cfg.archivetype == "bzip2" then
            extrcmd = string.format("cat %s | bzip2 -d | tar -xvf - 2>&1", f)
        else
            extrcmd = string.format("(%s/lzma-decode %s - 2>/dev/null | tar -xvf -) 2>&1", internal.bindir, f)
        end
        
        local olddir = os.getcwd()
        os.chdir(dest)

        local pipe = check(io.popen(extrcmd))
        local line = pipe:read()
        while line do
            local file = string.gsub(line, "^x ", "")
            file = string.gsub(file, "\n$", "")
            
            if os.osname == "sunos" then
                -- Solaris puts some extra info after filename, remove
                file = string.gsub(file, ", [%d]* bytes, [%d]* tape blocks", "")
            end
            
            if szmap[f] and szmap[f][file] and totalsize > 0 then
                extractedsz = extractedsz + szmap[f][file]
                setprogress(extractedsz / totalsize * 100)
            end
            line = pipe:read()
        end
        
        pipe:close()
        os.chdir(olddir)
    end

    setprogress(100)
    if install.unattended then
        install.print("\n")
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
    local function setprogress(p)
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

    enablesecbar(true)

    if internal.fastrun then
        copydep(d, setprogress)
    else
        local src = string.format("%s/deps/%s", internal.rundir, d.name)
        local dest = string.format("%s/files", src)
        os.mkdirrec(dest)
        
        local archfiles = { }
        archfiles[string.format("%s_files_all", d.name)] = true
        archfiles[string.format("%s_files_%s_all", d.name, os.osname)] = true
        archfiles[string.format("%s_files_all_%s", d.name, os.arch)] = true
        archfiles[string.format("%s_files_%s_%s", d.name, os.osname, os.arch)] = true
        
        local dlfile = src .. "/dlfiles"
        if os.fileexists(dlfile) and d.baseurl then
            if not downloaddep(d, archfiles, dlfile, setprogress) then
                initdeps[d] = false
                enablesecbar(false)
                return false
            end
        end
        
        -- Used by unattended installs
        printedprog = 0
        
        extractdep(d, dest, archfiles, setprogress)
    end
    
    enablesecbar(false)
    initdeps[d] = true
    return true
end

function resetfaileddl()
    for dep, v in pairs(depprocess.wrongdeps) do
        if v.faileddl then
            initdeps[dep] = nil
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

function guesslibpaths(bins, libs)
    -- As there doesn't seem to be a cross platform way to figure out where
    -- libraries are usually found and even the OS specific ways are rather
    -- hackish, we just take a guess from the paths from all the libraries
    -- that are found.
    
    -- First add LD_LIBRARY_PATH's
    local ldpath = os.getenv("LD_LIBRARY_PATH")
    if ldpath then
        for p in string.gmatch(ldpath, "[^\:]+") do
            depprocess.libpaths[p] = true
        end
    end    
    
    local searched = { }
    local function getpaths(bin, deps)
        local map = maplibs(bin)
        if map then
            for l, p in pairs(map) do
                if not searched[l] then
                    searched[l] = true
                    if p then
                        depprocess.libpaths[utils.dirname(p)] = true
                        local ndeps = getnativedeps(deps, l)
                        if ndeps then
                            getpaths(p, ndeps)
                        end
                    else
                        local dep = getdepfromlib(deps, l)
                        if dep then
                            local path = getdeplibpath(dep, l)
                            getpaths(path, dep.deps)
                        end
                    end
                end
            end
        end
    end
    
    if bins then
        for _, b in ipairs(bins) do
            getpaths(install.getpkgdir(b), pkg.deps)
        end
    end
    
    if libs then
        for _, l in ipairs(libs) do
            getpaths(install.getpkgdir(l), pkg.deps)
        end
    end
end

-- Gets libraries with the same name, but different versions
function getversionlibs(lib)
    local libnover = string.match(lib, "^.*%.so") -- libname without versions
    local ret = { }

    if not utils.emptystring(libnover) then
        for p in pairs(depprocess.libpaths) do
            for f in io.dir(p) do
                local path = p .. "/" .. f
                if not os.isdir(path) then
                    if string.find(f, escapepat(libnover)) then
                        table.insert(ret, path)
                    end
                end
            end
        end
    end
    
    return ret
end

-- Splits library version into a table
function splitlibversion(lib)
    local split = { }
    for v in string.gmatch(lib, "%.(%d+)") do
        table.insert(split, v)
    end
    return split
end

-- Returns newer and older library (if any)
function checklibversion(lib)
    local older, newer = nil, nil
            
    local version = splitlibversion(lib)
    local others = getversionlibs(lib)
    
    if not utils.emptytable(version) and not utils.emptytable(others) then
        for _, l in ipairs(others) do
            local otherver = splitlibversion(l)
            for i, v in ipairs(version) do
                if v < otherver[i] then
                    newer = l
                elseif v > otherver[i] then
                    older = l
                end
                
                if newer and older then
                    return newer, older
                end
            end
        end
    end
    
    return newer, older
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
    
    if dep and dep.full and (dep.standalone or bininfo.native) then
        if initdep(dep) then
            local lpath = getdeplibpath(dep, lib)
            if os.fileexists(lpath) then
                bininfo.found = true
                bininfo.native = false
                bininfo.path = lpath
                bininfo.dep = dep
                bininfo.deps = dep.deps
                return true
            else
                install.print(string.format("WARNING: Supplied dependency '%s' does not have library '%s'\n", dep.name, lib))
                adddeplibprob(dep, "missing", lib)
            end
        end
    else
        if dep then
            local newer, older = checklibversion(lib)
            
            if older or newer then
                if not newer then
                    adddeplibprob(dep, "older", lib, older)
                elseif not older then
                    adddeplibprob(dep, "newer", lib, newer)
                else -- both
                    adddeplibprob(dep, "mismatch", lib)
                end
            else
                adddeplibprob(dep, "missing", lib)
            end
        else
            if bininfo.native then
                addlibprob(lib, "incompatlib", true)
            else
                local newer, older = checklibversion(lib)
                if older or newer then
                    if not newer then
                        addlibprob(lib, "older", older)
                    elseif not older then
                        addlibprob(lib, "newer", newer)
                    else -- both
                        addlibprob(lib, "mismatch", true)
                    end
                else
                    addlibprob(lib, "missing", true)
                end
            end            
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
                if (dep and dep.full and dep.required and dep:required()) or not path then
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
                            install.print(string.format("Overrided dependency: %s (%s)\n", infomap[lib].dep.name, lib))
                        elseif not infomap[lib].dep.handlecompat or not infomap[lib].dep:handlecompat(lib) then
                            adddeplibprob(infomap[lib].dep, "incompatdep", lib)
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
                ok = infomap[incompatlib].dep.handlecompat and infomap[incompatlib].dep:handlecompat(incompatlib)
            end
            
            if not ok then
                adddeplibprob(infomap[incompatlib].dep, "incompatdep", incompatlib)
                install.print(string.format("Incompatible dependency: %s (%s, %s)\n", infomap[incompatlib].dep.name, incompatlib, sym))
            else
                install.print(string.format("Overrided dependency: %s (%s)\n", infomap[incompatlib].dep.name, incompatlib))
                return true
            end
        end
    elseif not infomap[incompatlib].dep.handlecompat or not infomap[incompatlib].dep:handlecompat(incompatlib) then
        adddeplibprob(infomap[incompatlib].dep, "incompatdep", incompatlib)
        install.print(string.format("Incompatible dependency: %s (%s, %s)\n", infomap[incompatlib].dep.name, incompatlib, sym))
    end
    return false
end

function verifysyms(infomap)
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
                    -- Use basename, for main bins/libs as these may contain directories
                    local symb = utils.basename(b)
                    local supposedlib = loadedsyms and loadedsyms[symb] and loadedsyms[symb][sym]
                    
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
                        if supposedlib then
                            -- Main libs aren't checked, since we and the user can't fix them (UNDONE?)
                            if infomap[supposedlib] and infomap[supposedlib].found and
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
                                    addlibprob(supposedlib, "incompatlib", true)
                                elseif infomap[supposedlib].dep and (not infomap[supposedlib].dep.handlecompat or
                                        not infomap[supposedlib].dep:handlecompat(supposedlib)) then
                                    adddeplibprob(infomap[supposedlib].dep, "incompatdep", supposedlib)
                                end
                            end
                        else
                            -- Don't know where the sym should be defined
                            install.print(string.format("WARNING: Undefined symbol '%s' (used by %s)\n", sym, b))
                        end
                    end
                end
            end
        end
        checked[b] = true
    end
end

function checkdeps(bins, libs, bdir)
    if internal.fastrun and lsymstat == false and pkg.autosymmap then
        install.print("Automatically generating symbol map file.")
        autosymmap(internal.configdir, gettmpinterndir(), pkg.depmap)

        lsymstat, loadedsyms = pcall(dofile, gettmpinterndir("symmap.lua"))
    end
    
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
            
            local path
            if dep then
                path = getdeplibpath(dep, bin)
            else
                path = string.format("%s/%s", bdir, bin)
            end
            
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
                    verifysyms(infomap)
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
        if not needs[dep] and dep.required and dep:required() and dep.full then
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
                if dep.caninstall and not dep:caninstall() then
                    adddepprob(dep, "failed")
                else
                    dep:install()
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
        internal.newprogressdialog(function(self)
            depprocess.notifier = self
            local deps = checkdeps(bins, libs, install.getpkgdir())
            instdeps(deps, self)
        end)
    end
    
    local ret = { }
    for dep, probs in pairs(depprocess.wrongdeps) do
        ret[dep.name] = { }
        ret[dep.name].desc = dep.description
        
        for k, v in pairs(probs) do
            local str
            if k == "failed" then
                str = "Failed to install."
            elseif k == "faileddl" then
                str = "Failed to download."
            else -- Lib specific error
                for lib, other in pairs(v) do
                    if k == "incompatdep" then
                        str = tr("Incompatible library: %s.", lib)
                    elseif k == "missing" then
                        str = tr("Missing library: %s.", lib)
                    elseif k == "older" then
                        str = tr("Library %s is too old (required: %s).", other, lib)
                    elseif k == "newer" then
                        str = tr("Library %s is too new (required: %s).", other, lib)
                    elseif k == "mismatch" then
                        str = tr("Wrong library version (required: %s).", lib)
                    end
                end
            end
            
            assert(str)
            
            if ret[dep.name].problem then
                ret[dep.name].problem = ret[dep.name].problem .. "\n" .. str
            else
                ret[dep.name].problem = str
            end
        end
    end
    
    resetfaileddl()
    
    for k, v in pairs(depprocess.wronglibs) do
        ret[k] = { }
        ret[k].desc = ""
        if v.missing then
            ret[k].problem = "File missing."
        elseif v.incompatlib then
            ret[k].problem = "(Binary) incompatible."
        elseif v.older then
            ret[k].problem = tr("Too new (available: %s)", v.older)
        elseif v.newer then
            ret[k].problem = tr("Too old (available: %s)", v.older)
        elseif v.mismatch then
            ret[k].problem = "Version mismatch."
        end
        
        assert(ret[k].problem)
    end

    return ret
end

loadlua("deps-public.lua")
