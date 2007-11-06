local OLDG = _G
module (..., package.seeall)

function getpkgpath()
    return "/usr/local" -- UNDONE?
end

function present()
    return os.execute("(rpm --version && rpmbuild --version) >/dev/null 2>&1") == 0
end

function create(src)
    local specdir = src .. "/SPECS"
    check(os.mkdir(specdir))
    check(os.mkdir(src .. "/RPMS"))
    
    spec = check(io.open(specdir .. "/pkg.spec", "w"))
    -- UNDONE: Sepcifying Group?
    check(spec:write(string.format([[
%%define _topdir %s
%%define _tmppath %s/tmp

Prefix: %s/files
Prefix: %s/bins
Name: %s
Version: %s
Release: %s
Packager: %s
Summary: %s
Group: %s
License: %s

%%description
%s

%%files
%s/files/*
%s/bins/*
]], src, src, src, src, pkg.name, pkg.version, pkg.release, pkg.maintainer, pkg.summary, "Development/Tools", pkg.license, pkg.description, src, src)))
    spec:close()
    checkcmd(OLDG.install.execute, string.format("rpmbuild -bb %s/pkg.spec", specdir))
end

function install(src)
    local locked = OLDG.install.executeasroot("lsof -au 0 +d /var/lib/rpm >/dev/null")
    while locked == 0 do
        if gui.choicebox("Another program seems to be using the RPM database. \
                          Please close all applications that may use the database (ie Smart or YaST) \
                          and hit continue.", "Continue", "Abort") == 2 then
            os.exit(1)
        end
        locked = OLDG.install.executeasroot("lsof -au 0 +d /var/lib/rpm >/dev/null")
    end
    
    checkcmd(OLDG.install.executeasroot, string.format("rpm --relocate %s/files=%s/%s --relocate %s/bins=%s/bin -i %s/RPMS/%s/%s-%s-%s.%s.rpm", src, getpkgpath(), pkgprefix, src, getpkgpath(), src, os.arch, pkg.name, pkg.version, pkg.release, os.arch))
end

function rollback(src)
end
