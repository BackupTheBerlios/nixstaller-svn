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
        install.showdepscreen(function ()
            local overridedeps, incompatdeps, incompatlibs = { }, { }, { }
            faileddeps = { }
            
            gui.newprogressdialog({"Checking dependencies", "Installing dependencies", "Verifying binary compatibility", "Overiding incompatible native dependencies", "Handling incompatible dependencies"}, function(self)
                local function wait() for n=1,600000 do install.updateui() end end
                
                local needs, fails = checkdeps(bins, install.getpkgdir(), pkg.deps, self)
                utils.tablemerge(faileddeps, fails)
                
                wait()
                self:nextstep()
                instdeps(needs, false, installeddeps, faileddeps, self)
                
                wait()
                self:nextstep()
                
                wait()
                self:nextstep()
                checkcompat(bins, overridedeps, incompatdeps, incompatlibs)
                
                if not utils.emptytable(overridedeps) then
                    utils.tableunmerge(installeddeps, overridedeps)
                    needs = gatherdepdeps(overridedeps, self, faileddeps)
                    utils.tablemerge(overridedeps, needs)
                    instdeps(overridedeps, false, installeddeps, faileddeps, self)
                    overridedeps = { }
                    checkcompat(bins, overridedeps, incompatdeps, incompatlibs)
                    assert(utils.emptytable(overridedeps))
                end
        
                wait()
                self:nextstep()
                
                utils.tableunmerge(installeddeps, incompatdeps)
                instdeps(incompatdeps, true, installeddeps, faileddeps, self)
                
                print("overridedeps:")
                table.foreach(overridedeps, print)
                print("incompatdeps:")
                table.foreach(incompatdeps, print)
                print("incompatlibs:")
                table.foreach(incompatlibs, print)
                print("faileddeps:")
                table.foreach(faileddeps, print)
            end)
            
            local ret = { }
            for k, v in pairs(incompatdeps) do
                ret[k.name] = ret[k.name] or { }
                ret[k.name].desc = "UNDONE"
                ret[k.name].problem = "Incompatible"
            end
            for k, v in pairs(incompatlibs) do
                ret[k.name] = ret[k.name] or { }
                ret[k.name].desc = ""
                ret[k.name].problem = "File missing"
            end
            for k, v in pairs(faileddeps) do
                ret[k.name] = ret[k.name] or { }
                ret[k.name].desc = "UNDONE"
                ret[k.name].problem = "Failed to install"
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
