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

loadluash("utils.lua")
loadluash("utils-public.lua")
loadlua("shared/utils.lua")

strings = { }

function eatwhite(s)
    return string.gsub(string.gsub(s, "^[ \n\r\t]*", ""), "[ \n\r\t]*$", "")
end

function dumpluafile(path)
    local file = io.open(path, "r")
    if not file then
        print("WARNING: Failed to open file " .. path)
        return
    end
    
    local printed = false
    local functions = { "tr", "gui%.msgbox", "gui%.warnbox", "gui%.choicebox", "gui%.yesnobox", "abort" }
    for l in file:lines() do
        for _, func in ipairs(functions) do
            local pat = string.format("^(.*)%s%%(\"(.-)\"", func)
            local _, _, _, str = string.find(l, pat)
            if str then
                if not printed then
                    print(string.format("\nLua file %s:", path))
                    printed = true
                end
                print(string.format("%s (%s)", str, func))
                strings[str] = false
            end
        end
    end
    
    file:close()
end

function dumpcfile(path)
    local file = io.open(path, "r")
    if not file then
        print("WARNING: Failed to open file " .. path)
        return
    end
    
    local printed = false
    for l in file:lines() do
        local _, _, _, str = string.find(l, "^(.*)GetTranslation%(\"(.-)\"")
        if str then
            str = eatwhite(str)
            if not printed then
                print(string.format("\nC/C++ file %s:", path))
                printed = true
            end
            print(str)
            strings[str] = false
        end
    end
    
    file:close()
end

utils.recursivedir(internal.nixstdir .. "/src", function (path, relpath)
    if string.match(path, "%.lua$") then -- Lua file?
        dumpluafile(path)
    elseif string.match(path, "%.cpp$") then -- C++ file?
        dumpcfile(path)
    elseif string.match(path, "%.h$") then -- C header?
        dumpcfile(path)
    end
end)

local english = io.open(string.format("%s/lang/english/strings", internal.nixstdir), "r")
if not english then
    error("Failed to load English strings")
end

for l in english:lines() do
    if not string.find(l, "^#") then
        l = eatwhite(l)
        if strings[l] == false then
            strings[l] = true
        end
    end
end

for s, v in pairs(strings) do
    if not v then
        print("WARNING: string not found: " .. s)
    end
end

english:close()
