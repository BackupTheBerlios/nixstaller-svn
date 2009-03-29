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

#include <QtGui>

#include "nbstandard.h"

QString packageVars[] = { "pkg.enable=", "pkg.name=", "pkg.version=", "pkg.summary=", "pkg.description=",
                            "pkg.license=", "pkg.maintainer=", "pkg.url=" };

CNixstbuildStandard::CNixstbuildStandard()
{
    addPage(createIntro());
    addPage(createFileSelector());
    addPage(createPackageProps());
    addPage(createBuilding());
    addPage(createFinish());
}

QWizardPage *CNixstbuildStandard::createIntro()
{
    QWizardPage *np = new QWizardPage();
    np->setTitle("Introduction");

    QLabel *nlabel = new QLabel("This wizard will guide you through Nixstaller package building process."
                                "It will help you to setup an installer for your project capable of "
                                "generating native packages for users system.\n"
                                "You will be asked to select 3 types of files to include into your project."
                                "Those are binary executable files, application libraries and other data files."
                                "After you have selected files to include into your installer you will be asked"
                                "to specify some data about the application.\n"
                                "To continue the process please click Next.");
    nlabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(nlabel);
    np->setLayout(layout);

    return np;
}

QWizardPage *CNixstbuildStandard::createWelcome()
{
    QWizardPage *np = new QWizardPage();
    np->setTitle("Welcome screen");
    np->setSubTitle("Please enter the text you would like users to see as a welcome screen.");

    welcomeText = new QTextEdit();
    QHBoxLayout *hbl = new QHBoxLayout();
    hbl->addWidget(welcomeText);
    np->setLayout(hbl);

    return np;
}

QWizardPage *CNixstbuildStandard::createFileSelector()
{
    QWizardPage *np = new QWizardPage();
    np->setTitle("File selection");
    np->setSubTitle("Please select files to include to your application.");


    return np;
}

QWizardPage *CNixstbuildStandard::createPackageProps()
{
    QWizardPage *np = new QWizardPage();
    np->setTitle("Package information");
    np->setSubTitle("Specify information required for package generation here.");

    tw = new QTableWidget(8, 2, np);
    tw->setItem(0, 0, new QTableWidgetItem("Name"));
    tw->setItem(1, 0, new QTableWidgetItem("Version"));
    tw->setItem(2, 0, new QTableWidgetItem("Summary"));
    tw->setItem(3, 0, new QTableWidgetItem("Description"));
    tw->setItem(4, 0, new QTableWidgetItem("Group"));
    tw->setItem(5, 0, new QTableWidgetItem("License"));
    tw->setItem(6, 0, new QTableWidgetItem("Maintainer"));
    tw->setItem(7, 0, new QTableWidgetItem("URL"));
    tw->setItem(1, 1, new QTableWidgetItem("1.0.0"));
    tw->setItem(4, 1, new QTableWidgetItem("Application"));
    tw->setItem(5, 1, new QTableWidgetItem("GPLv2"));

    for (int i=0; i<8; i++)
    {
        tw->item(i, 0)->setFlags(Qt::ItemIsEnabled);
    }

    QStringList headers;
    headers << "Porperty" << "Value";
    tw->setHorizontalHeaderLabels(headers);

    QHBoxLayout *hl = new QHBoxLayout();

    hl->addWidget(tw);
    np->setLayout(hl);

    return np;
}

QWizardPage *CNixstbuildStandard::createBuilding()
{
    building = new CNBBuilding();
    connect(building, SIGNAL(initialize()), this, SLOT(buildPackage()));
    return building;
}

QWizardPage *CNixstbuildStandard::createFinish()
{
    QWizardPage *np = new QWizardPage();
    np->setTitle("Installer building completed");

    QLabel *info = new QLabel("Your package has bin successfuly built and placed at your home path.");
    QHBoxLayout *hl = new QHBoxLayout();

    hl->addWidget(info);
    np->setLayout(hl);

    return np;
}

void CNixstbuildStandard::addBinFile()
{
    //binfilelb->addItems(QFileDialog::getOpenFileNames(this, "Select binary executables to include"));
}

void CNixstbuildStandard::addLibFile()
{
    //libfilelb->addItems(QFileDialog::getOpenFileNames(this, "Select application libraries to include"));
}

void CNixstbuildStandard::addDatFile()
{
    //datfilelb->addItem(QFileDialog::getExistingDirectory(this, "Select data directory to include"));
}

void CNixstbuildStandard::delBinFile()
{
    //binfilelb->takeItem(binfilelb->currentRow());
}

void CNixstbuildStandard::delLibFile()
{
    //libfilelb->takeItem(libfilelb->currentRow());
}

void CNixstbuildStandard::delDatFile()
{
    //datfilelb->takeItem(datfilelb->currentRow());
}

void CNixstbuildStandard::buildPackage()
{
    // Generate project directory by using genprojdir.sh
    QString genproj = "genprojdir.sh -n ";
    genproj.append(tw->item(0, 1)->text());
    genproj.append(" /tmp/nbtemp");
    system(genproj.toStdString().c_str());

    // Save welcome screen
    QFile wfile("/tmp/nbtemp/welcome");
    wfile.open(QIODevice::Text); // TODO: Handle error
    QTextStream wstream(&wfile);
    wstream << welcomeText->toPlainText();
    wstream.flush();
    wfile.close();

    // Copy user specified files to their directories
    /*
    genproj = "/tmp/nbtemp/files_all/bin/";

    for (int i=0; i<binfilelb->count(); i++)
    {
        QFileInfo cfile(binfilelb->item(i)->text());
        QFile::copy(binfilelb->item(i)->text(), genproj+cfile.fileName());
    } */

/*
    // Generate package.lua file
    QFile packageFile("/tmp/nbtemp/package.lua");
    packageFile.open(QIODevice::Text);
    QTextStream packageStream(&packageFile);

    for (int i = 0; i < 8; i++)
    {
        packageStream << packageVars[i];
        packageStream << "\"" << tw->item(0, i)->text() << "\"\n";
    }
    packageStream << "pkg.enabled = true";
    packageStream << "pkg.release = \"1\"";
    packageStream << "pkg.bins = {";

    for (int i=0; i<binfilelb->count(); i++)
    {
        QFileInfo cfile(binfilelb->item(i)->text());
        packageStream << "\"" << "bin/"+cfile.fileName() << "\"";
        if (i != binfilelb->count()-2)
            packageStream << ", ";
    }
    packageStream << "}\n";
    packageStream.flush();
    packageFile.close();
*/
    building->done();

}

CNBBuilding::CNBBuilding()
{
    setTitle("Building");
    setSubTitle("At the moment your package is being built");
    complete = false;
    emit completeChanged();
}

void CNBBuilding::done()
{
    complete = true;
    emit completeChanged();
}

void CNBBuilding::initializePage()
{
    emit initialize();
}

bool CNBBuilding::isComplete() const
{
    return complete;
}
