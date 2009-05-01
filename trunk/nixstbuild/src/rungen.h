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

#ifndef RUNGEN_H
#define RUNGEN_H

#include <QWizard>
#include <QWizardPage>

#include "main/main.h"
#include "deskentrywidget.h"
#include "instscreenwidget.h"

class QCheckBox;
class QRadioButton;

class CDestDirPage;
class CDeskEntryPage;
class CDesktopEntryWidget;
class CInstScreenPage;
class CInstScreenWidget;
class CPreConfigPage;

class CRunGenerator: public QWizard
{
    QWizardPage *createIntro(void);
    QWizardPage *createPreConfig(void);
    QWizardPage *createInstScreenPage(void);
    QWizardPage *createDestDirPage(void);
    QWizardPage *createDeskEntryPage(void);

    CPreConfigPage *preConfigPage;
    CInstScreenPage *instScreenPage;
    CDestDirPage *destDirPage;
    CDeskEntryPage *deskEntryPage;
    
public:
    CRunGenerator(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    QString getRun(void);
};

class CPreConfigPage: public QWizardPage
{
    QCheckBox *attCheckBox, *unattCheckBox, *pkgCheckBox;

public:
    CPreConfigPage(QWidget *parent = 0);

    virtual void initializePage(void);

    const char *getMode(void) const;
    bool pkgMode(void) const;
};

class CInstScreenPage: public QWizardPage
{
    CInstScreenWidget *instScreenWidget;
        
public:
    CInstScreenPage(QWidget *parent = 0);
    
    virtual void initializePage(void);
    
    void getScreens(TStringVec &screenlist, CInstScreenWidget::screenvec &customs);
};

class CDestDirPage: public QWizardPage
{
    Q_OBJECT
    
    QRadioButton *homeRadio, *manualRadio, *tempRadio, *pkgRadio;
    QLineEdit *manualEdit;
    bool firstTime;

private slots:
    void toggledManual(bool checked);
    
public:
    CDestDirPage(QWidget *parent = 0);

    virtual void initializePage(void);

    std::string getDestDir(void) const;
};

class CDeskEntryPage: public QWizardPage
{
    CDesktopEntryWidget *deskEntryWidget;
    
public:
    CDeskEntryPage(QWidget *parent = 0);
    
    void getEntries(CDesktopEntryWidget::entryvec &entries);
};

#endif
