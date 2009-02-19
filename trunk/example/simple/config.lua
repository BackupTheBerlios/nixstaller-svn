-- Automaticly generated on wo feb 11 14:19:40 CET 2009.
-- Global configuration file, used for generating and running installers.

-- Installer mode. Valid values: "both", "attended" (default) and "unattended".
cfg.mode = "attended"

-- The application name
cfg.appname = "My Software 1.0"

-- Archive type used to pack the installer
cfg.archivetype = "lzma"

-- Target Operating Systems
cfg.targetos = nil

-- Target CPU Architectures
cfg.targetarch = nil

-- Frontends to include
cfg.frontends = { "gtk", "fltk", "ncurses" }

-- Default language (you can use this to change the language of the language
-- selection screen)
cfg.defaultlang = "english"

-- Translations to include
cfg.languages = { "english", "dutch", "lithuanian" }

-- When enabled the installer will automaticly guess the right language. If this fails
-- or when this option is disabled, the user has to choose a language.
cfg.autolang = true

-- Picture used for the 'WelcomeScreen'
cfg.intropic = nil

-- Logo image file
cfg.logo = nil

-- Appication icon used by the installer
cfg.appicon = nil
