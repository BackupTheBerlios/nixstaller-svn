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

utils = utils or { }

function utils.mkdirperm(dir)
    local path = ""
    local ret = false
    for d in string.gmatch(dir, "/[^/]*") do
        path = path .. d
        if not os.fileexists(path) then
            break
        end
        ret = (os.writeperm(path) and os.readperm(path))
    end
    return ret
end

function utils.opendoc(f)
    local bin
    if os.execute("xdg-open --version >/dev/null 2>&1") == 0 then
        bin = "xdg-open"
    else
        bin = string.format("%s/xdg-utils/xdg-open", curdir)
    end
    
    os.execute(string.format("%s %s >/dev/null 2>&1", bin, f))
end


