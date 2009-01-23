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

-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)


dofile(ndir .. "/src/lua/shared/utils.lua")
dofile(ndir .. "/src/lua/shared/utils-public.lua")
dofile(ndir .. "/src/lua/shared/package-public.lua")


function Usage()
    print(string.format("Usage: %s <project dir> [ <installer name> ]\n", callscript))
    print([[
 <project dir>      The project directory which should be used to generate the installer.
 <installer name>   The file name of the created installer. Default: setup.sh.
]])
end

function CheckArgs()
    local opts, msg = getopt(args, "h", { {"help", false} })

    if not opts then
        print("Error: " .. msg)
        Usage()
        os.exit(1)
    end
    
    for _, o in ipairs(opts) do
        if o.name == "h" or o.name == "help" then
            Usage()
            os.exit(0)
        end
    end
    
    if not args[1] then
        print("Error: No project directory given.")
        Usage()
        os.exit(1)
    end

    confdir = args[1]
    outname = args[2] or "setup.sh"
    
    if not os.isdir(confdir) then
        ThrowError("'%s' does not exists or is not a directory.", confdir)
    end
end

function Clean()
    utils.removerec(confdir .. "/tmp")
end

function ThrowError(msg, ...)
--     print(debug.traceback())
    Clean()
    abort("Error: " .. string.format(msg, ...) .. "\n", 0)
end

function RequiredCopy(src, dest)
    local stat, msg = os.copy(src, dest)
    if not stat then
        ThrowError("Could not copy required file %s to %s: %s", src, dest, msg or "(No error message)")
    end
end

function RequiredCopyRec(src, dest)
    local stat, msg = utils.copyrec(src, dest)
    if not stat then
        ThrowError(msg)
    end
end

function PackDirectory(dir, file)
    if io.dir(dir)() == nil then -- Empty directory?
        return
    end
    
    local olddir = os.getcwd()
    local dirlist = { "." } -- Directories to process
    
    local sizesfile, msg = io.open(file .. ".sizes", "w")
    
    if sizesfile == nil then
        ThrowError("Could not create sizes file for archive %s: %s", file, msg)
    end
    
    local tarlistfname = file .. ".list"
    local tarlistfile, msg = io.open(tarlistfname, "w")
    
    if tarlistfile == nil then
        ThrowError("Could not create temporary file-list file for archive %s: %s", file, msg)
    end

    os.chdir(dir)

    while (#dirlist > 0) do
        local subdir = table.remove(dirlist) -- pop

        for f in io.dir(subdir) do
            local dpath
             
            -- Don't add "." to path, otherwise tar output may not be parsed well
            if (subdir ~= ".") then
                dpath = string.format("%s/%s", subdir, f)
            else
                dpath = f
            end
            
            local continue = true
            if (os.isdir(dpath)) then
                table.insert(dirlist, dpath)
                continue = (io.dir(dpath)() == nil) -- Only continue adding empty directories
            end
            
            if continue then
                local fsize, msg = os.filesize(dpath)
                if fsize == nil then
                    ThrowError("Could not get file size: %s", msg)
                end
                
                if os.isdir(dpath) then
                    dpath = dpath .. "/" -- Write tailing slash: most tar's will output it like this while extracting
                end
    
                local ret, msg = sizesfile:write(fsize, " ", dpath, "\n")
                if ret == nil then
                    ThrowError("Could not write to size file for archive %s: %s", file, msg)
                end
                
                ret, msg = tarlistfile:write(dpath, "\n")
                if ret == nil then
                    ThrowError("Could not write to temporary file-list file for archive %s: %s", file, msg)
                end
            end
        end
    end
    
    sizesfile:close()
    tarlistfile:close()
    
    -- Tar all files from the temp list file. Note that solaris (sunos) tar uses -I instead of -T used by other
    -- systems as list file option
    local listopt = "-T" -- Default to -T
    if (os.osname == "sunos") then
        -- Check if we're using GNU tar (Nexenta uses this by default, Solaris 10 doesn't)
        if (os.execute("tar --version 2>&1 | grep GNU >/dev/null") ~= 0) then
            listopt = "-I"
        end
    elseif (os.osname == "openbsd") then
        listopt = "-I"
    end
        
    if os.execute(string.format('tar cf "%s.tmp" %s "%s"', file, listopt, tarlistfname)) ~= 0 then
        ThrowError("Failed to create tar archive")
    end
    
    local stat
    if cfg.archivetype == "gzip" then
        -- use the '-n' option to omit timestamps. This is a quick hack to make sure that
        -- file sums stay the same when no files of the archive were changed.
        stat = os.execute(string.format('gzip -cn9 "%s.tmp" > "%s"', file, file))
    elseif cfg.archivetype == "bzip2" then
        stat = os.execute(string.format('cat "%s.tmp" | bzip2 -9 > "%s"', file, file)) -- Use cat so that bzip won't append ".bz2" to filename
    elseif cfg.archivetype == "lzma" then
        stat = os.execute(string.format('"%s" e "%s.tmp" "%s" 2>/dev/null', LZMABin, file, file))
    end
    
    if stat ~= 0 then
        ThrowError("Failed to compress tar archive.")
    end
    
    os.remove(tarlistfname)
    os.remove(file .. ".tmp")
    os.chdir(olddir)
end

-- Don't use this for large tables :)
function StrPack(tab)
    local ret
    for _, s in pairs(tab) do
        ret = (ret == nil) and s or ret .. " " .. s
    end
    return ret or ""
end

-- Used to traverse library specific directories for frontends(libc's, libstdc's)
function TraverseBinLibDir(bindir, lib)
    local dirs = { }
    for d in io.dir(bindir) do
        if (string.find(d, lib) and os.isdir(bindir .. "/" .. d)) then
            table.insert(dirs, d)
        end
    end
    
    -- Sort in reverse dir, so that we start with libraries with the highest version
    table.sort(dirs, function(a,b) return a > b end)
    
    local index = 0
    return function()
        index = index + 1
        return dirs[index]
    end
end

function GetFileDirs(basedir)
    -- Platform indepent files
    local ret = { }
    local dir = string.format("%s/files_all", basedir)
    
    if os.isdir(dir) then
        table.insert(ret, dir)
    end
    
    -- CPU dependent files
    for _, ARCH in pairs(cfg.targetarch) do
        dir = string.format("%s/files_all_%s", basedir, ARCH) 
        if os.isdir(dir) then
            table.insert(ret, dir)
        end
    end
    
    for _, OS in pairs(cfg.targetos) do
        -- OS dependent files
        dir = string.format("%s/files_%s_all", basedir, OS) 
        if os.isdir(dir) then
            table.insert(ret, dir)
        end
        
        -- OS/ARCH dependent files
        for _, ARCH in pairs(cfg.targetarch) do
            dir = string.format("%s/files_%s_%s", basedir, OS, ARCH) 
            if os.isdir(dir) then
                table.insert(ret, dir)
            end
        end
    end
    
    return ret
end

function GetAllBins(path, bins)
    local allbins = { }
    local function getbins(path) -- Gets bins from all highest sub directories
        for f in io.dir(path) do
            local p = path .. "/" .. f
            if os.isdir(p) then
                getbins(p)
            elseif utils.tablefind(bins, f) then
                table.insert(allbins, p)
            end
        end
    end
    
    -- Get all binaries
    getbins(path)
    return allbins
end

function InitDeltaBins()
    local path = string.format("%s/bin/bindeltas.lua", ndir)
    if os.fileexists(path) then
        local regen = false
        if not pcall(dofile, path) then
            print("WARNING: Failed to load bin delta file, regenerating...")
            regen = true
        elseif not binsums or not deltasizes then
            print("WARNING: Incorrect bin delta file, regenerating...")
            regen = true
        elseif false then -- UNDONE: Remove!
            for bin, sum in pairs(binsums) do
                if io.md5(bin) ~= sum then
                    print("WARNING: One or more binaries changed since last time, regenerating bin delta file...")
                    regen = true
                    break
                end
            end
        end
        
        if not regen then
            return
        end
    end
    -- Generate new one
    
    print("WARNING: No bin delta file, regenerating...")
    
    local allbins = GetAllBins(ndir .. "/bin", { "fltk", "ncurs", "gtk" })
    
    -- Create big map, containing all possible delta sizes for every bin
    deltasizes = { }
    local tmpfile = os.tmpname()
    for _, base in ipairs(allbins) do
        for _, bin in ipairs(allbins) do
            if bin ~= base then
                os.execute(string.format("\"%s\" -q delta \"%s\" \"%s\" \"%s\"", EdeltaBin, base, bin, tmpfile))
                deltasizes[bin] = deltasizes[bin] or { }
                deltasizes[bin][base] = os.filesize(tmpfile)
            end
        end
    end
    os.remove(tmpfile)
    
    -- Store results
    local out, msg = io.open(path, "w")
    if not out then
        ThrowError("Failed to create bin delta file: %s", msg)
    end
    
    out:write(string.format([[
-- Automaticly generated on %s by geninstall.lua.
deltasizes = { }
]], os.date()))
    
    for bin, bases in pairs(deltasizes) do
        out:write(string.format("deltasizes[\"%s\"] = { }\n", bin))
        for b, s in pairs(bases) do
            out:write(string.format("deltasizes[\"%s\"][\"%s\"] = %d\n", bin, b, s))
        end
    end
    
    out:write("binsums = { }\n")
    for _, bin in ipairs(allbins) do
        out:write(string.format("binsums[\"%s\"] = \"%s\"\n", bin, io.md5(bin)))
    end
    
    out:close()
end

function GetOptDeltas(binlist)
    local topbin
    local optdeltas = { }
    local stack = { binlist }
    while true do
        local bins = table.remove(stack)
        local opts = { }
        
        for _, bin in ipairs(bins) do
            for base, s in pairs(deltasizes[bin]) do
                if (not opts[bin] or opts[bin].size > s) and utils.tablefind(bins, base) then
                    opts[bin] = opts[bin] or { }
                    opts[bin].size = s
                    opts[bin].base = base
                end
            end
        end
        
        local newbins = { }
        for bin, ideal in pairs(opts) do
            if opts[ideal.base] and opts[ideal.base].base == bin then -- Already got sub as base?
                local cursizediff = os.filesize(bin) - ideal.size
                local othersizediff = os.filesize(ideal.base) - opts[ideal.base].size
                if cursizediff <= othersizediff then
                    -- Current is fine, clear other
                    opts[ideal.base] = nil
                    table.insert(newbins, ideal.base)
                else
                    -- Other one is better, clear current
                    opts[bin] = nil
                    table.insert(newbins, bin)
                end
            end
        end
        
        utils.tablemerge(optdeltas, opts)
    
        if not utils.emptytable(newbins) then
            table.insert(stack, newbins)
        else
            assert(#bins == 1)
            topbin = bins[1]
            break
        end
    end
    return optdeltas, topbin
end

function LoadPackage()
    if not os.fileexists(confdir .. "/package.lua") then
        return
    end
    
    loadpackagecfg(confdir)
    
    local function checkfield(s)
        if not pkg[s] or #pkg[s] == 0 then
            ThrowError("Package " .. s .. " not specified.")
        end
    end
    
    checkfield("name")
    checkfield("version")
    checkfield("maintainer")
    checkfield("description")
    checkfield("summary")
    checkfield("group")
    
    dofile(ndir .. "/src/lua/pkg/groups.lua")
    if not pkg.grouplist[pkg.group] then
        ThrowError("Wrong package group specified.")
    end
end

function Init()
    -- Strip all trailing /'s
    local _, i = string.find(string.reverse(confdir), "^/+")
    
    if (i) then
        OLDG.confdir = string.sub(confdir, 1, (-i- 1))
    end
    
    if (not string.find(confdir, "^/")) then
        confdir = curdir .. "/" .. confdir -- Append current dir if confdir isn't an absolute path
    end
    
    loadconfig(confdir)
    
    LoadPackage()
    
    -- Find a LZMA and edelta bin which we can use
    local basebindir = string.format("%s/bin/%s/%s", ndir, os.osname, os.arch)
    local validbin = function(bin)
                        -- Does the bin exists and 'ldd' can find all dependend libs?
                        if os.fileexists(bin) then
                            if os.osname == "openbsd" then
                                return (os.execute(string.format("ldd \"%s\" >/dev/null 2>&1", bin)) == 0)
                            else
                                return (os.execute(string.format("ldd \"%s\" | grep \"not found\" >/dev/null", bin)) ~= 0)
                            end
                        end
                        return false
                     end
    
    for lc in TraverseBinLibDir(basebindir, "^libc") do
        local lcdir = string.format("%s/%s", basebindir, lc)
        local lz = lcdir .. "/" .. "lzma"
        local ed = lcdir .. "/" .. "edelta"

        -- File exists and 'ldd' doesn't report missing lib deps?
        if (validbin(lz)) then
            LZMABin = lz
        end
        
        if (validbin(ed)) then
            EdeltaBin = ed
        end
        
        if LZMABin and EdeltaBin then
            break
        end
    end
    
    if (not LZMABin) then
        ThrowError("Could not find a suitable LZMA encoder")
    elseif (not EdeltaBin) then
        ThrowError("Could not find a suitable edelta encoder")
    end
    
    InitDeltaBins()
    
print(string.format([[
Configuration:
---------------------------------
Installer name: %s
            OS: %s
         Archs: %s
  Archive type: %s
     Languages: %s
    Config dir: %s
     Frontends: %s
     Intro pic: %s
          Logo: %s
       Appicon: %s
---------------------------------
]], outname, StrPack(cfg.targetos), StrPack(cfg.targetarch), cfg.archivetype, StrPack(cfg.languages), confdir, StrPack(cfg.frontends), cfg.intropic or "None", cfg.logo or "Default", cfg.appicon or "Default"))
end

-- Creates directory layout for installer archive
function PrepareArchive()
    local destdir = confdir .. "/tmp/config"
    os.mkdirrec(destdir)
    
    -- Configuration files
    RequiredCopy(confdir .. "/config.lua", destdir)
    os.copy(confdir .. "/package.lua", destdir)
    os.copy(confdir .. "/run.lua", destdir)
    os.copy(confdir .. "/welcome", destdir)
    os.copy(confdir .. "/license", destdir)
    os.copy(confdir .. "/finish", destdir)
    
    -- Some internal stuff
    RequiredCopy(ndir .. "/src/internal/startupinstaller.sh", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/internal/utils.sh", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/internal/about", confdir .. "/tmp")
    
    -- Lua scripts
    os.mkdirrec(confdir .. "/tmp/shared")
    RequiredCopy(ndir .. "/src/lua/deps.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/deps-public.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/main.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/install.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/attinstall.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/unattinstall.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/package.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/package-public.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/pkg/dpkg.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/pkg/generic.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/pkg/groups.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/pkg/pacman.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/pkg/rpm.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/pkg/slack.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/shared/package-public.lua", confdir .. "/tmp/shared")
    RequiredCopy(ndir .. "/src/lua/shared/utils.lua", confdir .. "/tmp/shared")
    RequiredCopy(ndir .. "/src/lua/shared/utils-public.lua", confdir .. "/tmp/shared")
    RequiredCopy(ndir .. "/src/lua/screens/finishscreen.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/screens/installscreen.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/screens/langscreen.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/screens/licensescreen.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/screens/packagedirscreen.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/screens/packagetogglescreen.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/screens/selectdirscreen.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/screens/summaryscreen.lua", confdir .. "/tmp")
    RequiredCopy(ndir .. "/src/lua/screens/welcomescreen.lua", confdir .. "/tmp")
    
    -- XDG utils
    os.mkdir(confdir .. "/tmp/xdg-utils")
    RequiredCopy(ndir .. "/src/xdg-utils/xdg-desktop-menu", confdir .. "/tmp/xdg-utils")
    RequiredCopy(ndir .. "/src/xdg-utils/xdg-open", confdir .. "/tmp/xdg-utils")
    
    -- Language files
    for _, f in pairs(cfg.languages) do
        local langsrc = confdir .. "/lang/" .. f
        local langdest = destdir .. "/lang/" .. f
        
        os.mkdirrec(langdest)
        
        os.copy(langsrc .. "/strings", langdest)
        os.copy(langsrc .. "/config.lua", langdest)
        os.copy(langsrc .. "/welcome", langdest)
        os.copy(langsrc .. "/license", langdest)
        os.copy(langsrc .. "/finish", langdest)
    end
    
    -- 'Extra' files
    if os.isdir(confdir .. "/files_extra/") then
        os.mkdir(confdir .. "/tmp/files_extra")
        RequiredCopyRec(confdir .. "/files_extra/", confdir .. "/tmp/files_extra/")
    end
    
    print("Preparing/copying frontend binaries")
    
    local binlist, frlist = { }, { }
    local copybins = { "edelta", "surunner" }
    
    if cfg.archivetype == "lzma" then
        table.insert(copybins, "lzma-decode")
    end
    
    local copyfr = { }
    for _, fr in ipairs(cfg.frontends) do
        if fr == "ncurses" then
            table.insert(copyfr, "ncurs")
        else
            table.insert(copyfr, fr)
        end
    end
    
    for _, OS in ipairs(cfg.targetos) do
        for _, ARCH in ipairs(cfg.targetarch) do
            local dir = string.format("%s/bin/%s/%s", ndir, OS, ARCH)
            if not os.fileexists(dir) then
                print(string.format("Warning: No frontends for %s/%s", OS, ARCH))
            else
                utils.tableappend(binlist, GetAllBins(dir, copybins))
                utils.tableappend(frlist, GetAllBins(dir, copyfr))
            end
        end
    end
    
    -- Copy all helper bins
    for _, bin in ipairs(binlist) do
        local relbinpath = string.gsub(bin, "^.*/bin", "bin")
        local destpath = string.format("%s/tmp/%s", confdir, relbinpath)
        os.mkdirrec(utils.dirname(destpath))
        RequiredCopy(bin, destpath)
    end
    
    -- Find optimal delta paths
    local optdeltas, topbin = GetOptDeltas(frlist)
    os.mkdir(confdir .. "/tmp/bin")
    local deltafile = io.open(string.format("%s/tmp/bin/deltas", confdir), "w")
    
    if not deltafile then
        ThrowError("Failed to open delta file")
    end
    
    -- Copy and prepare all frontends
    for _, bin in ipairs(frlist) do
        local relbinpath = string.gsub(bin, "^.*/bin", "bin")
        local destpath = string.format("%s/tmp/%s", confdir, relbinpath)
        local binname = utils.basename(bin)
        
        os.mkdirrec(utils.dirname(destpath))

        if optdeltas[bin] then
            local relbase = string.gsub(optdeltas[bin].base, "^.*/bin", "bin")
            deltafile:write(string.format("%s %s\n", relbinpath, relbase))
        end

        if not optdeltas[bin] then -- Top level bin?
            if cfg.archivetype == "lzma" then
                if os.execute(string.format("\"%s\" e \"%s\" \"%s.lzma\" 2>/dev/null", LZMABin,
                            bin, destpath)) ~= 0 then
                    ThrowError("Failed to pack binary %s", relbinpath)
                end
            else
                RequiredCopy(bin, destpath)
            end
        else
            local base = optdeltas[bin].base
            if os.execute(string.format("\"%s\" -q delta \"%s\" \"%s\" \"%s.diff\"", EdeltaBin,
                                        base, bin, destpath)) ~= 0 then
                ThrowError("Failed to diff binary: %s", relbinpath)
            end
            
            if cfg.archivetype == "lzma" then
                if os.execute(string.format("\"%s\" e \"%s.diff\" \"%s.diff.lzma\" 2>/dev/null", LZMABin,
                            destpath, destpath)) ~= 0 then
                    ThrowError("Failed to pack binary %s", relbinpath)
                end
                os.remove(destpath)
            end
        end
    end
    
    deltafile:close()
    
    -- Intro picture; for backward compatibility we also check the main project dir
    if cfg.intropic ~= nil then
        if not os.fileexists(string.format("%s/files_extra/%s" , confdir, cfg.intropic)) then
            ret, msg = os.copy(string.format("%s/%s" , confdir, cfg.intropic), string.format("%s/tmp/files_extra/", confdir))
            if ret == nil then
                print(string.format("Warning could not copy intro picture: %s", msg))
            end
        end
    end

    if cfg.logo ~= nil then
        RequiredCopy(string.format("%s/files_extra/%s" , confdir, cfg.logo), string.format("%s/tmp/", confdir))
    else
        -- Default logo
        RequiredCopy(ndir .. "/src/img/installer.png", confdir .. "/tmp")
    end
    
    if cfg.appicon ~= nil then
        RequiredCopy(string.format("%s/files_extra/%s" , confdir, cfg.appicon), string.format("%s/tmp/", confdir))
    else
        -- Default icon
        RequiredCopy(ndir .. "/src/img/appicon.xpm", confdir .. "/tmp")
    end

    -- Internal config file(only stores archive type for now)
    local inffile, msg = io.open(string.format("%s/tmp/info", confdir), "w")

    if inffile == nil then
        ThrowError("Could not create internal info file for archive: %s", msg)
    end
                
    inffile:write(cfg.archivetype)
    inffile:close()
    
    if pkg.enable then
        local depmap = { }

        -- Add deps
        if pkg.deps then
            local stack = { pkg.deps }
            while not utils.emptytable(stack) do
                local deps = table.remove(stack)
                for _, dep in ipairs(deps) do
                    if not depmap[dep] then
                        local d, msg = loaddep(confdir, dep)
                        if not d then
                            ThrowError("Failed to load dependency: %s", msg)
                        end
                        depmap[dep] = d
                        table.insert(stack, d.deps)
                    end
                end
            end
            
            for dep in pairs(depmap) do
                local src = string.format("%s/deps/%s", confdir, dep)
                local dest = string.format("%s/tmp/deps/%s", confdir, dep)
                local dlfile
                
                os.mkdirrec(dest)
                
                local cfgfile = string.format("%s/config.lua", src)
                RequiredCopy(cfgfile, dest)
                
                if pkg.externdeps and utils.tablefind(pkg.externdeps, dep) then
                    dlfile = io.open(string.format("%s/tmp/deps/%s/dlfiles", confdir, dep), "w")
                    if not dlfile then
                        ThrowError("Failed to create list file for downloadable files.")
                    end
                    dlfile:write("local ret = { }\n")
                end
                
                local dirs = GetFileDirs(src)
                local archdest = string.format("%s/deps", confdir)
                for _, d in ipairs(dirs) do
                    local f = string.format("%s/%s_%s", dest, dep, utils.basename(d))
                    PackDirectory(d, f)
                    if dlfile then
                        dlfile:write(string.format("ret[\"%s_%s\"] = \"%s\"\n", dep, utils.basename(d), io.md5(f)))
                        -- Move archive away, but keep .sizes file
                        if os.execute(string.format("mv %s %s", f, archdest)) ~= 0 then
                            ThrowError("Failed to move downloadable dependency archive.")
                        end
                    end
                end
                
                if dlfile then
                    dlfile:write("return ret\n")
                    dlfile:close()
                end
            end
        end
        
        -- Symbol map
        local sympath = confdir .. "/symmap.lua"
        if os.fileexists(sympath) then
            os.copy(sympath, destdir)
        elseif pkg.autosymmap then
            local binlist = "" -- All bins/libs for symmap
            local pathlist = ""
            local givenpaths = { }
            local function addbins(startpath, bins, subpath)
                for _, b in ipairs(bins) do
                    if subpath then
                        b = subpath .. "/" .. b
                    end
                    local path = string.format("%s/files_%s_%s/%s", startpath, os.osname, os.arch, b)
                    binlist = binlist .. " " .. path
                    
                    local dirp = utils.dirname(path)
                    if not givenpaths[p] then
                        pathlist = string.format("%s -l %s", pathlist, dirp)
                        givenpaths[dirp] = true
                    end
                end
            end
            
            if pkg.bins then
                addbins(confdir, pkg.bins)
            end
            
            if pkg.libs then
                addbins(confdir, pkg.libs)
            end
            
            for n, d in pairs(depmap) do
                if d.libs and d.full then
                    addbins(string.format("%s/deps/%s", confdir, n), d.libs, d.libdir)
                end
            end
            
            if os.execute(string.format("%s/gensyms.sh %s -d %s %s", ndir, pathlist, destdir, binlist)) ~= 0 then
                print("WARNING: Failed to automaticly generate symbol map file.")
            end
        end
    end
end

function CreateArchive()
    local archdir
    local basename = "instarchive"

    print("Generating archive...")
    
    local dirs = GetFileDirs(confdir)
    
    for _, d in ipairs(dirs) do
        PackDirectory(d, string.format("%s/tmp/%s", confdir, string.gsub(utils.basename(d), "files", basename)))
    end
end

function CreateInstaller()
    print("Generating installer...")
    if (not string.find(outname, "^/")) then
        outname = curdir .. "/" .. outname
    end
    
    -- Add nixstaller specific options
    local nixstopts = ""
    local function addarg(arg, val)
        if val then
            nixstopts = string.format("%s %s \"%s\"", nixstopts, arg, val)
        end
    end
    
    addarg("--instname", cfg.appname)
    addarg("--instpack", cfg.archivetype)
    addarg("--instmode", cfg.mode)
    addarg("--instos", tabtostr(cfg.targetos))
    addarg("--instarch", tabtostr(cfg.targetarch))
    addarg("--instfrontends", tabtostr(cfg.frontends))
    addarg("--pkgname", pkg.name)
    if pkg.version then
        addarg("--pkgversion", pkg.version .. "-" .. pkg.release)
    end
    addarg("--pkgsummary", pkg.summary)
    addarg("--pkgdesc", pkg.description)
    addarg("--pkggroup", pkg.group)
    addarg("--pkglicense", pkg.license)
    addarg("--pkgmaint", pkg.maintainer)
    addarg("--pkgurl", pkg.url)
    
    local function addoptarg(n, a)
        local left
        if not utils.emptystring(a.short) then
            left = string.format("--%s, -%s", n, a.short)
        else
            left = string.format("--%s", n)
        end
        
        if not utils.emptystring(a.optname) then
            left = string.format("%s <%s>", left, a.optname)
        end
        
        local right = a.desc
        if a.required then
            right = right .. " (Required)"
        end
        
        -- Get rid of single ' chars
        right = string.gsub(right, "'", "'\"'\"'")

        local opthandler, optchecker
        if not utils.emptystring(a.short) then
            opthandler = string.format("   -%s | --%s)\n", a.short, n)
        else
            opthandler = string.format("   --%s)\n", n)
        end
        
        if a.required then
            local optchk = string.format("has%s", n)
            optchk = string.gsub(optchk, "%-", "%_") -- sh doesn't like dashes inside variable names
            opthandler = string.format("%s    %s=1\n", opthandler, optchk)
            -- $unattended is set by script header.
            optchecker = string.format([[
if [ -z "$%s" -a ! -z "$unattended" ]; then
    echo Required option "%s" not given. >&2
    MS_Help
    exit 1
fi]], optchk, n)
        end

        if a.opttype then
            opthandler = string.format("%s    scriptargs=\"\$scriptargs --%s $2\"\n    shift 2\n", opthandler, n)
        else
            opthandler = string.format("%s    scriptargs=\"\$scriptargs --%s\"\n    shift\n", opthandler, n)
        end
        
        opthandler = opthandler .. "    gaveunopt=1\n    ;;\n"
        nixstopts = string.format("%s --addopthandler \'%s\' --addopthelp \'%-28s%s\'", nixstopts, opthandler, left, right)
        
        if optchecker then
            nixstopts = string.format("%s --addoptchecker \'%s\'", nixstopts, optchecker)
        end
    end
    
    if cfg.mode == "both" or cfg.mode == "unattended" then
        for n, a in pairs(cfg.unopts) do
            addoptarg(n, a)
        end
    end
    
    local label = string.format("Installer for %s", cfg.appname)
    os.execute(string.format("\"%s/makeself.sh\" --gzip --header %s/src/internal/instheader.sh %s \"%s/tmp\" \"%s\" \"%s\" sh ./startupinstaller.sh > /dev/null 2>&1", ndir, ndir, nixstopts, confdir, outname, label))
end

CheckArgs()
Clean()
Init()
PrepareArchive()
CreateArchive()
CreateInstaller()
Clean()
