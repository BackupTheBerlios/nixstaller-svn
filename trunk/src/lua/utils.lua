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

-- Util functions used by various scripts

function pkgsize(src)
    local ret = 0
    utils.recursivedir(src, function (f)
        local size = os.filesize(f)
        if size ~= nil then
            ret = ret + size
        end
    end)
    return ret
end

function xdgmenudirs(global)
    local path
    
    if global then
        path = os.getenv("XDG_DATA_DIRS")
        if not path or #path == 0 then
            path = "/usr/local/share/:/usr/share/"
        end
    else
        path = os.getenv("XDG_DATA_HOME")
        if not path or #path == 0 then
            path = os.getenv("HOME") .. "/.local/share"
        end
    end
    
    if path then
        path = path .. "/applications"
    end
    
    for d in string.gmatch(path, "[^:]+") do
        local ret = d .. "/applications"
        if os.fileexists(ret) then
            return ret
        end
    end
end

function xdgentryfname(f)
    return string.format("%s/%s.desktop", curdir, f)
end

function instxdgentries(global, fname)
    local xdgbin
    if os.execute("(xdg-desktop-menu --version) >/dev/null 2>&1") == 0 then
        xdgbin = "xdg-desktop-menu"
    else
        xdgbin = string.format("%s/xdg-utils/xdg-desktop-menu", curdir)
    end
    
    local cmd = (global and install.executeasroot) or install.execute
    cmd(string.format("%s install --novendor %s", xdgbin, fname), false)
end

function xdguninstscript()
    if not OLDG.install.menuentries then
        return nil
    end
    
    local ret = string.format([[
EXEC=""
(xdg-desktop-menu --version) >/dev/null 2>&1 && EXEC="xdg-desktop-menu"
[ -z "$EXEC" ] && ("%s/xdg-utils/xdg-desktop-menu" --version) 2>&1 >/dev/null && EXEC="%s/xdg-utils/xdg-desktop-menu"
]], pkg.getdatadir(), pkg.getdatadir())
    
    if not utils.emptytable(OLDG.install.menuentries) then
        ret = ret .. "if [ ! -z \"$EXEC\" ]; then\n"
        for n, _ in pairs(OLDG.install.menuentries) do
            ret = ret .. string.format("    $EXEC uninstall --novendor \"%s.desktop\"\n", n)
        end
        ret = ret .. "fi\n"
    end
    
    return ret
end

function maplibs(bin, extrapath)
    local elf = os.openelf(bin)
    
    if not elf then
        return
    end
    
    local map = { }
    
    local n = 1
    local need = elf:getneeded(n)
    while need do
        map[need] = false
        n = n + 1
        need = elf:getneeded(n)
    end
    local rpath = elf:getrpath()
    elf:close()
    
    if os.osname ~= "openbsd" then
        local pipe = check(io.popen("ldd " .. bin))
        
        local line = pipe:read()
        while line do
            local path = string.gsub(line, "^.*=> ", "")
            path = string.gsub(path, " %(.*%)$", "")
            path = string.gsub(path, "^[%s]*", "")
            local file = (path and utils.basename(path))
            if path and os.fileexists(path) and map[file] ~= nil then
                map[file] = path
            end
            line = pipe:read()
        end
        pipe:close()
        
        if extrapath then
            for n in pairs(map) do
                local file = string.format("%s/%s", extrapath, n)
                if os.fileexists(file) then
                    map[n] = file
                end
            end
        end
    else
        -- OpenBSD's ldd will throw an error when a lib isn't found.
        -- For this reason we have to search the lib manually.
        -- 
        -- Search order: LD_LIBRARY_PATH, f's own rpath or runpath, ldconfig hints
        local lpaths = extrapath or { }
        
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

        for nl in pairs(map) do
            for _, p in ipairs(lpaths) do
                local lpath = string.format("%s/%s", p, nl)
                if os.fileexists(lpath) then
                    map[nl] = lpath
                end
            end
        end
    end
    
    return map
end