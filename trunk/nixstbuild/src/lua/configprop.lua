--  ***************************************************************************
--  *   Copyright (C) 2009 by Rick Helmus / InternetNightmare   *
--  *   rhelmus_AT_gmail.com / internetnightmare_AT_gmail.com   *
--  *                                                                         *
--  *   This program is free software; you can redistribute it and/or modify  *
--  *   it under the terms of the GNU General Public License as published by  *
--  *   the Free Software Foundation; either version 2 of the License, or     *
--  *   (at your option) any later version.                                   *
--  *                                                                         *
--  *   This program is distributed in the hope that it will be useful,       *
--  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
--  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
--  *   GNU General Public License for more details.                          *
--  *                                                                         *
--  *   You should have received a copy of the GNU General Public License     *
--  *   along with this program; if not, write to the                         *
--  *   Free Software Foundation, Inc.,                                       *
--  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
--  ***************************************************************************

-- Property lists
--


properties_config = {
    { name = "Application icon", var = "cfg.appicon", ptype = "file", description = "Icon for installer application in XPM format" },
    { name = "Application name", var = "cfg.appname", ptype = "string", default = "", description = "Application name used by installer" },
    { name = "Archive type", var = "cfg.archivetype", ptype = "choice", description = "Type of compression to use for installer archive", choices = {"gzip", "lzma", "bzip2"} },
    { name = "Autodetect language", var = "cfg.autolang", ptype = "boolean", description = "When selected installer will automaticly try to detect system language" },
    { name = "Default language", var = "cfg.defaultlang", ptype = "string", default = "english", description = "Defines the default language to use for installer" },
    { name = "Frontends", var = "cfg.frontends", ptype = "multi-choice", choices = { "ncurses", "fltk", "gtk" }, description = "Defines frontends to include into installer package" },
    { name = "Intro picture", var = "cfg.intropic", ptype = "file", description = "Picture to use in welcome screen" },
    { name = "Languages", var = "cfg.languages", ptype = "multi-choice", choices = { "english", "dutch", "lithuanian", "bulgarian" }, description = "Languages to include in installer package" },
    { name = "Logo", var = "cfg.logo", ptype = "file", description = "Picture to use as logo" },
    { name = "Modes", var = "cfg.mode", ptype = "choice", choices = { "attended", "unattended", "both" }, description = "Set installer mode. When 'both' is used, the user needs to pass the --unattend (or -u) commandline option to start an unattended installation." },
    { name = "Target architecture", var = "cfg.targetarch", ptype = "multi-choice", choices = { "x86", "x86_64" }, description = "Defines atchitectures which installer should support" },
    { name = "Operating systems", var = "cfg.targetos", ptype = "multi-choice", choices = { "Linux", "FreeBSD", "NetBSD", "OpenBSD", "SunOS" }, description = "Operating systems to be supported by installer package" },
    { name = "Unattended options", var = "cfg.unopts", ptype = "multi-string", columns = { "desc", "optname", "opttype", "short" }, description = "Holds all command line arguments to be used by unattended installation.<br/> <b>desc</b> - description for an argument<br\> <b>optname</b> - Short (few characters at most) name for the variable or setting that it will be used for. Like desc this variable is used to display usage information. <br/> <b>opttype</b> - This variable is used to specify what kind of data this argument can be set. Valid values are: 'string' (uses the given value directly), 'list' (argument expects a comma seperated list, which will be automatically converted to a Lua table) and nil (None, this is the default). Note that when opttype is not nil, optname has to be set as well.<br\> <b>short</b> - This variable should contain a string of exactly 1 character, and is used as a short alternative for this parameter." }
}

print "Testy!"
