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

-- Util functions used by various scripts

function gettmpinterndir(file, dontcreate)
    -- Note that this directory should be removed manually.
    -- From normal installation mode this is done automatically
    -- by makeself. For 'fastrun' install mode this is done via
    -- the lua exit func.
    local path
    if internal.fastrun then
        path = internal.configdir .. "/tmp"
    else
        path = internal.rundir
    end

    if not dontcreate and not os.fileexists(path) then
        os.mkdir(path)
    end

    if file then
        return path .. "/" .. file
    else
        return path
    end
end

function getxdgutilsdir(file)
    local path
    if internal.fastrun then
        path = internal.nixstdir .. "/src/xdg-utils"
    else
        path = internal.rundir .. "/xdg-utils"
    end

    if file then
        return path .. "/" .. file
    else
        return path
    end
end

function pkgsize(src)
    local ret = 0
    utils.recursivedir(src, function (f)
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
    return gettmpinterndir(f .. ".desktop")
end

function instxdgentries(global, fname)
    local xdgbin
    if os.execute("(xdg-desktop-menu --version) >/dev/null 2>&1") == 0 then
        xdgbin = "xdg-desktop-menu"
    else
        xdgbin = getxdgutilsdir("xdg-desktop-menu")
    end
    
    local cmd = (global and install.executeasroot) or install.execute
    cmd(string.format("%s install --novendor %s", xdgbin, fname), false)
end

function xdguninstscript()
    if not OLDG.install.menuentries then
        return nil
    end
    
    local ret = string.format([[
EXEC=""
(xdg-desktop-menu --version) >/dev/null 2>&1 && EXEC="xdg-desktop-menu"
[ -z "$EXEC" ] && ("%s/xdg-utils/xdg-desktop-menu" --version) 2>&1 >/dev/null && EXEC="%s/xdg-utils/xdg-desktop-menu"
]], pkg.getdatadir(), pkg.getdatadir())
    
    if not utils.emptytable(OLDG.install.menuentries) then
        ret = ret .. "if [ ! -z \"$EXEC\" ]; then\n"
        for n, _ in pairs(OLDG.install.menuentries) do
            ret = ret .. string.format("    $EXEC uninstall --novendor \"%s.desktop\"\n", n)
        end
        ret = ret .. "fi\n"
    end
    
    return ret
end

function getopt(args, sopts, lopts, ignore)
    local curopt
    local ret = { }
    while #args > 0 do
        local a = table.remove(args, 1)
        
        if a == "--" then
            return ret
        end

        if not curopt then
            if not string.find(a, "^(%-)") then
                table.insert(args, 1, a) -- Not a commandline option; stop parsing and put arg back
                return ret
            end
            
            if string.find(a, "^(%-%-)") then -- long option
                local aname = string.gsub(a, "^(%-%-)", "")
                local found = false
                for _, lo in ipairs(lopts) do
                    if lo[1] == aname then
                        if lo[2] then
                            curopt = aname
                        else
                            table.insert(ret, { name = aname, val = true })
                        end
                        found = true
                        break
                    end
                end
                if not ignore and not found then
                    return nil, "Unknown commandline option: " .. a
                end
            else
                for aname in string.gmatch(string.gsub(a, "^%-", ""), ".") do
                    if curopt then -- Argument with option has been given before, can't be good.
                        if not ignore then
                            return nil, string.format("Commandline option %s requires a value.", curopt)
                        end
                    elseif string.find(sopts, aname .. ":") then -- argument with option
                        curopt = aname
                    elseif string.find(sopts, aname) then
                        table.insert(ret, { name = aname, val = true })
                    elseif not ignore then
                        return nil, "Unknown commandline option: " .. a
                    end
                end
            end
        else
            table.insert(ret, { name = curopt, val = a })
            curopt = nil
        end
    end
    
    if not ignore and curopt then
        return nil, "No value specified for " .. curopt
    end
    
    return ret
end

function optlisttotab(val)
    local ret = { }
    for i in string.gmatch(val, "[^,]+") do
        if not utils.tablefind(ret, i) then
            table.insert(ret, i)
        end
    end
    return ret
end

function haveunopt(arg)
    return cfg.unopts[arg] and cfg.unopts[arg].value and cfg.unopts[arg].internal
end

function loaddep(prdir, name)
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
    default("standalone", true)
    default("libdir", "lib/")
    default("description", "")
    default("libs", { })
    default("deps", { })
    
    ret.name = name
    
    return ret
end

function autosymmap(confdir, dest, depmap)
    local binlist = "" -- All bins/libs for symmap
    local pathlist = ""
    local givenpaths = { }
    
    local function addbins(startpath, bins, subpath)
        for _, b in ipairs(bins) do
            if subpath then
                b = subpath .. "/" .. b
            end
            local path = string.format("%s/files_%s_%s/%s", startpath, os.osname, os.arch, b)
            binlist = binlist .. " " .. path

            local dirp = utils.dirname(path)
            if not givenpaths[p] then
                pathlist = string.format("%s -l %s", pathlist, dirp)
                givenpaths[dirp] = true
            end
        end
    end

    if pkg.bins then
        addbins(confdir, pkg.bins)
    end

    if pkg.libs then
        addbins(confdir, pkg.libs)
    end

    for n, d in pairs(depmap) do
        if d.libs and d.full then
            addbins(string.format("%s/deps/%s", confdir, n), d.libs, d.libdir)
        end
    end

    if os.execute(string.format("%s/gensyms.sh %s -d %s %s", internal.nixstdir, pathlist, dest, binlist)) ~= 0 then
        print("WARNING: Failed to automatically generate symbol map file.")
    end
end
