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

function utils.moverec(dir, dest)
    for f in io.dir(dir) do
        checkcmd(OLDG.install.execute, string.format("mv '%s/%s' '%s/'", dir, f, dest))
    end
end

function utils.recursivedir(src, func)
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

function utils.basename(p)
    return string.gsub(p, "/*.+/", "")
end

function utils.opendoc(f)
    os.execute(string.format("%s/xdg-utils/xdg-open %s", curdir, f))
end

