module (..., package.seeall)

screen = install.newscreen("Please select a language")

menu = screen:addmenu("", cfg.languages or {})

function menu:datachanged()
    setlang(menu:get())
    return true
end

function screen:canactivate()
    return #cfg.languages > 1
end

function screen:activate()
    for i, v in ipairs(cfg.languages) do
        if v == getlang() then
            menu:set(i)
            break
        end
    end
end

return screen
