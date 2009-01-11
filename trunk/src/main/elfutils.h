/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.
    
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version. 
    
    This program is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE. See the GNU General Public License for more details. 
    
    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <vector>
#include <libelf/libelf.h>
#include <libelf/gelf.h>

class CElfWrapper
{
public:
    struct SSymData
    {
        std::string name, version, binding;
        bool undefined;
        SSymData(const std::string &n, const std::string &b,
                 bool u) : name(n), binding(b), undefined(u) {}
    };
    
    struct SVerSymNeedData
    {
        std::string name, flags, lib;
        SVerSymNeedData(const std::string &n, const std::string &f,
                        const std::string &l) : name(n), flags(f), lib(l) { }
    };

private:
    Elf *m_pElf;
    int m_iFD;
    
    typedef std::vector<SSymData> TSymVec;
    typedef std::map<int, std::string> TSymverMap;
    typedef std::vector<std::string> TSymverDefVec;
    typedef std::vector<SVerSymNeedData> TSymverNeedVec;
    typedef std::vector<std::string> TNeededLibs;
    
    TSymVec m_Symbols;
    TSymverMap m_SymVerDefMap, m_SymVerNeedMap;
    TSymverDefVec m_SymVerDef;
    TSymverNeedVec m_SymVerNeed;
    TNeededLibs m_NeededLibs;
    std::string m_RPath;
    
    void ReadSymbols(Elf_Scn *section);
    void ReadVerDef(Elf_Scn *section);
    void ReadVerNeed(Elf_Scn *section);
    void MapVersions(Elf_Scn *section);
    void ReadDyn(Elf_Scn *section);
    void CleanSymbols(void);
    
public:
    CElfWrapper(const std::string &file);
    ~CElfWrapper(void) { Close(); }
    
    const SSymData &GetSym(TSTLVecSize n) { return m_Symbols.at(n); }
    TSTLVecSize GetSymSize(void) const { return m_Symbols.size(); }
    
    const std::string &GetSymVerDef(TSTLVecSize n) { return m_SymVerDef.at(n); }
    TSTLVecSize GetSymVerDefSize(void) const { return m_SymVerDef.size(); }

    const SVerSymNeedData &GetSymVerNeed(TSTLVecSize n) { return m_SymVerNeed.at(n); }
    TSTLVecSize GetSymVerNeedSize(void) const { return m_SymVerNeed.size(); }
    
    const std::string &GetNeeded(TSTLVecSize n) { return m_NeededLibs.at(n); }
    TSTLVecSize GetNeededSize(void) const { return m_NeededLibs.size(); }
    
    const std::string &GetRPath(void) { return m_RPath; }

    void Close(void) { if (m_pElf) elf_end(m_pElf); if (m_iFD) close(m_iFD); }
};
