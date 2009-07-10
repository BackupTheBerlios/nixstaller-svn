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

loadluash("utils.lua")
loadluash("utils-public.lua")
loadlua("shared/utils.lua")
loadlua("shared/utils-public.lua")
loadlua("package.lua")

-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)

function install.gettempdir()
    local path = gettmpinterndir("tmp")
    if not os.fileexists(path) then
        local stat, msg = os.mkdirrec(path)
        if not stat then
            error("Failed to create temporary file directory: " .. msg)
        end
    end
    return path
end

function install.extrafilespath(file)
    local path
    if internal.fastrun then
        path = internal.configdir .. "/files_extra"
    else
        path = gettmpinterndir("files_extra")
    end
    return (file and (path .. "/" .. file)) or path
end

function install.extract(luaout)
    if internal.fastrun then
        copyinstfiles(luaout)
        return
    end
    
    local archives = { } -- Files to extract
    
    local function checkarch(a)
        local path = string.format("%s/%s", internal.rundir, a)
        if os.fileexists(path) then
            table.insert(archives, path)
        end
    end
    
    checkarch("instarchive_all")
    checkarch(string.format("instarchive_%s_all", os.osname))
    checkarch(string.format("instarchive_all_%s", os.arch))
    checkarch(string.format("instarchive_%s_%s", os.osname, os.arch))
    
    if utils.emptytable(archives) then
        return
    end
    
    local szmap = { }
    local totalsize = 0
    for _, f in ipairs(archives) do
        local szfile = io.open(f .. ".sizes", "r")
        if szfile then
            szmap[f] = { }
            for line in szfile:lines() do
                local sz = tonumber(string.match(line, "^[^%s]*"))
                szmap[f][string.gsub(line, "^[^%s]*%s*", "")] = sz
                totalsize = totalsize + sz
            end
            szfile:close()
        end
    end
    
    local doasroot = not os.writeperm(install.destdir)
    local extractedsz = 0
    
    for _, f in ipairs(archives) do
        local extrcmd
        if cfg.archivetype == "gzip" then
            extrcmd = string.format("cat %s | gzip -cd | tar xvf -", f)
        elseif cfg.archivetype == "bzip2" then
            extrcmd = string.format("cat %s | bzip2 -d | tar xvf -", f)
        else
            extrcmd = string.format("(%s/lzma-decode %s - 2>/dev/null | tar xvf -)", internal.bindir, f)
        end
        
        local exec = (doasroot and install.executecmdasroot) or install.executecmd
        exec(extrcmd, function (s)
            local file = string.gsub(s, "^x ", "")
            file = string.gsub(file, "\n$", "")
            
            if os.osname == "sunos" then
                -- Solaris puts some extra info after filename, remove
                file = string.gsub(file, ", [%d]* bytes, [%d]* tape blocks", "")
            end
            
            if szmap[f] and szmap[f][file] and totalsize > 0 then
                extractedsz = extractedsz + szmap[f][file]
                luaout("Extracting file: " .. file, extractedsz / totalsize * 100)
            end
        end, true)
    end
end

function install.newdesktopentry(b, i, c)
    -- Defaults
    t = {}
    t.Name = cfg.appname
    t.Type = "Application"
    t.Encoding = "UTF-8"
    t.Exec = b
    t.Icon = i
    t.Categories = c
    return t
end

function install.gendesktopentries(global)
    if not install.menuentries then
        return
    end
    
    for n, e in pairs(install.menuentries) do
        local fname = xdgentryfname(n)
        local f = check(io.open(fname, "w"))
        check(f:write("[Desktop Entry]\n"))
        for i, v in pairs(e) do
            check(f:write(i .. "=" .. v .. "\n"))
        end
        f:close()
        
        if not pkg.enable then
            instxdgentries(global, fname) -- Package installations do it in generatepkg()
        end
    end
end

-- End Public functions
-----------------------------

local function initlang()
    install.langinfo = { }
    local hasutf8 = internal.hasutf8()
    
    -- Read language configs
    for _, l in ipairs(cfg.languages) do
        local conf = string.format("%s/lang/%s/config.lua", internal.configdir, l)
        
        local lang
        if os.fileexists(conf) then
            lang = dofile(conf)
        end
        
        lang = lang or { }
        
        local function default(var, val)
            if lang[var] == nil then
                lang[var] = val
            end
        end
        
        default("name", l)
        default("utf8", true)
        
        if hasutf8 or not lang.utf8 then
            install.langinfo[l] = lang
        end
    end
    
    -- Set language
    install.didautolang = false -- Used by language screen
    if cfg.autolang then
        local curlang = os.setlocale()
        for l, v in pairs(install.langinfo) do
            if v.locales then
                for _, loc in ipairs(v.locales) do
                    if string.find(curlang, loc) then
                        internal.setlang(l)
                        install.didautolang = true
                        break
                    end
                end
                if install.didautolang then
                    break
                end
            end
        end
    end
    
    if not install.didautolang and cfg.defaultlang then
        internal.setlang(cfg.defaultlang)
    end
end

-- Used by fastrun mode as alternative to file extraction
function copyinstfiles(luaout)
    local filemap = { }
    local totalsize = 0

    local function getinstfiles(dir)
        if not os.fileexists(dir) then
            return
        end
        utils.recursivedir(dir, function(path, relpath)
            filemap[relpath] = { }
            filemap[relpath].size = os.filesize(path)
            filemap[relpath].path = path
            totalsize = totalsize + filemap[relpath].size
        end)
    end

    getinstfiles(string.format("%s/files_all", internal.configdir))
    getinstfiles(string.format("%s/files_%s_all", internal.configdir, os.osname))
    getinstfiles(string.format("%s/files_all_%s", internal.configdir, os.arch))
    getinstfiles(string.format("%s/files_%s_%s", internal.configdir,
                 os.osname, os.arch))

    local copiedsz = 0
    for rp in pairs(filemap) do
        local destdir
        local isdir = os.isdir(filemap[rp].path)
        if not isdir then
            destdir = utils.dirname(rp)
        else
            destdir = rp
        end

        if destdir then
            os.mkdirrec(destdir)
        end

        copiedsz = copiedsz + filemap[rp].size

        local stat, msg
        if not isdir then -- only copy files as dir should already be created
            stat, msg = os.copy(filemap[rp].path, rp)
            if not stat then
                install.print(string.format("WARNING: Failed to copy file '%s': %s\n", rp, msg))
            end
        end
        
        if stat and totalsize > 0 then
            luaout("Extracting file (fast mode): " .. rp,
                    copiedsz / totalsize * 100)
        end
    end
end

loadconfig(internal.configdir)
initlang()

install.destdir = os.getenv("HOME") or "/"
install.menuentries = { }

if install.unattended then
    loadlua("unattinstall.lua")
else
    loadlua("attinstall.lua")
end
