module (..., package.seeall)

screen = install.newscreen("Heh")

-- progressbar = screen:addgroup():addprogressbar("Progress")
progressbar = screen:addprogressbar("Progress")
statlabel = screen:addlabel("")
output = screen:addtextfield()
output:follow(true)

function screen:activate()
    install.startinstall(function (s)
                            statlabel:set(s)
                         end,
                         function (s)
                            progressbar:set(s)
                         end,
                         function (s)
                            output:add(s)
                         end)
end

return screen
