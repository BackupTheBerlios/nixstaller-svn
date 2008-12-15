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

dofile(ndir .. "/src/lua/shared/utils.lua")
dofile(ndir .. "/src/lua/shared/utils-public.lua")

-- Get all dependant libraries from bin, recursively
function getalllibs(startlib, map, lpath)
    local ret = { }
    local stack = { }
    local par = utils.basename(startlib)
    
    for l, p in pairs(map) do
        if p then
            table.insert(stack, { lib = l, path = p, parent = par })
        end
    end
    
    while not utils.emptytable(stack) do
        local libinfo = table.remove(stack)
        local libmap = maplibs(libinfo.path, lpath)
        if libmap then
            ret[libinfo.lib] = libinfo
            for l, p in pairs(libmap) do
                if p and not ret[l] then
                    table.insert(stack, { lib = l, path = p, parent = libinfo.lib })
                end
            end
        end
    end
    
    return ret
end

local processedbins, symoutmap = { }, { }
function getallsyms(bin, lpath)
    if not processedbins[bin] then
        processedbins[bin] = true
        local map = maplibs(bin, lpath)
        local bsyms = getsyms(bin)
        
        if not map or not bsyms then
            print(string.format("WARNING: Could not process file %s", bin))
        else
            local binname = utils.basename(bin)
            for l, lp in pairs(map) do
                local lsyms = lp and getsyms(lp)
                if not lsyms then
                    print(string.format("WARNING: Could not process file %s", l))
                else
                    for s, v in pairs(lsyms) do
                        if bsyms[s] and bsyms[s].undefined and not v.undefined then
                            symoutmap[binname] = symoutmap[binname] or { }
                            symoutmap[binname][s] = l
                            bsyms[s].undefined = false -- Don't have to check again
                        end
                    end
                end
                getallsyms(lp, lpath)
            end
            for s, v in pairs(bsyms) do
                if v.undefined then
                    -- Check if it's in a non direct lib dep
                    local libs = getalllibs(bin, map, lpath)
                    for l, info in pairs(libs) do
                        local isyms = info.path and getsyms(info.path)
                        if isyms and isyms[s] and not isyms[s].undefined then
                            print(string.format("Binary '%s' has symbol dependency on %s which is indirectly provided by '%s'", bin, s, l))
                            symoutmap[binname] = symoutmap[binname] or { }
                            symoutmap[binname][s] = l
                            v.undefined = false
                            break
                        end
                    end
                    
                    if v.undefined then
                        -- Still not found
                        print(string.format("Binary %s has undefined symbol: %s", bin, s))
                    end
                end
            end
        end
    end
end

function Usage()
    print(string.format("Usage: %s [<options>] <files>\n", callscript))
    print([[
 <options> can be one of the following things (all are optional):
  --libpath, -l <dir>      Appends the directory path <dir> to the search path used for finding shared libraries.
  --dest, -d <dir>         Destination path for the output file. Default: current directory.

 <files>:                 File list of binaries and libraries to examine.
]])
end

if utils.emptytable(args) then
    Usage()
    os.exit(1)
end

searchpaths = { }
binaries = { }
dest = os.getcwd()
opts, msg = getopt(args, "l:d:", { {"libpath", true}, {"dest", true} })

if not opts then
    print("Error: " .. msg)
    Usage()
    os.exit(1)
end

for _, o in ipairs(opts) do
    if o.name == "h" or o.name == "help" then
        Usage()
        os.exit(0)
    elseif o.name == "l" or o.name == "libpath" then
        table.insert(searchpaths, o.val)
    elseif o.name == "d" or o.name == "dest" then
        dest = o.val
    end
end

if utils.emptytable(args) then
    Usage()
    os.exit(1)
end

-- Threat all remaining args as binaries/libraries to examine
for _, f in ipairs(args) do
    table.insert(binaries, f)
end

for i, f in ipairs(binaries) do
    if not os.fileexists(f) then
        abort(string.format("Could not locate file: %s", f))
    end
    
    local elf, msg = os.openelf(f)
    if not elf then
        abort(string.format("Could not open file: %s (%s)", f, msg))
    end
    
    getallsyms(f, searchpaths)
end

-- Dump gathered symbol info to file
local fname = string.format("%s/symmap", dest)
out = io.open(fname, "w")

if not out then
    abort("Could not open output file.")
end

out:write("local syms = { }\n")

for bin, syms in pairs(symoutmap) do
    out:write(string.format("syms[\"%s\"] = { }\n", bin))
    
    for s, l in pairs(syms) do
        out:write(string.format("syms[\"%s\"][\"%s\"] = \"%s\"\n", bin, s, l))
    end
end

out:write("return syms\n")
out:close()
