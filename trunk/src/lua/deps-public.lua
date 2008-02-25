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
        gui.newprogressdialog({"Extracting dependencies", "Checking dependencies", "Installing dependencies", "Verifying binary compatibility", "Overiding incompatible native dependencies", "Handling incompatible dependencies"}, function(self)
            local function wait() for n=1,600000 do install.updateui() end end
            
            extractdeps(self)
            
            wait()
            self:nextstep()
            local needs = checkdeps(bins, install.getpkgdir(), pkg.deps)
            
            wait()
            self:nextstep()
            local function instdeps(deps, incompat)
                for d, rd in pairs(deps) do
                    if type(rd) == "table" and not utils.emptytable(rd) then
                        instdeps(rd, incompat)
                    end
                    if not installeddeps[d] and not faileddeps[d] then
                        -- Check if dep is usable
                        if incompat then
                            if not d.HandleCompat or not d:HandleCompat() then
                                faileddeps[d] = true
                                install.print(string.format("Failed dependency: %s\n", d.name))
                            else
                                installeddeps[d] = true
                                install.print(string.format("Installed dependency: %s\n", d.name))
                            end
                        elseif d.CanInstall and not d:CanInstall() then
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
            
            wait()
            self:nextstep()
            
            local overridedeps, incompatdeps, incompatlibs = { }, { }, { }
            
            local function checkcompat()
                local checkfiles = bins
                
                while #checkfiles > 0 do
                    local b = table.remove(checkfiles)
                    
                    local map = maplibs(b, { install.getpkgdir("lib") })
                    
                    for l, p in pairs(map) do
                        if p and os.fileexists(p) then
                            table.insert(checkfiles, p)
                        else
                            install.print(string.format("WARNING: Missing library dependency: %s\n", l))
                        end
                    end
                    
                    local i, od, id, il = checkelf(b)
                    if i then
                        utils.tablemerge(overridedeps, od)
                        utils.tablemerge(incompatdeps, id)
                        utils.tablemerge(incompatlibs, il)
                    end
                end
            end
            
            wait()
            self:nextstep()
            
            checkcompat()
            
            if not utils.emptytable(overridedeps) then
                installeddeps = { }
                instdeps(overridedeps)
                overridedeps = { }
                checkcompat()
                assert(utils.emptytable(overridedeps))
            end
    
            wait()
            self:nextstep()
            installeddeps = { }
            instdeps(incompatdeps, true)
            
            print("overridedeps:")
            table.foreach(overridedeps, print)
            print("incompatdeps:")
            table.foreach(incompatdeps, print)
            print("incompatlibs:")
            table.foreach(incompatlibs, print)
            print("faileddeps:")
            table.foreach(faileddeps, print)
        end)
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
