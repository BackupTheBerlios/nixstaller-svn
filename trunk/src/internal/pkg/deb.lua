local OLDG = _G
module (..., package.seeall)

function getpkgpath()
    return "/usr" -- UNDONE?
end

function present()
    return os.execute("dpkg --version >/dev/null") == 0
end

function create(src)
    -- Set up deb work directory
    local pkgdir = curdir .. "/deb"
    os.mkdirrec(pkgdir .. "/DEBIAN")
    
    -- Generate control file
    local control = io.open(pkgdir .. "/DEBIAN/control", "w")
    control:write(string.format([[
Package: %s
Version: %s
Maintainer: %s
Description: %s
]], pkg.name, pkg.version, pkg.maintainer, pkg.description))
    control:close()

    -- Link or copy installation files to work directory
    OLDG.install.execute(string.format("ln -s %s %s/files || cp -R %s/ %s/files", src, pkgdir, src, pkgdir))
    
    -- Create the package
    OLDG.install.executeasroot(string.format("dpkg -b %s/ %s/pkg.deb", pkgdir, curdir))
end

function install(src)
    OLDG.install.executeasroot(string.format("dpkg -i %s/pkg.deb", curdir))
end