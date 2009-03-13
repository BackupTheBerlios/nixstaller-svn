-- Automaticly generated on wo feb 11 14:19:40 CET 2009.
-- This file is called when the installer is run.
-- Don't place any (initialization) code outside the Init(), UnattendedInit() or Install() functions.

function Init()
    -- This function is called as soon as an attended (default) installation is started.
    
    -- The destination directory for the installation files. The 'SelectDirScreen' lets the user
    -- change this variable.
    install.destdir = os.getenv("HOME")
    
    -- Installation screens to show (in given order). Custom screens should be placed here.
    install.screenlist = { WelcomeScreen, LicenseScreen, SelectDirScreen, InstallScreen, FinishScreen }
end

function Install()
    -- This function is called as soon as the installation starts (for attended installs,
    -- when the 'InstallScreen' is shown).
    
    -- This function extracts the files to 'install.destdir'.
    install.extractfiles()
end

function Finish(err)
    -- This function is called when the installer exits. The variable 'err' is a bool,
    -- that is true when an error occured. Common usages for this function are clearing temporary
    -- files and executing a final command (when err is false).
end

