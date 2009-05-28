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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "main.h"
#include "elfutils.h"

namespace {

#ifdef HAVE_ELF32_VERDEF

// Functions copied from RPM (lib/rpmds.c, author unknown...)

GElf_Verdef *gelf_getverdef(Elf_Data *data, int offset)
{
    return (GElf_Verdef *) ((char *) data->d_buf + offset);
}

GElf_Verdaux *gelf_getverdaux(Elf_Data *data, int offset)
{
    return (GElf_Verdaux *) ((char *) data->d_buf + offset);
}

GElf_Verneed *gelf_getverneed(Elf_Data *data, int offset)
{
    return (GElf_Verneed *) ((char *) data->d_buf + offset);
}

GElf_Vernaux *gelf_getvernaux(Elf_Data *data, int offset)
{
    return (GElf_Vernaux *) ((char *) data->d_buf + offset);
}

GElf_Half *gelf_getversym(Elf_Data *data, int offset)
{
    return (GElf_Half *) ((char *) data->d_buf + offset);
}

#endif

}

// -------------------------------------
// Frontend class for symbols using libelf/gelf
// -------------------------------------

CElfWrapper::CElfWrapper(const std::string &file) : m_pElf(NULL), m_iFD(0), m_iMachine(EM_NONE)
{
    if (elf_version(EV_CURRENT) == EV_NONE)
        throw Exceptions::CExElf("Elf library out of date.");
    
    m_iFD = Open(file.c_str(), O_RDONLY);
    m_pElf = elf_begin(m_iFD, ELF_C_READ, NULL);
    
    if (!m_pElf)
    {
        close(m_iFD);
        m_iFD = 0;
        throw Exceptions::CExElf("Could not open ELF descriptor.");
    }
    
    GElf_Ehdr elfhdr;
    if (gelf_getehdr(m_pElf, &elfhdr) == 0)
    {
        elf_end(m_pElf);
        close(m_iFD);
        m_pElf = NULL;
        m_iFD = 0;
        throw Exceptions::CExElf("Could not open ELF descriptor.");
    }
    
    m_iMachine = elfhdr.e_machine;
    int c = gelf_getclass(m_pElf);
    if (c == ELFCLASS32)
        m_Class = "32";
    else if (c == ELFCLASS64)
        m_Class = "64";
    else
        m_Class = "unknown";
    
    Elf_Scn *section = NULL;
    Elf_Scn *sym = NULL, *verdef = NULL, *verneed = NULL, *versym = NULL;
    
    while ((section = elf_nextscn(m_pElf, section)) != NULL)
    {
        GElf_Shdr shdr;
        gelf_getshdr(section, &shdr);
        
        switch (shdr.sh_type)
        {
            case SHT_DYNSYM:
                sym = section;
                break;
#ifdef HAVE_ELF32_VERDEF                
            case SHT_GNU_verdef:
                verdef = section;
                break;
            case SHT_GNU_verneed:
                verneed = section;
                break;
            case SHT_GNU_versym:
                versym = section;
                break;
#endif
            case SHT_DYNAMIC:
                ReadDyn(section);
                break;
        }
    }
    
    if (sym)
        ReadSymbols(sym);
    if (verdef)
        ReadVerDef(verdef);
    if (verneed)
        ReadVerNeed(verneed);
    if (versym)
        MapVersions(versym);
    
    CleanSymbols();
    
    elf_end(m_pElf);
    close(m_iFD);
    m_pElf = NULL;
    m_iFD = 0;
}

void CElfWrapper::ReadSymbols(Elf_Scn *section)
{
    Elf_Data *data = NULL;
    GElf_Shdr shdr;
    gelf_getshdr(section, &shdr);

    while ((data = elf_getdata(section, data)) != NULL)
    {
        int count = shdr.sh_size / shdr.sh_entsize;
        for (int i=0; i<count; i++)
        {
            GElf_Sym sym;
            std::string binding;
            
            gelf_getsym(data, i, &sym);
            
            switch (ELF32_ST_BIND(sym.st_info))
            {
                case STB_LOCAL: binding = "LOCAL"; break;
                case STB_GLOBAL: binding = "GLOBAL"; break;
                case STB_WEAK: binding = "WEAK"; break;
                default: binding = "OTHER"; break;
            }
            
            const char *name = elf_strptr(m_pElf, shdr.sh_link, sym.st_name);
            
            // HACK: We basicly want to add any symbols, even if they have invalid names.
            // This is so to keep the vector range the same as the one from libelf. When
            // everything is processed, the vector will be cleaned from any nameless symbols.
            if (!name || !name[0])
                name = "";
            
            m_Symbols.push_back(SSymData(name, binding, (sym.st_shndx == SHN_UNDEF)));
        }
    }
}

void CElfWrapper::ReadVerDef(Elf_Scn *section)
{
#ifdef HAVE_ELF32_VERDEF
    Elf_Data *data = NULL;
    GElf_Shdr shdr;
    gelf_getshdr(section, &shdr);

    while ((data = elf_getdata(section, data)) != NULL)
    {
        int offset = 0;
        for (unsigned i=0; i<shdr.sh_info; i++)
        {
            GElf_Verdef *def = gelf_getverdef(data, offset);
            if (!def)
                break;
            
            if (def->vd_version != VER_DEF_CURRENT)
                continue; // UNDONE?
                
            int auxoffset = offset + def->vd_aux;
            GElf_Verdaux *aux = gelf_getverdaux(data, auxoffset);
            if (aux)
            {
                char *s = elf_strptr(m_pElf, shdr.sh_link, aux->vda_name);
                if (!s)
                    break;
                
                m_SymVerDefMap[def->vd_ndx] = s;
                m_SymVerDef.push_back(s);
            }
            offset += def->vd_next;
        }
    }
#endif
}

void CElfWrapper::ReadVerNeed(Elf_Scn *section)
{
#ifdef HAVE_ELF32_VERDEF
    Elf_Data *data = NULL;
    GElf_Shdr shdr;
    gelf_getshdr(section, &shdr);

    while ((data = elf_getdata (section, data)) != NULL)
    {
        int offset = 0;
        for (unsigned i=0; i<shdr.sh_info; i++ )
        {
            GElf_Verneed *need = gelf_getverneed(data, offset);
            if (!need)
                break;

            if (need->vn_version != VER_NEED_CURRENT)
                continue; // UNDONE?

            int auxoffset = offset + need->vn_aux;
            
            char *libname = elf_strptr(m_pElf, shdr.sh_link, need->vn_file);
            assert(libname != NULL);
            if (!libname)
                continue;
            
            for (unsigned j=0; j<need->vn_cnt; j++)
            {
                GElf_Vernaux *aux = gelf_getvernaux(data, auxoffset);
                if (!aux)
                    break;

                char *s = elf_strptr(m_pElf, shdr.sh_link, aux->vna_name);
                if (!s)
                    break;
                
                m_SymVerNeedMap[aux->vna_other] = s;
                
                const char *f = "";
                if (aux->vna_flags & VER_FLG_BASE)
                    f = "BASE";
                else if (aux->vna_flags & VER_FLG_WEAK)
                    f = "WEAK";

                m_SymVerNeed.push_back(SVerSymNeedData(s, f, libname));

                auxoffset += aux->vna_next;
            }
            offset += need->vn_next;
        }
    }
#endif
}

void CElfWrapper::MapVersions(Elf_Scn *section)
{
#ifdef HAVE_ELF32_VERDEF
    Elf_Data *data = NULL;
    GElf_Shdr shdr;
    gelf_getshdr(section, &shdr);

    int x = 1;
    while ((data = elf_getdata (section, data)) != NULL)
    {
        int count = shdr.sh_size / shdr.sh_entsize, offset = 0;
        for (int i=0; i<count; i++)
        {
            GElf_Half *sym = gelf_getversym(data, offset);
            if (!sym)
                break;
            
            assert(m_Symbols.at(i*x).version.empty());
            
            if (m_Symbols.at(i*x).undefined)
                m_Symbols[i*x].version = m_SymVerNeedMap[*sym];
            else
                m_Symbols[i*x].version = m_SymVerDefMap[*sym];
            
            offset += shdr.sh_entsize;
        }
        x++;
    }
#endif
}

void CElfWrapper::ReadDyn(Elf_Scn *section)
{
    Elf_Data *data = NULL;
    GElf_Shdr shdr;
    gelf_getshdr(section, &shdr);

    while ((data = elf_getdata (section, data)) != NULL)
    {
        int count = shdr.sh_size / shdr.sh_entsize;
        for (int i=0; i<count; i++)
        {
            GElf_Dyn *dyn, sdyn;
            dyn = gelf_getdyn(data, i, &sdyn);

            if (!dyn)
                break;

            if (dyn->d_tag == DT_NEEDED)
            {
                char *s = elf_strptr(m_pElf, shdr.sh_link, dyn->d_un.d_val);
                
                if (!s)
                    break;
                
                m_NeededLibs.push_back(s);
            }
            else if ((dyn->d_tag == DT_RPATH)
#ifdef DT_RUNPATH            
                     || (dyn->d_tag == DT_RUNPATH)
#endif
                    )
            {
                char *s = elf_strptr(m_pElf, shdr.sh_link, dyn->d_un.d_val);
                if (s)
                {
                    if (!m_RPath.empty())
                        m_RPath += ":";
                    m_RPath += s;
                }
            }
        }
    }
}

void CElfWrapper::CleanSymbols()
{
    TSymVec::iterator it=m_Symbols.begin();
    while (it != m_Symbols.end())
    {
        if (it->name.empty())
            it = m_Symbols.erase(it);
        else
            it++;
    }
}

void CElfWrapper::Close()
{
    if (m_pElf)
        elf_end(m_pElf);
    if (m_iFD)
        close(m_iFD);
}
