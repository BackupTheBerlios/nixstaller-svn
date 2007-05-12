module (..., package.seeall)

screen = install.newscreen("Welcome")

function screen:next()
    return true
end

return screen
