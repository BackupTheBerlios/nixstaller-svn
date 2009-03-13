function UnattendedInit()
    -- Set install.destdir to a default incase the user did not gave then --destdir
    -- option (remove this if cfg.adddestunopt() is not called in config.lua!).
    if not cfg.unopts["destdir"].value then
        install.destdir = os.getenv("HOME") -- Default directory
    end
end

function Install()
    install.extractfiles()

    if (cfg.unopts["script"].value) then
        local filename = string.format("%s/cube.sh", os.getenv("HOME"))
        script = io.open(filename, "w")

        if (not script) then
            install.print("Failed to create launcher script")
            return -- Exit from function
        end

        script:write(string.format(
[[#!/bin/sh
cd %s/cube/
./cube_unix
]], install.destdir))
        script:close()

        os.chmod(filename, 700)
    end
end

