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


depclass = { __index = depclass }
-- UNDONE

function getmissinglibs(bin)
    local elf = os.openelf(bin)
    
    if not elf then
        return
    end
    
    local neededlibs = { }
    
    local n = 1
    local need = elf:getneeded(n)
    while need do
        neededlibs[need] = true
        n = n + 1
        need = elf:getneeded(n)
    end
    local rpath = elf:getrpath()
    elf:close()
    
    if os.osname ~= "openbsd" then
        local pipe = check(io.popen("ldd " .. bin))
        
        local line = pipe:read()
        while line do
            line = string.gsub(line, "^.*=> ", "")
            line = string.gsub(line, " %(.*%)$", "")
            if line and os.fileexists(line) then
                neededlibs[utils.basename(line)] = nil
            else
                print("Missing lib", line)
            end
            line = pipe:read()
        end
        pipe:close()
    else
        -- OpenBSD's ldd will throw an error when a lib isn't found.
        -- For this reason we have to search the lib manually.
        -- 
        -- Search order: LD_LIBRARY_PATH, f's own rpath or runpath, ldconfig hints
        local lpaths = { }
        
        local ldpath = os.getenv("LD_LIBRARY_PATH")
        if ldpath then
            for p in string.gmatch(ldpath, "[^\:]+") do
                table.insert(lpaths, p)
            end
        end

        -- rpath/runpath
        if rpath and #rpath > 0 then
            for p in string.gmatch(rpath, "[^\:]+") do
                -- Do we need this? (OpenBSD doesn't seem to support this at the moment)
                p = string.gsub(p, "${ORIGIN}", dirname(f))
                p = string.gsub(p, "$ORIGIN", dirname(f))
                table.insert(lpaths, p)
            end
        end
        
        -- ldconfig hints
        local pipe = io.popen("ldconfig -r | grep \"search directories\"")
        local libs = pipe:read()
        pipe:close()

        libs = string.gsub(libs, ".*: ", "")
        for p in string.gmatch(libs, "[^\:]+") do
            table.insert(lpaths, p)
        end

        for _, nl in ipairs(neededlibs) do
            for _, p in ipairs(lpaths) do
                local lpath = string.format("%s/%s", p, nl)
                if os.fileexists(lpath) then
                    neededlibs[nl] = nil
                end
            end
        end
    end
    
    return neededlibs
end

function getdepsfromlibs(libs) -- libs is a map, not a list
    local ret = { }
    for l in pairs(libs) do
        local found = false
        for _, d in pairs(pkg.depmap) do
            for _, dl in ipairs(d.libs) do
                if utils.basename(dl) == l then
                    ret[d] = true
                    found = true
                    break
                end
            end
            if found then
                break
            end
        end
    end
    return ret
end

function extractdeps()
    for n in pairs(pkg.depmap) do
        local src = string.format("%s/deps/%s", curdir, n)
        local dest = string.format("%s/files", src)
        os.mkdirrec(dest)
        
        local archives = { }
        local function checkarch(a)
            if os.fileexists(a) then
                table.insert(archives, a)
            end
        end
        
        checkarch(string.format("%s/files_all", src))
        checkarch(string.format("%s/files_%s_all", src, os.osname))
        checkarch(string.format("%s/files_all_%s", src, os.arch))
        checkarch(string.format("%s/files_%s_%s", src, os.osname, os.arch))

        for _, f in ipairs(archives) do
            local extrcmd
            if cfg.archivetype == "gzip" then
                extrcmd = string.format("cat %s | gzip -cd | tar -C %s -xvf -", f, dest)
            elseif cfg.archivetype == "bzip2" then
                extrcmd = string.format("cat %s | bzip -d | tar -C %s -xvf -", f, dest)
            else
                extrcmd = string.format("(%s/lzma-decode %s dep.tar >/dev/null 2>&1 && tar -C %s -xvf dep.tar)", bindir, f, dest)
            end
            
            install.execute(extrcmd)
        end
    end
end

function checkdeps(bins, bdir, deps)
    local ret = { }
    for _, b in ipairs(bins) do
        local path = string.format("%s/%s", bdir, b)
        libs = getmissinglibs(path) -- Collect any dep libs which are not found
        for rd in pairs(getdepsfromlibs(libs)) do -- Check which deps provide missing libs
            if not ret[rd] then
                ret[rd] = checkdeps(rd.libs, pkg.getdepdir(rd), rd.deps) or { }
            end
        end
    end
    
    for _, d in ipairs(deps) do
        if not ret[d] and d.required and d:required() then
            -- Add deps which are always required
            ret[d] = checkdeps(d.libs, pkg.getdepdir(d), d.deps) or { }
        end
    end
    
    return ret
end

dofile("deps-public.lua")
