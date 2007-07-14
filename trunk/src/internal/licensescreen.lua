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
check:setcheck(function ()
                   if not check:get(1) then
                       gui.msgbox("Before you can continue, you have to agree to the license agreement")
                       return false
                   end
                   return true
               end)

function screen:canactivate()
    checkfile()
    return filename ~= nil
end

function screen:activate()
    if filename ~= nil then -- checkfile() has been called by canactivate
        text:load(filename)
    end
end

return screen
