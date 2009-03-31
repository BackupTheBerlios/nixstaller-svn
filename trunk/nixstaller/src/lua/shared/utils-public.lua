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

utils = {}

function utils.moverec(dir, dest)
    for f in io.dir(dir) do
        local ret = install.execute(string.format("mv \"%s/%s\" \"%s/\"", dir, f, dest))
        if ret ~= 0 then
            return ret
        end
    end
    return 0
end

function utils.copyrec(src, dest)
    local dirlist = { "." }
    local ret, msg
    
    while (#dirlist > 0) do
        local subdir = table.remove(dirlist) -- pop
        local srcpath = string.format("%s/%s/", src, subdir)
        local destpath = string.format("%s/%s/", dest, subdir)
        
        ret, msg = os.mkdirrec(destpath)
        if ret == nil then
            return nil, "Could not create subdirectory: " .. msg
        end

        if not os.fileexists(srcpath) then
            return nil, "Non existing source path: " .. srcpath
        end
        
        for f in io.dir(srcpath) do
            local fsrc = srcpath .. f
            local fdest = destpath .. f
            if os.isdir(fsrc) then
                table.insert(dirlist, subdir .. "/" .. f)
                ret, msg = os.mkdirrec(fdest)
                if ret == nil then
                    return nil, "Could not create subdirectory: " .. msg
                end
            else
                ret, msg = os.copy(fsrc, fdest)
                if ret == nil then
                    return nil, string.format("Error: could not copy file %s to %s: %s", src, dest, msg or "(No error message)")
                end
            end
        end
    end
    return true
end

function utils.recursivedir(src, func)
    local dirstack = { "." }
    
    while #dirstack > 0 do
        local dir = table.remove(dirstack)
        for f in io.dir(src .. "/" .. dir) do
            local relpath
            if dir ~= "." then
                relpath = string.format("%s/%s", dir, f)
            else
                relpath = f
            end
            
            local path = src .. "/" .. relpath
            if os.isdir(path) then
                table.insert(dirstack, relpath)
            end
            func(path, relpath)
        end
    end
end

function utils.removerec(path)
    if os.fileexists(path) then
        local newdirlist = { path }
        local olddirlist = { }
        
        while (#newdirlist > 0) do
            local d = newdirlist[#newdirlist]
            local newdir = false
    
            for f in io.dir(d) do
                local dpath = string.format("%s/%s", d, f)
                if os.isdir(dpath) then
                    if not utils.tablefind(olddirlist, dpath) then
                        table.insert(newdirlist, dpath)
                        newdir = true
                        break
                    end
                else
                    local ret, msg = os.remove(dpath)
                    if not ret then
                        return ret, msg
                    end
                end
            end
            
            if not newdir then
                local d = table.remove(newdirlist)
                table.insert(olddirlist, d)
                local ret, msg = os.remove(d)
                if not ret then
                    return ret, msg
                end
            end
        end
    end
    return true
end

function utils.basename(p)
    local ret = string.gsub(p, "/*.+/", "")
    return ret
end

function utils.dirname(p)
    local ret = string.match(p, "/*.*/")
    return ret
end

function utils.opendoc(f)
    local bin
    if os.execute("xdg-open --version >/dev/null 2>&1") == 0 then
        bin = "xdg-open"
    else
        bin = string.format("%s/xdg-utils/xdg-open", curdir)
    end
    
    os.execute(string.format("%s %s >/dev/null 2>&1", bin, f))
end

function utils.emptytable(t)
    return next(t) == nil
end

function utils.emptystring(s)
    return s == nil or #s == 0
end

function utils.tablefind(t, val)
    for i, v in ipairs(t) do
        if v == val then
            return i
        end
    end
end

function utils.tablemerge(t1, t2)
    for k, v in pairs(t2) do
        t1[k] = v
    end
end

function utils.tableunmerge(t1, t2)
    for k in pairs(t2) do
        t1[k] = nil
    end
end

function utils.mapsize(t)
    local ret = 0
    for k in pairs(t) do
        ret = ret + 1
    end
    return ret
end

function utils.tableappend(t, t2)
    for _, v in ipairs(t2) do
        table.insert(t, v)
    end
end

function utils.patch(file, pattern, newstr)
    local tmpf = file .. ".tmp"
    local stat, msg = os.rename(file, tmpf)
    if not stat then
        return nil, msg
    end
    
    local out = io.open(file, "w")
    if out then
        for l in io.lines(tmpf) do
            out:write(string.gsub(l, pattern, newstr), "\n")
        end
        out:close()
    else
        os.rename(tmpf, file)
        return nil, "Failed to open output file."
    end
    os.remove(tmpf)
    return true
end

function utils.mkdirperm(dir)
    local path = ""
    local ret = false
    for d in string.gmatch(dir, "/[^/]*") do
        path = path .. d
        if not os.fileexists(path) then
            break
        end
        ret = (os.writeperm(path) and os.readperm(path))
    end
    return ret
end
