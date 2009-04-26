--     Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)
-- 
--     This file is part of Nixstaller.
-- 
--     This program is free software; you can redistribute it and/or modify it under
--     the terms of the GNU General Public License as published by the Free Software
--     Foundation; either version 2 of the License, or (at your option) any later
--     version. 
-- 
--     This program is distributed in the hope that it will be useful, but WITHOUT ANY
--     WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
--     PARTICULAR PURPOSE. See the GNU General Public License for more details. 
-- 
--     You should have received a copy of the GNU General Public License along with
--     this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
--     St, Fifth Floor, Boston, MA 02110-1301 USA

pkg.grouplist = {}

-- Default available groups. If you think something is missing please let me know.

pkg.grouplist["Archiving"] = {
["rpm"] = "Applications/Archiving",
["deb"] = "utils"
}

pkg.grouplist["Communication"] = {
["rpm"] = "Applications/Communications",
["deb"] = "comm"
}

pkg.grouplist["Databases"] = {
["rpm"] = "Applications/Databases",
["deb"] = "misc"
}

pkg.grouplist["Development-languages"] = {
["rpm"] = "Development/Languages",
["deb"] = "interpreters"
}

pkg.grouplist["Development-libraries"] = {
["rpm"] = "Development/Libraries",
["deb"] = "libdevel"
}

pkg.grouplist["Development-tools"] = {
["rpm"] = "Development/Tools",
["deb"] = "devel"
}

pkg.grouplist["Documentation"] = {
["rpm"] = "Documentation",
["deb"] = "doc"
}

pkg.grouplist["Editors"] = {
["rpm"] = "Applications/Editors",
["deb"] = "editors"
}

pkg.grouplist["Emulators"] = {
["rpm"] = "Applications/Emulators",
["deb"] = "otherosfs"
}

pkg.grouplist["File"] = {
["rpm"] = "Applications/File",
["deb"] = "utils"
}

pkg.grouplist["Games"] = {
["rpm"] = "Amusements/Games",
["deb"] = "games"
}

pkg.grouplist["Graphics"] = {
["rpm"] = "Amusements/Graphics",
["deb"] = "graphics"
}

pkg.grouplist["Libraries"] = {
["rpm"] = "System Environment/Libraries",
["deb"] = "libs"
}

pkg.grouplist["Multimedia"] = {
["rpm"] = "Applications/Multimedia",
["deb"] = "graphics"
}

pkg.grouplist["Network"] = {
["rpm"] = "Applications/Internet",
["deb"] = "net"
}

pkg.grouplist["Perl"] = {
["rpm"] = "Development/Languages",
["deb"] = "perl"
}

pkg.grouplist["Python"] = {
["rpm"] = "Development/Languages",
["deb"] = "python"
}

pkg.grouplist["Shells"] = {
["rpm"] = "System Environment/Shells",
["deb"] = "shells"
}

pkg.grouplist["Sound"] = {
["rpm"] = "Applications/Multimedia",
["deb"] = "sound"
}

pkg.grouplist["System"] = {
["rpm"] = "Applications/System",
["deb"] = "admin"
}

pkg.grouplist["Text"] = {
["rpm"] = "Applications/Text",
["deb"] = "text"
}
