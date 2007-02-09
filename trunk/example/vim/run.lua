destdir = install.gettempdir()

cfgscreen = install.newcfgscreen("Configuration options for VIM installation")

prefixfield = cfgscreen:adddirselector("Base destination directory for VIM.", "/usr")

scriptbox = cfgscreen:addcheckbox("VIM can use several scripting languages for command interpreting. Please note that the selected languages need to be installed in order to work", { "MzScheme", "perl", "python", "ruby", "tcl" })

screenlist = { WelcomeScreen, cfgscreen, InstallScreen, FinishScreen }

function getconfigureopts()
    local ret = string.format("--prefix=%s ", prefixfield:get())
    
    if (scriptbox:get("MzScheme")) then
        ret = ret .. "--enable-MzScheme "
    end
    if (scriptbox:get("perl")) then
        ret = ret .. "--enable-perl "
    end
    if (scriptbox:get("python")) then
        ret = ret .. "--enable-python "
    end
    if (scriptbox:get("ruby")) then
        ret = ret .. "--enable-ruby "
    end
    if (scriptbox:get("tcl")) then
        ret = ret .. "--enable-tcl "
    end
    
    return ret
end

getconfigureopts()

function Install()
    gui.warnbox("opts: ", getconfigureopts())
    local prefix = prefixfield:get()
    
    -- 1: Extracting Files (Automaticly set by install.extractfiles)
    -- 2: ./configure
    -- 3: make
    -- 4: make install
    install.setstepcount(4)
    
    if (os.writeperm(prefix) == false) then
        install.askrootpw()
    end
    
    install.extractfiles()
    os.chdir("vim70/")
    
    install.setstatus("Configuring VIM for this system")
    install.execute(string.format("./configure %s", getconfigureopts()))
    
    install.setstatus("Compiling VIM")
    install.execute("make")
    
    install.setstatus("Installing files")
    if (os.writeperm(prefix) == false) then
        install.executeasroot("make install")
    else
        install.execute("make install")
    end
end
