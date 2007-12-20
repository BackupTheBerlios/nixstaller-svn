--     Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)
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

local OLDG = _G
module (..., package.seeall)

function getpkgpath()
    return "/usr/local" -- Seems like a good default prefix on many systems
end

function present()
    return true
end

function missingtool()
end

function installedver()
    return nil
end

function canxdg()
    return true
end

function create(src)
    -- Generate uninstall script
    local scriptname = string.format("%s/uninstall-%s", src .. "/bins", pkg.name)
    local script = check(io.open(scriptname, "w"))
    check(script:write(string.format([[
#!/bin/sh
PROGDIR="%s"
BINDIR="%s"
PROGNAME="%s"
PROGVER="%s"

echo "This script will remove $PROGNAME version $PROGVER from this system."
printf "Continue? (y/N) "
read yn
if [ "$yn" != "y" -a "$yn" != "Y" ]; then
    exit 1
fi

if [ ! -w "$PROGDIR" -o ! -w "$BINDIR" ]; then
    echo "Please run this script as a user who has permissions to remove this software (ie. root)."
    exit 1
fi

MD5BIN=`which md5sum 2>/dev/null`
[ -z "$MD5BIN" -a ! -z "`which md5 2>/dev/null`" ] && MD5BIN="`which md5` -q"

if [ -z "$MD5BIN" ]; then
    echo "WARNING: Could not find a suitable binary ('md5' or 'md5sum') to calculate MD5 sums in PATH."
    echo "For this reason files cannot be checked for modifications (files may be changed if they were overwritten by another package for example)."
    printf "Do you want to continue or abort? (y/N) "
    read yn
    if [ "$yn" != "y" -a "$yn" != "Y" ]; then
        exit 1
    fi
fi
    
DIFFILES=

checkfile()
{
    if [ ! -z "$MD5BIN" ]; then
        SUM=`$MD5BIN "$1" | awk '{ print $1 }'`
        if [ $SUM != $2 ]; then
            DIFFILES="$DIFFILES\n$1"
        fi
    fi
}

echo "Verifying installed files..."
]], pkg.getdatadir(), pkg.getbindir(), cfg.appname, pkg.version)))

    local addcheck = function(path, prefix, file)
        local md5 = io.md5(path)
        if md5 then
            check(script:write(string.format("checkfile \"%s/%s\" %s\n", prefix, file, md5)))
        end
    end
    
    utils.recursivedir(src .. "/files", function(path, relpath)
        if not os.isdir(path) then
            addcheck(path, pkg.getdatadir(), relpath)
        end
    end)
    
    utils.recursivedir(src .. "/bins", function(path, relpath)
        if not os.isdir(path) and path ~= scriptname then
            addcheck(path, pkg.getbindir(), relpath)
        end
    end)

    check(script:write([[

if [ ! -z "$DIFFILES" ]; then
    echo "WARNING: Following files are modified after they were installed."
    echo
    printf "$DIFFILES"
    echo
    echo "A possible reason for any file modifications is that they were overwritten by another package."
    printf "Do you want to continue? (y/N) "
    read yn
    if [ "$yn" != "y" -a "$yn" != "Y" ]; then
        exit 1
    fi
fi

]]))

    local xdgscript = xdguninstscript()
    
    if script then
        script:write("\necho Uninstalling menu entries...\n")
        check(script:write(xdgscript))
        check(script:write("\n\n"))
    end

    local dirs = {}
    local rmfile = function(path, prefix, file)
        local f = prefix .. "/" .. file
        if os.isdir(path) then
            table.insert(dirs, f)
        else
            check(script:write(string.format("echo REMOVE: %s && rm -f \"%s\"\n", f, f)))
        end
    end
    
    utils.recursivedir(src .. "/files", function(path, relpath)
        rmfile(path, pkg.getdatadir(), relpath)
    end)
    
    utils.recursivedir(src .. "/bins", function(path, relpath)
        rmfile(path, pkg.getbindir(), relpath)
    end)

    table.insert(dirs, pkg.getdatadir())
    
    local countslash = function(s)
        local ret = 0
        for sl in string.gmatch(s, "/") do
            ret = ret + 1
        end
        return ret
    end
    
    -- Sort directories on their amount of slashes (= amount of subdirectories), in reversed order
    table.sort(dirs, function(k1, k2)
        return countslash(k1) > countslash(k2)
    end)
    
    for _, d in ipairs(dirs) do
        check(script:write(string.format("echo REMOVE DIRECTORY: %s && rmdir \"%s\"\n", d, d)))
    end
        
    check(script:write("echo Uninstallation of $PROGNAME done.\n"))
    
    script:close()
    os.chmod(scriptname, "755")
end

function install(src)
    -- UNDONE: Check for conflicting files/directories
    checkcmd(instcmd(), string.format("mkdir -p %s/ %s/ && cp -R %s/files/* %s/ && cp -R %s/bins/* %s", pkg.getdatadir(), pkg.bindir, src, pkg.getdatadir(), src, pkg.bindir))
end
