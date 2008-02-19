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


function pkg.newdependency()
    local ret = { }
    setmetatable(ret, depclass)
    
    ret.libs = { }
    ret.deps = { }
    
    return ret
end

function pkg.verifydeps(bins)
    local installeddeps = { }
    local faileddeps = { }

    local success, msg = pcall(function ()
        install.print("Extracting dependencies\n")
        extractdeps()
        
        install.print("Checking dependencies\n")
        local needs = checkdeps(bins, install.getpkgdir(), pkg.deps)
        
        install.print("Installing dependencies\n")
        local function instdeps(deps)
            for d, rd in pairs(deps) do
                if #rd > 0 then
                    instdeps(rd)
                end
                if not installeddeps[d] and not faileddeps[d] then
                    -- Check if dep is usable
                    if d.CanInstall and not d:CanInstall() then
                        faileddeps[d] = true
                        install.print(string.format("Failed dependency: %s\n", d.name))
                    else
                        -- UNDONE
        --                 if d.needsdownload then
        --                     download(d)
        --                 end
    
                        d:Install()
                        installeddeps[d] = true
                        install.print(string.format("Installed dependency: %s\n", d.name))
                    end
                end
            end
        end
        
        instdeps(needs)
    end)
    
    if not success then
        if type(msg) == "table" and msg[2] == "!check" then
            msg = "Dependency handling failed: " .. msg[1]
        end
        error(msg, 0) -- Rethrow (use level '0' to avoid adding extra info to msg)
    end
    
    if not utils.emptytable(faileddeps) then
        return false, faileddeps
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
