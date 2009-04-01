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

-- Called when frontend is in preview mode (used by Nixstbuild)

loadluash("utils.lua")
loadlua("shared/utils.lua")
loadlua("shared/utils-public.lua")

install.langinfo = { }
OLDG = _G -- HACK

function setproxy(tab, allowed)
    local oldtab = tab
    tab = { }
    setmetatable(tab, { __index = function(t, k)
        if t == tab then -- Do we need this?
            if tab[k] then
                if true or type(tab[k]) == "function" then -- UNDONE
                    if utils.tablefind(allowed, k) then
                        return oldtab[k]
                    elseif tab[k] then -- Don't generate errors for blocked stuff
                        print("Blocked function/variable", k)
                        return function() end
                    end
                else
                    return tab[k] -- UNDONE?
                end
            else
                print("nil k:", k)
            end
        else
            print("other tab")
        end
    end})
end

setproxy(install, { "addscreen" })
setproxy(_G, { "install", "gui", "OLDG" })

loadconfig(install.configdir) -- UNDONE

install.destdir = os.getenv("HOME") or "/"
install.menuentries = { }

loadlua("attinstall.lua")
