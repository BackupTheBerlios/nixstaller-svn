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

module (..., package.seeall)

installing = false
function verifyinstalling()
    if not installing then
        error("Function called when install is not in progress.")
    end
end

screen = install.newscreen("Files are being installed")

progressbar = screen:addprogressbar("Progress")
statlabel = screen:addlabel("")
output = screen:addtextfield("", true)
output:follow(true)

stepcount = 1 -- Default 1 for extraction
curstep = 0
installprogress = 0


function screen:activate()
    installing = true
    
    install.lockscreen(false, true, true) -- Disable Back and Next buttons
    
    if gui.choicebox(tr("This will install %s\nContinue?", cfg.appname), "Exit program", "Continue") == 1 then
        os.exit(1)
    end
    
    if os.fileexists(install.destdir) then
        if not os.readperm(install.destdir) then
            gui.msgbox(tr([[
This installer will install files to the following directory:
%s
However you don't have read permissions to this directory
Please restart the installer as a user who does or as the root user]], install.destdir))
            os.exit(1)
        elseif not os.writeperm(install.destdir) then
            install.askrootpw()
        end
    else
        output:write("Creating destination directory...")
        
        if not utils.mkdirperm(install.destdir) then
            install.executecmdasroot("mkdir -p " .. install.destdir, function () end, true)
        else
            os.mkdirrec(install.destdir)
        end
    end
    
    os.chdir(install.destdir)
    
    if Install ~= nil then
        Install()
    else
        install.extractfiles() -- Default behaviour
    end
    
    progressbar:set(100)
    gui.msgbox(tr("Installation of %s complete!", cfg.appname))
    
    install.lockscreen(true, true, false) -- Re-enable Next button, lock Back and Cancel buttons
    installing = false
end

-- Handy wrapper functions
function install.execute(cmd, req, path)
    verifyinstalling()
    
    if not os.writeperm(install.destdir) then
        return install.executeasroot(cmd, req, path)
    end
    
    output:add(string.format("\n=============================\nExecute: %s\n\n", cmd))
    local ret = install.executecmd(cmd, function (s)
                                            output:add(s)
                                        end, req, path)
    output:add("=============================\n")
    return ret
end

function install.executeasroot(cmd, req, path)
    verifyinstalling()
    
    output:add(string.format("\n=============================\nExecute: %s\n\n", cmd))
    local ret = install.executecmdasroot(cmd, function (s)
                                                  output:add(s)
                                              end, req, path)
    output:add("=============================\n")
    return ret
end

function install.extractfiles()
    verifyinstalling()
    
    install.setstatus("Extracting files")

    install.extract(function(f, p)
                        output:add(f .. "\n")
                        progressbar:set(installprogress + (p / stepcount))
                    end)
end

function install.setstepcount(n)
    if stepcount < 1 then
        error("stepcount should be atleast 1")
    end
    stepcount = n
end

function install.setstatus(msg)
    verifyinstalling()
    
    if curstep > 0 then
        installprogress = installprogress + (1 / stepcount) * 100
        progressbar:set(installprogress)
    end
    
    curstep = curstep + 1
    
    if stepcount > 1 then
        statlabel:set(string.format("%s: %s (%d/%d)", tr("Status"), tr(msg), curstep, stepcount))
    else
        statlabel:set(string.format("%s: %s", tr("Status"), tr(msg)))
    end
end

function install.print(msg)
    verifyinstalling()
    output:add(msg)
end

install.setaskquit(function ()
    local msg
    if installing then
        msg = [[
Installation is currently in progress.
If you abort now this may lead to a broken installation.
Are you sure?]]
    else
        msg = "This will abort the installation\nAre you sure?"
    end
    
    return gui.yesnobox(msg)
end)

return screen
