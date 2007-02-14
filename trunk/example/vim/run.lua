function Init()
    destdir = install.gettempdir()
    
    cfgscreen = install.newcfgscreen("Configuration options for Vim installation")
    
    prefixfield = cfgscreen:adddirselector("Base destination directory for Vim. This directory will be " ..
                                           "used to populate subdirectories such as bin/ to store executables " ..
                                           "and etc for configuration files.", "/usr")
    
    scriptbox = cfgscreen:addcheckbox("Vim can use several scripting languages for command interpreting. "..
                                      "Please note that the selected languages need to be installed in " ..
                                      "order to work.", { "MzScheme", "perl", "python", "ruby", "tcl" })
    
    guibox = cfgscreen:addcheckbox("Vim can use several different GUI frontends. You can specify here which " ..
                                   "toolkits can be used to find a suitable GUI. If the toolkit cannot be used " ..
                                   "it will be skipped and the next is checked.", { "GTK", "GTK2", "Gnome", "Motif", "Athena (XAW)", "nexTaw" })
    guibox:set(1, 2, 3, 4, 5, 6, true)
    
    idebox = cfgscreen:addcheckbox("Vim can be integrated in several Integrated Development Environments (IDEs). " ..
                                   "Here you can select for which you want to include support.",
                                   { "Sun Visual Workshop", "NetBeans", "Sniff Interface" })
    idebox:set(2, true)
    
    advmenu = cfgscreen:addcfgmenu("Advanced compiler options. If you don't know what these do just leave them blank.")
    advmenu:addstring("CC", "Used C compiler")
    advmenu:addstring("CFLAGS", "Custom compiler flags (ie -O2)")
    advmenu:addstring("CPPFLAGS", "Custom preprocessor flags (ie -I/usr/local/include)")
    advmenu:addstring("LDFLAGS", "Custom linker flags (ie -L/usr/local/lib)")
    
    ScreenList = { WelcomeScreen, cfgscreen, InstallScreen, FinishScreen }
end

function getprefixconf()
    return string.format("--prefix=%s", prefixfield:get())
end

function getscriptconf()
    local ret = "" -- Need to initialize variable as a string, otherwise concatting it won't work
    
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

function getguiconf()
    local ret = ""
    
    if (not guibox:get("GTK")) then
        ret = ret .. "--disable-gtk-check "
    end
    if (not guibox:get("GTK2")) then
        ret = ret .. "--disable-gtk2-check "
    end
    if (not guibox:get("Gnome")) then
        ret = ret .. "--disable-gnome-check "
    end
    if (not guibox:get("Athena (XAW)")) then
        ret = ret .. "--disable-athena-check "
    end
    if (not guibox:get("nexTaw")) then
        ret = ret .. "--disable-nextaw-check "
    end

    return ret
end

function getideconf()
    local ret = ""
    
    if (idebox:get("Sun Visual Workshop")) then
        ret = ret .. "--enable-workshop "
    end
    if (not idebox:get("NetBeans")) then
        ret = ret .. "--disable-netbeans "
    end
    if (idebox:get("Sniff Interface")) then
        ret = ret .. "--enable-sniff "
    end
    
    return ret
end

function handleadvmenu()
    if (#advmenu:get("CC") > 0) then
        os.setenv("CC", advmenu:get("CC"))
    end
    if (#advmenu:get("CFLAGS") > 0) then
        os.setenv("CFLAGS", advmenu:get("CFLAGS"))
    end
    if (#advmenu:get("CPPFLAGS") > 0) then
        os.setenv("CPPFLAGS", advmenu:get("CPPFLAGS"))
    end
    if (#advmenu:get("LDFLAGS") > 0) then
        os.setenv("LDFLAGS", advmenu:get("LDFLAGS"))
    end
end

function getconfigureopts()
    return getprefixconf() .. " " .. getscriptconf() .. " " .. getguiconf() .. " " .. getideconf()
end

function Install()
    local prefix = prefixfield:get()
    handleadvmenu()
    
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
    
    install.setstatus("Configuring Vim for this system")
    install.execute(string.format("./configure %s", getconfigureopts()))
    
    install.setstatus("Compiling Vim")
    install.execute("make")
    
    install.setstatus("Installing files")
    if (os.writeperm(prefix) == false) then
        install.executeasroot("make install")
    else
        install.execute("make install")
    end
end
