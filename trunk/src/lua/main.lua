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

-- Called from main/main.cpp

-- Initialize globals
version = "0.5"
cfg = { unopts = { } }
pkg = { }

-- Default unattended startup options
-- cfg.unopts["verbose"] = { short = "v", desc = "Enable verbose output" }

-- Functions for easy way to enable common options
-- The internal variable is used to check if opts were defined here and
-- should be handled by the user or automaticly

function cfg.adddestunopt()
    cfg.unopts["destdir"] = {
        short = "d",
        desc = "Sets the destination directory. Make sure you have read/write access to this directory.",
        opttype = "string",
        optname = "d",
        internal = true }
end

function cfg.addlicenseunopts()
    cfg.unopts["accept-license"] = {
        short = "l",
        desc = "Accepts the software's license agreement.",
        internal = true }
    cfg.unopts["show-license"] = {
        desc = "Shows the software's license agreement.",
        internal = true }
end
