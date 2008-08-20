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


function pkg.verifydeps(bins)
    local installeddeps = { }
    local wrongdeps, wronglibs

    local success, msg = pcall(function ()
        install.showdepscreen(function ()
            wrongdeps, wronglibs = { }, { }
            
            gui.newprogressdialog(function(self)
                initprogress(bins)
                
                local needs = checkdeps(bins, install.getpkgdir(), pkg.deps, self, wrongdeps, wronglibs)
                
                instdeps(needs, installeddeps, wrongdeps, self)
            end)
            
            local ret = { }
            for k, v in pairs(wrongdeps) do
                ret[k.name] = { }
                ret[k.name].desc = k.description
                if v.failed then
                    ret[k.name].problem = "Failed to install."
                elseif v.faileddl then
                    ret[k.name].problem = "Failed to download."
                elseif v.incompatdep then
                    ret[k.name].problem = "(Binary) incompatible."
                end
            end
            
            resetfaileddl(wrongdeps)
            
            for k, v in pairs(wronglibs) do
                ret[k] = { }
                ret[k].desc = ""
                if v.missinglib then
                    ret[k].problem = "File missing."
                elseif v.incompatlib then
                    ret[k].problem = "(Binary) incompatible."
                end
            end

            return ret
        end)
    end)
    
    if not success then
        if type(msg) == "table" and msg[2] == "!check" then
            msg = "Dependency handling failed: " .. msg[1]
        end
        error(msg, 0) -- Rethrow (use level '0' to avoid adding extra info to msg)
    end
    
    if not utils.emptytable(wrongdeps) or not utils.emptytable(wronglibs) then
        return false, wrongdeps, wronglibs
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
