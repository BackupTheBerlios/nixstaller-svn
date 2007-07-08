module (..., package.seeall)

screen = install.newscreen("Heh")

progressbar = screen:addgroup():addprogressbar("Progress")
statlabel = screen:addgroup():addlabel("")
output = screen:addgroup():addtextfield()

function wrapper(class, func)
    local c = class
    local f = func
    return function (...)
        f(c, ...)
    end
end

function screen:activate()
    install.startinstall(wrapper(statlabel, statlabel.set), wrapper(progressbar, progressbar.set), wrapper(output, output.add))
end

return screen
