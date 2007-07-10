module (..., package.seeall)

screen = install.newscreen("Heh")

progressbar = screen:addprogressbar("Progress")
statlabel = screen:addlabel("")
output = screen:addtextfield()
output:follow(true)

function screen:activate()
    install.lockscreen(true, true) -- Disable Back and Next buttons
    install.startinstall(function (s)
                            statlabel:set(s)
                         end,
                         function (s)
                            progressbar:set(s)
                         end,
                         function (s)
                            output:add(s)
                         end)
    install.lockscreen(true, false) -- Re-enable Next button, keep Back button locked
end

return screen
