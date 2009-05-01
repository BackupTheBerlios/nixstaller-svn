--  ***************************************************************************
--  *   Copyright (C) 2009 by Rick Helmus / InternetNightmare                 *
--  *   rhelmus_AT_gmail.com / internetnightmare_AT_gmail.com                 *
--  *                                                                         *
--  *   This program is free software; you can redistribute it and/or modify  *
--  *   it under the terms of the GNU General Public License as published by  *
--  *   the Free Software Foundation; either version 2 of the License, or     *
--  *   (at your option) any later version.                                   *
--  *                                                                         *
--  *   This program is distributed in the hope that it will be useful,       *
--  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
--  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
--  *   GNU General Public License for more details.                          *
--  *                                                                         *
--  *   You should have received a copy of the GNU General Public License     *
--  *   along with this program; if not, write to the                         *
--  *   Free Software Foundation, Inc.,                                       *
--  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
--  ***************************************************************************

-- UNDONE: Move this to shared and let it use by genprojdir.sh?

local function getDestLuaCode(destdir, unattended)
    local ret
    
    if destdir == "package" then
        ret = [[

    -- The files that need to be packaged should be placed inside the
    -- directory returned from 'install.getpkgdir()'. By setting
    -- install.destdir to this path, the packed installation files
    -- are directly extracted, (re)packaged and installed to the
    -- user's system. If you need to compile the software on the
    -- user's system, you probably need to extract the files
    -- somewhere else and place any compiled files inside the
    -- 'temporary package directory' later.
    install.destdir = install.getpkgdir()
]]
    else
        local d
        if destdir == "home" then
            d = "os.getenv(\"HOME\")"
        elseif destdir == "temporary" then
            d = "install.gettempdir()"
        else -- destdir is an absolute path
            d = "\"" .. destdir .. "\""
        end

        if unattended then
            ret = string.format([[
    -- Set install.destdir to a default in case the user did not gave
    -- the --destdir option (remove this if cfg.adddestunopt() is not
    -- called in config.lua!).
    if not cfg.unopts["destdir"].value then
        install.destdir = %s
    end
]], d)
        else
            ret = string.format([[

    -- The destination directory for the installation files. The
    -- 'SelectDirScreen' lets the user change this variable.
    install.destdir = %s
]], d)
        end
    end

    return ret
end

local function genInit(screenlist, custscreens, destdir)
    local ret = [[
function Init()
    -- This function is called as soon as an attended (default)
    -- installation is started.
]]

    ret = ret .. getDestLuaCode(destdir, false)
    
    -- Generate custom screens
    if custscreens then
        for _, screen in ipairs(custscreens) do
            ret = ret .. string.format([[

    %s = install.newscreen("%s")
]], screen.variable, screen.title)

            local function genfunc(f, s)
                ret = ret .. string.format([[

    function %s:%s()
        %s
    end
]], screen.variable, f, s or "")
            end

            if screen.genCanAct then
                genfunc("canactivate", "return true")
            end
            if screen.genAct then
                genfunc("activate")
            end
            if screen.genUpdate then
                genfunc("update")
            end

            for _, widget in ipairs(screen.widgets) do
                ret = ret .. string.format([[

    %s = %s:%s(%s)
]], widget.variable, screen.variable, widget.func, tabtostr(widget.args) or "")
            end
        end
    end

    ret = ret .. string.format([[

    install.screenlist = { %s }]], tabtostr(screenlist))

    ret = ret .. "\nend\n\n"
    
    return ret
end

local function genUnattInit(destdir)
    return string.format([[
function UnattendedInit()
    -- This function is called as soon as an unattended
    -- installation is started.

%s
end

]], getDestLuaCode(destdir, true))
end

local function genInstall(pkgmode, deskentries)
    local content, deskcontent = nil, ""

    if not utils.emptytable(deskentries) then
        for _, v in ipairs(deskentries) do
            deskcontent = deskcontent .. string.format([[
    install.menuentries["%s"] = install.newdesktopentry("%s", %s, "%s")

]], v.name, v.exec, v.icon, v.categories)
        end

        deskcontent = deskcontent .. string.format([[
    -- This function will install all desktop entries from
    -- install.menuentries. The first argument should be true when
    -- the entries need to be installed globally.
    install.gendesktopentries(%s)]], (pkgmode and "pkg.needroot()") or "not os.writeperm(install.destdir)")
    end

    if pkgmode then
        content = string.format([[
    -- How many 'steps' the installation has (used to divide the
    -- progressbar). Since install.extractfiles, pkg.verifydeps and
    -- install.generatepkg all 'use' one step, we start with 3.
    install.setstepcount(3)
    
    -- Check if we need root access. By asking the user here, he or
    -- she can decide to proceed or not before the actual installation
    -- begins.
    if pkg.needroot() then
        install.askrootpw()
    end
    
    -- This function extracts the files to 'install.destdir'.
    install.extractfiles()
    
    -- Verifies, downloads and merges any dependencies to the
    -- temporary package directory.
    pkg.verifydeps()

%s

    -- This function generates and installs the package.
    -- If you need to call 'install.gendesktopentries', do this
    -- before this function.
    install.generatepkg()]], deskcontent)
    else
        content = string.format([[
    -- This function extracts the files to 'install.destdir'.
    install.extractfiles()

%s]], deskcontent)
    end

    return string.format([[
function Install()
    -- This function is called as soon as the installation starts
    -- (for attended installs, when the 'InstallScreen' is shown).

%s
end

]], content)
end

local function genFinish()
    return [[
function Finish(err)
    -- This function is called when the installer exits. The variable
    -- 'err' is a bool, that is true when an error occured. Common
    -- usages for this function are clearing temporary files and
    -- executing a final command (when err is false).
end

]]
end

function genRun(mode, pkgmode, screenlist, custscreens, destdir, deskentries)
    local ret = [[
-- This file is called when the installer is run.
-- Don't place any (initialization) code outside the Init(),
-- UnattendedInit(), Install() or Finish() functions.

]]
    if mode == "attended" or mode == "both" then
        ret = ret .. genInit(screenlist, custscreens, destdir)
    end

    if mode == "unattended" or mode == "both" then
        ret = ret .. genUnattInit(destdir)
    end

    ret = ret .. genInstall(pkgmode, deskentries)

    ret = ret .. genFinish()
    
    return ret
end
