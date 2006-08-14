--     Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)
-- 
--     This file is part of Nixstaller.
-- 
--     Nixstaller is free software; you can redistribute it and/or modify
--     it under the terms of the GNU General Public License as published by
--     the Free Software Foundation; either version 2 of the License, or
--     (at your option) any later version.
-- 
--     Nixstaller is distributed in the hope that it will be useful,
--     but WITHOUT ANY WARRANTY; without even the implied warranty of
--     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--     GNU General Public License for more details.
-- 
--     You should have received a copy of the GNU General Public License
--     along with Nixstaller; if not, write to the Free Software
--     Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
-- 
--     Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
--     conditions of the GNU General Public License cover the whole combination.
-- 
--     In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
--     software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
--     DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
--     such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
--     you include the source code of that other code when and as the GNU GPL requires distribution of source code.
-- 
--     Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
--     versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
--     version without this exception; this exception also makes it possible to release a modified version which carries forward
--     this exception.

-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)
    
function Clean()
    local newdirlist = { confdir .. "/tmp" }
    local olddirlist = { }
    
    while (#newdirlist > 0) do
        local d = newdirlist[#newdirlist] -- pop last element
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

function ThrowError(msg, ...)
    -- Cleanup...
    print(debug.traceback())
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
            local dpath = string.format("%s/%s", subdir, f)
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
    
    -- Tar all files from the temp list file. Note that solaris(sunos) tar uses -I instead of -T used by other
    -- systems as list file option
    os.execute(string.format("tar c %s %s -f %s.tmp", (os.osname == "sunos") and "-I" or "-T", tarlistfname, file))
    
    if archivetype == "gzip" then
        os.execute(string.format("gzip -c9 %s.tmp > %s", file, file))
    elseif archivetype == "bzip2" then
        os.execute(string.format("cat %s.tmp | bzip2 -9 > %s", file, file)) -- Use cat so that bzip won't append ".bz2" to filename
    elseif archivetype == "lzma" then
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
    return ret
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
    
    dofile(confdir .. "/install.lua")
    
    -- Find a LZMA bin which we can use
    local LCDirs = { } -- List containing all libc subdirectories for this system
    local binpath = string.format("%s/bin/%s/%s", curdir, os.osname, os.arch)
    for s in io.dir(binpath) do
        if (string.find(s, "^libc") and os.isdir(binpath .. "/" .. s)) then
            table.insert(LCDirs, s)
        end
    end
    
    -- Sort in reverse order, this way we start with binaries which use the highest libc version
    table.sort(LCDirs, function(a, b) return a > b end)
    
    for _, s in ipairs(LCDirs) do
        -- Check if we can a bin from the directory
        local bin = binpath .. "/" .. s .. "/lzma"
        if (os.fileexists(bin) and os.execute(string.format("ldd %s | grep \"not found\"", bin))) then
            LZMABin = bin
        end
    end
    
    if (not LZMABin) then
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
---------------------------------
]], outname, StrPack(targetos), StrPack(targetarch), archivetype, StrPack(languages), confdir, StrPack(frontends), intropic or "None"))
end

-- Creates directory layout for installer archive
function PrepareArchive()
    local destdir = confdir .. "/tmp/config"
    os.mkdirrec(destdir)
    
    -- Configuration files
    RequiredCopy(confdir .. "/install.lua", destdir)
    os.copy(confdir .. "/welcome", destdir)
    os.copy(confdir .. "/license", destdir)
    os.copy(confdir .. "/finish", destdir)

    -- Some internal stuff
    RequiredCopy(curdir .. "/internal/startupinstaller.sh", confdir .. "/tmp")
    RequiredCopy(curdir .. "/internal/about", confdir .. "/tmp")
    
    -- Language files
    for _, f in pairs(languages) do
        local langsrc = confdir .. "/lang/" .. f
        local langdest = destdir .. "/lang/" .. f
        
        os.mkdirrec(langdest)
        
        os.copy(langsrc .. "/strings", langdest)
        os.copy(langsrc .. "/welcome", langdest)
        os.copy(langsrc .. "/license", langdest)
        os.copy(langsrc .. "/finish", langdest)
    end
    
    -- Copy all specified frontends for every given OS/ARCH and every availiable libc/libstdc++ version
    for _, OS in pairs(targetos) do
        for _, ARCH in pairs(targetarch) do
            local tmpdir = string.format("%s/bin/%s/%s", curdir, OS, ARCH)
            if (not os.fileexists(tmpdir)) then
                ThrowError("No bins for %s/%s", OS, ARCH)
            end
            
            for LC in io.dir(tmpdir) do
                local lcpath = tmpdir .. "/" .. LC
                if (string.find(LC, "^libc") and os.isdir(lcpath)) then
                    for LCPP in io.dir(lcpath) do
                        local lcpppath = lcpath .. "/" .. LCPP
                        if (string.find(LCPP, "^libstdc++") and os.isdir(lcpppath)) then
                            for _, FR in pairs(frontends) do
                                local frfound = false
                                local binname
                                
                                if (FR == "fltk") then
                                    binname = "fltk"
                                elseif (FR == "ncurses") then
                                    binname = "ncurs"
                                else
                                    ThrowError("Unknown frontend: %s", FR)
                                end
                                
                                binpath = string.format("%s/%s", lcpppath, binname)
                                if os.fileexists(binpath) then
                                    local destpath = string.format("%s/tmp/bin/%s/%s/%s/%s", confdir, OS, ARCH, LC, LCPP)
                                    os.mkdirrec(destpath)
                                    if (archivetype == "lzma") then
                                        if os.execute(string.format("%s e %s %s/%s.lzma 2>/dev/null", LZMABin,
                                                      binpath, destpath, binname)) == 0 then
                                            frfound = true
                                        end
                                    else
                                        if os.copy(binpath, destpath) ~= nil then
                                            frfound = true
                                        end
                                    end
                                end
                                
                                if not frfound then
                                    print(string.format("Warning: no frontend '%s' found for %s/%s/%s/%s", binname, OS,
                                          ARCH, LC, LCPP))
                                end
                            end
                        end
                    end
                    if (archivetype == "lzma") then
                        RequiredCopy(lcpath .. "/lzma-decode", string.format("%s/tmp/bin/%s/%s/%s", confdir, OS, ARCH, LC))
                    end
                end
            end
        end
    end
    
    -- Intro picture
    if intropic ~= nil then
        ret, msg = os.copy(string.format("%s/%s %s/tmp/", curdir, intropic, confdir))
        if ret == nil then
            print(string.format("Warning could not copy intro picture: %s", msg))
        end
    end
end

function CreateArchive()
    local src, dest
    local basename = "instarchive"

    print("Generating archive...")
    
    -- Platform indepent files
    src = confdir .. "/files_all"
    dest = string.format("%s/tmp/%s_all", confdir, basename)
    if os.isdir(src) then
        PackDirectory(src, dest)
    end
    
    -- Pack every OS/ARCH dependent file
    for _, OS in pairs(targetos) do
        local archname = string.format("%s/files_%s_all", confdir, OS) 
        if os.isdir(archname) then
            PackDirectory(archname, string.format("%s/tmp/%s_%s", confdir, basename, OS))
        end
        
        for _, ARCH in pairs(targetarch) do
            archname = string.format("%s/files_%s_%s", confdir, OS, ARCH) 
            if os.isdir(archname) then
                PackDirectory(archname, string.format("%s/tmp/%s_%s_%s", confdir, basename, OS, ARCH))
            end
        end
    end
end

function CreateInstaller()
    print("Generating installer...")
    os.execute(string.format("%s/makeself.sh --gzip %s/tmp %s/%s \"nixstaller\" sh ./startupinstaller.sh > /dev/null 2>&1",
                             curdir, confdir, curdir, outname))
end

Init()
PrepareArchive()
CreateArchive()
CreateInstaller()
Clean()
