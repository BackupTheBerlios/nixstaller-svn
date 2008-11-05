--     Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)
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


-- Default dependency templates. New submissions are very welcome...
-- 
-- NOTE: Template searching is order dependent!
-- 
-- Table structure fields:
-- name         Dependency name.
-- description  Dependency description.
-- check        Function that is used to determine if a library is part of this template.
-- libs         As an alternative to 'check', this field can be used to list libraries which
--              belong to this this template. It should contain a table (array) containing strings
--              of the library name, without any starting "lib" and ending ".so.<version> part".
--              So if you want to look for "libfoo.so.<version>" simply specify "foo".
-- patlibs      As 'libs', but allows lua search patterns.
-- full         Recommended usage. Default is true.
-- notes        Notes for this dependency.
-- post         Function that is called after the dependency is generated. Can be used for example to
--              copy additional files.
-- install      String that contains Lua code used in the Install() function inside the generated config.lua.
--              Default is to call self:CopyFiles().
-- required     As the 'install' field, but for the Required() function. Default is to simply return false.
-- compat       As the 'install' field, but for the HandleCompat() function. Default is to simply return false.
-- caninstall   As the 'install' field, but for the CanInstall() function. Default is to simply return true.
--
-- New structures should be created with the newtemplate() function

pkg.deptemplates = { }

function newtemplate(t)
    local function default(field, val)
        if t[field] == nil then
            t[field] = val
        end
    end
    
    default("full", true)
    default("install", "    self:CopyFiles()")
    default("required", "    return false")
    default("compat", "    return false")
    default("caninstall", "    return true")
    
    table.insert(pkg.deptemplates, t)
end


-- Core system libraries
newtemplate{
name = "core-libs",
description = "Core system libraries.",
libs = { "c", "rt", "dl" },
patlibs = { "ld-linux.*" },
full = false,
notes = "Never use this as a full dependency!"
}

-- Other core system libraries; I don't know if they can be a full dep, so they are put in this seperate template for now.
newtemplate{
name = "core-libs-other",
description = "Other core system libraries.",
libs = { "anl", "cidn", "crypt", "m", "nsl", "memusage", "nss_compat", "nss_dns", "nss_files", "nss_hesiod", "nss_nis", "nss_nisplus", "pcprofile", "pthread", "resolv", "thread_db", "util", "gcc_s" },
full = false,
}

-- C++ Library
newtemplate{
name = "libstdc++",
description = "C++ system library.",
libs = { "stdc++" }
}

-- Bzip2
newtemplate{
name = "Bzip2",
description = "Bzip2 compression library.",
libs = { "bz2" },
full = true,
}

-- zlib
newtemplate{
name = "zlib",
description = "Zlib compression library.",
libs = { "z" },
full = true,
}

-- fam
newtemplate{
name = "fam",
description = "File alteration monitor.",
libs = { "fam" },
full = false,
}

-- libacl
newtemplate{
name = "libacl",
description = "Library for handling Access Control Lists.",
libs = { "acl" },
full = false,
}

-- libattr
newtemplate{
name = "attr",
description = "Library for handling Extended Attributes",
libs = { "attr" },
full = false,
}

-- libidn
newtemplate{
name = "libidn",
description = "Support for Internationalized Domain Names.",
libs = { "idn" },
full = false,
}

-- OpenSSL
newtemplate{
name = "OpenSSL",
description = "The Open Source toolkit for SSL/TLS.",
libs = { "ssl", "crypto" },
full = false,
}

-- OpenLDAP client
newtemplate{
name = "lber-2.4",
description = "OpenLDAP client libraries.",
libs = { "lber-2.4", "ldap-2.4", "ldap_r-2.4" },
full = false,
}

-- Cyrus SASL
newtemplate{
name = "sasl",
description = "The Cyrus SASL API.",
libs = { "sasl2" },
full = false,
}

-- libxml2
newtemplate{
name = "libxml2",
description = "Library for handling xml files.",
libs = { "xml2" },
full = true,
}

-- expat
newtemplate{
name = "expat",
description = "An XML 1.0 parser written in C.",
libs = { "expat" },
full = false,
}

-- musicbrainz
newtemplate{
name = "musicbrainz",
description = "MusicBrainz is the second generation incarnation of the CD Index.",
libs = { "musicbrainz" },
full = true,
}

-- asound (ALSA)
newtemplate{
name = "ALSA",
description = "Advanced Linux Sound Architecture.",
libs = { "asound" },
full = false,
}

-- libogg
newtemplate{
name = "libogg",
description = "Libogg is a library for manipulating ogg bitstreams.",
libs = { "ogg" },
full = false,
}

-- libtag
newtemplate{
name = "libtag",
description = "Provides an interface for reading additional data from MP3, Ogg Vorbis, and MPEG files.",
libs = { "tag", "tag_c" },
full = false,
}

-- pcre
newtemplate{
name = "pcre",
description = "Library providing PERL regexp functionality.",
libs = { "pcre", "pcrecpp", "pcreposix" },
full = false,
}

-- ICU
newtemplate{
name = "ICU",
description = "ICU is a set of libraries that provides robust and full-featured Unicode support.",
libs = { "icudata", "icui18n", "icuio", "icule", "iculx", "icutu", "icuuc" },
full = false,
}

-- X core libs (see http://www.x.org/wiki/ModuleDescriptions)
newtemplate{
name = "X11-core",
description = "Core X11 libraries.",
libs = { "X11", "Xext", "ICE", "FS", "SM", "Xau", "Xdmcp", "Xfont", "Xfontcache", "Xft", "Xi", "Xinerama", "Xmu", "Xpm", "Xrandr", "Xrender", "Xv", "Xxf86misc", "Xxf86vm", "fontenc", "lbxutil", "xkbfile", "Xss", "AppleWM", "WindowsWM", "pciaccess", "pixman-1" },
}

-- X extended libs (see http://www.x.org/wiki/ModuleDescriptions)
newtemplate{
name = "X11-extended",
description = "Extended X11 libraries.",
libs = { "XRes", "XScrnSaver", "XTrap", "Xcursor", "Xtst", "XvMC", "XvMCW", "Xxf86dga", "dmx", "xkbui", "Xp", "XprintAppUtil", "XprintUtil" },
patlibs = { "xkb-.+" }
}

-- X legacy libs (see http://www.x.org/wiki/ModuleDescriptions)
newtemplate{
name = "X11-legacy",
description = "Legacy X11 libraries.",
libs = { "Xaw", "Xaw6", "Xaw7", "Xaw8", "Xt", "oldX" },
}

-- xcb
newtemplate{
name = "xcb",
description = "XCB libraries.",
libs = { "xcb-composite", "xcb-damage", "xcb-dpms", "xcb-glx", "xcb-randr", "xcb-record", "xcb-render", "xcb-res", "xcb-screensaver", "xcb-shape", "xcb-shm", "xcb-sync", "xcb-xevie", "xcb-xf86dri", "xcb-xfixes", "xcb-xinerama", "xcb-xlib", "xcb-xprint", "xcb-xtest", "xcb-xv", "xcb-xvmc", "xcb" },
full = false,
}

-- X other libs
newtemplate{
name = "X11-other",
description = "Other X11 libraries.",
libs = { "Xcomposite", "Xdamage", "Xevie", "Xfixes", "VncExt", "Xcliplist", "" },
}

-- Mesa
newtemplate{
name = "Mesa",
description = "Mesa 3-D graphics library.",
libs = { "GL", "GLU", "GLcore", "IndirectGL", "OSMesa" },
full = false,
}

-- fontconfig
newtemplate{
name = "fontconfig",
description = "Library for configuring and customizing font access.",
libs = { "fontconfig" },
full = false,
}

-- freetype
newtemplate{
name = "freetype",
description = "This library features TrueType fonts for open source projects.",
libs = { "freetype" },
full = false,
}

-- gif
newtemplate{
name = "gif",
description = "Library handling GIF files.",
libs = { "gif", "ungif" },
full = true,
}

-- jpeg
newtemplate{
name = "jpeg",
description = "Library handling JPEG files.",
libs = { "jpeg" },
full = true,
}

-- png
newtemplate{
name = "png",
description = "Library handling PNG files.",
libs = { "png12", "png" },
full = true,
}

-- Libart
newtemplate{
name = "Libart",
description = " Libart is a rary for high-performance 2D graphics.",
libs = { "art_lgpl_2" },
full = true,
}

-- ATK
newtemplate{
name = "ATK",
description = "Accessibility toolkit.",
libs = { "atk-1.0" },
full = false
}

-- GLib
newtemplate{
name = "GLib2",
description = "Library of useful routines for C programming.",
libs = { "glib-2.0", "gio-2.0", "gmodule-2.0", "gobject-2.0", "gthread-2.0" },
full = false
}

-- Cairo
newtemplate{
name = "Cairo",
description = "Vector Graphics Library with Cross-Device Output Support.",
libs = { "cairo" },
full = false
}

-- QT3
newtemplate{
name = "Qt3",
description = "Qt is used to build graphical interfaces.",
libs = { "qt-mt", "qui", "qt3" },
full = false,
}

-- QT4
newtemplate{
name = "Qt4",
description = "Qt is used to build graphical interfaces.",
libs = { "Qt3Support", "QtCLucene", "QtCore", "QtDBus", "QtNetwork", "QtTest", "QtXml", "QtAssistantClient", "QtDesigner", "QtDesignerComponents", "QtGui", "QtHelp", "QtOpenGL", "QtScript", "QtSvg", "QtWebKit", "QtXmlPatterns", "QtSql" },
full = false,
}

-- kdelibs3
newtemplate{
name = "kdelibs3",
description = "Core libraries from the KDE 4 environment.",
libs = { "DCOP", "connectionmanager", "kabc", "kabc_dir", "kabc_file", "kabc_ldapkio", "katepartinterfaces", "kdecore", "kdefakes", "kdeinit_cupsdconf", "kdeinit_dcopserver", "kdeinit_kaddprinterwizard", "kdeinit_kbuildsycoca", "kdeinit_kcmshell", "kdeinit_kconf_update", "kdeinit_kcookiejar", "kdeinit_kded", "kdeinit_kio_http_cache_cleaner", "kdeinit_kio_uiserver", "kdeinit_klauncher", "kdeinit_knotify", "kdemm", "kdeprint", "kdeprint_management", "kdesasl", "kdesu", "kdeui", "kdnssd", "khtml", "kimproxy", "kio", "kjava", "kjs", "kmdi", "kmdi2", "kmediaplayer", "kmid", "knewstuff", "kntlm", "kparts", "kresources", "kscreensaver", "kscript", "kspell", "kspell2", "ktexteditor", "kunittest", "kutils", "kwalletbackend", "kwalletclient", "networkstatus", "vcard", "kdefx" },
full = false,
}

-- Phonon (either part of KDE4 or QT4, so we just use a seperate template)
newtemplate{
name = "Phonon",
description = "Multimedia abstraction framework.",
libs = { "phonon" },
full = false,
}

-- Strigi
newtemplate{
name = "Strigi",
description = " Lightweight and fast desktop search engine.",
libs = { "searchclient", "streamanalyzer", "streams", "strigihtmlgui", "strigiqtdbusclient" },
full = false,
}

-- kate4
newtemplate{
name = "kate",
description = "Kate is an advanced text editor for the KDE 4 environment.",
libs = { "kateinterfaces", "kdeinit4_kate" },
full = false,
}

-- kdepim4
newtemplate{
name = "kdepim4",
description = "KDE 4 PIM functionality.",
libs = { "gpgme++-pth", "gpgme++-pthread", "gpgme++", "kabc", "kabc_file_core", "kblog", "kcal", "kimap", "kldap", "kmime", "kpimidentities", "kpimutils", "kresources", "ktnef", "kxmlrpcclient", "mailtransport", "qgpgme", "syndicatio" },
full = false,
}

-- kdemultimedia4
newtemplate{
name = "kdemultimedia4",
description = "Multimedia applications for the KDE 4 environment.",
libs = { "kcddb", "kcompactdisc" },
full = false,
}

-- kopete 4
newtemplate{
name = "kopete4",
description = "Instant messenger client for the KDE 4 environment.",
libs = { "gadu_kopete", "iris_kopete", "kopete", "kopete_msn_shared", "kopete_oscar", "kopete_otr_shared", "kopete_videodevice", "kopeteaddaccountwizard", "kopetechatwindow_shared", "kopeteidentity", "kopeteprivacy", "kopetestatusmenu", "kyahoo", "oscar", "qgroupwise" },
full = false,
}

newtemplate{
name = "KDE4-libs",
description = "Core libraries from the KDE 4 environment.",
libs = { "kde3support", "kdesu", "kdeui", "kdnssd", "kfile", "khtml", "kimproxy", "kio", "kjs", "kjsapi", "kjsembed", "kmediaplayer", "knewstuff2", "knotifyconfig", "kntlm", "kparts", "krosscore", "krossui", "ktexteditor", "kunittest", "kutils", "kwalletbackend", "nepomuk", "solid", "threadweaver", "kdecore", "kdefakes", "kpty" },
patlibs = { "kdeinit4_.+" },
full = false,
}



-- Bigger templates, split in other files.
dofile(ndir .. "/src/lua/deptemplates/gtk.lua")
dofile(ndir .. "/src/lua/deptemplates/pango.lua")
