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

-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)

dofile(maindir .. "/src/lua/shared/utils-public.lua")
dofile(maindir .. "/src/lua/shared/package-public.lua")

function Clean()
    if (os.fileexists(confdir .. "/tmp")) then
        local newdirlist = { confdir .. "/tmp" }
        local olddirlist = { }
        
        while (#newdirlist > 0) do
            local d = newdirlist[#newdirlist]
            local newdir = false
    
            for f in io.dir(d) do
                local dpath = string.format("%s/%s", d, f)
                if (os.isdir(dpath)) then
                    local processed = false
                    for _, o in pairs(olddirlist) do
                        if (o == dpath) then
                            processed = true
                            break
                        end
                    end
                    
                    if not processed then
                        table.insert(newdirlist, dpath)
                        newdir = true
                        break
                    end
                else
                    os.remove(dpath)
                end
            end
            
            if not newdir then
                local d = table.remove(newdirlist)
                table.insert(olddirlist, d)
                os.remove(d)
            end
        end
    end
end

function ThrowError(msg, ...)
    print(debug.traceback())
    Clean()
    error(string.format(msg .. "\n", ...), 0)
end

function RequiredCopy(src, dest)
    local stat, msg = os.copy(src, dest)
    if (not stat) then
        ThrowError("Error could not copy required file %s to %s: %s", src, dest, msg or "(No error message)")
    end
end

function RecursiveCopy(src, dest)
    local dirlist = { "." }
    local ret, msg
    
    while (#dirlist > 0) do
        local subdir = table.remove(dirlist) -- pop
        local srcpath = string.format("%s/%s/", src, subdir)
        local destpath = string.format("%s/%s/", dest, subdir)
        
        for f in io.dir(srcpath) do
            local fsrc = srcpath .. f
            local fdest = destpath .. f
            if (os.isdir(fsrc)) then
                table.insert(dirlist, subdir .. "/" .. f)
                ret, msg = os.mkdir(fdest)
                if ret == nil then
                    ThrowError("Warning could not create subdirectory file: %s", msg)
                end
            else
                RequiredCopy(fsrc, fdest)
            end
        end
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
        stat = os.execute(string.format('gzip -c9 "%s.tmp" > "%s"', file, file))
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

function LoadPackage()
    if not os.fileexists(confdir .. "/package.lua") then
        return
    end
    
    -- These functions may be needed in the config package.lua, but are defined only at install
    -- UNDONE: Still needed?
    function pkg.addbinenv() end
    function pkg.addbinopts() end
    
    dofile(confdir .. "/package.lua")
    
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
    
    dofile(maindir .. "/src/lua/pkg/groups.lua")
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
        OLDG.confdir = curdir .. "/" .. confdir -- Append current dir if confdir isn't an absolute path
    end
    
    dofile(confdir .. "/config.lua")
    LoadPackage()
    
    -- Find a LZMA and edelta bin which we can use
    local basebindir = string.format("%s/bin/%s/%s", maindir, os.osname, os.arch)
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
---------------------------------
]], outname, StrPack(cfg.targetos), StrPack(cfg.targetarch), cfg.archivetype, StrPack(cfg.languages), confdir, StrPack(cfg.frontends), cfg.intropic or "None", cfg.logo or "Default"))
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
    RequiredCopy(maindir .. "/src/internal/startupinstaller.sh", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/internal/about", confdir .. "/tmp")
    
    -- Lua scripts
    os.mkdirrec(confdir .. "/tmp/shared")
    RequiredCopy(maindir .. "/src/lua/deps.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/deps-public.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/install.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/package.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/package-public.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/utils.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/pkg/deb.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/pkg/generic.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/pkg/groups.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/pkg/pacman.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/pkg/rpm.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/pkg/slack.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/shared/package-public.lua", confdir .. "/tmp/shared")
    RequiredCopy(maindir .. "/src/lua/shared/utils-public.lua", confdir .. "/tmp/shared")
    RequiredCopy(maindir .. "/src/lua/screens/finishscreen.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/screens/installscreen.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/screens/langscreen.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/screens/licensescreen.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/screens/packagedirscreen.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/screens/packagetogglescreen.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/screens/selectdirscreen.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/screens/summaryscreen.lua", confdir .. "/tmp")
    RequiredCopy(maindir .. "/src/lua/screens/welcomescreen.lua", confdir .. "/tmp")
    
    -- XDG utils
    os.mkdir(confdir .. "/tmp/xdg-utils")
    RequiredCopy(maindir .. "/src/xdg-utils/xdg-desktop-menu", confdir .. "/tmp/xdg-utils")
    RequiredCopy(maindir .. "/src/xdg-utils/xdg-open", confdir .. "/tmp/xdg-utils")
    
    -- Language files
    for _, f in pairs(cfg.languages) do
        local langsrc = confdir .. "/lang/" .. f
        local langdest = destdir .. "/lang/" .. f
        
        os.mkdirrec(langdest)
        
        os.copy(langsrc .. "/strings", langdest)
        os.copy(langsrc .. "/welcome", langdest)
        os.copy(langsrc .. "/license", langdest)
        os.copy(langsrc .. "/finish", langdest)
    end
    
    -- 'Extra' files
    os.mkdir(confdir .. "/tmp/files_extra")
    RecursiveCopy(confdir .. "/files_extra/", confdir .. "/tmp/files_extra/")
    
    print("Preparing/copying frontend binaries")
    
    -- Copy all specified frontends for every given OS/ARCH and every available libc version
    for _, OS in pairs(cfg.targetos) do
        for _, ARCH in pairs(cfg.targetarch) do
            local osdir = string.format("%s/bin/%s/%s", maindir, OS, ARCH)
            if (not os.fileexists(osdir)) then
                print(string.format("Warning: No frontends for %s/%s", OS, ARCH))
            else
                local fr_diff_src = { }
                
                for LC in io.dir(osdir) do
                    local lcpath = osdir .. "/" .. LC
                    if (string.find(LC, "^libc") and os.isdir(lcpath)) then
                        for _, FR in pairs(cfg.frontends) do
                            local frfound = false
                            local binname
                            
                            if (FR == "fltk") then
                                binname = "fltk"
                            elseif (FR == "ncurses") then
                                binname = "ncurs"
                            elseif (FR == "gtk") then
                                binname = "gtk"
                            else
                                ThrowError("Unknown frontend: %s", FR)
                            end
                            
                            binpath = string.format("%s/%s", lcpath, binname)
                            if os.fileexists(binpath) then
                                local destpath = string.format("%s/tmp/bin/%s/%s/%s", confdir, OS, ARCH, LC)
                                os.mkdirrec(destpath)
                                
                                if not fr_diff_src[binname] then
                                    fr_diff_src[binname] = binpath
                                    
                                    local ed_src = io.open(string.format("%s/tmp/bin/%s/%s/edelta_src", confdir, OS, ARCH), "a")
                                    ed_src:write(string.format("%s bin/%s/%s/%s/%s\n", binname, OS, ARCH, LC, binname))
                                    ed_src:close()

                                    if (cfg.archivetype == "lzma") then
                                        if os.execute(string.format("\"%s\" e \"%s\" \"%\s/%s.lzma\" 2>/dev/null", LZMABin,
                                                    binpath, destpath, binname)) == 0 then
                                            frfound = true
                                        end
                                    else
                                        if os.copy(binpath, destpath) ~= nil then
                                            frfound = true
                                        end
                                    end
                                else
                                    local destbin = destpath .. "/" .. binname
                                    
                                    if os.execute(string.format("\"%s\" -q delta \"%s\" \"%s\" \"%s\"", EdeltaBin,
                                                                fr_diff_src[binname], binpath, destbin)) == 0 then
                                        frfound = true
                                    end
                                    
                                    if (cfg.archivetype == "lzma") then
                                        os.execute(string.format("\"%s\" e \"%s\" \"%s\" 2>/dev/null", LZMABin,
                                                destbin, destbin .. ".lzma"))
                                        os.remove(destbin)
                                    end
                                end
                            end
                            
                            if not frfound then
                                print(string.format("Warning: no frontend '%s' found for %s/%s/%s",
                                                    FR, OS, ARCH, LC))
                            end
                        end
                    end
                    if (cfg.archivetype == "lzma") then
                        RequiredCopy(lcpath .. "/lzma-decode", string.format("%s/tmp/bin/%s/%s/%s", confdir, OS, ARCH, LC))
                    end
                    RequiredCopy(lcpath .. "/edelta", string.format("%s/tmp/bin/%s/%s/%s", confdir, OS, ARCH, LC))
                    RequiredCopy(lcpath .. "/surunner", string.format("%s/tmp/bin/%s/%s/%s", confdir, OS, ARCH, LC))
                end
            end
        end
    end
    
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
        ret, msg = os.copy(string.format("%s/files_extra/%s" , confdir, cfg.logo), string.format("%s/tmp/", confdir))
        if ret == nil then
            print(string.format("Warning could not copy logo: %s", msg))
        end
    else
        -- Default logo
        RequiredCopy(maindir .. "/src/img/installer.png", confdir .. "/tmp")
    end
    
    -- Internal config file(only stores archive type for now)
    local inffile, msg = io.open(string.format("%s/tmp/info", confdir), "w")

    if inffile == nil then
        ThrowError("Could not create internal info file for archive: %s", msg)
    end
                
    inffile:write(cfg.archivetype)
    inffile:close()
    
    -- Add deps
    if pkg.deps then
        for _, d in ipairs(pkg.deps) do
            local src = string.format("%s/deps/%s", confdir, d)
            local dest = string.format("%s/tmp/deps/%s", confdir, d)
            
            os.mkdirrec(dest)
            RequiredCopy(string.format("%s/config.lua", src), dest)
            
            local dirs = GetFileDirs(src)
            for _, d in ipairs(dirs) do
                PackDirectory(d, string.format("%s/%s", dest, utils.basename(d)))
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
    os.execute(string.format("\"%s/makeself.sh\" --gzip \"%s/tmp\" \"%s\" \"nixstaller\" sh ./startupinstaller.sh > /dev/null 2>&1",
                             maindir, confdir, outname))
end

Clean()
Init()
PrepareArchive()
CreateArchive()
CreateInstaller()
Clean()
