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


function pkg.verifydeps(bins, libs)
    install.setstatus("Verifying dependencies")
    
    bins = bins or pkg.bins
    libs = libs or pkg.libs
    
    local success, msg = pcall(function ()
        if install.unattended then
            local failed = verifydeps(bins, libs)
            if not utils.emptytable(failed) then -- Something went wrong?
                install.print(tr("One or more package dependencies could not be resolved. Details are given below."))
                install.print("\n\n")
                
                for n, d in pairs(failed) do
                    install.print(string.format([[
%s:
    %s:    %s
    %s:    %s
]], n, tr("Description"), tr(d.desc), tr("Problem"), tr(d.problem)))
                end
                
                install.print("\n")
                
                if cfg.unopts["ignore-failed-deps"] and cfg.unopts["ignore-failed-deps"].value and
                   cfg.unopts["ignore-failed-deps"].internal then
                    install.print("\n" .. tr("Continuing installation anyway...") .. "\n")
                else
                    install.print(tr("Please fix these issues now and rerun the installer.") .. "\n")
                    if cfg.unopts["ignore-failed-deps"] and cfg.unopts["ignore-failed-deps"].internal then
                        install.print(tr("Alternatively rerun with the --ignore-failed-deps options, risking any dependency errors when using the installed software.") .. "\n")
                    end
                    os.exit(1)
                end
            end
        else
            install.showdepscreen(function () return verifydeps(bins, libs) end)
        end
    end)
    
    if not success then
        if type(msg) == "table" and msg[2] == "!check" then
            msg = "Dependency handling failed: " .. msg[1]
        end
        error(msg, 0) -- Rethrow (use level '0' to avoid adding extra info to msg)
    end
    
    if not utils.emptytable(depprocess.wrongdeps) or not utils.emptytable(depprocess.wronglibs) then
        return false, depprocess.wrongdeps, depprocess.wronglibs
    end
    
    return true
end

function pkg.getdepdir(dep, file)
    for n, d in pairs(pkg.depmap) do
        if d == dep then
            return string.format("%s/deps/%s/files/%s", curdir, n, (file or ""))
        end
    end
end
