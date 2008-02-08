/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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

class CElfSymbolWrapper
{
public:
    struct SSymData
    {
        std::string name, version, binding;
        bool undefined;
        SSymData(const std::string &n, const std::string &b,
                 bool u) : name(n), binding(b), undefined(u) {}
    };
    
    struct SVerSymData
    {
        std::string name, flags;
        SVerSymData(const std::string &n, const std::string &f) : name(n), flags(f) { }
    };

private:
    Elf *m_pElf;
    int m_iFD;
    
    typedef std::vector<SSymData> TSymVec;
    typedef std::map<int, std::string> TSymverMap;
    typedef std::vector<SVerSymData> TSymverVec;
    
    TSymVec m_Symbols;
    TSymverMap m_SymVerDefMap, m_SymVerNeedMap;
    TSymverVec m_SymVerDef, m_SymVerNeed;
    
    void ReadSymbols(Elf_Scn *section);
    void ReadVerDef(Elf_Scn *section);
    void ReadVerNeed(Elf_Scn *section);
    void MapVersions(Elf_Scn *section);
    
public:
    CElfSymbolWrapper(const std::string &file);
    ~CElfSymbolWrapper(void) { if (m_pElf) elf_end(m_pElf); if (m_iFD) close(m_iFD); }
    
    const SSymData &GetSym(TSTLVecSize n) { return m_Symbols.at(n); }
    TSTLVecSize GetSymSize(void) const { return m_Symbols.size(); }
    
    const SVerSymData &GetSymVerDef(TSTLVecSize n) { return m_SymVerDef.at(n); }
    TSTLVecSize GetSymVerDefSize(void) const { return m_SymVerDef.size(); }

    const SVerSymData &GetSymVerNeed(TSTLVecSize n) { return m_SymVerNeed.at(n); }
    TSTLVecSize GetSymVerNeedSize(void) const { return m_SymVerNeed.size(); }
};
