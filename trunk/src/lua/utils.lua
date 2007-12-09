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

-- Util functions used by various scripts

function moverec(dir, dest)
    for f in io.dir(dir) do
        checkcmd(OLDG.install.execute, string.format("mv '%s/%s' '%s/'", dir, f, dest))
    end
end

function recursivedir(src, func)
    local dirstack = { "." }
    
    while #dirstack > 0 do
        local dir = table.remove(dirstack)
        for f in io.dir(src .. "/" .. dir) do
            local relpath
            if dir ~= "." then
                relpath = string.format("%s/%s", dir, f)
            else
                relpath = f
            end
            
            local path = src .. "/" .. relpath
            if os.isdir(path) then
                table.insert(dirstack, relpath)
            end
            func(path, relpath)
        end
    end
end

function pkgsize(src)
    local ret = 0
    recursivedir(src, function (f)
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
    local cmd = (global and install.executeasroot) or install.execute
    cmd(string.format("%s/xdg-utils/xdg-desktop-menu install --novendor %s", curdir, fname), false)
end

function xdguninstscript()
    if not OLDG.install.menuentries then
        return nil
    end
    
    local ret = string.format([[
EXEC=""
(xdg-desktop-menu --version >/dev/null 2>&1) && EXEC="xdg-desktop-menu"
[ -z $EXEC ] && ("%s/xdg-utils/xdg-desktop-menu" --version 2>&1 >/dev/null) && EXEC="%s/xdg-utils/xdg-desktop-menu"
if [ -z "$EXEC" ]; then
    exit 0
fi
]], pkg.getdatadir(), pkg.getdatadir())
    
    for n, _ in pairs(OLDG.install.menuentries) do
        ret = ret .. string.format("$EXEC uninstall --novendor %s.desktop\n", n)
    end
    
    return ret
end
