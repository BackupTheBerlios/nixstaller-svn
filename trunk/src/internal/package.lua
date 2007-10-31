-- Prefix in <prefix> directory (ie in /usr/local)
pkgprefix = string.format("share/%s", pkg.name)


-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)


function genscript(file, dir)
    -- UNDONE: Check what other vars should be set (GTK, Qt, KDE etc)

    local fullpkgpath = string.format("%s/%s", packager.getpkgpath(), pkgprefix)
    local filename = string.format("%s/%s", dir, string.gsub(file, "/*.+/", ""))
    
    script = io.open(filename, "w")
    script:write(string.format([[
#!/bin/sh
LD_LIBRARY_PATH="%s/files/lib:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH
exec %s/%s
]], fullpkgpath, fullpkgpath, file))
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
    local dir = curdir .. "/pkg"
    
    os.mkdirrec(dir .. "/bins")
    if pkg.bins then
        install.print("Generating executable scripts\n")
        for _, f in ipairs(pkg.bins) do
            genscript(f, dir .. "/bins")
        end
    end

    install.setstatus("Installing package")
    install.print("Generating system native package\n")
    packager.create(dir)
    install.print("Installing system native package\n")
    packager.install(dir)
end
