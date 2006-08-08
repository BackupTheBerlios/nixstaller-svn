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
    
print("OS:", os.osname)
print("ARCH:", os.arch)
print("current directory:", curdir)
print("confdir:", confdir)
print("outname:", outname)

function Traverse(dir, func)
    dirlist = { dir }
    while (#dirlist > 0) do
        local d = table.remove(dirlist) -- pop last element

        for f in io.dir(d) do
            local dpath = string.format("%s/%s", d, f)
            if (io.isdir(dpath)) then
                table.insert(dirlist, dpath)
            else
                func(d, f)
            end
        end
    end
end

function Init()
    -- Strip all trailing /'s
    local _, i = string.find(string.reverse(confdir), "^/+")
    
    if (i) then
        confdir = string.sub(confdir, 1, (-i- 1))
    end
    
    if (not string.find(confdir, "^/")) then
        confdir = curdir .. "/" .. confdir -- Append current dir if confdir isn't an absolute path
    end
    
    dofile(confdir .. "/install.lua")
    
    -- Find a LZMA bin which we can use
    local LCDirs = { } -- List containing all libc subdirectories for this system
    local binpath = string.format("%s/bin/%s/%s", curdir, os.osname, os.arch)
    for s in io.dir(binpath) do
        if (string.find(s, "^libc") and io.isdir(binpath .. "/" .. s)) then
            table.insert(LCDirs, s)
        end
    end
    
    table.sort(LCDirs, function(a, b) return a > b end) -- Sort in reverse order, this way we start with binaries which use the highest libc version
    
    for _, s in ipairs(LCDirs) do
        -- Check if we can a bin from the directory
        local bin = binpath .. "/" .. s .. "/lzma"
        if (io.fileexists(bin) and os.execute(string.format("ldd %s | grep \"not found\"", bin))) then
            LZMABin = bin
        end
    end
    
    if (not LZMABin) then
        error("Could not find a suitable LZMA encoder")
    end
    
    print("LZMA:", LZMABin)
end

function PrepareArchive()
end

Init()
io.mkdir("teste", 777)

