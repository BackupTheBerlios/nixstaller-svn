module (..., package.seeall)

screen = install.newscreen("Select destination directory")

dir = screen:adddirselector("", install.destdir or os.getenv("HOME"))

function dir:datachanged()
    install.destdir = dir:get()
    return true
end

function dir:verify()
    return install.verifydestdir()
end

return screen
