module (..., package.seeall)

-- Check which file to use
function checkfile()
    filename = string.format("config/lang/%s/finish", getlang())
    
    if not os.fileexists(filename) then
        filename = string.format("config/finish")
    end
    
    if not os.fileexists(filename) then
        filename = nil
    end
end

screen = install.newscreen("Please read the following text")
group = screen:addgroup()

text = group:addtextfield()

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
