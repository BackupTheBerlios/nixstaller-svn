function Init()
    screen = install.newcfgscreen("Installation options")
    scriptbox = screen:addradiobutton("Install launcher script to your home directory that launches Cube?",
                                      { "Yes", "No" })
    
    install.screenlist = { WelcomeScreen, SelectDirScreen, screen, InstallScreen }
end

function Install()
    install.extractfiles()
    
    if (scriptbox:get() == "Yes") then
        local filename = string.format("%s/cube.sh", os.getenv("HOME"))
        script = io.open(filename, "w")
        
        if (not script) then
            install.print("Failed to create launcher script")
            return -- Exit from function
        end
        
        script:write(string.format(
[[#!/bin/sh
cd %s/cube/
./cube_unix
]], install.destdir))
        script:close()
        
        os.chmod(filename, 700)
    end
end
