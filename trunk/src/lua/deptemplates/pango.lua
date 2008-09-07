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

-- Pango
newtemplate{
name = "Pango",
description = "System for Layout and Rendering of Internationalised Text.",
libs = { "pango-1.0", "pangocairo-1.0", "pangoft2-1.0", "pangox-1.0", "pangoxft-1.0" },
full = false,

post = function (dest, destdir, libmap, full, copy)
    -- If we're creating a full dependency and are copying files, we have to make
    -- sure that all modules and relevant configs are copied.
    if full and copy then
        -- Find out Pango lib path
        local pangolibpath
        for k, v in pairs(libmap) do
            if v and string.find(k, "libpango") then
                pangolibpath = utils.dirname(v)
                break
            end
        end

        local pangosublibs, etcpath
        if pangolibpath then
            local tmp = string.format("%s/pango", utils.dirname(pangolibpath))
            if os.isdir(tmp) then
                -- Go to dir with highest version
                local hm, hs, hu = -1, -1, -1
                for d in io.dir(tmp) do
                    local lpath = tmp .. "/" .. d
                    if os.isdir(lpath) and string.find(d, "^%d+%.%d+%.%d+") then -- Match "x.y.z"
                        local it = string.gmatch(d, "%d+")
                        local m, s, u = tonumber(it()), tonumber(it()), tonumber(it())
                        if m > hm or (m == hm and s > hs) or (m == hm and s == hs and u > hu) then -- Higher version than current?
                            hm, hs, hu = m, s, u
                            pangosublibs = lpath
                        end
                    end
                end
            end
            
            tmp = string.format("%s/../etc/pango", utils.dirname(pangolibpath))
            if os.isdir(tmp) then
                etcpath = tmp
            end
        end
        
        if not etcpath and os.isdir("/etc/pango") then
            etcpath = "/etc/pango"
        end

        local good = true
        local destmods = string.format("%s/%s/pango", dest, destdir)
        if not pangosublibs then
            print("WARNING: Unable to locate Pango modules. Please copy them manually to " .. destmods)
            good = false
        else
            local stat, msg = utils.copyrec(pangosublibs .. "/modules", destmods)
            if not stat then
                print(string.format("WARNING: No pango modules copied (%s).", msg))
                good = false
            end
        end
        
        local destetc = string.format("%s/%s/etc/pango", dest, destdir)
        local stat, msg = os.mkdirrec(destetc)
        if not stat then
            print("WARNING: Failed to create destination directory for Pango configuration files: " .. msg)
            good = false
        else
            -- Generate Pango loaders file.
            local exec = "pango-querymodules"
            if os.arch == "x86_64" then
                -- Some OS'es suffix executables with '-64' to seperate 64 bit bins with their 32 bit counterparts.
                if os.execute(string.format("%s-64 >/dev/null 2>&1", exec)) == 0 then
                    exec = exec .. "-64"
                end
            end
            
            -- pango-querymodules doesn't return error status codes, simply check stderr for any errors.
            local pipe = io.popen(string.format("%s %s/*\\.so 2>&1 >%s/pango.modules", exec, destmods, destetc))
            local err
            if pipe then
                err = pipe:read()
                pipe:close()
            end
            
            if not pipe or (err and #err > 0) then
                print("WARNING: Encountered errors during generation of 'pango.modules', please verify this file.")
                good = false
            end
            
            -- Get pangox.aliases
            stat, msg = os.copy(etcpath .. "/pangox.aliases", destetc)
            if not stat then
                print("WARNING: Failed to copy 'pangox.aliases': " .. msg)
                good = false
            end
        end
        
        if not good then
            -- UNDONE: Reference
            print("WARNING: Some errors occurred. You're strongly advised to fix them, see the manual for more information how to set up this dependency manually.")
        end
    end
end,

install = [[
    local pattern = "^.+/pango/" -- match <path>/pango/
    utils.patch(pkg.getdepdir(self, "etc/pango/pango.modules"), pattern, pkg.getdatadir("pango/"))
    
    -- Generate pangorc
    local file = io.open(string.format("%s/etc/pango/pangorc", pkg.getdepdir(self)), "w")
    if file then
        file:write(string.format([=[
[Pango]
ModuleFiles=%s/etc/pango/pango.modules
[PangoX]
AliasFiles=%s/etc/pango/pangox.aliases
]=], pkg.getdatadir(), pkg.getdatadir()))
        file:close()
    end
    
    self:CopyFiles()
    pkg.addbinenv("PANGO_RC_FILE", pkg.getdatadir("etc/pango/pangorc"))]]
}
