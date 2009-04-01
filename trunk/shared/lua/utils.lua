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

function maplibs(bin, extrapath)
    local elf = os.openelf(bin)
    
    if not elf then
        return
    end
    
    local map = { }
    
    local n = 1
    local need = elf:getneeded(n)
    while need do
        map[need] = false
        n = n + 1
        need = elf:getneeded(n)
    end
    local rpath = elf:getrpath()
    
    local machine = elf:getmachine()
    local class = elf:getclass()

    elf:close()

    local function validlib(lib)
        -- Must match machine (CPU arch) and bitness
        local e = os.openelf(lib)
        if not e then
            return false
        end
        
        local m, c = e:getmachine(), e:getclass()
        e:close()
        return m == machine and c == class
    end
    
    -- FreeBSD 7's ldd will throw an error when a specified rpath is not existant (bug?)
    -- OpenBSD will throw an error when one or more libs are not found. Therefore we need
    -- to manually check for libs in these two cases
    if os.osname == "openbsd" or (os.osname == "freebsd" and not utils.emptystring(rpath)) then
        -- Search order: extrapath, LD_LIBRARY_PATH, f's own rpath or runpath, ldconfig hints
        local lpaths = extrapath or { }
        
        local ldpath = os.getenv("LD_LIBRARY_PATH")
        if ldpath then
            for p in string.gmatch(ldpath, "[^\:]+") do
                table.insert(lpaths, p)
            end
        end

        -- rpath/runpath
        if rpath and #rpath > 0 then
            local bdir = utils.dirname(bin)
            if utils.emptystring(bdir) then
                bdir = curdir
            end
            
            for p in string.gmatch(rpath, "[^\:]+") do
                p = string.gsub(p, "${ORIGIN}", bdir)
                p = string.gsub(p, "$ORIGIN", bdir)
                table.insert(lpaths, p)
            end
        end
        
        -- ldconfig hints
        local pipe = io.popen("ldconfig -r | grep \"search directories\"")
        local libs = pipe:read()
        pipe:close()

        libs = string.gsub(libs, ".*: ", "")
        for p in string.gmatch(libs, "[^\:]+") do
            table.insert(lpaths, p)
        end

        for nl in pairs(map) do
            for _, p in ipairs(lpaths) do
                local lpath = string.format("%s/%s", p, nl)
                if os.fileexists(lpath) and validlib(lpath) then
                    map[nl] = lpath
                end
            end
        end
    else
        -- Search libs using 'ldd'
        local pipe = io.popen(string.format("ldd %s 2>/dev/null", bin))
        
        if not pipe then
            return
        end
        
        local line = pipe:read()
        while line do
            local path = string.gsub(line, "^.*=>", "")
            path = string.gsub(path, " %(.*%)$", "")
            path = string.gsub(path, "^[%s]*", "")
            local file = (path and utils.basename(path))
            if file and os.fileexists(path) and map[file] ~= nil and validlib(path) then
                map[file] = path
            end
            line = pipe:read()
        end
        pipe:close()
        
        if extrapath and not utils.emptytable(extrapath) then
            for n in pairs(map) do
                for _, p in ipairs(extrapath) do
                    local file = string.format("%s/%s", p, n)
                    if os.fileexists(file) then
                        map[n] = file
                        break
                    end
                end
            end
        end
    end
    
    return map
end

function getsyms(bin)
    local elf = os.openelf(bin)
    
    if not elf then
        return
    end
    
    local ret = { }
    local n = 1
    local sym = elf:getsym(1)
    while sym do
        ret[sym.name] = sym
        ret[sym.name].undefined = (sym.undefined and sym.binding == "GLOBAL")
        n = n + 1
        sym = elf:getsym(n)
    end
    
    elf:close()
    
    return ret
end

function tabtostr(t)
    local ret
    for _, v in ipairs(t) do
        if not ret then
            ret = v
        else
            ret = ret .. ", " .. v
        end
    end
    return ret
end

function mapkeytotab(t)
    local ret = { }
    for k in pairs(t) do
        table.insert(ret, k)
    end
    return ret
end

function escapepat(s)
    local ret = string.gsub(s, "([%%%.%?%*%+%-%(%)%[%]%^%$])", "%%%1")
    return ret
end

function loadconfig(path)
    dofile(path .. "/config.lua")
    
    local function default(var, val)
        if cfg[var] == nil then
            cfg[var] = val
        end
    end
    
    default("appname", "")
    default("languages", { "english", "dutch", "lithuanian", "bulgarian" })
    default("defaultlang", "english")
    default("targetos", { os.osname })
    default("targetarch", { os.arch })
    default("frontends", { "gtk", "fltk", "ncurses" })
    default("archivetype", "lzma")
    default("mode", "attended")
    default("autolang", true)

    if not utils.tablefind(cfg.languages, cfg.defaultlang) then
        cfg.defaultlang = cfg.languages[1]
    end
    
    if cfg.archivetype ~= "gzip" and cfg.archivetype ~= "bzip2" and cfg.archivetype ~= "lzma" then
        error("Wrong archive type specified!", 0)
    end
    
    if cfg.mode ~= "attended" and cfg.mode ~= "unattended" and cfg.mode ~= "both" then
        error("Wrong mode specified!", 0)
    end
    
    for _, fr in ipairs(cfg.frontends) do
        if fr ~= "fltk" and fr ~= "gtk" and fr ~= "ncurses" then
            error("One or more invalid frontend(s) specified!")
        end
    end
    
    for n, o in pairs(cfg.unopts) do
        if o.opttype then
            if o.opttype ~= "string" and o.opttype ~= "list" then
                error(string.format("Wrong opttype specified for unattended option '%s'", n))
            elseif not o.optname then
                error(string.format("No optname specified for unattended option '%s'", n))
            end
        end
    end
end

function loadpackagecfg(path)
    dofile(path .. "/package.lua")
    
    local function default(var, val)
        if pkg[var] == nil then
            pkg[var] = val
        end
    end
    
    default("deps", { })
    default("enable", false)
    default("register", true)
    default("setkdeenv", false)
    default("release", "1")
    default("url", "")
    default("license", "")
    default("autosymmap", false)
end

function loadrun(path)
    install.destdir = os.getenv("HOME") or "/"
    install.menuentries = { }
    dofile(path .. "/run.lua")
end
