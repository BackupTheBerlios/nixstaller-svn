local OLDG = _G
module (..., package.seeall)

function moverec(dir, dest)
    for f in io.dir(dir) do
        print(string.format("Moving file %s from %s to %s", f, dir, dest))
        OLDG.install.execute(string.format("mv %s/%s %s/", dir, f, dest))
    end
end

function getpkgpath()
    return "/usr" -- UNDONE?
end

function present()
    return os.execute("dpkg --version 2>&1 >/dev/null") == 0
end

function create(src)
    -- Set up deb work directory
    local debdir = curdir .. "/deb"
    os.mkdirrec(debdir .. "/DEBIAN")

    -- Create directory structure
    fstruct = string.format("%s/%s/%s", debdir, getpkgpath(), pkgprefix)
    os.mkdirrec(fstruct)
    moverec(src, fstruct)

    -- Generate control file
    local control = io.open(debdir .. "/DEBIAN/control", "w")
    control:write(string.format([[
Package: %s
Version: %s
Maintainer: %s
Description: %s
]], pkg.name, pkg.version, pkg.maintainer, pkg.description))
    control:close() -- important, otherwise data may still be buffered and not in the file

    -- Create the package
    OLDG.install.executeasroot(string.format("dpkg -b %s/ %s/pkg.deb", debdir, curdir))
end

function install(src)
    OLDG.install.executeasroot(string.format("dpkg -i %s/pkg.deb", curdir))
    -- Move install files back
    moverec(fstruct, src)
end
