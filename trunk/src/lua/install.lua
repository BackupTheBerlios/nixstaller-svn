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

dofile("shared/utils.lua")
dofile("shared/utils-public.lua")
dofile("package.lua")

do
    local oldf = install.newscreen
    
    function install.newscreen(t) -- Overide function
        local ret = oldf(t)
        local wrapfuncs = { "addinput", "addcheckbox", "addradiobutton", "adddirselector", "addcfgmenu", "addmenu", "addimage",
                            "addprogressbar", "addtextfield", "addlabel" }
        
        -- Create wrappers for widget creation functions from luagroups: the functions given in 'wrapfuncs'
        -- will be added to the installscreen. These will create a new luagroup and call the function
        -- with the same name from that group.
        for i,v in pairs(wrapfuncs) do
            ret[v] = function(self, ...)
                        local group = self:addgroup()
                        return group[v](group, ...)
                     end
        end

        return ret
    end
end

-- UNDONE: Function name?
function install.extract(luaout)
    local archives = { } -- Files to extract
    
    local function checkarch(a)
        local path = string.format("%s/%s", curdir, a)
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
        else
            print("No sizes for", f)
        end
    end
    
    local doasroot = not os.writeperm(install.destdir)
    local extractedsz = 0
    
    for _, f in ipairs(archives) do
        local extrcmd
        if cfg.archivetype == "gzip" then
            extrcmd = string.format("cat %s | gzip -cd | tar xvf -", f)
        elseif cfg.archivetype == "bzip2" then
            extrcmd = string.format("cat %s | bzip -d | tar xvf -", f)
        else
            extrcmd = string.format("(%s/lzma-decode %s - 2>/dev/null | tar xvf -)", bindir, f)
        end
        
        local exec = (doasroot and install.executecmdasroot) or install.executecmd
        exec(extrcmd, function (s)
            local file = string.gsub(s, "^x ", "")
            
            if os.osname == "sunos" then
                -- Solaris puts some extra info after filename, remove
                file = string.gsub(file, ", [%d]* bytes, [%d]* tape blocks", "")
            end
            
            if szmap[f] and szmap[f][file] and totalsize > 0 then
                extractedsz = extractedsz + szmap[f][file]
            end
            
            luaout("Extracting file: " .. file .. "\n", extractedsz / totalsize * 100)
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


-- Private functions
-----------------------------

-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)

function GenerateDefaultScreens()
    package.path = "?.lua"
    package.cpath = ""
    LangScreen = require "langscreen"
    OLDG.WelcomeScreen = require "welcomescreen"
    OLDG.LicenseScreen = require "licensescreen"
    OLDG.SelectDirScreen = require "selectdirscreen"
    OLDG.PackageDirScreen = require "packagedirscreen"
    OLDG.PackageToggleScreen = require "packagetogglescreen"
    OLDG.InstallScreen = require "installscreen"
    OLDG.SummaryScreen = require "summaryscreen"
    OLDG.FinishScreen = require "finishscreen"
    OLDG.install.screenlist = { WelcomeScreen, LicenseScreen, SelectDirScreen, InstallScreen, FinishScreen }
end

function LoadConfig()
    local file = "config/run.lua"
    
    if (os.fileexists(file)) then
        dofile(file)
        
        if (Init) then
            Init()
        end
    end
end

function AddScreens()
    install.addscreen(LangScreen)
    
    if (install.screenlist ~= nil and #install.screenlist > 0) then
        for _, s in pairs(install.screenlist) do
            install.addscreen(s)
        end
    else
        install.addscreen(WelcomeScreen)
        install.addscreen(LicenseScreen)
        install.addscreen(SelectDirScreen)
        install.addscreen(InstallScreen)
        install.addscreen(FinishScreen)
    end
end

GenerateDefaultScreens()
LoadConfig()
AddScreens()

