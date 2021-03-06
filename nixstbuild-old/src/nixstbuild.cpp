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

#include <string>
#include <map>
#include <iostream>

using namespace std;

#include <QtGui>
#include "bdialog.h"
#include "nixstbuild.h"
#include "ui_textfedit.h"
#include "luahl.h"

#include <QDir>
#include <QDialog>
#include <QToolTip>
#include <QProcess>
#include <QDirModel>
#include <QTextEdit>
#include <QListView>
#include <QSplitter>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QBoxLayout>
#include <QTabWidget>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QScrollArea>

#include "sdialog.h"
#include "rungen.h"
#include "cvaroutput.h"

var_s pkgvars[] = {
{ "pkg.enable", "Enable package building", VT_BOOLEAN, "false", 0 },
{ "pkg.name", "Package name", VT_STRING, "", 0 },
{ "pkg.version", "Package version", VT_STRING, "1.0", 0 },
{ "pkg.release", "Package release", VT_STRING, "1", 0 },
{ "pkg.maintainer", "Package Maintainer", VT_STRING, "You <you@you.com>", 0},
{ "pkg.url", "URL", VT_STRING, "http://", 0},
{ "pkg.bins", "Package Binaries", VT_LIST, "", 0},
{ "pkg.license", "Software License", VT_STRING, "GPLv2", 0},
{ "pkg.description", "Description", VT_MULTILINE, "Nixstaller generated package", 0},
{ "pkg.summary", "Summary", VT_STRING, "Nixstaller generated package", 0},
{ "pkg.group", "Package Group", VT_STRING, "File", 0},
{ "pkg.destdir", "Destination DIrectory", VT_STRING, "", 0},
{ "pkg.bindir", "Binary Directory", VT_STRING, "", 0},
{ "pkg.register", "Register", VT_BOOLEAN, "true", 0},
{ "pkg.setkdeenv", "Set KDE environment", VT_BOOLEAN, "false", 0},
{ "pkg.addbinopts", "App run parameters", VT_FUNC2, "Bin,Arg", 0},
{ "pkg.binenv", "Binary Environment", VT_FUNC2, "Var,Value", 0 },
{ 0, 0, VT_NULL, 0 }
};

map<QString, int> iimap;

const char *templates[] = {
    "cfgscreen:addcfgmenu(\"title\")",
    "cfgscreen:addcheckbox(\"title\", {\"name1\"})",
    "cfgscreen:adddirselector()",
    "cfgscreen:addinput(\"label\")",
    "cfgscreen:addradiobutton(\"title\", {\"opt1\", \"opt2\"})",
    "cfgscreen:addimage(\"\")",
    "cfgscreen:addlabel(\"text\")",
    "cfgscreen:addmenu(\"title\", {\"entry1\"})",
    "cfgscreen:addprogressbar()",
    "cfgscreen:addtextfield("", true)"
};

const char *template_tooltips[] = {
    "cfgscreen:addcfgmenu( title )",
    "cfgscreen:addcheckbox( title, { checkbox1, checkbox2, ... } )",
    "cfgscreen:adddirselector( title, dir ) <br><i>dir</i> is optional, default value is users home directory.",
    "cfgscreen:addinput( label, maxchars, value, type ) <br>Types:<br> <i>\"string\"</i><br> <i>\"number\"</i><br> <i>\"float\"</i>",
    "cfgscreen:addradiobutton(\"title\", {\"opt1\", \"opt2\"} )",
    "cfgscreen:addimage(filename)",
    "cfgscreen:addlabel(\"caption\")",
    "cfgscreen:addmenu(\"title\", {\"entry1\", ...})",
    "cfgscreen:addprogressbar(title) <br><i>title</i> - optional",
    "cfgscreen:addtextfield(\"\", w) <br> <i>w</i> - word wrapping enabled true/false<br>Both parameters optional"
};

nixstbuild::nixstbuild()
{
    setWindowTitle("Nixstbuild");

    msplitter = new QSplitter;
    initMainControls();
    setCentralWidget(msplitter);
    tabs = new QTabWidget;
    msplitter->addWidget(folderview);
    msplitter->addWidget(tabs);
    msplitter->setStretchFactor(0, 1);
    msplitter->setStretchFactor(1, 3);

    addFileTab();
    addConfigTab();
    addRunTab();
    addPackagesTab();
    addTextTabs();

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    readSettings();
}

void nixstbuild::initMainControls()
{
    qd = new QDir("/");
    system("rm -rf /tmp/.nbtemp");
    qd->cd("/tmp");
    qd->mkdir("./.nbtemp");
    qd->cd("/tmp/.nbtemp");

    qdmodel = new QDirModel();
    folderview = new QTreeView();

    writeConfig();

    folderview->setModel(qdmodel);
    folderview->setRootIndex(qdmodel->index(qd->absolutePath()));

    folderview->setColumnHidden(1, true);
    folderview->setColumnHidden(2, true);
    folderview->setColumnHidden(3, true);

    folderview->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(folderview, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(fvCustomContext(QPoint)));
}

void nixstbuild::closeEvent(QCloseEvent *event)
{

}

void nixstbuild::newFile()
{
    system("rm -rf /tmp/.nbtemp");
    qd->cd("/tmp");
    qd->mkdir("./.nbtemp");
    qd->cd("/tmp/.nbtemp");

    writeConfig();
    //statusBar()->showMessage("License saved", 5000);
    reloadView();

    //qdmodel->refresh(qdmodel->index(qd->absolutePath()));
    //cout << qd->absolutePath().toStdString() << endl;
}

bool nixstbuild::save()
{
    QSettings settings("INightmare", "Nixstbuild");
    if ((!settings.contains("geninstall")) || (settings.value("geninstall")=="") || (!QFileInfo(settings.value("geninstall").toString()).exists()))
    {
        QMessageBox::warning(this, "Nixstbuild", "Please specify location for geninstall.sh");
        return false;
    }

    settings.setValue("dir", qd->absolutePath());

    saveConfig();
    saveRun();

    consoleView = new QTextEdit();
    consoleView->setReadOnly(true);
    consoleView->setGeometry((QApplication::desktop() ->screenGeometry().width()-720)/2, (QApplication::desktop() ->screenGeometry().height()-540)/2,720, 540);

    genproc = new QProcess();
    genproc->setWorkingDirectory(QFileInfo(settings.value("geninstall").toString()).absolutePath());
    genproc->setProcessChannelMode(QProcess::MergedChannels);

    QString cmd = settings.value("geninstall").toString() + " " + settings.value("dir").toString();

    connect(genproc, SIGNAL(readyRead()), this, SLOT(cv_read()));
    connect(genproc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(cv_finished(int, QProcess::ExitStatus)));

    consoleView->show();
    genproc->start(cmd, QIODevice::ReadOnly | QIODevice::Unbuffered);
    genproc->waitForStarted();

    return true;
}

void nixstbuild::settingsDialog()
{
    NBSettingsDialog sdlg;
    sdlg.exec();
}

void nixstbuild::about()
{
    QMessageBox::about(this, tr("About Nixstbuild"),
            tr("<b>Nixstbuild</b> version 0.1 by InternetNightmare<br><br>This software is released under GPLv2 license that can be found here:<br><a href=\"http://www.gnu.org/licenses/gpl.txt\">http://www.gnu.org/licenses/gpl.txt</a>"));
    cout << qd->absolutePath().toStdString() << endl;
}

void nixstbuild::documentWasModified()
{
    //setWindowModified(true);
}

void nixstbuild::createActions()
{
    newAct = new QAction(QIcon(":/cr32-action-filenew.png"), tr("&New"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new project"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    saveAct = new QAction(QIcon(":/folder_tar.png"), tr("&Build"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Build installer package"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    settingsAct = new QAction(QIcon(":/gear.png"), tr("&Settings"), this);
    settingsAct->setShortcut(tr("Alt+S"));
    settingsAct->setStatusTip(tr("Setup various program options"));
    connect(settingsAct, SIGNAL(triggered()), this, SLOT(settingsDialog()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

}

void nixstbuild::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(settingsAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    foldermenu = new QMenu();
    QAction *dela = foldermenu->addAction("Delete");
    dela->setIcon(QIcon(":/xdel.png"));
    connect(dela, SIGNAL(triggered()), this, SLOT(fvDeleteFile()));

    rt_insertmenu = new QMenu();
    rt_item = rt_insertmenu->addAction("Config menu");
    connect(rt_insertmenu, SIGNAL(triggered(QAction*)), this, SLOT(rt_clicked(QAction*)));
    connect(rt_insertmenu, SIGNAL(hovered(QAction*)), this, SLOT(rt_hovered(QAction*)));
    iimap["Config menu"] = 0;
    rt_item = rt_insertmenu->addAction("Checkbox");
    iimap["Checkbox"] = 1;
    rt_item = rt_insertmenu->addAction("Directory selector");
    iimap["Directory selector"] = 2;
    rt_item = rt_insertmenu->addAction("Input Field");
    iimap["Input Field"] = 3;
    rt_item = rt_insertmenu->addAction("Radiobutton");
    iimap["Radiobutton"] = 4;
    rt_item = rt_insertmenu->addAction("Image");
    iimap["Image"] = 5;
    rt_item = rt_insertmenu->addAction("Label");
    iimap["Label"] = 6;
    rt_item = rt_insertmenu->addAction("Menu");
    iimap["Menu"] = 7;
    rt_item = rt_insertmenu->addAction("ProgressBar");
    iimap["ProgressBar"] = 8;
    rt_item = rt_insertmenu->addAction("Text Field");
    iimap["Text Field"] = 9;
}

void nixstbuild::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(saveAct);
}

void nixstbuild::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void nixstbuild::addFileTab()
{
    ft_parent = new QWidget;

    ft_layout = new QGridLayout(ft_parent);

    tabs->addTab(ft_parent, QIcon(":/folder_blue_open.png"), tr("Files"));

    ft_arch = new QComboBox(ft_parent);
    ft_arch->insertItem(0, "all");
    ft_arch->insertItem(1, "x86");
    ft_arch->insertItem(2, "x86_64");

    ft_os = new QComboBox(ft_parent);
    ft_os->insertItem(0, "all");
    ft_os->insertItem(1, "linux");
    ft_os->insertItem(2, "freebsd");
    ft_os->insertItem(3, "openbsd");
    ft_os->insertItem(4, "netbsd");
    ft_os->insertItem(5, "sunos");

    ft_layout->addWidget(new QLabel("Architecture:"), 0, 0);
    ft_layout->addWidget(new QLabel("OS:"), 1, 0);
    ft_layout->addWidget(ft_arch,0, 1);
    ft_layout->addWidget(ft_os, 1, 1);
    ft_layout->setSpacing(4);
    ft_layout->setColumnStretch(1, 24);
    ft_layout->setAlignment(Qt::AlignTop);

    ft_addfile = new QPushButton(QIcon(":/cr32-mime-source.png"), tr("Add file"));
    ft_adddir = new QPushButton(QIcon(":/folder_blue.png"), tr("Add directory"));

    QHBoxLayout *hboxl = new QHBoxLayout;

    hboxl->addWidget(ft_addfile);
    hboxl->addWidget(ft_adddir);
    ft_layout->addLayout(hboxl, 2, 1);

    connect(ft_addfile, SIGNAL(clicked()), this, SLOT(ft_addFile()));
    connect(ft_adddir, SIGNAL(clicked()), this, SLOT(ft_addDir()));
}

void nixstbuild::addPackagesTab()
{
    varout = new CVarOutput;
    varout->init(pkgvars);
    QScrollArea *scrolla = new QScrollArea;
    scrolla->setWidget(varout->getLayout());
    tabs->addTab(scrolla, QIcon(":/ark_addfile.png"), "Packaging");
    connect(varout->getSaveButton(), SIGNAL(clicked()), this, SLOT(savePackage()));
}

void nixstbuild::addTextTabs()
{
    te_license = new QWidget;
    te_welcome = new QWidget;
    te_finish = new QWidget;

    teu_license = new Ui_TextEditW();
    teu_license->setupUi(te_license);
    connect(teu_license->saveButton, SIGNAL(clicked()), this, SLOT(saveLicense()));
    connect(teu_license->openButton, SIGNAL(clicked()), this, SLOT(openLicense()));
    tabs->addTab(te_license, QIcon(":/script.png"), "License");

    teu_welcome = new Ui_TextEditW();
    teu_welcome->setupUi(te_welcome);
    connect(teu_welcome->saveButton, SIGNAL(clicked()), this, SLOT(saveWelcome()));
    connect(teu_welcome->openButton, SIGNAL(clicked()), this, SLOT(openWelcome()));
    tabs->addTab(te_welcome, QIcon(":/script.png"), "Welcome");

    teu_finish = new Ui_TextEditW();
    teu_finish->setupUi(te_finish);
    connect(teu_finish->saveButton, SIGNAL(clicked()), this, SLOT(saveFinish()));
    connect(teu_finish->openButton, SIGNAL(clicked()), this, SLOT(openFinish()));
    tabs->addTab(te_finish, QIcon(":/script.png"), "Finish"); 
}

void nixstbuild::addConfigTab()
{
    ct_widget = new QWidget;

    QGridLayout *ct_layout = new QGridLayout(ct_widget);
    QPushButton *ct_save = new QPushButton(QIcon(":/cr32-action-filesave.png"), "Save", ct_widget);
    connect(ct_save, SIGNAL(clicked()), this, SLOT(saveConfig()));
    ct_layout->addWidget(ct_save, 0, 0);
    ct_layout->addWidget(new QLabel("Application name:"), 1, 0);
    ct_appName = new QLineEdit;
    ct_layout->addWidget(ct_appName, 1, 1);
    ct_layout->addWidget(new QLabel("Archive type:"), 2, 0);
    ct_archiveType = new QComboBox;
    ct_archiveType->insertItem(0, "lzma");
    ct_archiveType->insertItem(1, "bzip2");
    ct_archiveType->insertItem(2, "gzip");
    ct_layout->addWidget(ct_archiveType, 2, 1);
    ct_layout->addWidget(new QLabel("Default language:"), 3, 0);
    ct_defaultLang = new QLineEdit("english");
    ct_layout->addWidget(ct_defaultLang, 3, 1);

    QHBoxLayout *ct_hlayout = new QHBoxLayout;
    ct_feNcurses = new QCheckBox("ncurses");
    ct_feNcurses->setChecked(true);
    ct_feFltk = new QCheckBox("fltk");
    ct_feFltk->setChecked(true);
    ct_feGtk = new QCheckBox("gtk+");
    ct_feGtk->setChecked(true);
    ct_hlayout->addWidget(ct_feNcurses);
    ct_hlayout->addWidget(ct_feFltk);
    ct_hlayout->addWidget(ct_feGtk);
    ct_layout->addWidget(new QLabel("Frontends:"), 4, 0);
    ct_layout->addLayout(ct_hlayout, 4, 1);

    ct_layout->addWidget(new QLabel("Target architectures"), 5, 0);
    QHBoxLayout *ct_hlayout2 = new QHBoxLayout;
    ct_hlayout2->addWidget(ct_archx86 = new QCheckBox("x86"));
    ct_archx86->setChecked(true);
    ct_hlayout2->addWidget(ct_archx86_64 = new QCheckBox("x86_64"));
    ct_layout->addLayout(ct_hlayout2, 5, 1);

    QHBoxLayout *ct_hlayout3 = new QHBoxLayout;
    ct_layout->addWidget(new QLabel("Intro picture:"), 6, 0);
    ct_img = new QLineEdit;
    QPushButton *ct_oip = new QPushButton(QIcon(":/cr32-mime-image.png"), "", 0);
    connect(ct_oip, SIGNAL(clicked()), this, SLOT(openIntroPic()));
    ct_hlayout3->addWidget(ct_img);
    ct_hlayout3->addWidget(ct_oip);
    ct_layout->addLayout(ct_hlayout3, 6, 1);

    ct_layout->setAlignment(Qt::AlignTop);

    tabs->addTab(ct_widget, QIcon(":/tux_config.png"), "Config");
}

void nixstbuild::addRunTab()
{
    QWidget *rt_widget = new QWidget;

    QVBoxLayout *rt_mainlayout = new QVBoxLayout(rt_widget);
    QHBoxLayout *rt_btns = new QHBoxLayout;
    QPushButton *rt_save = new QPushButton(QIcon(":/cr32-action-filesave.png"), "Save");
    connect(rt_save, SIGNAL(clicked()), this, SLOT(saveRun()));

    QPushButton *rt_generate = new QPushButton("Generate Script");
    connect(rt_generate, SIGNAL(clicked()), this, SLOT(generateRun()));

    rt_insert = new QPushButton("Insert");
    connect(rt_insert, SIGNAL(clicked()), this, SLOT(insertTemplate()));

    rt_btns->addWidget(rt_save);
    rt_btns->addWidget(rt_generate);
    rt_btns->addWidget(rt_insert);
    rt_btns->setAlignment(Qt::AlignLeft);

    rt_textedit = new QTextEdit;
    new LuaHighlighter(rt_textedit->document());

    rt_mainlayout->addLayout(rt_btns);
    rt_mainlayout->addWidget(rt_textedit);

    tabs->addTab(rt_widget, QIcon(":/gear.png"), "Run");
}

void nixstbuild::reloadView()
{
    qd->cd("/tmp/.nbtemp");

    folderview->setModel(qdmodel);
    folderview->setRootIndex(qdmodel->index(qd->absolutePath()));

    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
}

void nixstbuild::fvCustomContext(const QPoint &pos)
{
    foldermenu->popup(folderview->mapToGlobal(pos));
}

void nixstbuild::fvDeleteFile()
{
    QString fpath = "rm -rf ";
    if (qdmodel->filePath(folderview->currentIndex()) != "/tmp/.nbtemp/nixstbuild")
    {
        fpath += qdmodel->filePath(folderview->currentIndex());
        system(fpath.toStdString().c_str());
    } else QMessageBox::warning(this, "Delete", "Can not delete Nixstbuild configuration file");

    reloadView();
}

void nixstbuild::ft_addFile()
{
    QStringList files = QFileDialog::getOpenFileNames(0, tr("Add files"));

    if ((ft_arch->currentText()=="all") && (ft_os->currentText()=="all"))
    {
        qd->mkdir("./files_all");
        qd->cd("./files_all");
    } else {
        qd->mkdir("./files_"+ft_os->currentText()+"_"+ft_arch->currentText());
        qd->cd("./files_"+ft_os->currentText()+"_"+ft_arch->currentText());
    }

    for (int i = 0; i<files.size(); i++)
    {
        QFile::copy(files[i], qd->absolutePath()+"/"+strippedName(files[i]));
    }

    reloadView();
}

void nixstbuild::ft_addDir()
{
    QString dir = QFileDialog::getExistingDirectory(0, tr("Add folder"));

    if ((ft_arch->itemText(ft_arch->currentIndex())=="all") && (ft_os->itemText(ft_os->currentIndex())=="all"))
    {
        qd->mkdir("./files_all");
        qd->cd("./files_all");
    } else {
        QMessageBox::warning(this, "", "./files_"+ft_os->itemText(ft_os->currentIndex())+"_"+ft_arch->itemText(ft_arch->currentIndex()));
        qd->mkdir("./files_"+ft_os->itemText(ft_os->currentIndex())+"_"+ft_arch->itemText(ft_arch->currentIndex()));
        qd->cd("./files_"+ft_os->itemText(ft_os->currentIndex())+"_"+ft_arch->itemText(ft_arch->currentIndex()));
    }

    QString cmd = "cp -r " + dir +" "+ qd->absolutePath();
    system(cmd.toStdString().c_str());
    reloadView();
}

void nixstbuild::saveLicense()
{
    QFile file(qd->absolutePath()+"/license");
    file.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&file);
    out << teu_license->textEdit->toPlainText();

    statusBar()->showMessage("License saved", 5000);
    reloadView();
}

void nixstbuild::saveWelcome()
{
    QFile file(qd->absolutePath()+"/welcome");
    file.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&file);
    out << teu_welcome->textEdit->toPlainText();

    statusBar()->showMessage("Welcome saved", 5000);
    reloadView();
}

void nixstbuild::saveFinish()
{
    QFile file(qd->absolutePath()+"/finish");
    file.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&file);
    out << teu_finish->textEdit->toPlainText();

    statusBar()->showMessage("Finish saved", 5000);
    reloadView();
}

void nixstbuild::openLicense()
{
    QFile file(QFileDialog::getOpenFileName(0, tr("Open License file")));
    file.open(QFile::ReadOnly | QFile::Text);

    QTextStream content(&file);
    teu_license->textEdit->setPlainText(content.readAll());
}

void nixstbuild::openWelcome()
{
    QFile file(QFileDialog::getOpenFileName(0, tr("Open Welcome file")));
    file.open(QFile::ReadOnly | QFile::Text);

    QTextStream content(&file);
    teu_welcome->textEdit->setPlainText(content.readAll());
}

void nixstbuild::openFinish()
{
    QFile file(QFileDialog::getOpenFileName(0, tr("Open Finish file")));
    file.open(QFile::ReadOnly | QFile::Text);

    QTextStream content(&file);
    teu_finish->textEdit->setPlainText(content.readAll());
}

void nixstbuild::saveConfig()
{
    QFile cfile(qd->absolutePath()+"/config.lua");
    cfile.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&cfile);
    out << "cfg.appname = \"" << ct_appName->displayText() << "\"\n";
    out << "cfg.archivetype = \"" << ct_archiveType->currentText() << "\"\n";
    out << "cfg.defaultlang = \"" << ct_defaultLang->displayText() << "\"\n";

    if ((ct_feNcurses->checkState()!=Qt::Checked) && (ct_feFltk->checkState()!=Qt::Checked) && (ct_feFltk->checkState()!=Qt::Checked))
    {
        QMessageBox::warning(this, "Frontends", "You must include at least one frontend in your installer package");
        cfile.remove();
        return;
    }

    out << "cfg.frontends = { ";
    if (ct_feNcurses->checkState()==Qt::Checked)
    {
        out << "\"ncurses\"";
    }
    if (ct_feFltk->checkState()==Qt::Checked)
    {
        if (ct_feNcurses->checkState()==Qt::Checked)
            out << ", ";
        out << "\"fltk\"";
    } 
    if (ct_feGtk->checkState()==Qt::Checked)
    {
        if ((ct_feNcurses->checkState()==Qt::Checked) | (ct_feFltk->checkState()==Qt::Checked))
            out << ", ";
        out << "\"gtk\"";
    }
    out << " }\n";

    out << "cfg.targetarch = { ";
    if (ct_archx86->checkState()==Qt::Checked)
    {
        out << "\"x86\"";
    }
    if (ct_archx86_64->checkState()==Qt::Checked)
    {
        if (ct_archx86->checkState()==Qt::Checked)
            out << ", ";
        out << "\"x86_64\"";
    }
    out << " }\n";

    if (ct_img->text()!="")
    {
        out << "cfg.intropic = \"" << ct_img->text() << "\"";
    }

    out << "\nlanguages = {'english'}\n";

    statusBar()->showMessage("Config saved", 5000);
    reloadView();
}

void nixstbuild::saveRun()
{
    QFile file(qd->absolutePath()+"/run.lua");
    file.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&file);
    out << rt_textedit->toPlainText();

    statusBar()->showMessage("Run script saved", 5000);
    reloadView();
}

void nixstbuild::savePackage()
{
    QFile cfile(qd->absolutePath()+"/package.lua");
    cfile.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&cfile);
    out << QString(varout->getScript().c_str());
    reloadView();
}

void nixstbuild::generateRun()
{
    NBRunGen rg(this);
    if (rg.exec())
        rt_textedit->setText(rg.script().c_str());
}

void nixstbuild::insertTemplate()
{
    rt_insertmenu->popup(rt_insert->mapToGlobal(QPoint(0, 28)));
}

void nixstbuild::rt_clicked(QAction *action)
{
    rt_textedit->insertPlainText(templates[iimap[action->text()]]);
}

void nixstbuild::rt_hovered(QAction *action)
{
	QToolTip::showText(rt_insertmenu->mapToGlobal(rt_insertmenu->actionGeometry(rt_insertmenu->activeAction()).topRight()), template_tooltips[iimap[rt_insertmenu->activeAction()->text()]]);
}

void nixstbuild::cv_appendline(QString line)
{
    consoleView->insertPlainText(line);
}

void nixstbuild::cv_read()
{
    consoleView->insertPlainText(genproc->readAllStandardOutput());
}

void nixstbuild::cv_close()
{
    QMessageBox::information(consoleView, "Nixstbuild", "Finished building installer package.");
    delete consoleView;
}

void nixstbuild::cv_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode==1)
    {
        QMessageBox::warning(consoleView, "Nixstbuild", "There was an error while building a package.");
    } else {
        QMessageBox::information(consoleView, "Nixstbuild", "Finished building installer package.");
    }
    delete genproc;
    delete consoleView;

}

void nixstbuild::openIntroPic()
{
    QString file = QFileDialog::getOpenFileName(0, tr("Select an intro pic"));
    QFile::copy(file, qd->absolutePath()+"/"+strippedName(file));
    ct_img->setText(strippedName(file));
}

void nixstbuild::readSettings()
{
    QSettings settings("INightmare", "Nixstbuild");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void nixstbuild::writeSettings()
{
    QSettings settings("INightmare", "Nixstbuild");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

void nixstbuild::writeConfig()
{
    QFile file(qd->absolutePath()+"/nixstbuild");
    file.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&file);
    out << " ";

    file.close();
}

QString nixstbuild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

nixstbuild::~nixstbuild()
{

}

