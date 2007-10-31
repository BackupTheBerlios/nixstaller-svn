local OLDG = _G
module (..., package.seeall)

function getpkgpath()
    return "/usr/local" -- Seems like a good default prefix on many systems
end

function create(src)
end

function install(src)
    -- UNDONE: Check for conflicting files/directories
    dest = string.format("%s/%s/", getpkgpath(), pkgprefix)
    OLDG.install.executeasroot(string.format("mkdir -p %s/ && cp -R %s/files/* %s/ && cp -R %s/bins/* %s/bin", dest, src, dest, src, getpkgpath()))
end
