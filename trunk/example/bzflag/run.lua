function Init()
    install.destdir = install.getpkgdir() -- Files can be installed right away
    install.screenlist = { WelcomeScreen, PackageToggleScreen, PackageDirScreen, InstallScreen, SummaryScreen }
end


function Install()
    install.setstepcount(4)

    if pkg.needroot() then
        install.askrootpw()
    end

    -- We need to let bzflag know where to find the data,
    -- this is accomplished by using the '-directory' commandline option
    pkg.addbinopts("bin/bzflag", string.format("-directory %s/bzflag", pkg.getdatadir()))

    install.menuentries["bzflag"] = install.newdesktopentry(pkg.getbindir("bin/bzflag"), pkg.getdatadir("bzflag/blue_icon.png"), "Game;ActionGame")

    install.extractfiles()
    pkg.verifydeps()
    install.gendesktopentries(pkg.needroot()) -- Assume global install if root access is necessary.
    install.generatepkg()
end