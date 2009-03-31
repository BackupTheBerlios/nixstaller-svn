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

loadlua("shared/utils.lua")
loadlua("shared/utils-public.lua")
loadlua("shared/package-public.lua")
loadlua("deptemplates.lua")

-- UNDONE: Remove(?)
local templibcount, tempcount = 0, 0
for n, t in ipairs(pkg.deptemplates) do
    templibcount = templibcount + #t.libs
    tempcount = n
end

print(string.format("Loaded %d libraries filed by %d templates.", templibcount, tempcount))

action = nil

function Usage()
    print(string.format("Usage: %s <action> <options> <files>\n", callscript))
    print([[
<action>        Should be one of the following:

 list       Lists all known dependency templates with info.
 scan       Scans a project directory for unspecified dependencies and suggests possible usable templates.
 gen        Generates a new dependency package file structure.
 gent       Generates a new dependency package file structure from a template.
 auto       Automaticly generates dependency packages from templates.
 edit       Edits an existing dependency package.
 deps       List all dependency packages from a project.


<options>       Depending on the specified action, the following options exist:

 General options:
    --help, -h              Show this message.
    --                      Stop parsing arguments.
    
 Valid options for the 'list' action:
    --template, -t <temp>   Lists info only about template <temp>.
    --only-names            Lists only template names.
    --tags                  Lists all known tags with descriptions.

 Valid options for the 'scan' action:
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.
    --prdir, -p <dir>       The project directory of the installer. This argument is required.
    
 Valid options for the 'gen' action:
    --simple, -s            Generates a simple dependency. This is the default.
    --full, -f              Generates a full dependency.
    --no-standalone         Generates a full non-standalone dependency.
    --name, -n <name>       Name of the dependency. This option is required.
    --desc, -d <desc>       Dependency description. This option is required.
    --libs, -a <libs>       Comma seperated list of all libraries for this dependency.
    --deps, -D <deps>       Comma seperated list of any dependencies for the generated dependency itself.

 Valid options for the 'gent' action:
    --simple, -s            Generates a simple dependency. This is the default.
    --full, -f              Generates a full dependency.
    --no-standalone         Generates a full non-standalone dependency.
    --simple-tag <tags>     Generates a simple dependency incase the given template has one or more of the specified tags. <tags> is a comma seperated list. Prefix a tag with '!' to exclude templates. Remaining templates give full dependencies.
    --full-tag <tags>       As 'simple-tag', but for full dependencies and remaining templates give simple dependencies.
    --nostand-tag <tags>    As 'simple-tag', but for non-standalone dependencies and remaining templates will be standalone. Note that this doesn't whether dependencies are full or not.
    --name, -n <name>       Name of the dependency. Default: template name.
    --desc, -d <desc>       Dependency description. Default: description from template.
    --template, -t <temp>   Generates the dependency from the given template <temp>. This option is required.
    --libs, -a <libs>       Comma seperated list of any additional libraries for this dependency.
    --deps, -D <deps>       Comma seperated list of any dependencies for the generated dependency itself.

 Valid options for the 'auto' action:
    --simple, -s            Generates simple dependencies. This is the default.
    --full, -f              Generates full dependencies.
    --no-standalone         Generates full non-standalone dependencies.
    --simple-tag <tags>     Generates simple dependencies from templates which have one or more of the specified tags. <tags> is a comma seperated list. Prefix a tag with '!' to exclude templates. Remaining templates give full dependencies.
    --full-tag <tags>       As 'simple-tag', but for full dependencies and remaining templates give simple dependencies.
    --nostand-tag <tags>    As 'simple-tag', but for non-standalone dependencies and remaining templates will be standalone. Note that this doesn't affect whether dependencies are full or not.
    --unknown-full, -U      Creates dependencies for libraries which are not listed by any template. The dependency will be named after the library, without the lib prefix and '.so.<version>' suffix (so libfoo.so.1 becomes foo), have an empty description and is defined as full.
    --unknown-simple        As 'unknown-full', but generates simple dependencies.
    --unknown-nostand       As 'unknown-full', but generates non-standalone full dependencies.
    --verbose, -v           Show more verbose output.
    --pretend               Don't do anything, just print what would happen.

 Valid options for the 'gen', 'gent' and 'auto' actions:
    --copy, -c              Copies shared libraries automaticly. The files are copied to the lib/ subdirectory inside the platform/OS specific files folder. This option only affects a full dependencies.
    --prdir, -p <dir>       The project directory of the installer. This argument is required.
    --libpath, -l <dir>     Appends the directory path <dir> to the search path used for finding shared libraries.
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
    --copy, -c              Copies shared libraries automaticly. The files are copied to the lib/ subdirectory inside the platform/OS specific files folder. This option only works when using '--full'.
    --destos <os>           Sets the <os> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not OS specific. Default is the current OS.
    --destarch <arch>       Sets the <arch> portion of the system specific files folder used by the --copy option. 'all' can be used so that copied files are not architecture specific. Default is the current architecture.

 Valid options for the 'deps' action:
    --prdir, -p <dir>       The project directory of the installer. This argument is required.

<files>         A list off all binaries and libraries from the project, used to gather necessary libraries for dependency generation. This is required for the 'edit', 'scan', 'gent', 'auto' actions and when 'gen' is used with the --full and --copy options.
]])
end

function ErrUsage(msg, ...)
    print("Error: " .. string.format(msg, ...) .. "\n")
    Usage()
    os.exit(1)
end

function IsInTemplate(t, l, map)
    if t.check and not t.check(l, map) then
        return false
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
        -- Check libs, allowing search patterns and custom prefixes/suffixes
        for _, tl in ipairs(t.patlibs) do
            if string.find(l, tl) then
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

function GetBins()
    if utils.emptytable(args) then
        ErrUsage("No files given to examine.")
    end

    return args -- Final commandline parameters are always the binaries
end

function CollectLibs(map, f, lpath, rec)
    local m = maplibs(f, lpath)
    
    if not m then
        print(string.format("WARNING: Could not process file %s", f))
    else
        for l, p in pairs(m) do
            if not map[l] then
                map[l] = p
                if rec and p then
                    CollectLibs(map, p, lpath, rec)
                end
            end
        end
    end
end

function GetLibMap(rec)
    local bins, map, lpath = GetBins(), { }, GetLibPath()

    for _, b in ipairs(bins) do
        if not os.fileexists(b) then
            abort(tr("Could not locate file: %s", b))
        end
        CollectLibs(map, b, lpath, rec)
    end
    
    return map
end

function GetLibDepPath(prdir, dep, lib)
    local deppath = string.format("%s/deps/%s", prdir, dep.name)

    for d in io.dir(deppath) do
        if string.find(d, string.format("files_%s_all", os.osname)) or
           string.find(d, string.format("files_all_%s", os.arch)) or
           string.find(d, string.format("files_%s_%s", os.osname, os.arch)) or
           string.find(d, string.format("files_all", os.osname)) then
            local lpath = string.format("%s/%s/%s/%s", deppath, d, dep.libdir, lib)
            if os.fileexists(lpath) then
                return lpath
            end
        end
    end
end

function GetAllLibDepPaths(prdir, dep, lib)
    local deppath = string.format("%s/deps/%s", prdir, dep.name)
    local ret = { }

    for d in io.dir(deppath) do
        if string.find(d, "files_.+_.+") then
            local lpath = string.format("%s/%s/%s/%s", deppath, d, dep.libdir, lib)
            if os.fileexists(lpath) then
                table.insert(ret, lpath)
            end
        end
    end

    return ret
end

function CheckExisting(prdir, d, exist)
    local path = string.format("%s/deps/%s", prdir, d)
    if exist == "overwrite" or not os.isdir(path) then
        return true
    end
    
    if exist == "rm-existing" then
        local ret, msg = utils.removerec(path)
        if not ret then
            abort(string.format("Could not remove existing dependency: %s", msg))
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

function CreateDep(name, desc, libs, libdir, full, standalone, baseurl, deps, prdir, copy, libmap, destos, destarch, postf, installf, requiredf, compatf, caninstallf)
    local path = string.format("%s/deps/%s", prdir, name)
    os.mkdirrec(path)
    
    local out = io.open(string.format("%s/config.lua", path), "w")
    if not out then
        abort("Could not open output config.lua file.")
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
    
    installf = installf or "    self:copyfiles()"
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

-- If this is a full dependency or not.
dep.full = %s

-- If this is a standalone dependency or not (only applies to full dependencies).
dep.standalone = %s

-- Server directory (http, ftp) from where dependency files can be fetched.
-- This is fully optional, works only with full dependencies and only effects
-- dependencies set as extern in package.lua.
dep.baseurl = %s

-- Dependencies for this dependency itself (similar to pkg.deps).
dep.deps = { %s }

-- Install function. This is called when the dependency is installed.
function dep:install()
%s
end

-- required function. Called to check if dependency is required. Note that even if this function
-- returns false, the dependency may still be installed when autodetection marks it as necessary.
function dep:required()
%s
end

-- Called if dependency itself seems to be incompatible. You can use this for example to recompile
-- any libraries. The 'lib' argument contains the faulty library. By returning true the dependency
-- will be installed as usual, otherwise it will be marked as incompatible.
function dep:handlecompat(lib)
%s
end

-- Called just before this dependency is about to be installed. Return false if this dependency can't be
-- installed for some reason, otherwise return true.
function dep:caninstall()
%s
end

-- Return dependency (this line is required!)
return dep
]], os.date(), desc, libsstr, (full and "true") or "false", (standalone and "true") or "false", url, depsstr, installf, requiredf, compatf, caninstallf))

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

function ParseFullArgs()
    local ret = { fullall = false, standall = true, restfull = nil, reststand = nil, tags = { }, sttags = { } }
    for _, o in ipairs(opts) do
        if o.name == "s" or o.name == "simple" then
            ret.fullall, ret.standall = false, false
        elseif o.name == "f" or o.name == "full" then
            ret.fullall, ret.standall = true, true
        elseif o.name == "no-standalone" then
            ret.fullall, ret.standall = true, false
        elseif o.name == "S" or o.name == "simple-tag" or
               o.name == "F" or o.name == "full-tag" or
               o.name == "nostand-tag" then
            local tagtab
            if o.name == "nostand-tag" then
                ret.standall = nil
                ret.reststand = (o.name == "nostand-tag")
                tagtab = ret.sttags
            else
                ret.fullall = nil
                ret.restfull = (o.name == "S" or o.name == "simple-tag")
                tagtab = ret.tags
            end
            
            local tags = optlisttotab(o.val)
            for _, t in ipairs(tags) do
                if string.find(t, "^%!") then -- Starts with '!' ?
                    tagtab[string.sub(t, 2)] = false -- mark as false, trim '!'
                else
                    tagtab[t] = true
                end
            end
        end
    end
    
    return ret
end

function UseTempFull(fulltab, temp)
    -- Returns true if only found normal tags (those not prefixed with !)
    local function positivetag(tagtab)
        local ret = false
        for _, t in ipairs(temp.tags) do
            if tagtab[t] ~= nil then
                ret = tagtab[t]
                if not ret then
                    break
                end
            end
        end
        return ret
    end
    
    local retf, rets
    
    if fulltab.fullall ~= nil then
        retf = fulltab.fullall
    else
        local pos = positivetag(fulltab.tags)
        if fulltab.restfull then
            retf = not pos -- simple-tag was specified
        else
            retf = pos -- full-tag was specified
        end
    end
    
    if fulltab.standall ~= nil then
        rets = fulltab.standall
    else
        local pos = positivetag(fulltab.sttags)
        if fulltab.reststand then
            rets = not pos -- nostand-tag was specified
        else
            rets = pos
        end
    end
    
    return retf, rets
end

function ParseGenArgs()
    local copy = false
    local prdir, baseurl
    local libdir = "lib/"
    local destos, destarch = os.osname, os.arch
    local exist = "ask"
    
    for _, o in ipairs(opts) do
        if o.name == "c" or o.name == "copy" then
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
    
    return copy, prdir, baseurl, libdir, destos, destarch, exist
end

function CheckArgs()
    if not args[1] then
        ErrUsage("No action specified.")
    elseif args[1] ~= "list" and args[1] ~= "scan" and args[1] ~= "gen" and
           args[1] ~= "gent" and args[1] ~= "auto" and args[1] ~= "edit" and
           args[1] ~= "deps" and args[1] ~= "mktemps" then
        ErrUsage("Wrong or no action specified.")
    end
    
    action = args[1] -- set action variable (eg scan)
    table.remove(args, 1)
    
    local msg, sopts, lopts
    
    if action == "list" then
        sopts = "ht:"
        lopts = { {"help"}, {"template", true}, {"only-names"}, {"tags"} }
    elseif action == "scan" then
        sopts = "hp:l:"
        lopts = { {"help"}, {"prdir", true}, {"libpath", true} }
    elseif action == "gen" then
        sopts = "hbsfcn:d:p:l:u:a:D:"
        lopts = { {"help"}, {"simple"}, {"full"}, {"no-standalone"}, {"copy"}, {"name", true}, {"desc", true}, {"prdir", true}, {"libpath", true}, {"libdir", true}, {"baseurl", true}, {"destos", true}, {"destarch", true}, {"overwrite"}, {"rm-existing"}, {"skip-existing"}, {"libs", true}, {"deps", true} }
    elseif action == "gent" then
        sopts = "hsS:fF:cn:d:t:p:l:u:a:D:"
        lopts = { {"help"}, {"simple"}, { "simple-tag", true }, {"full"}, { "full-tag", true },  {"no-standalone"},  {"nostand-tag", true}, {"copy"}, {"name", true}, {"desc", true}, {"template", true}, {"prdir", true}, {"libpath", true}, {"libdir", true}, {"baseurl", true}, {"destos", true}, {"destarch", true}, {"overwrite"}, {"rm-existing"}, {"skip-existing"}, {"libs", true}, {"deps", true} }
    elseif action == "auto" then
        sopts = "hsS:fF:cp:l:u:Uv"
        lopts = { {"help"}, {"simple"}, { "simple-tag", true }, {"full"}, { "full-tag", true }, {"no-standalone"},  {"nostand-tag", true}, {"unknown-simple"}, {"unknown-full"}, {"copy"}, {"prdir", true}, {"libpath", true}, {"libdir", true}, {"baseurl", true}, {"destos", true}, {"destarch", true}, {"overwrite"}, {"rm-existing"}, {"skip-existing"}, {"verbose"}, {"pretend"} }
    elseif action == "edit" then
        sopts = "hsfp:d:rcl:"
        lopts = { {"help"}, {"simple"}, {"full"}, {"prdir", true}, {"dep", true}, {"remove"}, {"copy"}, {"libpath", true}, {"destos", true}, {"destarch", true} }
    elseif action == "deps" then
        sopts = "hp:"
        lopts = { {"help"}, {"prdir", true} }
    elseif action == "mktemps" then
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
        if o.name == "t" or o.name == "template" then
            onlytemp = o.val
        elseif o.name == "only-names" then
            onlynames = true
        elseif o.name == "tags" then
            print(string.format("%-20s%s", "TAG", "DESCRIPTION"))
            for t, d in pairs(pkg.templatetags) do
                print(string.format("%-20s%s", t, d))
            end
            return
        end
    end
    
    local function printtemp(t)
        if onlynames then
            print(t.name)
        else
            print("Dependency name      " .. t.name)
            print("Description          " .. t.description)
            print("Tags                 " .. (tabtostr(t.tags) or ""))
            
            io.write("Filed libraries      ")
            if t.libs then
                print(tabtostr(t.libs))
            end
            if t.patlibs then
                print(tabtostr(t.patlibs))
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
        abort("No such template: " .. onlytemp)
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

    print("Scanning...")

    -- Retrieve all current filed dependencies
    local stat, msg = pcall(loadpackagecfg, prdir)
    if not stat then
        abort("Failed to load package.lua: " .. msg)
    end
    
    local loadeddeps = { }
    local function loaddeps(deps, dep)
        dep = dep or "main"
        for _, d in ipairs(deps) do
            if loadeddeps[d] == nil then
                local ret, msg = loaddep(prdir, d)
                if not ret then
                    print("WARNING: Failed to load config.lua: " .. msg)
                    loadeddeps[d] = false
                else
                    loadeddeps[d] = ret
                    loaddeps(loadeddeps[d].deps, loadeddeps[d])
                end
            end
        end
    end
    
    if pkg.deps then
        loaddeps(pkg.deps) -- Load all deps
    end

    local libdepmap = { }
    local alllibmap = { }
    local lpath = GetLibPath()
    local mainbins = GetBins()
    local warnedlibs = { }

    local function getmaps(bin, path)
        if not libdepmap[bin] then
            libdepmap[bin] = { }
            CollectLibs(libdepmap[bin], path, lpath, false)
            utils.tablemerge(alllibmap, libdepmap[bin])

            for l, p in pairs(libdepmap[bin]) do
                if p then
                    getmaps(l, p)
                elseif not warnedlibs[l] then
                    print("WARNING: Failed to locate " .. l)
                    warnedlibs[l] = true
                end
            end
        end
    end

    for _, b in ipairs(mainbins) do
        getmaps(utils.basename(b), b)
    end
    
    for _, d in pairs(loadeddeps) do
        if d and d.libs and d.full then
            for _, l in ipairs(d.libs) do
                local p = GetLibDepPath(prdir, d, l)
                if p then
                    getmaps(l, p)
                elseif not warnedlibs[l] then
                    print("WARNING: Failed to locate " .. l)
                    warnedlibs[l] = true
                end
            end
        end
    end

    local checklist = { }
    table.insert(checklist, { cat = "main" })

    for _, d in pairs(loadeddeps) do
        if d then
            table.insert(checklist, { cat = "dep", data = d })
        end
    end

    local function findlib(lib)
        if pkg.libs and utils.tablefind(pkg.libs, lib) then
            return true
        end
        
        for _, d in pairs(loadeddeps) do
            if d and d.libs and utils.tablefind(d.libs, lib) then
                return true
            end
        end
        return false
    end
    
    for lib in pairs(alllibmap) do
        if not findlib(lib) then
            table.insert(checklist, { cat = "unknownlib", data = lib })
        end
    end

    local sugtemps, sugdeps, unknownlibs = { }, { }, { }
    for _, item in ipairs(checklist) do
        local bins, deps
        if item.cat == "main" then
            io.write("Primary dependency results... ")
            bins = { }
            for _, b in ipairs(mainbins) do
                table.insert(bins, utils.basename(b))
            end
            deps = pkg.deps
        elseif item.cat == "dep" then
            io.write(string.format("Results for dependency '%s'... ", item.data.name))
            bins = item.data.libs
            deps = item.data.deps
        else -- Unknown lib
            bins = { item.data }
        end

        local ulibs = { } -- Unknown dependency libs
        for _, b in ipairs(bins) do
            if libdepmap[b] then
                for l, p in pairs(libdepmap[b]) do
                    if not findlib(l) then
                        ulibs[l] = true
                    end
                end
            end
        end

        local ok = utils.emptytable(ulibs)

        local function notelib(nmap, name, l)
            nmap[name] = nmap[name] or { }
            if l then
                nmap[name].libs = nmap[name].libs or { }
                nmap[name].libs[l] = true
            end
            if item.cat == "main" then
                nmap[name].main = true
            elseif item.cat == "dep" then
                nmap[name].reqdeps = nmap[name].reqdeps or { }
                nmap[name].reqdeps[item.data.name] = true
            else
                nmap[name].requlibs = nmap[name].requlibs or { }
                nmap[name].requlibs[item.data] = true
            end
        end
        
        for l in pairs(ulibs) do
            -- Check if non-registrated deps provide library
            for _, d in pairs(loadeddeps) do
                if d and utils.tablefind(d.libs, l) then
                    notelib(sugdeps, d.name, l)
                    ulibs[l] = nil
                    break
                end
            end

            if ulibs[l] and libdepmap[l] then
                -- Check if we can make a suggestion
                for _, t in ipairs(pkg.deptemplates) do
                    if IsInTemplate(t, l, libdepmap[l]) then
                        notelib(sugtemps, t.name, l)
                        ulibs[l] = nil
                        break
                    end
                end
            end

            if ulibs[l] then
                notelib(unknownlibs, l)
            end
        end

        if item.cat ~= "unknownlib" then
            print((ok and "OK") or "Not OK")
        end
    end

    print("----------\nRESULTS\n----------")

    local function printnote(notes)
        for n, info in pairs(notes) do
            io.write(string.format(" * %s", n))

            if info.libs then
                io.write(" (")
                local init = true
                for l in pairs(info.libs) do
                    if not init then
                        io.write(", ")
                    else
                        init = false
                    end
                    io.write(l)
                end
                io.write(")")
            end
            
            print(" required by:")

            local function printentry(e)
                print("    - " .. e)
            end

            if info.main then
                printentry("Main binaries/libraries")
            end

            if info.reqdeps then
                for d in pairs(info.reqdeps) do
                    printentry(string.format("Dependency %s", d))
                end
            end

            if info.requlibs then
                local secsugtemps = { } -- Suggested templates for unknown libs
                for ul in pairs(info.requlibs) do
                    if libdepmap[ul] then
                        for _, t in ipairs(pkg.deptemplates) do
                            if IsInTemplate(t, ul, libdepmap[ul]) then
                                secsugtemps[t.name] = true
                                info.requlibs[ul] = nil
                                break
                            end
                        end
                    end
                end

                for st in pairs(secsugtemps) do
                    printentry("Dependency template " .. st)
                end

                -- Remaing unknown libs
                for ul in pairs(info.requlibs) do
                    printentry("Unknown library " .. ul)
                end
            end
        end
    end
    
    if not utils.emptytable(sugdeps) then
        print("Existing dependencies who provide missing libraries and are not registrated yet:")
        printnote(sugdeps)
        print("")
    end

    if not utils.emptytable(sugtemps) then
        print("Suggested templates:")
        printnote(sugtemps)
        print("")
    end

    if not utils.emptytable(unknownlibs) then
        print("Unknown library dependencies")
        printnote(unknownlibs)
        print("")
    end

    print("\n\nNOTE: If one or more templates were used to create a dependency, but are still suggested it's likely that the result dependenc(y)(ies) don't provide the missing libraries yet. Please verify this by checking the libs field from the respective dependencies. To regenerate a dependency (with the gen or gent actions) with additional libraries use the --libs option.\n")

    if not utils.emptytable(warnedlibs) then
        print("WARNING: One or more libraries were not found, this will give incomplete results! Use the --libpath option to specifiy additional search directories.")
    end
end

function Generate()
    local fulltab = ParseFullArgs()
    local copy, prdir, baseurl, libdir, destos, destarch, exist = ParseGenArgs()
    local name, desc
    local libs = { }
    local deps
    
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

    local map = (copy and fulltab.fullall and GetLibMap(true))
    CreateDep(name, desc, libs, libdir, fulltab.fullall, fulltab.standall, baseurl, deps, prdir, copy, map, destos, destarch)
    
    local typ
    if fulltab.fullall then
        if fulltab.standall then
            typ = "full (standalone)"
        else
            typ = "full (not standalone)"
        end
    else
        typ = "simple"
    end
    
    print(string.format([[
Dependency generation complete:
Name                    %s
Description             %s
Libraries               %s
Libraries copied        %s
Type                    %s
Dependencies            %s
]], name, desc, tabtostr(libs) or "-", tostring(copy and fulltab.fullall), typ, tabtostr(deps) or "-"))

end

function GenerateFromTemp()
    local fulltab = ParseFullArgs()
    local copy, prdir, baseurl, libdir, destos, destarch, exist = ParseGenArgs()
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
    
    local full, standalone
    local notes
    local found = false
    local map = GetLibMap(true)
    local postf, installf, requiredf, compatf, caninstallf
    
    for _, t in ipairs(pkg.deptemplates) do
        if t.name == temp then
            name = name or t.name
            desc = desc or t.description
            
            for l in pairs(map) do
                if not utils.tablefind(libs, l) and IsInTemplate(t, l, map) then
                    table.insert(libs, l)
                end
            end
            
            full, standalone = UseTempFull(fulltab, t)
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
        for _, t in ipairs(pkg.deptemplates) do
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
    
    CreateDep(name, desc, libs, libdir, full, standalone, baseurl, deps, prdir, copy, map, destos, destarch, postf, installf, requiredf, compatf, caninstallf)
    
    local typ
    if full then
        if standalone then
            typ = "full (standalone)"
        else
            typ = "full (not standalone)"
        end
    else
        typ = "simple"
    end
    
    print(string.format([[
Dependency generation complete:
Name                    %s
Description             %s
Libraries               %s
Libraries copied        %s
Type                    %s
Notes                   %s
Dependencies            %s
]], name, desc, tabtostr(libs) or "-", tostring(copy and full), typ, notes or "-", tabtostr(deps) or "-"))

end

function Autogen()
    local fulltab = ParseFullArgs()
    local fullunknown = false
    local standunknown = true
    local copy, prdir, baseurl, libdir, destos, destarch, exist = ParseGenArgs()
    local autounknown = false
    local verbose = false
    local pretend = false
    
    for _, o in ipairs(opts) do
        if o.name == "U" or o.name == "unknown-full" then
            autounknown, fullunknown, standunknown = true, true, true
        elseif o.name == "unknown-simple" then
            autounknown, fullunknown = true, false
        elseif o.name == "unknown-nostand" then
            autounknown, fullunknown, standunknown = true, true, false
        elseif o.name == "v" or o.name == "verbose" then
            verbose = true
        elseif o.name == "pretend" then
            pretend = true
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
                    CollectLibs(map, p, lpath, true)
                elseif not copy then --HACK: When files are copied, warning is displayed somewhere else.
                    print("WARNING: Missing library: ".. l)
                end
            end
        else
            init = false
            map = GetLibMap(true)
        end
        
        for l, p in pairs(map) do
            local template
            for _, t in ipairs(pkg.deptemplates) do
                if IsInTemplate(t, l, map) then
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
                                    unknown = true }
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
            local fl, st
            if t.unknown then
                fl, st = fullunknown, standunknown
            else
                fl, st = UseTempFull(fulltab, t)
            end
            
            local libs = mapkeytotab(di.libs)
            local deps = mapkeytotab(di.deps)
            
            if not pretend then
                CreateDep(t.name, t.description, libs, libdir, fl, st, baseurl, deps, prdir, copy, totallibmap, destos, destarch, t.post, t.install, t.required, t.compat, t.caninstall)
            end
            
            local typ
            if fl then
                if st then
                    typ = "standalone"
                else
                    typ = "full (not standalone)"
                end
            else
                typ = "simple"
            end
            
            local genstr = (pretend and "Would generate") or "Generated"
            if verbose then
                if t.unknown then
                    print(string.format("%s %s dependency for unknown library %s (libs: %s; deps %s)", genstr, typ, libs[1], tabtostr(libs) or "-", tabtostr(deps) or "-"))
                else
                    print(string.format("%s %s dependency %s (libs: %s; deps: %s; tags: %s)", genstr, typ, t.name, tabtostr(libs) or "-", tabtostr(deps) or "-", tabtostr(t.tags)))
                end
            else
                if t.unknown then
                    print(string.format("%s %s dependency for unknown library %s", genstr, typ, libs[1]))
                else
                    print(string.format("%s %s dependency %s", genstr, typ, t.name))
                end
            end
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
    
    local dep, msg = loaddep(prdir, depname)
    if not dep then
        abort("Failed to load config.lua: " .. msg)
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
            local paths = GetAllLibDepPaths(prdir, dep, l)
            for _, lp in ipairs(paths) do
                local stat, msg = os.remove(lp)
                if not stat then
                    print(string.format("WARNING: Failed to remove '%s': %s", lp, msg))
                else
                    print("Removed lib: " .. lp)
                end
            end
        end
    elseif full and copy then
        local map = GetLibMap(true)
        
        for dir in io.dir(string.format("%s/deps", prdir)) do
            if dir ~= dep.name then
                local subdep = loaddep(prdir, dir)
                if subdep then
                    -- Dependency depends on given dep?
                    if utils.tablefind(subdep.deps, dep.name) then
                        for _, l in ipairs(subdep.libs) do
                            local lp = GetLibDepPath(prdir, subdep, l)
                            if lp then
                                CollectLibs(map, lp, lpath, true)
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
                CollectLibs(map, map[l], lpath, true)
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

function ListDeps()
    local prdir
    
    for _, o in ipairs(opts) do
        if o.name == "p" or o.name == "prdir" then
            prdir = o.val
            if not string.find(prdir, "^/") then -- Not an absolute path?
                prdir = os.getcwd() .. "/" .. prdir
            end
        end
    end
    
    if not prdir or not os.isdir(prdir) then
        ErrUsage("Wrong or no project directory specified.")
    end
    
    -- Retrieve all current filed dependencies
    local stat, msg = pcall(loadpackagecfg, prdir)
    if not stat then
        abort("Failed to load package.lua: " .. msg)
    end
    
    local loadeddeps, depmap = { }, { }
    local function loaddeps(deps, dep)
        dep = dep or "Main"
        for _, d in ipairs(deps) do
            depmap[d] = depmap[d] or { }
            depmap[d][dep] = true

            if loadeddeps[d] == nil then
                local ret, msg = loaddep(prdir, d)
                if not ret then
                    print("WARNING: Failed to load config.lua: " .. msg)
                    loadeddeps[d] = false
                else
                    loadeddeps[d] = ret
                    loaddeps(loadeddeps[d].deps, loadeddeps[d].name) -- Go through all deps recursively
                end
            end
        end
    end
    
    if pkg.deps then
        loaddeps(pkg.deps) -- Load all deps
    else
        print("No dependencies filed in project's package.lua")
        return
    end
    
    for n, d in pairs(loadeddeps) do
        local libs = (d.libs and not utils.emptytable(d.libs) and tabtostr(d.libs)) or "None"
        local deps = (d.deps and not utils.emptytable(d.deps) and tabtostr(d.deps)) or "None"
        local reqs = (depmap[d.name] and not utils.emptytable(depmap[d.name]) and tabtostr(mapkeytotab(depmap[d.name]))) or "None"
        local stnd = (d.full and ((d.standalone and "Yes") or "No")) or "-"
        local url = (utils.emptystring(d.baseurl) and "-") or d.baseurl
        print(string.format([[
DEPENDENCY "%s"
    - Description       %s
    - Libraries         %s
    - Dependencies      %s
    - Required by       %s
    - Full              %s
    - Standalone        %s
    - Base URL          %s
]], n, d.description, libs, deps, reqs, (d.full and "Yes") or "No", stnd, url))
    end
    
    print("\nNOTE: Only dependencies are shown if they are registrated in package.lua or by another dependency.")
end

-- UNDONE: Document this?
function MakeTemps()
    if utils.emptytable(args) then
        ErrUsage("A list of binaries/libraries must be given (used to collect libraries for generated dependencies).")
    end

    local lpath = GetLibPath()
    local checkstack, created = { }, { }
    
    for _, b in ipairs(args) do
        if not os.fileexists(b) then
            abort(tr("Could not locate file: %s", b))
        end
        table.insert(checkstack, b)
    end
    
    repeat
        local map
        local depinfo
        
        local p = table.remove(checkstack, 1) -- Pop oldest
        if not p then
            break
        end
        
        map = { }
        CollectLibs(map, p, lpath, true)
        
        for l, p in pairs(map) do
            local exists = false
            if not created[l] then
                for _, t in ipairs(pkg.deptemplates) do
                    if IsInTemplate(t, l, map) then
                        exists = true
                        break
                    end
                end
                
                if not exists then
                    created[l] = 1
                    local lname = string.gsub(l, "(lib)(.+)%.so.*", "%2")
                    print(string.format([[
-- %s
newtemplate{
name = "%s",
description = "",
libs = { "%s" },
tags = { },
}
]], lname, lname, lname))

                    if not p then
                        print("WARNING: Missing library: ".. l)
                    else
                        table.insert(checkstack, p)
                    end
                end
            else
                created[l] = created[l] + 1
            end
        end
    until false
    
    print("Library usage:")
    local sorted = { }
    for l, n in pairs(created) do
        sorted[n] = sorted[n] or { }
        table.insert(sorted[n], l)
    end
    for n, lt in ipairs(sorted) do
        for _, l in ipairs(lt) do
            print(string.format("%s: %d", l, n))
        end
    end
end

CheckArgs()

if action == "list" then
    List()
elseif action == "scan" then
    Scan()
elseif action == "gen" then
    Generate()
elseif action == "gent" then
    GenerateFromTemp()
elseif action == "auto" then
    Autogen()
elseif action == "edit" then
    EditDep()
elseif action == "deps" then
    ListDeps()
elseif action == "mktemps" then
    MakeTemps()
end
