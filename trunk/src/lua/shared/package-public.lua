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

function pkg.addbinenv(var, val, bin)
    bin = bin or "All"
    pkg.binenv = pkg.binenv or {}
    pkg.binenv[var] = pkg.binenv[var] or {}
    pkg.binenv[var][bin] = val
end

function pkg.addbinopts(bin, opts)
    pkg.binopts = pkg.binopts or {}
    pkg.binopts[bin] = opts
end

function pkg.adddependency(name)
    pkg.deps = pkg.deps or { }
    table.insert(pkg.deps, name)
end

function pkg.newdependency()
    local ret = { }
    setmetatable(ret, depclass)
    
    ret.libs = { }
    ret.deps = { }
    ret.description = ""
    
    return ret
end
