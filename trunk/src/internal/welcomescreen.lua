module (..., package.seeall)

-- Check which file to use
function checkfile()
    filename = string.format("config/lang/%s/welcome", getlang())
    
    if not os.fileexists(filename) then
        filename = string.format("config/welcome")
    end
    
    if not os.fileexists(filename) then
        filename = nil
    end
end

screen = install.newscreen("Welcome")
group = screen:addgroup()

if os.fileexists(cfg.intropic) then
    group:addimage("", cfg.intropic)
end

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
