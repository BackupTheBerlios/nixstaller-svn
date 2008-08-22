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
dofile(ndir .. "/src/lua/shared/package-public.lua")
dofile(ndir .. "/src/lua/deptemplates.lua")


list, gen, auto = false, false, false


function Usage()
    print(string.format("Usage: %s <action> <options> <files>\n", callscript))
    print([[
<action>        Should be one of the following:

 list       Lists all known dependency templates with info.
 scan       Scans a project directory for unspecified dependencies and suggests possible usable templates.
 gen        Generates a new dependency file structure, optionally from a template.
 auto       Automaticly tries to generate dependencies from templates.


<options>       Depending on the specified action, the following options exist:

 Valid options for the 'list' action:
    --template, -t <temp>   Lists info only about template <temp>.
    --only-names            Lists only template names.

 Valid options for the 'scan' action:
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.
    --prdir, -p <dir>       The project directory of the installer. This argument is required.
    
 Valid options for the 'gen' action:
    --simple, -s            Generates 'simple dependencies'.
    --full, -f              Generates 'full dependencies'.
    --recommend, -r         Generates either single or full dependencies, depending on what is recommended. This is the default.
    --copy, -c              Copies shared libraries automaticly. The files are copied to a 'lib/' subdirectory, inside the platform/OS specific files folder. This option is only valid when a template is used. This option only affects a full dependency.
    --name, -n <name>       Name of the dependency. This option is required when not using a template.
    --desc, -d <desc>       Dependency description. Required when not using a template.
    --template, -t <temp>   Generates the dependency from a given template <temp>.
    --prdir, -p <dir>       The project directory of the installer. This argument is required.
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.
    --baseurl, -u <url>     Base URL (ie a server directory) where this dependency from can be fetched.
    --destos <os>           Sets the <os> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not OS specific. Default is the current OS.
    --destarch <arch>       Sets the <arch> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not architecture specific. Default is the current architecture.

 Valid options for the 'auto' action:
    --simple, -s            Generates 'simple dependencies'.
    --full, -f              Generates 'full dependencies'.
    --recommend, -r         Generates either single or full dependencies, depending on what is recommended. This is the default.
    --copy, -c              Copies shared libraries automaticly. The files are copied to a 'lib/' subdirectory, inside the platform/OS specific files folder. This option only affects full dependencies.
    --prdir, -p <dir>       The project directory of the installer. This argument is required.
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.
    --baseurl, -u <url>     Base URL (ie a server directory) from where the dependencies can be fetched.
    --destos <os>           Sets the <os> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not OS specific. Default is the current OS.
    --destarch <arch>       Sets the <arch> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not architecture specific. Default is the current architecture.


<files>         When using the 'gen' action without templates: a list of libraries for the generated dependency.
                Any other case: a list off all binaries and libraries from the project.
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
            if p and not map[l] then
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
    
    for _, b in ipairs(args) do
        if not os.fileexists(b) then
            error("Could not locate file: " .. b)
        end
        CollectLibs(map, b, lpath)
    end
    
    return map
end

function CreateDep(name, desc, libs, full, baseurl, prdir, copy, libmap, destos, destarch)
    local path = string.format("%s/deps/%s", prdir, name)
    os.mkdirrec(path)
    
    local out = io.open(string.format("%s/config.lua", path), "w")
    if not out then
        error("Could not open output file.")
    end
    
    -- Convert to comma seperated string list
    local libsstr
    for _, l in ipairs(libs) do
        local s = '"lib/' .. l .. '"'
        if not libsstr then
            libsstr = s
        else
            libsstr = libsstr .. ", " .. s
        end
    end
    
    local url
    if not baseurl or not full then
        url = "nil"
    else
        url = "\"" .. baseurl .. "\""
    end
    
    out:write(string.format([[
-- Automaticly generated on %s.
-- This file is used for dependency configuration.

-- Create new dependency structure.
local dep = pkg.newdependency()

-- Dependency description. Max one line.
dep.description = "%s"

-- Libraries this dependency provides. Incase this is a full dependency,
-- use relative paths from this dependency directory.
dep.libs = { %s }

-- If this is a full dependency or not
dep.full = %s

-- Server directory (http, ftp) from where dependency files can be fetched.
-- This is fully optional, works only with full dependencies and only effects
-- dependencies set as extern in package.lua.
pkg.baseurl = %s

-- Return dependency (this line is required!)
return dep
]], os.date(), desc, libsstr, (full and "true") or "false", url))

    out:close()
    
    if copy and full then
        local dest = string.format("%s/files_%s_%s/lib", path, destos, destarch)
        os.mkdirrec(dest)
        for l, p in pairs(libmap) do
            if utils.tablefind(libs, l) then
                if not p or not os.fileexists(p) then
                    print(string.format("WARNING: Failed to locate library '%s'", l))
                else
                    local stat, msg = os.copy(p, dest)
                    if not stat then
                        print(string.format("WARNING: could not copy library '%s' to '%s': %s", p, dest, msg or "(No error message)"))
                    end
                end
            end
        end
    end
end
    
function CheckArgs()
    if not args[1] then
        ErrUsage("No action specified.")
    elseif args[1] ~= "list" and args[1] ~= "scan" and args[1] ~= "gen" and args[1] ~= "auto" then
        ErrUsage("Wrong or no action specified.")
    end
    
    _G[args[1]] = true -- set list, gen or auto to true
    table.remove(args, 1)
    
    local msg, sopts, lopts
    
    if list then
        sopts = "ht:"
        lopts = { {"help"}, {"template", true}, {"only-names"} }
    elseif scan then
        sopts = "hp:l:"
        lopts = { {"help"}, {"prdir", true}, {"libpath", true} }
    elseif gen then
        sopts = "hbsfrcn:d:t:p:l:u:"
        lopts = { {"help"}, {"simple"}, {"full"}, {"recommend"}, {"copy"}, {"name", true}, {"desc", true}, {"template", true}, {"prdir", true}, {"libpath", true}, {"baseurl", true}, {"destos", true}, {"destarch", true} }
    else
        sopts = "hbsfrcp:l:u:"
        lopts = { {"help"}, {"simple"}, {"full"}, {"recommend"}, {"copy"}, {"prdir", true}, {"libpath", true}, {"baseurl", true}, {"destos", true}, {"destarch", true} }
    end
    
    opts, msg = getopt(args, sopts, lopts)
    
    if not opts then
        ErrUsage("Error: %s", msg)
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
    local onlynames = false
    local onlytemp
    
    for _, o in ipairs(opts) do
        if o.name == "-t" or o.name == "template" then
            onlytemp = o.val
        elseif o.name == "only-names" then
            onlynames = true
        end
    end
    
    function printtemp(t)
        if onlynames then
            print(t.name)
        else
            print("Dependency name      " .. t.name)
            print("Description          " .. t.description)
            print("Recommended usage    " .. ((t.full and "as full dependency") or "as simple dependency"))
            
            io.write("Filed libraries      ")
            if t.check then
                print("dynamic (use scan action to check relevancy)")
            else
                print(tabtostr(t.libs))
            end
            
            if t.notes then
                print("Notes                " .. t.notes)
            end
            
            print("")
        end
    end
    
    local found = false
    for _, t in ipairs(pkg.deptemplates) do
        if onlytemp then
            if t.name == onlytemp then
                found = true
                printtemp(t)
                break
            end
        else
            printtemp(t)
        end
    end
    
    if onlytemp and not found then
        error("No such template: " .. onlytemp)
    end
end

function Scan()
    local prdir
    
    for _, o in ipairs(opts) do
        if o.name == "p" or o.name == "prdir" then
            prdir = o.val
        end
    end
    
    if not prdir or not os.isdir(prdir) then
        ErrUsage("Wrong or no project directory specified.")
    end
    
    -- Retrieve all current filed dependencies
    local stat, msg = pcall(dofile, prdir .. "/package.lua")
    if not stat then
        error("Failed to load package.lua: " .. msg)
    end
    
    local depmap = { }
    
    -- Load all deps
    for _, d in ipairs(pkg.deps) do
        local stat, ret = pcall(dofile, string.format("%s/deps/%s/config.lua", prdir, d))
        if not stat then
            print("WARNING: Failed to load package.lua: " .. ret)
        else
            for _, l in ipairs(ret.libs) do
                depmap[utils.basename(l)] = d
            end
        end
    end
        
    local map = GetLibMap()
    local unknownlibs = { }
    
    for l, p in pairs(map) do
        if depmap[l] then
            print(string.format("Library '%s': OK (filed by dependency '%s')", l, depmap[l]))
        elseif not unknownlibs[l] then
            print(string.format("Library '%s': Not OK", l))
            unknownlibs[l] = true
        end
    end
    
    local sugsimtemps, sugfulltemps
    for l in pairs(unknownlibs) do
        for _, t in ipairs(pkg.deptemplates) do
            if IsInTemplate(t, l) then
                if t.full then
                    sugfulltemps = sugfulltemps or { }
                    table.insert(sugfulltemps, t.name)
                else
                    sugsimtemps = sugsimtemps or { }
                    table.insert(sugsimtemps, t.name)
                end
                unknownlibs[l] = nil
                break
            end
        end
    end
    
    print("")
    
    if sugsimtemps then
        print("Templates suggested to be used as simple dependencies: " .. tabtostr(sugsimtemps))
    end
    
    if sugsimtemps then
        print("Templates suggested to be used as full dependencies: " .. tabtostr(sugfulltemps))
    end

    if not utils.emptytable(unknownlibs) then
        local prheader = true
        for l in pairs(unknownlibs) do
            if prheader then
                prheader = false
                io.write("Unknown libraries: " .. l)
            else
                io.write(", " .. l)
            end
        end
        print("")
    end
end

function Generate()
    local full -- Keep it nil, so 'recommend' is enabled by default
    local copy = false
    local name, desc, temp, prdir, baseurl
    local destos, destarch = os.osname, os.arch
    
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
        elseif o.name == "u" or o.name == "baseurl" then
            baseurl = o.val
        elseif o.name == "destos" then
            destos = o.val
        elseif o.name == "destarch" then
            destarch = o.val
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
        elseif copy then
            ErrUsage("The copy option is only valid when a template is used.")
        end
    end
    
    local libs
    if temp then
        local found = false
        libs = { }
        local map = GetLibMap()
        
        for _, t in pairs(pkg.deptemplates) do
            if t.name == temp then
                name = name or t.name
                desc = desc or t.description
                
                for l in pairs(map) do
                    if IsInTemplate(t, l) then
                        table.insert(libs, l)
                    end
                end
                
                if full == nil then
                    full = t.full
                end
                found = true
                break
            end
        end
        
        if not found then
            print("Error: Unkown template specified.")
            io.write("Valid templates: ")
            for _, t in pairs(pkg.deptemplates) do
                io.write(t.name .. " ")
            end
            print("")
            exit(1)
        end
        
        if utils.emptytable(libs) then
            print("WARNING: Found no relevant libraries for specified template. Either re-run this script with other binaries or fill the required libs manually.")
        end
        
        CreateDep(name, desc, libs, full, baseurl, prdir, copy, map, destos, destarch)
    else
        libs = args
        CreateDep(name, desc, libs, full, baseurl, prdir, false)
    end
    
    print(string.format([[
Dependency generation complete:
Name                    %s
Description             %s
Libraries               %s
Libraries copied        %s
Type                    %s
Notes                   %s
]], name, desc, tabtostr(libs), tostring(copy and full), (full and "full") or "simple"), (t.notes or "-"))

end

function Autogen()
    local full -- Keep it nil, so 'recommend' is enabled by default
    local copy = false
    local prdir, baseurl
    local destos, destarch = os.osname, os.arch
    
    for _, o in ipairs(opts) do
        if o.name == "s" or o.name == "simple" then
            full = false
        elseif o.name == "f" or o.name == "full" then
            full = true
        elseif o.name == "r" or o.name == "recommend" then
            full = nil
        elseif o.name == "c" or o.name == "copy" then
            copy = true
        elseif o.name == "p" or o.name == "prdir" then
            prdir = o.val
        elseif o.name == "u" or o.name == "baseurl" then
            baseurl = o.val
        elseif o.name == "destos" then
            destos = o.val
        elseif o.name == "destarch" then
            destarch = o.val
        end
    end
    
    if not prdir or not os.isdir(prdir) then
        ErrUsage("Wrong or no project directory specified.")
    end
    
    local map = GetLibMap()
    local templatemap = {}
    for l in pairs(map) do
        local found = false
        for _, t in ipairs(pkg.deptemplates) do
            if IsInTemplate(t, l) then
                templatemap[t] = templatemap[t] or {}
                if not utils.tablefind(templatemap[t], l) then
                    table.insert(templatemap[t], l)
                end
                found = true
                break
            end
        end
        if not found then
            print("WARNING: No template found which provides library " .. l)
        end
    end
    
    for t, l in pairs(templatemap) do
        local fl = ((full == nil) and t.full) or full
        CreateDep(t.name, t.description, l, fl, baseurl, prdir, copy, map, destos, destarch)
        print(string.format("Generated dependency %s (\"%s\", %s, libs: \"%s\"%s%s)", t.name, t.description, (fl and "full") or "simple", tabtostr(l), (copy and fl and " (copied)") or "", ((t.notes and ", Notes: " .. t.notes) or "")))
    end
end

CheckArgs()

if list then
    List()
elseif scan then
    Scan()
elseif gen then
    Generate()
else
    Autogen()
end
