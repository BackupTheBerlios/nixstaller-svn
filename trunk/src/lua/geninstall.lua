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

function PackDirectory(dir, file, sizes)
    if sizes == nil then
        sizes = true -- Default
    end
    
    if io.dir(dir)() == nil then -- Empty directory?
        return
    end
    
    local olddir = os.getcwd()
    local dirlist = { "." } -- Directories to process
    
    local sizesfile, msg
    if sizes then
        sizesfile, msg = io.open(file .. ".sizes", "w")
        
        if sizesfile == nil then
            ThrowError("Could not create sizes file for archive %s: %s", file, msg)
        end
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
                
                if sizes then
                    local ret, msg = sizesfile:write(fsize, " ", dpath, "\n")
                    if ret == nil then
                        ThrowError("Could not write to size file for archive %s: %s", file, msg)
                    end
                end
                
                ret, msg = tarlistfile:write(dpath, "\n")
                if ret == nil then
                    ThrowError("Could not write to temporary file-list file for archive %s: %s", file, msg)
                end
            end
        end
    end
    
    if sizes then
        sizesfile:close()
    end
    
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
    local function getbins(path) -- Gets bins from current and all sub directories
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

function GetRelBinPath(bin)
    local relbinpath = string.gsub(bin, "^.*/bin", "bin")
    return relbinpath
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
    
    -- Find a LZMA bin which we can use
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

        -- File exists and 'ldd' doesn't report missing lib deps?
        if validbin(lz) then
            LZMABin = lz
            break
        end
    end
    
    -- Check for static bins if not found yet
    local path = basebindir .. "/lzma"
    if not LZMABin and os.fileexists(path) then
        LZMABin = path
    end   
    
    if not LZMABin then
        ThrowError("Could not find a suitable LZMA encoder")
    end   
    
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
    -- Nearly all files required at runtime are placed inside 'archdir'. These files are
    -- placed inside an additional tar archive for optimal compression. The other files consist
    -- of those who are required to handle this archive (eg startupinstaller.sh and lzma bins)
    local archdir = confdir .. "/tmp/arch"
    local rootdir = confdir .. "/tmp"
    local instconfdir = archdir .. "/config"

    -- Part 1: Copy files that should be packed to sub archive
    ----------------------------------------------------------
    
    os.mkdirrec(instconfdir)
    
    -- Configuration files
    RequiredCopy(confdir .. "/config.lua", instconfdir)
    os.copy(confdir .. "/package.lua", instconfdir)
    os.copy(confdir .. "/run.lua", instconfdir)
    os.copy(confdir .. "/welcome", instconfdir)
    os.copy(confdir .. "/license", instconfdir)
    os.copy(confdir .. "/finish", instconfdir)
    
    -- Some internal stuff
    RequiredCopy(ndir .. "/src/internal/about", archdir)
    RequiredCopy(ndir .. "/src/internal/start", archdir)
    
    -- Terminfo's for BSD's
    for _, tos in ipairs(cfg.targetos) do
        if tos == "freebsd" or tos == "netbsd" or tos == "openbsd" then
            local p = archdir .. "/terminfo"
            os.mkdirrec(p)
            RequiredCopyRec(ndir .. "/src/internal/terminfo", p)
            break
        end
    end
    
    -- Lua scripts
    os.mkdirrec(archdir .. "/shared")
    RequiredCopy(ndir .. "/src/lua/deps.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/deps-public.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/main.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/install.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/attinstall.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/unattinstall.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/package.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/package-public.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/pkg/dpkg.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/pkg/generic.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/pkg/groups.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/pkg/pacman.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/pkg/rpm.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/pkg/slack.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/shared/package-public.lua", archdir .. "/shared")
    RequiredCopy(ndir .. "/src/lua/shared/utils.lua", archdir .. "/shared")
    RequiredCopy(ndir .. "/src/lua/shared/utils-public.lua", archdir .. "/shared")
    RequiredCopy(ndir .. "/src/lua/screens/finishscreen.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/screens/installscreen.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/screens/langscreen.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/screens/licensescreen.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/screens/packagedirscreen.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/screens/packagetogglescreen.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/screens/selectdirscreen.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/screens/summaryscreen.lua", archdir)
    RequiredCopy(ndir .. "/src/lua/screens/welcomescreen.lua", archdir)
    
    -- XDG utils
    os.mkdir(archdir .. "/xdg-utils")
    RequiredCopy(ndir .. "/src/xdg-utils/xdg-desktop-menu", archdir .. "/xdg-utils")
    RequiredCopy(ndir .. "/src/xdg-utils/xdg-open", archdir .. "/xdg-utils")
    
    -- Language files
    for _, f in pairs(cfg.languages) do
        local langsrc = confdir .. "/lang/" .. f
        local langdest = instconfdir .. "/lang/" .. f
        
        os.mkdirrec(langdest)
        
        os.copy(langsrc .. "/strings", langdest)
        os.copy(langsrc .. "/config.lua", langdest)
        os.copy(langsrc .. "/welcome", langdest)
        os.copy(langsrc .. "/license", langdest)
        os.copy(langsrc .. "/finish", langdest)
    end
    
    -- 'Extra' files
    if os.isdir(confdir .. "/files_extra/") then
        os.mkdir(archdir .. "/files_extra")
        RequiredCopyRec(confdir .. "/files_extra/", archdir .. "/files_extra/")
    end
    
    print("Preparing/copying frontend binaries")
    
    local binlist = { }
    local copybins = { "surunner", "lock" }
    
    for _, fr in ipairs(cfg.frontends) do
        if fr == "ncurses" then
            table.insert(copybins, "ncurs")
        else
            table.insert(copybins, fr)
        end
    end
    
    for _, OS in ipairs(cfg.targetos) do
        for _, ARCH in ipairs(cfg.targetarch) do
            local dir = string.format("%s/bin/%s/%s", ndir, OS, ARCH)
            if not os.fileexists(dir) then
                print(string.format("Warning: No frontends for %s/%s", OS, ARCH))
            else
                utils.tableappend(binlist, GetAllBins(dir, copybins))
            end
        end
    end
    
    -- Copy all bins
    for _, bin in ipairs(binlist) do
        local relbinpath = GetRelBinPath(bin)
        local destpath = string.format("%s/%s", archdir, relbinpath)
        os.mkdirrec(utils.dirname(destpath))
        RequiredCopy(bin, destpath)
    end
       
    -- Intro picture; for backward compatibility we also check the main project dir
    if cfg.intropic ~= nil then
        if not os.fileexists(string.format("%s/files_extra/%s" , confdir, cfg.intropic)) then
            ret, msg = os.copy(string.format("%s/%s" , confdir, cfg.intropic), string.format("%s/files_extra/", archdir))
            if ret == nil then
                print(string.format("Warning could not copy intro picture: %s", msg))
            end
        end
    end

    if cfg.logo ~= nil then
        RequiredCopy(string.format("%s/files_extra/%s" , confdir, cfg.logo), archdir)
    else
        -- Default logo
        RequiredCopy(ndir .. "/src/img/installer.png", archdir)
    end
    
    if cfg.appicon ~= nil then
        RequiredCopy(string.format("%s/files_extra/%s" , confdir, cfg.appicon), archdir)
    else
        -- Default icon
        RequiredCopy(ndir .. "/src/img/appicon.xpm", archdir)
    end

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
                            ThrowError("Failed to load dependency %s: %s", dep, msg)
                        end
                        depmap[dep] = d
                        table.insert(stack, d.deps)
                    end
                end
            end
            
            for dep in pairs(depmap) do
                local src = string.format("%s/deps/%s", confdir, dep)
                local dest = string.format("%s/deps/%s", archdir, dep)
                local dlfile
                
                os.mkdirrec(dest)
                
                local cfgfile = string.format("%s/config.lua", src)
                RequiredCopy(cfgfile, dest)
                
                if pkg.externdeps and utils.tablefind(pkg.externdeps, dep) then
                    dlfile = io.open(string.format("%s/deps/%s/dlfiles", archdir, dep), "w")
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
            os.copy(sympath, instconfdir)
        elseif pkg.autosymmap then
            print("Automatically generating symbol map file.")
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
            
            if os.execute(string.format("%s/gensyms.sh %s -d %s %s", ndir, pathlist, instconfdir, binlist)) ~= 0 then
                print("WARNING: Failed to automaticly generate symbol map file.")
            end
        end
    end
    
    -- Installation files. These are archived additionally to reduce startup time.
    print("Generating file archive(s)...")

    local dirs = GetFileDirs(confdir)
    
    for _, d in ipairs(dirs) do
        PackDirectory(d, string.format("%s/%s", archdir, string.gsub(utils.basename(d), "files", "instarchive")))
    end

    -- Part 2: Copy all files required outside sub archive
    --------------------------------------------------------
    
    -- Startup scripts
    RequiredCopy(ndir .. "/src/internal/startupinstaller.sh", rootdir)
    RequiredCopy(ndir .. "/src/internal/utils.sh", rootdir)

    -- lzma bins
    if cfg.archivetype == "lzma" then
        local binlist = { }
        for _, OS in ipairs(cfg.targetos) do
            for _, ARCH in ipairs(cfg.targetarch) do
                local dir = string.format("%s/bin/%s/%s", ndir, OS, ARCH)
                if os.fileexists(dir) then
                    utils.tableappend(binlist, GetAllBins(dir, { "lzma-decode" }))
                end
            end
        end
        
        -- Copy all bins
        for _, bin in ipairs(binlist) do
            local relbinpath = GetRelBinPath(bin)
            local destpath = string.format("%s/%s", rootdir, relbinpath)
            os.mkdirrec(utils.dirname(destpath))
            RequiredCopy(bin, destpath)
        end
    end
    
        -- Internal config file (only stores archive type for now)
    local inffile, msg = io.open(string.format("%s/info", rootdir), "w")

    if inffile == nil then
        ThrowError("Could not create internal info file for archive: %s", msg)
    end
                
    inffile:write(cfg.archivetype)
    inffile:close()
    
    -- Part 3: Create sub archive
    -----------------------------------------------------------
    PackDirectory(archdir, rootdir .. "/subarch", false)
    utils.removerec(archdir) -- Don't need those anymore
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
            nixstopts = string.format("%s %s \'%s\'", nixstopts, arg, val)
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
CreateInstaller()
Clean()
