module (..., package.seeall)

-- Check which file to use
function checkfile()
    filename = string.format("config/lang/%s/license", getlang())
    
    if not os.fileexists(filename) then
        filename = string.format("config/license")
    end
    
    if not os.fileexists(filename) then
        filename = nil
    end
end

screen = install.newscreen("License agreement")
text = screen:addtextfield()
check = screen:addcheckbox("", { "I agree to this license agreement" } )

function check:datachanged()
    install.lockscreen(false, not check:get(1)) -- Unlock next button incase box is checked
end

function screen:canactivate()
    checkfile()
    return filename ~= nil
end

function screen:activate()
    if filename ~= nil then -- checkfile() has been called by canactivate
        text:load(filename)
        install.lockscreen(false, true)
    end
end

return screen
