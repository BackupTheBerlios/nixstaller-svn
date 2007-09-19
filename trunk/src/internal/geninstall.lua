--     Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)
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
    error(string.format(msg .. "\n", ...))
end

function RequiredCopy(src, dest)
    local stat, msg = os.copy(src, dest)
    if (not stat) then
        ThrowError("Error could not copy required file %s to %s: %s", src, dest, msg or "(No error message)")
    end
end

function PackDirectory(dir, file)
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
            
            
            if (os.isdir(dpath)) then
                table.insert(dirlist, dpath)
            else
                local fsize, msg = os.filesize(dpath)
                if fsize == nil then
                    ThrowError("Could not get file size: %s", msg)
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
        
    os.execute(string.format("tar cf %s.tmp %s %s", file, listopt, tarlistfname))
    
    if cfg.archivetype == "gzip" then
        os.execute(string.format("gzip -c9 %s.tmp > %s", file, file))
    elseif cfg.archivetype == "bzip2" then
        os.execute(string.format("cat %s.tmp | bzip2 -9 > %s", file, file)) -- Use cat so that bzip won't append ".bz2" to filename
    elseif cfg.archivetype == "lzma" then
        os.execute(string.format("%s e %s.tmp %s 2>/dev/null", LZMABin, file, file))
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
    
    -- Find a LZMA and edelta bin which we can use
    local basebindir = string.format("%s/bin/%s/%s", curdir, os.osname, os.arch)
    local validbin = function(bin)
                        -- Does the bin exists and 'ldd' can find all dependend libs?
                        return (os.fileexists(bin) and (os.execute(string.format("ldd %s | grep \"not found\" >/dev/null", bin)) ~= 0))
                     end
    
    for lc in TraverseBinLibDir(basebindir, "^libc") do
        local lcdir = basebindir .. "/" .. lc
        for lcpp in TraverseBinLibDir(lcdir, "^libstdc++") do
            local bin = lcdir .. "/" .. lcpp .. "/lzma"

            -- File exists and 'ldd' doesn't report missing lib deps?
            if (validbin(bin)) then
                LZMABin = bin
            end
        end
        
        local bin = lcdir .. "/edelta"
        
        -- File exists and 'ldd' doesn't report missing lib deps?
        if (validbin(bin)) then
            EdeltaBin = bin
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
---------------------------------
]], outname, StrPack(cfg.targetos), StrPack(cfg.targetarch), cfg.archivetype, StrPack(cfg.languages), confdir, StrPack(cfg.frontends), cfg.intropic or "None"))
end

-- Creates directory layout for installer archive
function PrepareArchive()
    local destdir = confdir .. "/tmp/config"
    os.mkdirrec(destdir)
    
    -- Configuration files
    RequiredCopy(confdir .. "/config.lua", destdir)
    os.copy(confdir .. "/run.lua", destdir)
    os.copy(confdir .. "/welcome", destdir)
    os.copy(confdir .. "/license", destdir)
    os.copy(confdir .. "/finish", destdir)

    -- Default logo
    RequiredCopy(curdir .. "/src/img/installer.png", confdir .. "/tmp")
    
    -- Some internal stuff
    RequiredCopy(curdir .. "/src/internal/startupinstaller.sh", confdir .. "/tmp")
    RequiredCopy(curdir .. "/src/internal/about", confdir .. "/tmp")
    RequiredCopy(curdir .. "/src/internal/finishscreen.lua", confdir .. "/tmp")
    RequiredCopy(curdir .. "/src/internal/install.lua", confdir .. "/tmp")
    RequiredCopy(curdir .. "/src/internal/installscreen.lua", confdir .. "/tmp")
    RequiredCopy(curdir .. "/src/internal/langscreen.lua", confdir .. "/tmp")
    RequiredCopy(curdir .. "/src/internal/licensescreen.lua", confdir .. "/tmp")
    RequiredCopy(curdir .. "/src/internal/selectdirscreen.lua", confdir .. "/tmp")
    RequiredCopy(curdir .. "/src/internal/welcomescreen.lua", confdir .. "/tmp")
    
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
    
    print("Preparing/copying frontend binaries")
    
    -- Copy all specified frontends for every given OS/ARCH and every availiable libc/libstdc++ version
    for _, OS in pairs(cfg.targetos) do
        for _, ARCH in pairs(cfg.targetarch) do
            local osdir = string.format("%s/bin/%s/%s", curdir, OS, ARCH)
            if (not os.fileexists(osdir)) then
                print(string.format("Warning: No frontends for %s/%s", OS, ARCH))
            else
                local fr_diff_src = { }
                
                for LC in io.dir(osdir) do
                    local lcpath = osdir .. "/" .. LC
                    if (string.find(LC, "^libc") and os.isdir(lcpath)) then
                        for LCPP in io.dir(lcpath) do
                            local lcpppath = lcpath .. "/" .. LCPP
                            if (string.find(LCPP, "^libstdc++") and os.isdir(lcpppath)) then
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
                                    
                                    binpath = string.format("%s/%s", lcpppath, binname)
                                    if os.fileexists(binpath) then
                                        local destpath = string.format("%s/tmp/bin/%s/%s/%s/%s", confdir, OS, ARCH, LC, LCPP)
                                        os.mkdirrec(destpath)
                                        
                                        if not fr_diff_src[binname] then
                                            fr_diff_src[binname] = binpath
                                            
                                            local ed_src = io.open(string.format("%s/tmp/bin/%s/%s/edelta_src", confdir, OS, ARCH), "a")
                                            ed_src:write(string.format("%s bin/%s/%s/%s/%s/%s\n", binname, OS, ARCH, LC, LCPP, binname))
                                            ed_src:close()
        
                                            if (cfg.archivetype == "lzma") then
                                                if os.execute(string.format("%s e %s %s/%s.lzma 2>/dev/null", LZMABin,
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
                                            
                                            if os.execute(string.format("%s -q delta %s %s %s", EdeltaBin,
                                                                        fr_diff_src[binname], binpath, destbin)) == 0 then
                                                frfound = true
                                            end
                                            
                                            if (cfg.archivetype == "lzma") then
                                                os.execute(string.format("%s e %s %s 2>/dev/null", LZMABin, destbin, destbin .. ".lzma"))
                                                os.remove(destbin)
                                            end
                                        end
                                    end
                                    
                                    if not frfound then
                                        print(string.format("Warning: no frontend '%s' found for %s/%s/%s/%s",
                                                            FR, OS, ARCH, LC, LCPP))
                                    end
                                end
                            end
                        end
                        if (cfg.archivetype == "lzma") then
                            RequiredCopy(lcpath .. "/lzma-decode", string.format("%s/tmp/bin/%s/%s/%s", confdir, OS, ARCH, LC))
                        end
                        RequiredCopy(lcpath .. "/edelta", string.format("%s/tmp/bin/%s/%s/%s", confdir, OS, ARCH, LC))
                    end
                end
            end
        end
    end
    
    -- Intro picture
    if cfg.intropic ~= nil then
        ret, msg = os.copy(string.format("%s/%s" , confdir, cfg.intropic), string.format("%s/tmp/", confdir))
        if ret == nil then
            print(string.format("Warning could not copy intro picture: %s", msg))
        end
    end
                    
    -- Internal config file(only stores archive type for now)
    local inffile, msg = io.open(string.format("%s/tmp/info", confdir), "w")

    if inffile == nil then
        ThrowError("Could not create internal info file for archive: %s", msg)
    end
                
    inffile:write(cfg.archivetype)
    inffile:close()
end

function CreateArchive()
    local archdir
    local basename = "instarchive"

    print("Generating archive...")
    
    -- Platform indepent files
    archdir = confdir .. "/files_all"
    if os.isdir(archdir) then
        PackDirectory(archdir, string.format("%s/tmp/%s_all", confdir, basename))
    end
    
    -- CPU dependent files
    for _, ARCH in pairs(cfg.targetarch) do
        archdir = string.format("%s/files_all_%s", confdir, ARCH) 
        if os.isdir(archdir) then
            PackDirectory(archdir, string.format("%s/tmp/%s_all_%s", confdir, basename, ARCH))
        end
    end
    
    for _, OS in pairs(cfg.targetos) do
        -- OS dependent files
        archdir = string.format("%s/files_%s_all", confdir, OS) 
        if os.isdir(archdir) then
            PackDirectory(archdir, string.format("%s/tmp/%s_%s_all", confdir, basename, OS))
        end
        
        -- OS/ARCH dependent files
        for _, ARCH in pairs(cfg.targetarch) do
            archdir = string.format("%s/files_%s_%s", confdir, OS, ARCH) 
            if os.isdir(archdir) then
                PackDirectory(archdir, string.format("%s/tmp/%s_%s_%s", confdir, basename, OS, ARCH))
            end
        end
    end
end

function CreateInstaller()
    print("Generating installer...")
    os.execute(string.format("%s/makeself.sh --gzip %s/tmp %s/%s \"nixstaller\" sh ./startupinstaller.sh > /dev/null 2>&1",
                             curdir, confdir, curdir, outname))
end

Clean()
Init()
PrepareArchive()
CreateArchive()
CreateInstaller()
Clean()
