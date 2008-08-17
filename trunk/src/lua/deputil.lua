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
dofile(ndir .. "/src/lua/deptemplates.lua")


list, gen, auto = false, false, false


function Usage()
    print(string.format("Usage: %s <action> <options> <files>\n", callscript))
    print([[
<action>        Should be one of the following:

 list       Lists dependency information from a given binary set.
 gen        Generates a new dependency file structure, optionally from a template.
 auto       Automaticly tries to generate dependencies from templates.


<options>       Depending on the specified action, the following options exist:

 Valid options for the 'list' action:
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.
 
 Valid options for the 'gen' action:
    --simple, -s            Generates 'simple dependencies'.
    --full, -f              Generates 'regular dependencies'.
    --recommend, -r         Generates either single or full dependencies, depending on what is recommended. This is the default.
    --copy, -c              Copies shared libraries automaticly. The files are copied to a 'lib/' subdirectory, inside the platform/OS specific files folder. This option automaticly implies -f.
    --name, -n <name>       Name of the dependency. This option is required when not using a template.
    --desc, -d <desc>       Dependency description. Required when not using a template.
    --template, -t <temp>   Generates the dependency from a given template <temp>.
    --prdir, -p             The project directory of the installer. This argument is required.
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.

 Valid options for the 'auto' action:
    --simple, -s            Generates 'simple dependencies'.
    --full, -f              Generates 'regular dependencies'.
    --recommend, -r         Generates either single or full dependencies, depending on what is recommended. This is the default.
    --copy, -c              Copies shared libraries automaticly. The files are copied to a 'lib/' subdirectory, inside the platform/OS specific files folder. This option automaticly imples -f.
    --prdir, -p             The project directory of the installer. This argument is required.
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.


<files>         File list of binaries and libraries to examine.
]])
end

function ErrUsage(msg, ...)
    print("Error: " .. string.format(msg, ...) .. "\n")
    Usage()
    os.exit(1)
end

function IsInTemplate(t, l)
    if t.check then
        return t.check(l)
    end
    return utils.tablefind(t.libs, l)
end

function CollectLibs(map, f, lpath)
    local m = maplibs(f, lpath)
    
    if not m then
        print(string.format("WARNING: Could not process file %s", f))
    else
        utils.tablemerge(map, m)

        for l, p in pairs(m) do
            if p then
                CollectLibs(map, p, lpath)
            end
        end
    end
end

function GetLibMap()
    local map, lpath = { }, { }
    for _, o in ipairs(opts) do
        if o.name == "l" or o.name == "libpath" then
            table.insert(lpath, o.val)
        end
    end

    if utils.emptytable(args) then
        ErrUsage("No files given to examine.")
    end
    
    for _, b in pairs(args) do
        if not os.fileexists(b) then
            error("Could not locate file: " .. b)
        end
        CollectLibs(map, b, lpath)
    end
    
    return map
end

function CreateDep(name, desc, libs, full, prdir)
    local path = string.format("%s/deps/%s", prdir, name)
    os.mkdirrec(path)
    
    local out = io:open(string.format("%s/config.lua", path))
    if not out then
        error("Could not open output file.")
    end
    
    -- Convert to comma seperated string
    local libsstr = ""
    for _, l in ipairs(libs) do
        if #libsstr == 0 then
            libsstr = l
        else
            libsstr = libsstr .. ", " .. l
        end
    end
    
    out:write(string.format([[
-- Automaticly generated on %s.
-- This file is used for dependency configuration.

-- Create new dependency structure.
local dep = pkg.newdependency()

-- Dependency description. Max one line.
dep.description = %s

-- Libraries this dependency provides. Incase this is a full dependency,
-- use relative paths from this dependency directory.
dep.libs = { %s }

-- If this is a full dependency or not
dep.full = %s

-- Return dependency (this line is required!)
return dep
]], os.date(), desc, libsstr, (full and "true") or "false")

    out:close()
end
    
end

function CheckArgs()
    if not args[1] then
        ErrUsage("No action specified.")
    elseif args[1] ~= "list" and args[1] ~= "gen" and args[1] ~= "auto" then
        ErrUsage("Wrong or no action specified.")
    end
    
    _G[args[1]] = true -- set list, gen or auto to true
    table.remove(args, 1)
    
    local failedarg, sopts, lopts
    
    if list then
        sopts = "hl:"
        lopts = { {"help"}, {"libpath"} }
    elseif gen then
        sopts = "hbsfrcn:d:t:"
        lopts = { {"help"}, {"simple"}, {"full"}, {"recommend"}, {"copy"}, {"name", true}, {"desc", true}, {"template", true} }
    else
        sopts = "hbsfrcl:"
        lopts = { {"help"}, {"simple"}, {"full"}, {"recommend"}, {"copy"}, {"libpath", true} }
    end
    
    opts, failedarg = getopt(args, sopts, lopts)
    
    if not opts then
        ErrUsage("Unknown commandline option: %s", failedarg)
    end
    
    for _, o in ipairs(opts) do
        if o.name == "h" or o.name == "help" then
            Usage()
            exit(0)
        end
    end
    
    -- Remaining of args is used by other functions.
end

function List()
    local processed, templates, knownlibs = { }, { }, { }
    local map = GetLibMap()
    
    for _, t in pairs(pkg.deptemplates) do
        local printedheader = false
        
        for l, p in pairs(map) do
            if not knownlibs[l] and IsInTemplate(t, l) then
                if not printedheader then
                    printedheader = true
                    print(string.format("\Libraries found from template '%s':", t.name))
                end
                print(string.format("\t%s (%s)", l, (not p and "Unknown path") or utils.dirname(p)))
                knownlibs[l] = true
                templates[t] = true
            end
        end
    end
    
    local printedheader = false
    for l, p in pairs(map) do
        if not knownlibs[l] then
            if not printedheader then
                printedheader = true
                print("\nLibraries which don't belong to any template:")
            end
            print(string.format("\t%s (%s)", l, (not p and "Unknown path") or utils.dirname(p)))
            knownlibs[l] = true -- Still not known, but make sure that libs are only mentioned once
        end
    end

    print("\n\nThe following templates are recommended to be used as simple dependencies:")
    for t in pairs(templates) do
        if t.recommend == "simple" then
            io.write(t.name .. " ")
        end
    end
    print("\n\nThe following templates are recommended to be used as full dependencies:")
    for t in pairs(templates) do
        if t.recommend == "full" then
            io.write(t.name .. " ")
        end
    end

    print("") -- Add newline
end

function Generate()
    local full -- Keep it nil, so 'recommend' is enabled by default
    local copy = false
    local name, desc, temp, prdir
    
    for _, o in ipairs(opts) do
        if o.name == "s" or o.name == "simple" then
            full = false
        elseif o.name == "f" or o.name == "full" then
            full = true
        elseif o.name == "r" or o.name == "recommend" then
            full = nil
        elseif o.name == "c" or o.name == "copy" then
            copy = true
        elseif o.name == "n" or o.name == "name" then
            name = o.val
        elseif o.name == "d" or o.name == "desc" then
            desc = o.val
        elseif o.name == "t" or o.name == "template" then
            temp = o.val
        elseif o.name == "p" or o.name == "prdir" then
            prdir = o.val
        end
    end
    
    if not prdir or not os.isdir(prdir) then
        ErrUsage("Wrong or no project directory specified.")
    elseif not temp then
        if not name then
            ErrUsage("A name must be given when no template is used.")
        elseif not desc then
            ErrUsage("A description must be given when no template is used.")
        elseif utils.emptytable(args) then
            ErrUsage("A library list must be given when no template is used.")
        end
    end
    
    local libs = args
    if temp then
        for _, t in pairs(pkg.deptemplates) do
            if t.name == temp then
                name = name or t.name
                desc = desc or t.description
--              UNDONE
--                 if not libs or utils.emptytable(libs) then
--                     libs = t.libs
--                 end
                if full == nil then
                    full = t.recommend
                end
            end
        end
    end
    
    CreateDep(name, desc, libs, full, prdir)
end

function Autogen()
end

CheckArgs()

if list then
    List()
elseif gen then
    Generate()
else
    Autogen()
end
