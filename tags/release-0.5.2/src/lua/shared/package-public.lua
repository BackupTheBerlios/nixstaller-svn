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

function pkg.newdependency()
    local ret = { }
    setmetatable(ret, depclass)
    
    -- Sync with LoadDep() from deputils.lua!
    ret.full = true
    ret.libs = { }
    ret.libdir = "lib/"
    ret.deps = { }
    ret.description = ""
    
    return ret
end

-- Function for easy way to enable common options
-- The internal variable is used to check if opts were defined here and
-- should be handled by the user or automaticly

function pkg.addpkgunopts(deps)
    cfg.unopts["overwrite"] = {
        short = "O",
        desc = "Install package, even if it overwrites any existing.",
        internal = true }
        
    cfg.unopts["datadir"] = {
        short = "d",
        desc = "Sets the data destination directory. The data files are installed inside a subdirectory inside this directory. Make sure you have read/write access to this directory.",
        opttype = "string",
        optname = "d",
        internal = true }
        
    cfg.unopts["bindir"] = {
        short = "b",
        desc = "Sets the destination directory for any executables. Make sure you have read/write access to this directory.",
        opttype = "string",
        optname = "d",
        internal = true }
    
    cfg.unopts["packager"] = {
        desc = "If the installer finds more than 1 package manager, use this to specify which one should be used. Run the installer without to see which are available.",
        opttype = "string",
        optname = "p",
        internal = true }

    if pkg.register then
        cfg.unopts["no-register"] = {
            short = "r",
            desc = "Disable package registration, therefore allowing non root installs.",
            internal = true }
    else
        cfg.unopts["register"] = {
            short = "r",
            desc = "Tries to register the package in the system's package manager. Works only when executed as root.",
            internal = true }
    end
    
    if deps then
        cfg.unopts["dlretries"] = {
            desc = "Amount of retries incase downloading a dependency fails. Default: 3.",
            opttype = "string",
            optname = "n",
            internal = true }
            
        cfg.unopts["ignore-failed-deps"] = {
            short = "D",
            desc = "Ignore any missing or faulty dependencies.",
            internal = true }
    end
end
