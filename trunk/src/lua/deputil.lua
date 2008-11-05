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


list, gen, gent, auto = false, false, false


function Usage()
    print(string.format("Usage: %s <action> <options> <files>\n", callscript))
    print([[
<action>        Should be one of the following:

 list       Lists all known dependency templates with info.
 scan       Scans a project directory for unspecified dependencies and suggests possible usable templates.
 gen        Generates a new dependency file structure.
 gent       Generates a new dependency file structure from a template.
 auto       Automaticly tries to generate dependencies from templates.
 edit       Edits an existing dependency.


<options>       Depending on the specified action, the following options exist:

 General options:
    --help, -h              Show this message.
    --                      Stop parsing arguments.
    
 Valid options for the 'list' action:
    --template, -t <temp>   Lists info only about template <temp>.
    --only-names            Lists only template names.

 Valid options for the 'scan' action:
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.
    --prdir, -p <dir>       The project directory of the installer. This argument is required.
    
 Valid options for the 'gen' action:
    --simple, -s            Generates 'simple dependencies'. This is the default.
    --full, -f              Generates 'full dependencies'.
    --name, -n <name>       Name of the dependency. This option is required.
    --desc, -d <desc>       Dependency description. This option is required.
    --libs, -a <libs>       Comma seperated list of all libraries for this dependency.
    --deps, -D <deps>       Comma seperated list of any dependencies for the generated dependency itself.

 Valid options for the 'gent' action:
    --simple, -s            Generates 'simple dependencies'. Default is what the given template recommends.
    --full, -f              Generates 'full dependencies'. Default is what the given template recommends.
    --name, -n <name>       Name of the dependency. Default: template name.
    --desc, -d <desc>       Dependency description. Default: description from template.
    --template, -t <temp>   Generates the dependency from the given template <temp>. This option is required.
    --libs, -a <libs>       Comma seperated list of any additional libraries for this dependency.
    --deps, -D <deps>       Comma seperated list of any dependencies for the generated dependency itself.

 Valid options for the 'auto' action:
    --simple, -s            Generates 'simple dependencies'. Default is what the relevant template recommends.
    --full, -f              Generates 'full dependencies'. Default is what the relevant template recommends.
    --unknown, -U           Creates dependencies for libraries which are not listed by any template. The dependency will be named after the library, without the lib prefix and '.so.<version>' suffix (so libfoo.so.1 becomes foo), have an empty description and is defined as full by default.

 Valid options for the 'gen', 'gent' and 'auto' actions:
    --copy, -c              Copies shared libraries automaticly. The files are copied to the subdirectory specified by the --libdir option, inside the platform/OS specific files folder. This option only affects a full dependencies.
    --prdir, -p <dir>       The project directory of the installer. This argument is required.
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.
    --libdir <dir>          Subdirectory (relative to files_<os>_<arch> directories) where libraries from the generated dependencies are found (also affects --copy). Default: 'lib/'.
    --baseurl, -u <url>     Base URL (ie a server directory) where this dependency from can be downloaded.
    --destos <os>           Sets the <os> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not OS specific. Default is the current OS.
    --destarch <arch>       Sets the <arch> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not architecture specific. Default is the current architecture.
    --overwrite             Overwrite any existing files. Default is to ask.
    --rm-existing           Removes any existing files. Default is to ask.
    --skip-existing         Skips generation of a dependency incase it already exists. Default is to ask.
 
 Valid options for the 'edit' action:
    --dep, -d <dep>         Dependency to edit. This option is required.
    --simple, -s            Converts a full dependency to a simple version. Either this or the --full option is required.
    --full, -f              Converts a simple dependency to a full version. Either this or the --simple option is required.
    --remove, -r            Removes any existing libraries. Only works when using the '--simple' option.
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.
    --copy, -c              Copies shared libraries automaticly. The files are copied to the subdirectory specified by the --libdir option, inside the platform/OS specific files folder. This option only works when using '--full'.
    --destos <os>           Sets the <os> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not OS specific. Default is the current OS.
    --destarch <arch>       Sets the <arch> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not architecture specific. Default is the current architecture.


<files>         A list off all binaries and libraries from the project, used to gather necessary libraries for dependency generation. This is required for the 'edit', 'scan', 'gent', 'auto' actions and when 'gen' is used with the --full and --copy options.
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
    
    if t.libs then
        -- Check libs, escaping any search patterns
        for _, tl in ipairs(t.libs) do
            local pattern = string.format("^lib%s%%.so%%.%%d", escapepat(tl))
            if string.find(l, pattern) then
                return true
            end
        end
    end
    
    if t.patlibs then
        -- Check libs, allowing search patterns
        for _, tl in ipairs(t.patlibs) do
            local pattern = string.format("^lib%s%%.so[%%.%%d]?", tl)
            if string.find(l, pattern) then
                return true
            end
        end
    end
end

function GetLibPath()
    local lpath = { }
    for _, o in ipairs(opts) do
        if o.name == "l" or o.name == "libpath" then
            table.insert(lpath, o.val)
        end
    end
    return lpath
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
    local map, lpath = { }, GetLibPath()

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

function GetLibDepPath(prdir, dep, lib)
    local deppath = string.format("%s/deps/%s", prdir, dep.name)
    for d in io.dir(deppath) do
        if string.find(d, "files_.+_.+") then
            local lpath = string.format("%s/%s/%s/%s", deppath, d, dep.libdir, lib)
            if os.fileexists(lpath) then
                return lpath
            end
        end
    end
end

function CheckExisting(prdir, d, exist)
    local path = string.format("%s/deps/%s", prdir, d)
    if exist == "overwrite" or not os.isdir(path) then
        return true
    end
    
    if exist == "rm-existing" then
        local ret, msg = utils.removerec(path)
        if not ret then
            error(string.format("Could not remove existing dependency: %s", msg))
        end
        return true
    elseif exist == "skip-existing" then
        return false
    elseif exist == "ask" then
        print(string.format("Dependency '%s' already exists.", d))
        io.write("Do you want to [o]verwrite, [r]emove it or [s]kip it? ")
        repeat
            local e = io.read(1)
            if e == "o" then
                return CheckExisting(prdir, d, "overwrite")
            elseif e == "r" then
                return CheckExisting(prdir, d, "rm-existing")
            elseif e == "s" then
                return CheckExisting(prdir, d, "skip-existing")
            end
        until false
    end
    assert(false)
end

function CreateDep(name, desc, libs, libdir, full, baseurl, deps, prdir, copy, libmap, destos, destarch, postf, installf, requiredf, compatf, caninstallf)
    local path = string.format("%s/deps/%s", prdir, name)
    os.mkdirrec(path)
    
    local out = io.open(string.format("%s/config.lua", path), "w")
    if not out then
        error("Could not open output file.")
    end
    
    -- Convert to comma seperated string list
    local function tolist(tab)
        local ret
        for _, l in ipairs(tab) do
            local s = '"' .. l .. '"'
            if not ret then
                ret = s
            else
                ret = ret .. ", " .. s
            end
        end
        
        return ret
    end
    
    local libsstr = tolist(libs) or ""
    local depsstr = tolist(deps) or ""
    
    local url
    if not baseurl or not full then
        url = "nil"
    else
        url = "\"" .. baseurl .. "\""
    end
    
    installf = installf or "    self:CopyFiles()"
    requiredf = requiredf or "    return false"
    compatf = compatf or "    return false"
    caninstallf = caninstallf or "    return true"
    
    out:write(string.format([[
-- Automaticly generated on %s.
-- This file is used for dependency configuration.

-- Create new dependency structure.
local dep = pkg.newdependency()

-- Dependency description. Max one line.
dep.description = "%s"

-- Libraries this dependency provides.
dep.libs = { %s }

-- Subdirectory (relative to files_<os>_<arch> directories) where libraries can be found.
dep.libdir = "%s"

-- If this is a full dependency or not
dep.full = %s

-- Server directory (http, ftp) from where dependency files can be fetched.
-- This is fully optional, works only with full dependencies and only effects
-- dependencies set as extern in package.lua.
dep.baseurl = %s

-- Dependencies for this dependency itself (similar to pkg.deps).
dep.deps = { %s }

-- Install function. This is called when the dependency is installed.
function dep:Install()
%s
end

-- Required function. Called to check if dependency is required. Note that even if this function
-- returns false, the dependency may still be installed when autodetection marks it as necessary.
function dep:Required()
%s
end

-- Called if dependency itself seems to be incompatible. You can use this for example to recompile
-- any libraries. The 'lib' argument contains the faulty library. By returning true the dependency
-- will be installed as usual, otherwise it will be marked as incompatible.
function dep:HandleCompat(lib)
%s
end

-- Called just before this dependency is about to be installed. Return false if this dependency can't be
-- installed for some reason, otherwise return true.
function dep:CanInstall()
%s
end

-- Return dependency (this line is required!)
return dep
]], os.date(), desc, libsstr, libdir, (full and "true") or "false", url, depsstr, installf, requiredf, compatf, caninstallf))

    out:close()
    
    local copydest = string.format("%s/files_%s_%s/%s", path, destos, destarch, libdir)
    if copy and full then
        os.mkdirrec(copydest)
        for l, p in pairs(libmap) do
            if utils.tablefind(libs, l) then
                if not p or not os.fileexists(p) then
                    print(string.format("WARNING: Failed to locate library '%s'", l))
                else
                    local stat, msg = os.copy(p, copydest)
                    if not stat then
                        print(string.format("WARNING: could not copy library '%s' to '%s': %s", p, copydest, msg or "(No error message)"))
                    end
                end
            end
        end
    end
    
    if postf then
        postf(path, string.format("files_%s_%s", destos, destarch), libmap, full, copy)
    end
end

function LoadDep(prdir, name)
    local path = string.format("%s/deps/%s/config.lua", prdir, name)
    local stat, ret = pcall(dofile, path)
    if not stat then
        return nil, ret
    end
    
    local function default(var, val)
        if ret[var] == nil then
            ret[var] = val
        end
    end
    
    default("full", true)
    default("libdir", "lib/")
    default("description", "")
    default("libs", { })
    default("deps", { })
    
    ret.name = name
    
    return ret
end

function ParseGenArgs()
    local full -- Keep it nil, so caller can handle a default.
    local copy = false
    local prdir, baseurl
    local libdir = "lib/"
    local destos, destarch = os.osname, os.arch
    local exist = "ask"
    
    for _, o in ipairs(opts) do
        if o.name == "s" or o.name == "simple" then
            full = false
        elseif o.name == "f" or o.name == "full" then
            full = true
        elseif o.name == "c" or o.name == "copy" then
            copy = true
        elseif o.name == "p" or o.name == "prdir" then
            prdir = o.val
            if not string.find(prdir, "^/") then -- Not an absolute path?
                prdir = os.getcwd() .. "/" .. prdir
            end
        elseif o.name == "u" or o.name == "baseurl" then
            baseurl = o.val
        elseif o.name == "libdir" then
            libdir = o.val
        elseif o.name == "destos" then
            destos = o.val
        elseif o.name == "destarch" then
            destarch = o.val
        elseif o.name == "overwrite" or o.name == "rm-existing" or o.name == "skip-existing" then
            exist = o.name
        end
    end
    
    if not prdir or not os.isdir(prdir) then
        ErrUsage("Wrong or no project directory specified.")
    end
    
    return full, copy, prdir, baseurl, libdir, destos, destarch, exist
end

function CheckArgs()
    if not args[1] then
        ErrUsage("No action specified.")
    elseif args[1] ~= "list" and args[1] ~= "scan" and args[1] ~= "gen" and
           args[1] ~= "gent" and args[1] ~= "auto" and args[1] ~= "edit" and
           args[1] ~= "mktemps" then
        ErrUsage("Wrong or no action specified.")
    end
    
    _G[args[1]] = true -- set action variable (eg scan) to true
    table.remove(args, 1)
    
    local msg, sopts, lopts
    
    if list then
        sopts = "ht:"
        lopts = { {"help"}, {"template", true}, {"only-names"} }
    elseif scan then
        sopts = "hp:l:"
        lopts = { {"help"}, {"prdir", true}, {"libpath", true} }
    elseif gen then
        sopts = "hbsfcn:d:p:l:u:a:D:"
        lopts = { {"help"}, {"simple"}, {"full"}, {"copy"}, {"name", true}, {"desc", true}, {"usebins"}, {"prdir", true}, {"libpath", true}, {"libdir", true}, {"baseurl", true}, {"destos", true}, {"destarch", true}, {"overwrite"}, {"rm-existing"}, {"skip-existing"}, {"libs", true}, {"deps", true} }
    elseif gent then
        sopts = "hsfcn:d:t:p:l:u:a:D:"
        lopts = { {"help"}, {"simple"}, {"full"}, {"copy"}, {"name", true}, {"desc", true}, {"template", true}, {"prdir", true}, {"libpath", true}, {"libdir", true}, {"baseurl", true}, {"destos", true}, {"destarch", true}, {"overwrite"}, {"rm-existing"}, {"skip-existing"}, {"libs", true}, {"deps", true} }
    elseif auto then
        sopts = "hsfcp:l:u:U"
        lopts = { {"help"}, {"simple"}, {"full"}, {"unknown"}, {"copy"}, {"prdir", true}, {"libpath", true}, {"libdir", true}, {"baseurl", true}, {"destos", true}, {"destarch", true}, {"overwrite"}, {"rm-existing"}, {"skip-existing"} }
    elseif edit then
        sopts = "hsfp:d:rcl:"
        lopts = { {"help"}, {"simple"}, {"full"}, {"prdir", true}, {"dep", true}, {"remove"}, {"copy"}, {"libpath", true}, {"destos", true}, {"destarch", true} }
    elseif mktemps then
        sopts = "hl:"
        lopts = { {"help"}, {"libpath", true} }
    end
    
    opts, msg = getopt(args, sopts, lopts)
    
    if not opts then
        ErrUsage(msg)
    end
    
    for _, o in ipairs(opts) do
        if o.name == "h" or o.name == "help" then
            Usage()
            os.exit(0)
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
    
    local function printtemp(t)
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
                if t.libs then
                    print(tabtostr(t.libs))
                end
                if t.patlibs then
                    print(tabtostr(t.patlibs))
                end
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
    
    local loadeddeps, depmap = { }, { }
    local function loaddeps(deps, dep)
        dep = dep or "main"
        for _, d in ipairs(deps) do
            if loadeddeps[d] == nil then
                local ret, msg = LoadDep(prdir, d)
                if not ret then
                    print("WARNING: Failed to load config.lua: " .. msg)
                    loadeddeps[d] = false
                else
                    loadeddeps[d] = ret
                end
            end
            
            if loadeddeps[d] then
                for _, l in ipairs(loadeddeps[d].libs) do
                    depmap[dep] = depmap[dep] or { }
                    depmap[dep][utils.basename(l)] = d
                end
                if loadeddeps[d].deps then
                    loaddeps(loadeddeps[d].deps, loadeddeps[d])
                end
            end
        end
    end
    
    if pkg.deps then
        loaddeps(pkg.deps) -- Load all deps
    end
    
    local function dumpinfo(dep, lpath)
        local map
        if dep then
            map = { }
            for _, l in ipairs(dep.libs) do
                local lp = GetLibDepPath(prdir, dep, l)
                if lp then
                    CollectLibs(map, lp, lpath)
                end
            end
            
            io.write(string.format("Results for dependency %s: ", dep.name))
        else
            map = GetLibMap()
            io.write("Primary dependency results: ")
        end
        
        -- Get all unknown libs
        local unknownlibs = { }
        local depind = dep or "main"
        for l in pairs(map) do
            if not unknownlibs[l] and (not depmap[depind] or not depmap[depind][l]) then
                -- Library is not part of dep itself?
                if not dep or not dep.libs or not utils.tablefind(dep.libs, l) then
                    unknownlibs[l] = true
                end
            end
        end
        
        local sugsimtemps, sugfulltemps, sugdeps = { }, { }, { }
        for l in pairs(unknownlibs) do
            for _, t in ipairs(pkg.deptemplates) do
                if IsInTemplate(t, l) then
                    if t.full then
                        sugfulltemps[t] = sugfulltemps[t] or { }
                        table.insert(sugfulltemps[t], l)
                    else
                        sugsimtemps[t] = sugsimtemps[t] or { }
                        table.insert(sugsimtemps[t], l)
                    end
                    unknownlibs[l] = nil
                    break
                end
            end

            -- Check if existing deps provide library
            for _, d in pairs(loadeddeps) do
                if d and utils.tablefind(d.libs, l) then
                    sugdeps[d] = sugdeps[d] or { }
                    table.insert(sugdeps[d], l)
                    unknownlibs[l] = nil
                    break
                end
            end
        end
        
        local ok = true
        
        if not utils.emptytable(sugsimtemps) then
            ok = false
            print("\n* (New) Templates suggested to be used as simple dependencies:")
            for t, l in pairs(sugsimtemps) do
                print(string.format("   - %s (libs: %s)", t.name, tabtostr(l)))
            end
            print("")
        end
        
        if not utils.emptytable(sugfulltemps) then
            if ok then
                ok = false
                print("")
            end
            print("* (New) Templates suggested to be used as full dependencies:")
            for t, l in pairs(sugfulltemps) do
                print(string.format("   - %s (libs: %s) ", t.name, tabtostr(l)))
            end
            print("")
        end
    
        if not utils.emptytable(sugdeps) then
            if ok then
                ok = false
                print("")
            end
            print("* Existing dependencies who provide missing libraries and are not registrated yet:")
            for d, l in pairs(sugdeps) do
                print(string.format("   - %s (libs: %s) ", d.name, tabtostr(l)))
            end
            print("")
        end

        if not utils.emptytable(unknownlibs) then
            if ok then
                ok = false
                print("")
            end
        
            io.write("* (Remaining) Unknown libraries: ")
            for l in pairs(unknownlibs) do
                io.write(l .. " ")
            end
            print("")
        end
        
        if ok then
            print("OK")
        else
            print("------------")
        end
    end
    
    local lpath = GetLibPath()
    dumpinfo(nil, lpath)
    
    for _, d in pairs(loadeddeps) do
        if d then
            dumpinfo(d, lpath)
        end
    end
    
    print("\n\nNOTE: If one or more templates were used to create a dependency, but are still suggested it's lickely that the result dependenc(y)(ies) don't provide the missing libraries yet. Please verify this by checking the libs field from the respective dependencies. To regenerate a dependency (with the gen or gent actions) with additional libraries use the --libs option.\n")
end

function Generate()
    local full, copy, prdir, baseurl, libdir, destos, destarch, exist = ParseGenArgs()
    local name, desc
    local libs = { }
    local deps
    
    if full == nil then
        full = false -- Default
    end
    
    for _, o in ipairs(opts) do
        if o.name == "n" or o.name == "name" then
            name = o.val
        elseif o.name == "d" or o.name == "desc" then
            desc = o.val
        elseif o.name == "a" or o.name == "libs" then
            libs = optlisttotab(o.val)
        elseif o.name == "D" or o.name == "deps" then
            deps = optlisttotab(o.val)
        end
    end
    
    deps = deps or { }
    
    if not name then
        ErrUsage("A name must be given when the 'gen' action is used.")
    elseif not desc then
        ErrUsage("A description must be given when the 'gen' action is used.")
    end
    
    if not utils.emptytable(args) then
        if utils.emptytable(libs) then
            print("WARNING: Use the --libs (or -a) option to specify any libraries from this dependency.")
        end
        if not copy then
            print("WARNING: --copy (or -c) option not specified, libraries will not be copied automaticly.")
        end
    end
    
    if not CheckExisting(prdir, name, exist) then
        print(string.format("Skipping existing dependency '%s'", name))
        os.exit(1)
    end

    local map = (copy and full and GetLibMap())
    CreateDep(name, desc, libs, libdir, full, baseurl, deps, prdir, copy, map, destos, destarch)
    
    print(string.format([[
Dependency generation complete:
Name                    %s
Description             %s
Libraries               %s
Libraries copied        %s
Library subdirectory    %s
Type                    %s
Dependencies            %s
]], name, desc, tabtostr(libs), tostring(copy and full), libdir, (full and "full") or "simple", tabtostr(deps)))

end

function GenerateFromTemp()
    local full, copy, prdir, baseurl, libdir, destos, destarch, exist = ParseGenArgs()
    local name, desc, temp
    local libs = { }
    local deps

    for _, o in ipairs(opts) do
        if o.name == "n" or o.name == "name" then
            name = o.val
        elseif o.name == "d" or o.name == "desc" then
            desc = o.val
        elseif o.name == "t" or o.name == "template" then
            temp = o.val
        elseif o.name == "a" or o.name == "libs" then
            libs = optlisttotab(o.val)
        elseif o.name == "D" or o.name == "deps" then
            deps = optlisttotab(o.val)
        end
    end
    
    deps = deps or { }
    
    if not temp then
        ErrUsage("No template specified.")
    end
    
    if utils.emptytable(args) then
        ErrUsage("A list of binaries/libraries must be given (used to collect libraries for generated dependencies).")
    end
    
    local notes
    local found = false
    local map = GetLibMap()
    local postf, installf, requiredf, compatf, caninstallf
    
    for _, t in pairs(pkg.deptemplates) do
        if t.name == temp then
            name = name or t.name
            desc = desc or t.description
            
            for l in pairs(map) do
                if not utils.tablefind(libs, l) and IsInTemplate(t, l) then
                    table.insert(libs, l)
                end
            end
            
            if full == nil then
                full = t.full
            end
            
            notes = t.notes
            postf = t.post
            installf = t.install
            requiredf = t.required
            compatf = t.compat
            caninstallf = t.caninstall
            
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
        os.exit(1)
    end
    
    if utils.emptytable(libs) then
        print("WARNING: Found no relevant libraries for specified template. Either re-run this script with other binaries or fill in the required libs manually.")
    end
    
    if not CheckExisting(prdir, name, exist) then
        print(string.format("Skipping existing dependency '%s'", name))
        os.exit(1)
    end
    
    CreateDep(name, desc, libs, libdir, full, baseurl, deps, prdir, copy, map, destos, destarch, postf, installf, requiredf, compatf, caninstallf)
    
    print(string.format([[
Dependency generation complete:
Name                    %s
Description             %s
Libraries               %s
Libraries copied        %s
Library subdirectory    %s
Type                    %s
Notes                   %s
Dependencies            %s
]], name, desc, tabtostr(libs), tostring(copy and full), libdir, (full and "full") or "simple", notes or "-", tabtostr(deps)))

end

function Autogen()
    local full, copy, prdir, baseurl, libdir, destos, destarch, exist = ParseGenArgs()
    local autounknown = false
    
    for _, o in ipairs(opts) do
        if o.name == "U" or o.name == "unknown" then
            autounknown = true
        end
    end
    
    if utils.emptytable(args) then
        ErrUsage("A list of binaries/libraries must be given (used to collect libraries for generated dependencies).")
    end

    local lpath = GetLibPath()
    local depmap, checkstack, missinglibs = { }, { }, { }
    local totallibmap = { }
    local init = true
    
    repeat
        local map
        local depinfo
        
        if not init then
            depinfo = table.remove(checkstack, 1) -- Pop oldest
            if not depinfo then
                break
            end

            map = { }
            for l, p in pairs(depinfo.libs) do
                if p then
                    CollectLibs(map, p, lpath)
                elseif not copy then --HACK: When files are copied, warning is displayed somewhere else.
                    print("WARNING: Missing library: ".. l)
                end
            end
        else
            init = false
            map = GetLibMap()
        end
        
        for l, p in pairs(map) do
            local template
            for _, t in ipairs(pkg.deptemplates) do
                if IsInTemplate(t, l) then
                    template = t
                    break
                end
            end
            
            if not template then
                if not missinglibs[l] then
                    if autounknown then
                        -- Create dummy template for this unknown lib.
                        local lname = string.gsub(l, "(lib)(.+)%.so.*", "%2")
                        template = { name = lname, description = "", libs = { lname },
                                    full = (full ~= nil and full) or true, unknown = true }
                        missinglibs[l] = template
                    else
                        print("WARNING: No template found which provides library " .. l)
                        missinglibs[l] = true -- Just mark as true incase no auto unknowns
                    end
                elseif autounknown then
                    template = missinglibs[l]
                end
            end
            
            if template then
                local newt = false
                if not depmap[template] then
                    depmap[template] = { libs = { }, deps = { }, name = template.name }
                    table.insert(checkstack, depmap[template])
                    newt = true
                end
                
                if not depmap[template].libs[l] then
                    depmap[template].libs[l] = p
                    if not newt and not utils.tablefind(checkstack, depmap[template]) then
                        table.insert(checkstack, depmap[template]) -- Recheck, as new lib(s) need to be checked
                    end
                elseif depmap[template].libs[l] ~= p then
                    print(string.format("WARNING: Found duplicate location for lib %s (%s, %s)", l, depmap[template].libs[l], p))
                end
                
                if depinfo and depinfo.name ~= template.name then
                    depinfo.deps[template.name] = true
                end
            end
        end
        
        utils.tablemerge(totallibmap, map)
    until false

    -- Generate all deps
    for t, di in pairs(depmap) do
        if not CheckExisting(prdir, t.name, exist) then
            print(string.format("Skipping existing dependency '%s'", t.name))
        else
            local fl = ((full == nil) and t.full) or full
            local libs = mapkeytotab(di.libs)
            local deps = mapkeytotab(di.deps)
            
            CreateDep(t.name, t.description, libs, libdir, fl, baseurl, deps, prdir, copy, totallibmap, destos, destarch, t.post, t.install, t.required, t.compat, t.caninstall)
            
            if t.unknown then
                io.write(string.format("Generated %s dependency for unknown library %s ", (fl and "full") or "simple", libs[1]))
            else
                io.write(string.format("Generated %s dependency %s ", (fl and "full") or "simple", t.name))
            end
            print(string.format("(libs: %s; deps: %s)", tabtostr(libs) or "-", tabtostr(deps) or "-"))
        end
    end
end

function EditDep()
    local full, depname, prdir, copy, rem
    local destos, destarch = os.osname, os.arch
    
    for _, o in ipairs(opts) do
        if o.name == "d" or o.name == "dep" then
            depname = o.val
        elseif o.name == "s" or o.name == "simple" then
            full = false
        elseif o.name == "f" or o.name == "full" then
            full = true
        elseif o.name == "p" or o.name == "prdir" then
            prdir = o.val
            if not string.find(prdir, "^/") then -- Not an absolute path?
                prdir = os.getcwd() .. "/" .. prdir
            end
        elseif o.name == "r" or o.name == "remove" then
            rem = true
        elseif o.name == "c" or o.name == "copy" then
            copy = true
        elseif o.name == "destos" then
            destos = o.val
        elseif o.name == "destarch" then
            destarch = o.val
        end
    end
    
    if not depname then
        ErrUsage("No dependency specified.")
    elseif full == nil then
        ErrUsage("Need to specify the --simple or --full option.")
    elseif not prdir or not os.isdir(prdir) then
        ErrUsage("Wrong or no project directory specified.")
    end
    
    local dep, msg = LoadDep(prdir, depname)
    if not dep then
        error("Error: Failed to load config.lua: " .. msg)
    end
    
    if dep.full and full then
        print("WARNING: Dependency is already marked as full.")
    elseif not dep.full and not full then
        print("WARNING: Dependency is already marked as simple.")
    else
        -- Try to patch config file.
        local cpath = string.format("%s/deps/%s/config.lua", prdir, depname)
        local tcpath = cpath .. ".tmp"
        os.copy(cpath, tcpath)
        utils.patch(tcpath, "[%s]*dep%.full[%s]*=[%s]*" .. tostring(not full), "dep.full = " .. tostring(full))
        
        local stat, newdep = pcall(dofile, tcpath)
        if stat and newdep and newdep.full == full then
            -- Copy temp script to original script
            if os.copy(tcpath, cpath) then
                print("Successfully patched config.lua")
            end
        else
            print("WARNING: Failed to patch config file, keeping original. Please change the 'full' field from the dependency manually.")
        end
    
        os.remove(tcpath)
    end
    
    if not full and rem then
        for _, l in ipairs(dep.libs) do
            local lp = GetLibDepPath(prdir, dep, l)
            if lp then
                local stat, msg = os.remove(lp)
                if not stat then
                    print(string.format("WARNING: Failed to remove '%s': %s", lp, msg))
                else
                    print("Removed lib: " .. lp)
                end
            end
        end
    elseif full and copy then
        local map = GetLibMap()
        
        for dir in io.dir(string.format("%s/deps", prdir)) do
            if dir ~= dep.name then
                local subdep = LoadDep(prdir, dir)
                if subdep then
                    -- Dependency depends on given dep?
                    if utils.tablefind(subdep.deps, dep.name) then
                        for _, l in ipairs(subdep.libs) do
                            local lp = GetLibDepPath(prdir, subdep, l)
                            if lp then
                                CollectLibs(map, lp, lpath)
                            end
                        end
                    end
                end
            end
            
            -- Check if we're set yet
            local done = true
            for _, l in ipairs(dep.libs) do
                if not map[l] then
                    done = false
                    break
                end
            end
            
            if done then
                break
            end
        end
        
        -- Also add own libs to map (this is so we can copy own dependent libs)
        for _, l in ipairs(dep.libs) do
            if map[l] then
                CollectLibs(map, map[l], lpath)
            end
        end

        local dest = string.format("%s/deps/%s/files_%s_%s/%s", prdir, dep.name, destos, destarch, dep.libdir)
        
        os.mkdirrec(dest)
        
        for _, l in ipairs(dep.libs) do
            if map[l] and os.fileexists(map[l]) then
                local stat, msg = os.copy(map[l], dest)
                if not stat then
                    print(string.format("WARNING: Failed to copy library '%s': %s", map[l], msg))
                else
                    print("Copied library: " .. l)
                end
            else
                print("WARNING: Could not locate library " .. l)
            end
        end
    end
end

-- UNDONE: Document this?
function MakeTemps()
    if utils.emptytable(args) then
        ErrUsage("A list of binaries/libraries must be given (used to collect libraries for generated dependencies).")
    end

    local lpath = GetLibPath()
    local checkstack, created = { }, { }
    local init = true
    
    repeat
        local map
        local depinfo
        
        if not init then
            local p = table.remove(checkstack, 1) -- Pop oldest
            if not p then
                break
            end
            
            map = { }
            CollectLibs(map, p, lpath)
        else
            init = false
            map = GetLibMap()
        end
        
        for l, p in pairs(map) do
            local exists = false
            if not created[l] then
                for _, t in ipairs(pkg.deptemplates) do
                    if IsInTemplate(t, l) then
                        exists = true
                        break
                    end
                end
                
                if not exists then
                    -- Create dummy template for this unknown lib.
                    created[l] = true
                    local lname = string.gsub(l, "(lib)(.+)%.so.*", "%2")
                    print(string.format([[
-- %s
newtemplate{
name = "%s",
description = "",
libs = { "%s" },
full = true,
}
]], lname, lname, lname))

                    if not p then
                        print("WARNING: Missing library: ".. l)
                    else
                        table.insert(checkstack, p)
                    end
                end
            end
        end
    until false
end

CheckArgs()

if list then
    List()
elseif scan then
    Scan()
elseif gen then
    Generate()
elseif gent then
    GenerateFromTemp()
elseif auto then
    Autogen()
elseif edit then
    EditDep()
elseif mktemps then
    MakeTemps()
end
