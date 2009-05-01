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

#include <QButtonGroup>
#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
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
    addPage(createDestDirPage());
    addPage(createDeskEntryPage());
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
    return (preConfigPage = new CPreConfigPage(this));
}

QWizardPage *CRunGenerator::createInstScreenPage()
{
    return (instScreenPage = new CInstScreenPage(this));
}

QWizardPage *CRunGenerator::createDestDirPage()
{
    return (destDirPage = new CDestDirPage(this));
}

QWizardPage *CRunGenerator::createDeskEntryPage()
{
    return (deskEntryPage = new CDeskEntryPage(this));
}

QString CRunGenerator::getRun()
{
    const char *ret = NULL;
    NLua::CLuaFunc func("genRun");
    assert(func);
    if (func)
    {
        func << preConfigPage->getMode() << preConfigPage->pkgMode();

        // Fill in screen tables
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

        func << NLua::CLuaTable(screenlist.begin(), screenlist.end()) << custtab;

        func << destDirPage->getDestDir();

        // Fill in desktop menu entries
        
        CDesktopEntryWidget::entryvec entries;
        deskEntryPage->getEntries(entries);
        NLua::CLuaTable deskTab;
        n = 1;
        for (CDesktopEntryWidget::entryvec::iterator it=entries.begin();
             it!=entries.end(); it++, n++)
        {
            NLua::CLuaTable tabItem;
            tabItem["name"] << it->name;
            tabItem["exec"] << it->exec;
            tabItem["categories"] << it->categories;
            tabItem["icon"] << it->icon;
            deskTab[n] << tabItem;
        }

        func << deskTab;
        
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

    // UNDONE: Validate mode checkboxes (atleast 1 checked)
}

void CPreConfigPage::initializePage()
{
    // UNDONE: Set fields
}

const char * CPreConfigPage::getMode(void) const
{
    bool att = attCheckBox->isChecked(), unatt = unattCheckBox->isChecked();
    if (att && unatt)
        return "both";
    else if (att)
        return "attended";
    else // unatt
        return "unattended";
}

bool CPreConfigPage::pkgMode(void) const
{
    return pkgCheckBox->isChecked();
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


CDestDirPage::CDestDirPage(QWidget *parent) : QWizardPage(parent), firstTime(true)
{
    setTitle("Install destination");
//     setSubTitle("<qt>In this screen you can specify the destination that will be used when the installation files are extracted. Selecting the temporary direction is usefull if the installer needs to prepare (eg. compile) the installation files. The temporary package directory is used only by Package Mode. When Package Mode is enabled it's most common to select this option.<br><b>Note:</b> When the <i>SelectDirScreen</i> is used the user is able to customize the destination directory.</qt>");

    QVBoxLayout *vbox = new QVBoxLayout(this);

    QLabel *label = new QLabel("<qt>In this screen you can specify the destination path for the installation files.<br><br>If the files need to be prepared in some way (eg. compiling) you should select the temporary directory option.<br><br>The temporary package directory option is used only by Package Mode. When your files need to be directly installed (no preperation) and Package Mode is enabled you should select this option.<br><br><b>Note:</b> When the <i>SelectDirScreen</i> is used the user is able to change the destination directory specified here.</qt>");
    label->setWordWrap(true);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    vbox->addWidget(label);

    QGridLayout *grid = new QGridLayout;
    vbox->addLayout(grid);

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    
    grid->addWidget(homeRadio = new QRadioButton, 1, 0);
    buttonGroup->addButton(homeRadio);
    grid->addWidget(new QLabel("User's home directory"), 1, 1);

    grid->addWidget(manualRadio = new QRadioButton, 2, 0);
    buttonGroup->addButton(manualRadio);
    connect(manualRadio, SIGNAL(toggled(bool)), this, SLOT(toggledManual(bool)));
    grid->addWidget(new QLabel("Specify manually"), 2, 1);
    grid->addWidget(manualEdit = new QLineEdit, 3, 1);
    manualEdit->setEnabled(false);
    
    grid->addWidget(tempRadio = new QRadioButton, 4, 0);
    buttonGroup->addButton(tempRadio);
    grid->addWidget(new QLabel("Temporary directory"), 4, 1);

    grid->addWidget(pkgRadio = new QRadioButton, 5, 0);
    buttonGroup->addButton(pkgRadio);
    grid->addWidget(new QLabel("Temporary package directory"), 5, 1);
}

void CDestDirPage::toggledManual(bool checked)
{
    manualEdit->setEnabled(checked);
}

void CDestDirPage::initializePage()
{
    if (firstTime)
    {
        firstTime = false;
        if (field("pkgCheckBox").toBool())
            pkgRadio->setChecked(true);
        else
            homeRadio->setChecked(true);
    }
}

std::string CDestDirPage::getDestDir(void) const
{
    // These strings are handled inside rungen.lua
    if (homeRadio->isChecked())
        return "home";
    else if (manualRadio->isChecked())
        return manualEdit->text().toStdString();
    else if (tempRadio->isChecked())
        return "temporary";
    else // package dir
        return "package";
}

CDeskEntryPage::CDeskEntryPage(QWidget *parent) : QWizardPage(parent)
{
    setTitle("Desktop menu entries");
    setSubTitle("In this screen you can define any desktop menu entries (.desktop files) that allow the user to easily launch any executables.");
    
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(deskEntryWidget = new CDesktopEntryWidget);
}

void CDeskEntryPage::getEntries(CDesktopEntryWidget::entryvec &entries)
{
    deskEntryWidget->getEntries(entries);
}
