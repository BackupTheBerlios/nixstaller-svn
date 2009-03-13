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


-- Default dependency templates. New submissions are very welcome...
-- 
-- NOTE: Template searching is order dependent!
-- 
-- Table structure fields:
-- name         Dependency name.
-- description  Dependency description.
-- check        Function that is used to determine if a library is part of this template. If true is
--              returned 'libs' and 'patlibs' will also be checked.
-- libs         List of libraries which belong to this this template. It should contain a table (array)
--              containing strings of the library name, without any starting "lib" and ending ".so.<version> part".
--              So if you want to look for "libfoo.so.<version>" simply specify "foo".
-- patlibs      As 'libs', but allows lua search patterns and searches only for given text (no lib prefix
--              and .so.<version> suffix).
-- full         Recommended usage. Default is true.
-- notes        Notes for this dependency.
-- post         Function that is called after the dependency is generated. Can be used for example to
--              copy additional files.
-- install      String that contains Lua code used in the Install() function inside the generated config.lua.
--              Default is to call self:copyfiles().
-- required     As the 'install' field, but for the required() function. Default is to simply return false.
-- compat       As the 'install' field, but for the handlecompat() function. Default is to simply return false.
-- caninstall   As the 'install' field, but for the caninstall() function. Default is to simply return true.
--
-- New structures should be created with the newtemplate() function

pkg.deptemplates = { }

function newtemplate(t)
    local function default(field, val)
        if t[field] == nil then
            t[field] = val
        end
    end
    
    default("tags", { })
    default("install", "    self:copyfiles()")
    default("required", "    return false")
    default("compat", "    return false")
    default("caninstall", "    return true")
    
    for _, t in ipairs(t.tags) do
        if not pkg.templatetags[t] then
            print("WARNING: Unknown tag: " .. t)
        end
    end
    
    table.insert(pkg.deptemplates, t)
end

-- Add all template tags (with description) in this table.
pkg.templatetags = {
["common"] = "Dependency which is commonly available on most systems.",
["common-desktop"] = "Dependency which is commonly available on most desktop systems.",
["small"] = "Small dependency.",
["full-never"] = "Should never be used as a full dependency.",
["full-config"] = "Needs one more more configuration file(s) in order to be a full dependency (and are not copied/generated when 'auto copy' is used).",
["unknown-full"] = "Unknown if it can be used as a full dependency.",
["unknown-copy"] = "Unknown if the 'auto copy' option copies all files for this template.",
["KDE4"] = "Dependencies from the KDE 4 environment.",
["KDE3"] = "Dependencies from the KDE 3 environment.",
["GNOME"] = "Dependencies from the GNOME environment.",
["GNOME-extra"] = "Unofficial GNOME dependencies.",
["GTK"] = "Dependencies from the GTK toolkit.",
["GTK-extra"] = "Unofficial GTK dependencies.",
["X11"] = "Dependencies from X11.",
["SDL"] = "Dependencies from the Simple DirectMedia Layer.",
["SDL-extra"] = "Unofficial SDL dependencies.",
}

-------------------------------------------------------------
-- System Libraries
-------------------------------------------------------------

-- Core system libraries
newtemplate{
name = "core-libs",
description = "Core system libraries.",
libs = { "c", "rt", "dl" },
patlibs = { "ld%-linux.*%.so%.%d" },
tags = { "full-never" },
}

-- Other core system libraries; I don't know if they can be a full dep, so they are put in this seperate template for now.
newtemplate{
name = "core-libs-other",
description = "Other core system libraries.",
libs = { "anl", "cidn", "crypt", "m", "nsl", "memusage", "nss_compat", "nss_dns", "nss_files", "nss_hesiod", "nss_nis", "nss_nisplus", "pcprofile", "pthread", "resolv", "thread_db", "util", "gcc_s" },
tags = { "common", "unknown-full" },
}

-- C++ Library
newtemplate{
name = "libstdc++",
description = "C++ system library.",
libs = { "stdc++" },
tags = { "common" }
}

-- zlib
newtemplate{
name = "zlib",
description = "Zlib compression library.",
libs = { "z" },
tags = { "common", "small" },
}

-------------------------------------------------------------
-- SDL
-------------------------------------------------------------

-- SDL-1.2
newtemplate{
name = "SDL-1.2",
description = "Core library from SDL (Simple DirectMedia Layer).",
libs = { "SDL-1.2" },
tags = { "SDL", "small" },
}

-- SDL_net-1.2
newtemplate{
name = "SDL_net-1.2",
description = "Small cross-platform networking library for use with SDL.",
libs = { "SDL_net-1.2" },
tags = { "SDL", "small" },
}

-- SDL_image-1.2
newtemplate{
name = "SDL_image-1.2",
description = "Library to load images of various formats as SDL surfaces.",
libs = { "SDL_image-1.2" },
tags = { "SDL", "small" },
}

-- SDL_mixer-1.2
newtemplate{
name = "SDL_mixer-1.2",
description = "A multichannel audio mixer from SDL.",
libs = { "SDL_image-1.2" },
tags = { "SDL", "small" },
}

-- SDL_Pango
newtemplate{
name = "SDL_Pango",
description = "Programming Pango via SDL.",
libs = { "SDL_Pango" },
tags = { "SDL-extra", "small" },
}

-- SDL_gfx
newtemplate{
name = "SDL_gfx",
description = "SDL graphics routines for primitives and other support functions.",
libs = { "SDL_gfx" },
tags = { "SDL-extra", "small" },
}

-------------------------------------------------------------
-- X11
-------------------------------------------------------------

-- X core libs (see http://www.x.org/wiki/ModuleDescriptions)
newtemplate{
name = "X11-core",
description = "Core X11 libraries.",
libs = { "X11", "Xext", "ICE", "FS", "SM", "Xau", "Xdmcp", "Xfont", "Xfontcache", "Xft", "Xi", "Xinerama", "Xmu", "Xpm", "Xrandr", "Xrender", "Xv", "Xxf86misc", "Xxf86vm", "fontenc", "lbxutil", "xkbfile", "Xss", "AppleWM", "WindowsWM", "pciaccess", "pixman-1" },
tags = { "common", "X11", "small" },
}

-- X extended libs (see http://www.x.org/wiki/ModuleDescriptions)
newtemplate{
name = "X11-extended",
description = "Extended X11 libraries.",
libs = { "XRes", "XScrnSaver", "XTrap", "Xcursor", "Xtst", "XvMC", "XvMCW", "Xxf86dga", "dmx", "xkbui", "Xp", "XprintAppUtil", "XprintUtil" },
patlibs = { "libxkb-.+%.so%.%d" },
tags = { "common", "X11", "small" },
}

-- X legacy libs (see http://www.x.org/wiki/ModuleDescriptions)
newtemplate{
name = "X11-legacy",
description = "Legacy X11 libraries.",
libs = { "Xaw", "Xaw6", "Xaw7", "Xaw8", "Xt", "oldX" },
tags = { "common", "X11", "small" },
}

-- xcb
newtemplate{
name = "xcb",
description = "XCB libraries.",
libs = { "xcb-atom, xcb-composite", "xcb-damage", "xcb-dpms", "xcb-glx", "xcb-randr", "xcb-record", "xcb-render", "xcb-render-util", "xcb-res", "xcb-screensaver", "xcb-shape", "xcb-shm", "xcb-sync", "xcb-xevie", "xcb-xf86dri", "xcb-xfixes", "xcb-xinerama", "xcb-xlib", "xcb-xprint", "xcb-xtest", "xcb-xv", "xcb-xvmc", "xcb" },
tags = { "common", "X11", "small" },
}

-- X other libs
newtemplate{
name = "X11-other",
description = "Other X11 libraries.",
libs = { "Xcomposite", "Xdamage", "Xevie", "Xfixes", "VncExt", "Xcliplist" },
tags = { "common", "X11", "small" },
}

-- Mesa
newtemplate{
name = "Mesa",
description = "Mesa 3-D graphics library.",
libs = { "GL", "GLU", "GLcore", "IndirectGL", "OSMesa" },
tags = { "common-desktop", "X11", "small" },
}

-------------------------------------------------------------
-- GTK
-------------------------------------------------------------

-- ATK
newtemplate{
name = "ATK",
description = "Accessibility toolkit.",
libs = { "atk-1.0" },
tags = { "common-desktop", "GTK", "small" },
}

-- GLib
newtemplate{
name = "GLib2",
description = "Library of useful routines for C programming.",
libs = { "glib-2.0", "gio-2.0", "gmodule-2.0", "gobject-2.0", "gthread-2.0" },
tags = { "common-desktop", "GTK", "small" },
}

-- Cairo
newtemplate{
name = "Cairo",
description = "Vector Graphics Library with Cross-Device Output Support.",
libs = { "cairo" },
tags = { "common-desktop", "GTK", "small" },
}

-- Bigger templates, split in other files.
dofile(ndir .. "/src/lua/deptemplates/gtk.lua")
dofile(ndir .. "/src/lua/deptemplates/pango.lua")

-- gtksourceview-2.0
newtemplate{
name = "gtksourceview-2.0",
description = "GTK Sourceview widget.",
libs = { "gtksourceview-2.0" },
tags = { "GTK-extra", "unknown-copy", "small" },
}

-- vte
newtemplate{
name = "vte",
description = "Terminal emulator widget for GTK+ 2.0.",
libs = { "vte" },
tags = { "GTK-extra", "unknown-copy", "small" },
}

-- libsexy
newtemplate{
name = "libsexy",
description = "Extended widgets for GTK+.",
libs = { "sexy" },
tags = { "GTK-extra", "small" },
}

-- gtkspell
newtemplate{
name = "gtkspell",
description = "Spell checking for GTK widgets.",
libs = { "gtkspell" },
tags = { "GTK-extra", "small" },
}

-- gtkmm2
newtemplate{
name = "gtkmm2",
description = "C++ Interface for GTK2",
libs = { "atkmm-1.6", "gdkmm-2.4", "gtkmm-2.4", "pangomm-1.4" },
tags = { "GTK-extra" },
}


-------------------------------------------------------------
-- QT
-------------------------------------------------------------

-- QT3
newtemplate{
name = "Qt3",
description = "Qt is used to build graphical interfaces.",
libs = { "qt-mt", "qui", "qt3" },
tags = { "common-desktop", "unknown-copy" },
}

-- QT4
newtemplate{
name = "Qt4",
description = "Qt is used to build graphical interfaces.",
libs = { "Qt3Support", "QtCLucene", "QtCore", "QtDBus", "QtNetwork", "QtTest", "QtXml", "QtAssistantClient", "QtDesigner", "QtDesignerComponents", "QtGui", "QtHelp", "QtOpenGL", "QtScript", "QtSvg", "QtWebKit", "QtXmlPatterns", "QtSql" },
tags = { "common-desktop", "unknown-copy" },
}

-------------------------------------------------------------
-- KDE 4
-------------------------------------------------------------

-- Phonon (either part of KDE4 or QT4, so we just use a seperate template)
newtemplate{
name = "Phonon",
description = "Multimedia abstraction framework.",
libs = { "phonon" },
tags = { "KDE4", "common-desktop", "unknown-copy" },
}

-- Strigi
newtemplate{
name = "Strigi",
description = " Lightweight and fast desktop search engine.",
libs = { "searchclient", "streamanalyzer", "streams", "strigihtmlgui", "strigiqtdbusclient" },
tags = { "KDE4", "common-desktop", "unknown-copy" },
}

-- kate4
newtemplate{
name = "kate",
description = "Kate is an advanced text editor for the KDE 4 environment.",
libs = { "kateinterfaces", "kdeinit4_kate" },
tags = { "KDE4", "common-desktop", "unknown-copy" },
}

-- kdepim4
newtemplate{
name = "kdepim4",
description = "KDE 4 PIM functionality.",
libs = { "gpgme++-pth", "gpgme++-pthread", "gpgme++", "kabc", "kabc_file_core", "kblog", "kcal", "kimap", "kldap", "kmime", "kpimidentities", "kpimutils", "kresources", "ktnef", "kxmlrpcclient", "mailtransport", "qgpgme", "syndicatio" },
tags = { "KDE4", "unknown-copy" },
}

-- kdemultimedia4
newtemplate{
name = "kdemultimedia4",
description = "Multimedia applications for the KDE 4 environment.",
libs = { "kcddb", "kcompactdisc" },
tags = { "KDE4", "unknown-copy" },
}

-- kopete 4
newtemplate{
name = "kopete4",
description = "Instant messenger client for the KDE 4 environment.",
libs = { "gadu_kopete", "iris_kopete", "kopete", "kopete_msn_shared", "kopete_oscar", "kopete_otr_shared", "kopete_videodevice", "kopeteaddaccountwizard", "kopetechatwindow_shared", "kopeteidentity", "kopeteprivacy", "kopetestatusmenu", "kyahoo", "oscar", "qgroupwise" },
tags = { "KDE4", "unknown-copy" },
}

-- KDE4 libs
newtemplate{
name = "KDE4-libs",
description = "Core libraries from the KDE 4 environment.",
check = function(lib, map)
    -- As kde4 and kde3 use many similar named libs it's hard to differentiate
    -- between them. As a simple fix check if bin needs a common QT4 lib, as KDE4 always need QT4
    for l in pairs(map) do
        if string.find(l, "^libQtCore%.so%.%d") then
            return true
        end
    end
    return false
end,
libs = { "kde3support", "kdesu", "kdeui", "kdnssd", "kfile", "khtml", "kimproxy", "kio", "kjs", "kjsapi", "kjsembed", "kmediaplayer", "knewstuff2", "knotifyconfig", "kntlm", "kparts", "krosscore", "krossui", "ktexteditor", "kunittest", "kutils", "kwalletbackend", "nepomuk", "solid", "threadweaver", "kdecore", "kdefakes", "kpty" },
patlibs = { "libkdeinit4%_.+%.so" },
tags = { "KDE4", "common-desktop", "unknown-copy" },
notes = "This template has some of the same libraries from KDE3, please verify the right one is used!",
}

-------------------------------------------------------------
-- KDE 3
-------------------------------------------------------------

-- kdelibs3
newtemplate{
name = "KDE3-libs",
description = "Core libraries from the KDE 3 environment.",
libs = { "DCOP", "connectionmanager", "kabc", "kabc_dir", "kabc_file", "kabc_ldapkio", "katepartinterfaces", "kdecore", "kdefakes", "kdeinit_cupsdconf", "kdeinit_dcopserver", "kdeinit_kaddprinterwizard", "kdeinit_kbuildsycoca", "kdeinit_kcmshell", "kdeinit_kconf_update", "kdeinit_kcookiejar", "kdeinit_kded", "kdeinit_kio_http_cache_cleaner", "kdeinit_kio_uiserver", "kdeinit_klauncher", "kdeinit_knotify", "kdemm", "kdeprint", "kdeprint_management", "kdesasl", "kdesu", "kdeui", "kdnssd", "khtml", "kimproxy", "kio", "kjava", "kjs", "kmdi", "kmdi2", "kmediaplayer", "kmid", "knewstuff", "kntlm", "kparts", "kresources", "kscreensaver", "kscript", "kspell", "kspell2", "ktexteditor", "kunittest", "kutils", "kwalletbackend", "kwalletclient", "networkstatus", "vcard", "kdefx" },
tags = { "KDE3", "common-desktop", "unknown-copy" },
notes = "This template has some of the same libraries from KDE4, please verify the right one is used!",
}

-------------------------------------------------------------
-- GNOME
-------------------------------------------------------------

-- gnomeui-2
newtemplate{
name = "gnomeui-2",
description = "GNOME 2 User Intreface library.",
libs = { "gnomeui-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- gnome-2
newtemplate{
name = "gnome-2",
description = "Core GNOME 2 library.",
libs = { "gnome-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- gnome-desktop-2
newtemplate{
name = "gnome-desktop-2",
description = "GNOME 2 utility library for loading .desktop files.",
libs = { "gnome-desktop-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- gnomecanvas-2
newtemplate{
name = "gnomecanvas-2",
description = "A powerful object-oriented display.",
libs = { "gnomecanvas-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- gnomevfs-2
newtemplate{
name = "gnomevfs-2",
description = "GNOME VFS is the GNOME virtual file system.",
libs = { "gnomevfs-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- gnome-keyring
newtemplate{
name = "gnome-keyring",
description = "GNOME Keyring password manager.",
libs = { "gnome-keyring" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- bonoboui-2
newtemplate{
name = "bonoboui-2",
description = "The GNOME bonobo UI library.",
libs = { "bonoboui-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- bonobo
newtemplate{
name = "bonobo",
description = "Bonobo CORBA interfaces library.",
libs = { "bonobo-activation", "bonobo-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- ORBit
newtemplate{
name = "ORBit",
description = "High-Performance CORBA Object Request Broker.",
libs = { "ORBit-2", "ORBitCosNaming-2", "ORBit-imodule-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- gconf
newtemplate{
name = "gconf",
description = "GNOME configuration database system.",
libs = { "gconf-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- startup-notification
newtemplate{
name = "startup-notification",
description = "Library which allows programs to give visual feedback when they are launched.",
libs = { "startup-notification-1" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- gnome-media-profiles
newtemplate{
name = "gnome-media-profiles",
description = "Libraries for the GNOME media utilities.",
libs = { "gnome-media-profiles" },
tags = { "GNOME", "unknown-copy" },
}

-- nautilus-burn
newtemplate{    
name = "nautilus-burn",
description = "Lets you burn CDs and DVDs easily with Nautilus.",
libs = { "nautilus-burn" },
tags = { "GNOME", "unknown-copy" },
}

-- nautilus-extension
newtemplate{
name = "nautilus-extension",
description = "Required for Nautilus extensions.",
libs = { "nautilus-extension" },
tags = { "GNOME", "unknown-copy" },
}

-- totem-plparser
newtemplate{
name = "totem-plparser",
description = "A library to parse playlists.",
libs = { "totem-plparser", "totem-plparser-mini" },
tags = { "GNOME", "unknown-copy" },
}

-- gsf
newtemplate{
name = "gsf",
description = "GNOME Structured File Library.",
libs = { "gsf-1" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- rsvg
newtemplate{
name = "rsvg",
description = "The rsvg library is an efficient renderer for Scalable Vector Graphics (SVG) pictures.",
libs = { "rsvg-2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- Evolution Data Center
newtemplate{
name = "evolution-datacenter-1.2",
description = "Provides a central location for your address book and calendar.",
libs = { "camel-1.2", "camel-provider-1.2", "ebook-1.2", "ecal-1.2", "edata-book-1.2", "edata-cal-1.2", "edataserver-1.2", "edataserverui-1.2", "egroupwise-1.2", "exchange-storage-1.2", "gdata-1.2", "gdata-google-1.2" },
tags = { "GNOME", "common-desktop", "unknown-copy" },
}

-- glade-2.0
newtemplate{
name = "glade-2.0",
description = "Used to load externally stored user interfaces into programs.",
libs = { "glade-2.0" },
tags = { "GNOME", "small" },
}

-- Libart
newtemplate{
name = "Libart",
description = "Libart is a library for high-performance 2D graphics.",
libs = { "art_lgpl_2" },
tags = { "GNOME", "common-desktop", "small" },
}

-- gnomemm
newtemplate{
name = "gnomemm",
description = " C++ Interface for GNOME Libraries.",
libs = { "gnomemm-2.6" },
tags = { "GNOME-extra", "small" },
}


-------------------------------------------------------------
-- Graphics support
-------------------------------------------------------------

-- fontconfig
newtemplate{
name = "fontconfig",
description = "Library for configuring and customizing font access.",
libs = { "fontconfig" },
tags = { "common-desktop", "unknown-copy" },
}

-- freetype
newtemplate{
name = "freetype",
description = "This library features TrueType fonts for open source projects.",
libs = { "freetype" },
tags = { "common-desktop", "small" },
}

-- gif
newtemplate{
name = "gif",
description = "Library handling GIF files.",
libs = { "gif", "ungif" },
tags = { "common-desktop", "small" },
}

-- jpeg
newtemplate{
name = "jpeg",
description = "Library handling JPEG files.",
libs = { "jpeg" },
tags = { "common-desktop", "small" },
}

-- png
newtemplate{
name = "png",
description = "Library handling PNG files.",
libs = { "png12", "png" },
tags = { "common-desktop", "small" },
}


-------------------------------------------------------------
-- Multimedia Support
-------------------------------------------------------------

-- musicbrainz
newtemplate{
name = "musicbrainz",
description = "MusicBrainz is the second generation incarnation of the CD Index.",
libs = { "musicbrainz" },
tags = { "small" },
}

-- asound (ALSA)
newtemplate{
name = "ALSA",
description = "Advanced Linux Sound Architecture.",
libs = { "asound" },
tags = { "common", "unknown-copy" },
}

-- libogg
newtemplate{
name = "libogg",
description = "Libogg is a library for manipulating ogg bitstreams.",
libs = { "ogg" },
tags = { "small" },
}

-- vorbis
newtemplate{
name = "vorbis",
description = "The vorbis general audio compression codec.",
libs = { "vorbis", "vorbisenc", "vorbisfile" },
tags = { "small" },
}

-- libtag
newtemplate{
name = "libtag",
description = "Provides an interface for reading additional data from MP3, Ogg Vorbis, and MPEG files.",
libs = { "tag", "tag_c" },
tags = { "small" },
}

-- openal
newtemplate{
name = "openal",
description = "Open Audio Library.",
libs = { "openal" },
tags = { "small", "full-config" },
}

-- mad
newtemplate{
name = "mad",
description = "MPEG Audio Decoder.",
libs = { "mad" },
tags = { "small" },
}

-- GStreamer base
newtemplate{
name = "GStreamer-base",
description = "A streaming media framework.",
libs = { "gstaudio-0.10", "gstcdda-0.10", "gstfft-0.10", "gstinterfaces-0.10", "gstnetbuffer-0.10", "gstpbutils-0.10", "gstriff-0.10", "gstrtp-0.10", "gstrtsp-0.10", "gstsdp-0.10", "gsttag-0.10", "gstvideo-0.10" },
tags = { "unknown-copy" },
}

-- GStreamer core
newtemplate{
name = "GStreamer-core",
description = "A streaming media framework.",
libs = { "gstbase-0.10", "gstcheck-0.10", "gstcontroller-0.10", "gstdataprotocol-0.10", "gstnet-0.10", "gstreamer-0.10" },
tags = { "unknown-copy" },
}


-------------------------------------------------------------
-- File system support
-------------------------------------------------------------

-- fam
newtemplate{
name = "fam",
description = "File alteration monitor.",
libs = { "fam" },
tags = { "common", "small" },
}

-- libacl
newtemplate{
name = "libacl",
description = "Library for handling Access Control Lists.",
libs = { "acl" },
tags = { "common", "small" },
}

-- libattr
newtemplate{
name = "attr",
description = "Library for handling Extended Attributes",
libs = { "attr" },
tags = { "common", "small", "full-config" },
}

-------------------------------------------------------------
-- Security
-------------------------------------------------------------

-- OpenSSL
newtemplate{
name = "OpenSSL",
description = "The Open Source toolkit for SSL/TLS.",
libs = { "ssl", "crypto" },
tags = { "common", "unknown-copy" },
}

-- Cyrus SASL
newtemplate{
name = "sasl",
description = "The Cyrus SASL API.",
libs = { "sasl2" },
tags = { "unknown-copy" },
}

-- gnutls
newtemplate{
name = "gnutls",
description = "Portable library which implements TLS and SSL.",
libs = { "gnutls", "gnutls-extra", "gnutls-openssl", "gnutlsxx" },
tags = { "common" },
}

-- nss
newtemplate{
name = "nss",
description = "Mozilla network security services.",
libs = { "freebl3", "nss3", "nssckbi", "nssdbm3", "nssutil3", "smime3", "softokn3", "ssl3" },
tags = { "unknown-copy" },
}

-- krb
newtemplate{
name = "krb",
description = "Kerberos is a system for authenticating users and services on a network.",
libs = { "des425", "gssapi_krb5", "k5crypto", "krb4", "krb5", "krb5support" },
tags = { "common", "unknown-copy" },
}

-- keyutils
newtemplate{
name = "keyutils",
description = "A set of utilities for managing the key retention facility in the kernel.",
libs = { "keyutils" },
tags = { "common", "small" },
}

-- gcrypt
newtemplate{
name = "gcrypt",
description = "GNU Crypto library.",
libs = { "gcrypt" },
tags = { "common", "small" },
}

-------------------------------------------------------------
-- dbus
-------------------------------------------------------------

-- dbus
newtemplate{
name = "dbus",
description = "Message bus, used for sending messages between applications.",
libs = { "dbus-1" },
tags = { "common", "unknown-copy" },
}

-- dbus-glib
newtemplate{
name = "dbus-glib",
description = "dbus glib binding.",
libs = { "dbus-glib-1" },
tags = { "small" },
}

-- dbus-qt3
newtemplate{
name = "dbus-qt3",
description = "dbus QT 3 binding.",
libs = { "dbus-qt-1" },
tags = { "small" },
}

-------------------------------------------------------------
-- Misc. libraries
-------------------------------------------------------------

-- Bzip2
newtemplate{
name = "Bzip2",
description = "Bzip2 compression library.",
libs = { "bz2" },
tags = { "common", "small" },
}

-- libidn
newtemplate{
name = "libidn",
description = "Support for Internationalized Domain Names.",
libs = { "idn" },
tags = { "unknown-copy" },
}

-- OpenLDAP client
newtemplate{
name = "lber-2.4",
description = "OpenLDAP client libraries.",
libs = { "lber-2.4", "ldap-2.4", "ldap_r-2.4" },
tags = { "unknown-copy" },
}

-- libxml2
newtemplate{
name = "libxml2",
description = "Library for handling xml files.",
libs = { "xml2" },
tags = { "common" },
}

-- expat
newtemplate{
name = "expat",
description = "An XML 1.0 parser written in C.",
libs = { "expat" },
tags = { "common", "small" },
}

-- pcre
newtemplate{
name = "pcre",
description = "Library providing PERL regexp functionality.",
libs = { "pcre", "pcrecpp", "pcreposix" },
tags = { "common", "small" },
}

-- ICU
newtemplate{
name = "ICU",
description = "ICU is a set of libraries that provides robust and full-featured Unicode support.",
libs = { "icudata", "icui18n", "icuio", "icule", "iculx", "icutu", "icuuc" },
tags = { },
}

-- physfs-1.0
newtemplate{
name = "physfs-1.0",
description = "PhysicsFS file abstraction layer for games.",
libs = { "physfs-1.0" },
tags = { "small" },
}

-- popt
newtemplate{
name = "popt",
description = "Library for parsing commandline parameters.",
libs = { "popt" },
tags = { "common", "small" },
notes = "Translation messages won't be automaticly copied when using 'auto copy'."
}

-- python 2.5
newtemplate{
name = "python-2.5",
description = "Interactive high-level object-oriented language (version 2.5).",
libs = { "python2.5" },
tags = { "common", "unknown-copy" },
}

-- sqlite3
newtemplate{
name = "sqlite3",
description = "SQLite is a C library that implements an SQL database engine.",
libs = { "sqlite3" },
tags = { "small" },
}

-- nspr
newtemplate{
name = "nspr",
description = "NetScape Portable Runtime Library",
libs = { "nspr4", "plc4", "plds4" },
tags = { "small" },
}

-- hal
newtemplate{
name = "hal",
description = "Hardware Abstraction Layer.",
libs = { "hal", "hal-storage" },
tags = { "small", "unknown-copy" },
}

-- com_err
newtemplate{
name = "com_err",
description = "A common error-handling mechanism.",
libs = { "com_err", "ss" },
tags = { "small", "unknown-copy" },
}

-- soup-2.4
newtemplate{
name = "soup-2.4",
description = "An HTTP library implementation in C.",
libs = { "soup-2.4" },
tags = { "small" },
}

-- ncurses
newtemplate{
name = "ncurses",
description = "ncurses allows the programmer to write text user interfaces in a terminal-independent manner.",
libs = { "ncurses", "ncursesw", "menu", "menuw", "form", "formw", "ncurses++", "ncurses++w", "panel", "panelw", "tic" },
tags = { "common", "small", "unknown-copy" },
}

-- libnotify
newtemplate{
name = "libnotify",
description = "dbus notifications library.",
libs = { "notify" },
tags = { "common-desktop", "small" },
}

-- libpurple
newtemplate{
name = "libpurple",
description = "Library providing support for various IM protocols.",
libs = { "purple", "purple-client" },
tags = { },
}

-- enchant
newtemplate{
name = "enchant",
description = "Wrapper library for various spell checking engines.",
libs = { "enchant" },
tags = { "small", "unknown-copy" },
}

-- cURL (libcurl)
newtemplate{
name = "cURL",
description = "A free and easy-to-use client-side URL transfer library.",
libs = { "curl" },
tags = { "common", "small" },
}
