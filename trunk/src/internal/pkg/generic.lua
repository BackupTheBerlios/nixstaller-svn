local OLDG = _G
module (..., package.seeall)

function getpkgpath()
    return "/usr/local" -- Seems like a good default prefix on many systems
end

function create(src)
end

function install(src)
    dest = string.format("%s/%s/", getpkgpath(), pkgprefix)
    OLDG.install.executeasroot(string.format("mkdir -p %s/ && cp -R %s/* %s/", dest, src, dest))
end
