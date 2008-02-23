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


local processedbins = { }
function getallsyms(bin, out, lpath)
    if not processedbins[bin] then
        processedbins[bin] = true
        local map = maplibs(bin, lpath)
        local bsyms = getsyms(bin)
        
        if not map or not bsyms then
            print(string.format("WARNING: Could not process file %s", bin))
        else
            out:write(string.format("syms[\"%s\"] = { }\n", utils.basename(bin)))
            for l, lp in pairs(map) do
                local lsyms = lp and getsyms(lp)
                if not lsyms then
                    print(string.format("WARNING: Could not process file %s", l))
                else
                    for s, v in pairs(lsyms) do
                        if bsyms[s] == false then
                            out:write(string.format("syms[\"%s\"][\"%s\"] = \"%s\"\n", utils.basename(bin), s, l))
                            bsyms[s] = true
                        end
                    end
                    getallsyms(lp, out)
                end
            end
        end
    end
end

table.remove(args, 1)
searchpaths = { }
binaries = { }
for i, a in ipairs(args) do
    if string.find(a, "-L", 1, true) then
        table.insert(searchpaths, string.sub(a, 3))
    else
        table.insert(binaries, a)
    end
end

dest = (#binaries > 1 and os.isdir(binaries[#binaries]) and binaries[#binaries]) or os.getcwd()
out = io.open(string.format("%s/symmap", dest), "w")

if not out then
    error("Could not open output file.")
end

out:write("local syms = { }\n")

for i, f in ipairs(binaries) do
    if f == dest then
        break
    end
    
    if not os.fileexists(f) then
        error(string.format("Could not locate file: %s", f))
    end
    
    local elf, msg = os.openelf(f)
    if not elf then
        error(string.format("Could not open file: %s (%s)", f, msg))
    end
    
    getallsyms(f, out, searchpaths)
end

out:write("return syms\n")
out:close()
