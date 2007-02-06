destdir = install.gettempdir()

cfgscreen = install.newcfgscreen("Configuration options for VIM installation")

prefixfield = cfgscreen:adddirselector("Base destination directory for VIM.", "/usr")

screenlist = { WelcomeScreen, cfgscreen, InstallScreen, FinishScreen }

function Install()
    local prefix = prefixfield:get() or "/usr"
    
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
    install.execute(string.format("./configure --prefix=%s", prefix))
    
    install.setstatus("Compiling VIM")
    install.execute("make")
    
    install.setstatus("Installing files")
    if (os.writeperm(prefix) == false) then
        install.executeasroot("make install")
    else
        install.execute("make install")
    end
end
