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

dofile(args[1] .. "/src/lua/shared/utils.lua")
dofile(args[1] .. "/src/lua/shared/utils-public.lua")

table.remove(args, 1)

dest = (#args > 1 and os.isdir(args[#args]) and args[#args]) or os.getcwd()
out = io.open(string.format("%s/symmap", dest), "w")

if not out then
    error("Could not open output file.")
end

out:write("local syms = { }\n")

for i, f in ipairs(args) do
    if f == dest then
        break
    end
    
    out:write(string.format("syms[%s] = { }\n", f))
    
    if not os.fileexists(f) then
        error(string.format("Could not locate file: %s", f))
    end
    
    local elf, msg = os.openelf(f)
    if not elf then
        error(string.format("Could not open file: %s (%s)", f, msg))
    end
    
    local sym = elf:getsym(1)
    local n = 1
    while sym do
        out:write(string.format("syms[%s][%s] = { undefined = %s, version = \"%s\" }\n", utils.basename(f), sym.name,
                  tostring((sym.undefined and sym.binding == "GLOBAL")), sym.version))
        n = n + 1
        sym = elf:getsym(n)
    end
end

out:write("return syms\n")
out:close()
