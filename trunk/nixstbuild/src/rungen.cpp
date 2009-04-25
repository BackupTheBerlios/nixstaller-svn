/***************************************************************************
 *   Copyright (C) 2009 by Rick Helmus / InternetNightmare   *
 *   rhelmus_AT_gmail.com / internetnightmare_AT_gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <assert.h>

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>
#include <QWizardPage>

#include "main/lua/lua.h"
#include "main/lua/luafunc.h"
#include "main/lua/luatable.h"
#include "rungen.h"

CRunGenerator::CRunGenerator(QWidget *parent,
                             Qt::WindowFlags flags) : QWizard(parent, flags)
{
    addPage(createIntro());
    addPage(createPreConfig());
    addPage(createInstScreenPage());
}

QWizardPage *CRunGenerator::createIntro()
{
    QWizardPage *page = new QWizardPage(this);
    page->setTitle("Lua run time code generator");
    
    QVBoxLayout *vbox = new QVBoxLayout(page);
    vbox->addWidget(new QLabel("<qt>This wizard will help you in setting up a new run.lua file.<br><b>Note</b>: Any existing code will be overwritten!</qt>"));

    return page;
}

QWizardPage *CRunGenerator::createPreConfig()
{
    return new CPreConfigPage(this);
}

QWizardPage *CRunGenerator::createInstScreenPage()
{
    return (instScreenPage = new CInstScreenPage(this));
}

QString CRunGenerator::getRun()
{
    const char *ret = NULL;
    NLua::CLuaFunc func("genRun");
    assert(func);
    if (func)
    {
        TStringVec screenlist;
        CInstScreenWidget::screenvec custscreens;
        instScreenPage->getScreens(screenlist, custscreens);
        
        NLua::CLuaTable custtab;
        int n = 1;
        for (CInstScreenWidget::screenvec::iterator it=custscreens.begin();
             it!=custscreens.end(); it++, n++)
        {
            NLua::CLuaTable tabItem;
            tabItem["variable"] << it->variable;
            tabItem["title"] << it->title;
            tabItem["genCanAct"] << it->canActivate;
            tabItem["genAct"] << it->activate;
            tabItem["genUpdate"] << it->update;

            NLua::CLuaTable wtab;
            int n2 = 1;
            for (CNewScreenDialog::widgetvec::iterator it2=it->widgets.begin();
                 it2!=it->widgets.end(); it2++, n2++)
            {
                NLua::CLuaTable wtabItem;
                wtabItem["func"] << it2->func;
                wtabItem["variable"] << it2->variable;
                wtabItem["args"] << NLua::CLuaTable(it2->args.begin(), it2->args.end());
                wtab[n2] << wtabItem;
            }

            tabItem["widgets"] << wtab;
            
            custtab[n] << tabItem;
        }
        func << "both" << true << NLua::CLuaTable(screenlist.begin(), screenlist.end()) << custtab;
        func(1);
        func >> ret;
    }
    return ret;
}

CPreConfigPage::CPreConfigPage(QWidget *parent) : QWizardPage(parent)
{
    setTitle("Basic settings");
    setSubTitle("In this screen you can set a few basic settings which influence the overal file layout and further wizard screens.");
    
    QFormLayout *form = new QFormLayout(this);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(attCheckBox = new QCheckBox("Attended"));
    vbox->addWidget(unattCheckBox = new QCheckBox("Unattended"));
    form->addRow("Installation mode", vbox);

    form->addRow("Package Mode", pkgCheckBox = new QCheckBox);
    registerField("pkgCheckBox", pkgCheckBox);
}

void CPreConfigPage::initializePage()
{
    // UNDONE: Set fields
}


CInstScreenPage::CInstScreenPage(QWidget *parent) : QWizardPage(parent)
{
    setTitle("Installation screen setup");
    setSubTitle("In this screen you can specify which installation screens should be shown and in which order. It's also possible to generate new installation screens.");

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(instScreenWidget = new CInstScreenWidget);
}

void CInstScreenPage::initializePage()
{
    instScreenWidget->setDefaults(field("pkgCheckBox").toBool());
}

void CInstScreenPage::getScreens(TStringVec &screenlist,
                                 CInstScreenWidget::screenvec &customs)
{
    instScreenWidget->getScreens(screenlist, customs);
}
