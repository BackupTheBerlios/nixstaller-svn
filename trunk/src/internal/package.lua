-- Prefix in <prefix> directory (ie in /usr/local)
pkgprefix = string.format("share/%s", pkg.name)

function check(g, msg, ...)
    if not g then
        -- Special string is used to identify errors thrown by this function.
        -- This is so install.generatepkg() can filter out other errors.
        error({msg, "!check"})
    end
    return g, msg, ...
end

-- f is a function that should execute the command and excepts a second argument
-- for not complaining when something went wrong
function checkcmd(f, cmd)
    if f(cmd, false) ~= 0 then
        error({"Failed to execute package shell command", "!check"})
    end
end

-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)


function genscript(file, dir)
    -- UNDONE: Check what other vars should be set (GTK, Qt, KDE etc)

    local fullpkgpath = string.format("%s/%s", packager.getpkgpath(), pkgprefix)
    local filename = string.format("%s/%s", dir, string.gsub(file, "/*.+/", ""))
    
    script = check(io.open(filename, "w"))
    check(script:write(string.format([[
#!/bin/sh
LD_LIBRARY_PATH="%s/files/lib:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH
exec %s/%s
]], fullpkgpath, fullpkgpath, file)))
    script:close() -- Flush
    os.chmod(filename, "0755")
end


package.path = "?.lua"
package.cpath = ""
require "generic"
require "deb"
require "rpm"

-- Check which package system user has
packager = (deb.present() and deb) or (rpm.present() and rpm) or generic

-- Called from Install()
function install.generatepkg()
    local success, msg = pcall(function ()
        local dir = curdir .. "/pkg"
        
        check(os.mkdirrec(dir .. "/bins"))
        if pkg.bins then
            install.print("Generating executable scripts\n")
            for _, f in ipairs(pkg.bins) do
                genscript(f, dir .. "/bins")
            end
        end
    
        install.setstatus("Installing package")
        
        local f = function()
            packager.create(dir)
            packager.install(dir)
        end

        if packager == generic then
            f()
        else
            local g, msg2 = pcall(f)
            if not g then
                if type(msg2) == "table" and msg2[2] == "!check" then
                    packager.rollback(dir)
                    packager = generic
                    install.print("WARNING: Failed to create system native package.\nReverting to generic package.")
                    f()
                else
                    error((type(msg2) == "table" and msg2[1]) or msg2, 0) -- Rethrow
                end
            end
        end
    end)
    
    if not success then
        if type(msg) == "table" then
            msg = "Failed to create package: " .. msg[1]
        end
        error(msg, 0) -- Rethrow (use level '0' to avoid adding extra info to msg)
    end
end
