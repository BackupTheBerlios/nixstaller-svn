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


-- Default dependency templates. New submissions are very welcome...
-- 
-- NOTE: Template searching is order dependent!
-- 
-- Table structure fields:
-- name         Dependency name.
-- description  Dependency description.
-- check        Function that is used to determine if a library is part of this template.
-- libs         As an alternative to 'check', this field can be used to list libraries which
--              belong to this this template. It should contain a table (array) containing strings
--              of the library name, without any starting "lib" and ending ".so.<version> part".
--              So if you want to look for "libfoo.so.<version>" simply specify "foo".
-- full         Recommended usage. Default is true.
-- notes        Notes for this dependency.
-- 
-- New structures should be created with the newtemplate() function

pkg.deptemplates = { }

local function newtemplate(n, d, l, f, no, p)
    if f == nil then
        f = true
    end
    local t = { name = n, description = d, full = f, notes = no, post = p }
    if type(l) == "table" then
        t.libs = l
    else
        t.check = l
    end
    table.insert(pkg.deptemplates, t)
end


-- C library
newtemplate("libc", "C system library.", {"c"}, false, "Never create a full dependency for this library!")

-- C++ Library
newtemplate("libstdc++", "C++ system library.", {"stdc++"})

-- Core X Libraries (UNDONE)
newtemplate("X-core", "Core libraries from X.", {"X11"}, false)

-- ATK
newtemplate("ATK", "Accessibility toolkit.", {"atk-1.0"}, false)

-- GLib
newtemplate("GLib2", "Library of useful routines for C programming.", {"glib-2.0", "gio-2.0", "gmodule-2.0", "gobject-2.0", "gthread-2.0"}, false)

-- GTK2
newtemplate("GTK2", "Used to build graphical interfaces.", {"gtk-x11-2.0", "gdk-x11-2.0", "gdk_pixbuf-2.0", "gdk_pixbuf_xlib-2.0", "gthread-2.0"}, false, "",
function (dest, destdir, libmap, full, copy)
    -- If we're creating a full dependency and are copying files, we have to make
    -- sure that all modules and relevant configs are copied.
    if full and copy then
        -- Find out GTK lib path
        local gtklibpath
        for k, v in pairs(libmap) do
            if v and string.find(k, "libgtk") then
                gtklibpath = utils.dirname(v)
                break
            end
        end

        local gtksublibs
        if gtklibpath then
            local tmp = string.format("%s/gtk-2.0", utils.dirname(gtklibpath))
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
                            gtksublibs = lpath
                        end
                    end
                end
            end
        end
        
        local good = true
        local destmods = string.format("%s/%s/gtk", dest, destdir)
        if not gtksublibs then
            print("WARNING: Unable to locate GTK2 modules. Please copy them manually to " .. destmods)
            good = false
        else
            for _, d in ipairs{"immodules", "loaders", "printbackends"} do
                local stat, msg = utils.copyrec(gtksublibs .. "/" .. d, destmods .. "/" .. d)
                if not stat then
                    print(string.format("WARNING: No modules copied from %s (%s)", d, msg))
                    good = false
                end
            end
        end
        
        -- GTK2 (and co) query programs don't return any error code incase of failure. To see if
        -- the call was successfull, we simply check if anything was written to stderr.
        function runquery(cmd, output)
            local pipe = io.popen(string.format("%s 2>&1 >%s", cmd, output), "r")
            
            if not pipe then
                return false
            end
            
            local err = pipe:read() -- Read something, anything should indicate an error.
            pipe:close()
            return not err or #err == 0
        end
        
        local destetc = string.format("%s/%s/etc/gtk-2.0", dest, destdir)
        local stat, msg = os.mkdirrec(destetc)
        if not stat then
            print("WARNING: Failed to create destination directory for GTK2 configuration files: " .. msg)
            good = false
        else
            -- Generate gdk loaders file.
            local exec = "gdk-pixbuf-query-loaders"
            if os.arch == "x86_64" then
                -- Some OS'es suffix executables with '-64' to seperate 64 bit bins with their 32 bit counterparts.
                if os.execute(string.format("%s-64 >/dev/null 2>&1", exec)) == 0 then
                    exec = exec .. "-64"
                end
            end
            
            if not runquery(string.format("%s %s/loaders/*\\.so", exec, destmods), destetc .. "/gdk-pixbuf.loaders") then
                print("WARNING: Encountered errors during generation of 'gdk-pixbuf.loaders', please verify this file.")
                good = false
            end
            
            -- Generate GTK immodules file.
            exec = "gtk-query-immodules-2.0"
            if os.arch == "x86_64" then
                if os.execute(string.format("%s-64 >/dev/null 2>&1", exec)) == 0 then
                    exec = exec .. "-64"
                end
            end
            
            if not runquery(string.format("%s %s/immodules/*\\.so", exec, destmods), destetc .. "/gtk.immodules") then
                print("WARNING: Encountered errors during generation of 'gtk.immodules', please verify this file.")
                good = false
            end
        end
        
        if not good then
            -- UNDONE: Reference
            print("WARNING: Some errors occurred. You're strongly advised to fix them, see the manual for more information how to set up this dependency manually.")
        end
    end
end)

-- Pango
newtemplate("Pango", "System for Layout and Rendering of Internationalised Text.", {"pango-1.0", "pangocairo-1.0", "pangoft2-1.0", "pangox-1.0", "pangoxft-1.0"}, false, "",
function (dest, destdir, libmap, full, copy)
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
end)

-- Cairo
newtemplate("Cairo", "Vector Graphics Library with Cross-Device Output Support.", {"cairo"}, false)
