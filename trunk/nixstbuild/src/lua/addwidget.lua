--  ***************************************************************************
--  *   Copyright (C) 2009 by Rick Helmus / InternetNightmare                 *
--  *   rhelmus_AT_gmail.com / internetnightmare_AT_gmail.com                 *
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

-- UNDONE: Image widget
-- Note: fields need to be in function argument order!
widgetProperties = {
["Checkbox"] = { func = "addcheckbox", fields = {
    { name = "Title", type = "string" },
    { name = "Options", type = "list", listtype = "multi" }}
},
["Config menu"] = { func = "addcfgmenu", fields = {
    { name = "Title", type = "string" }}
},
["Directory selector"] = { func = "adddirselector", fields = {
    { name = "Title", type = "string" },
    { name = "Value (empty for user's home dir.)", type = "string", emptynil = true }}
},
["Input field"] = { func = "addinput", fields = {
    { name = "Label", type = "string" },
    { name = "Title", type = "string" },
    { name = "Value", type = "string" },
    { name = "Max characters", type = "int", default = 1024 },
    { name = "Type", type = "choice", options = { "string", "number", "float" }, default = 1 }}
},
["Menu"] = { func = "addmenu", fields = {
    { name = "Title", type = "string" },
    { name = "Options", type = "list", listtype = "single" }}
},
["Progressbar"] = { func = "addprogressbar", fields = {
    { name = "Title", type = "string" }}
},
["Radio button"] = { func = "addradiobutton", fields = {
    { name = "Title", type = "string" },
    { name = "Options", type = "list", listtype = "single" }}
},
["Screen end marker"] = { func = "addscreenend" },
["Text field"] = { func = "addtextfield", fields = {
    { name = "Title", type = "string" },
    { name = "Wrap lines", type = "bool", default = false },
    { name = "Widget size", type = "choice", options = { "small", "medium", "big" }, default = 2 }}
},
["Text label"] = { func = "addlabel", fields = {
    { name = "Text", type = "string" }}
},
}
