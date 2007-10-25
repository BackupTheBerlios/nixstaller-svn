-- Prefix in <prefix> directory (ie in /usr/local)
pkgprefix = string.format("share/%s", pkg.name)


-- 'secure' our environment by creating a new one. This is mainly for dofile not corrupting our own env.
OLDG = getfenv(1)
local P = {}
setmetatable(P, {__index = _G})
setfenv(1, P)


function genscript(file)
    -- UNDONE: Check what other vars should be set (GTK, Qt, KDE etc)

    local fullpkgpath = string.format("%s/%s", packager.getpkgpath(), pkgprefix)
    local filename = string.format("%s/%s", fullpkgpath, string.gsub(file, "/*.+/", "")) -- gsub: strip path
    
    install.executeasroot(string.format([[
cat > %s << EOF
#!/bin/sh
LD_LIBRARY_PATH="%s/lib:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH
exec %s/files/%s
EOF
]], filename, fullpkgpath, fullpkgpath, file))
    install.executeasroot("chmod 0755 " .. filename)
end


package.path = "?.lua"
package.cpath = ""
require "generic"
require "deb"
--require "rpm"

-- Check which package system user has
packager = (deb.present() and deb) or generic

-- Called from Install()
function install.generatepkg() -- Assume fixed dir
    local dir = install.gettempdir()
    install.setstatus("Installing package")
    install.print("Generating system native package")
    packager.create(dir)
    install.print("Installing system native package")
    packager.install(dir)
    
    if pkg.bins then
        install.print("Generating executable scripts")
        for _, f in ipairs(pkg.bins) do
            genscript(f)
        end
    end
end
