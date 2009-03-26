/***************************************************************************
 *   Copyright (C) 2007 by InternetNightmare   *
 *   internetnightmare@gmail.com   *
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

#ifndef _NBSTANDARD_H
#define _NBSTANDARD_H

#include <QObject>
#include <QWizard>

class QTextEdit;
class QListWidget;
class QWizardPage;
class QTableWidget;

class CNBBuilding: public QWizardPage
{
    Q_OBJECT

    bool complete;
public:
    CNBBuilding();
    void initializePage();
    bool isComplete() const;
    void done();

signals:
    void initialize();
};

class CNixstbuildStandard :public QWizard
{
    Q_OBJECT

public:
    CNixstbuildStandard();

private:
    QTextEdit *welcomeText;

    QTableWidget *tw;

    CNBBuilding *building;

    QWizardPage *createIntro();
    QWizardPage *createWelcome();
    QWizardPage *createFileSelector();
    QWizardPage *createPackageProps();
    QWizardPage *createBuilding();
    QWizardPage *createFinish();

private slots:
    void addBinFile();
    void addLibFile();
    void addDatFile();
    void delBinFile();
    void delLibFile();
    void delDatFile();

    void buildPackage();
};

#endif //_NBSTANDARD_H
