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

#include "sdialog.h"
#include "rungen.h"


const char *templates[] = {
    "cfgscreen:addcfgmenu(\"title\")",
    "cfgscreen:addcheckbox(\"title\", {\"name1\"})",
    "cfgscreen:adddirselector()",
    "cfgscreen:addinput(\"label\")",
    "cfgscreen:addradiobutton(\"title\", {\"opt1\", \"opt2\"})"
};

const char *template_tooltips[] = {
    "cfgscreen:addcfgmenu( title )",
    "cfgscreen:addcheckbox( title, { checkbox1, checkbox2, ... } )",
    "cfgscreen:adddirselector( title, dir ) <br><i>dir</i> is optional, default value is users home directory.",
    "cfgscreen:addinput( label, maxchars, value, type ) <br>Types:<br> <i>\"string\"</i><br> <i>\"number\"</i><br> <i>\"float\"</i>",
    "cfgscreen:addradiobutton(\"title\", {\"opt1\", \"opt2\"} )"
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
    addTextTabs();

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    readSettings();

}

void nixstbuild::initMainControls()
{
    qd = new QDir(getenv("HOME"));
    string cmd = "rm -r " + qd->absolutePath().toStdString()+"/.nbtemp";
    system(cmd.c_str());
    qd->mkdir("./.nbtemp");
    qd->cd("./.nbtemp");

    qd->mkdir("./lang");

    qdmodel = new QDirModel();
    folderview = new QTreeView();
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
    string cmd = "rm -rf " + qd->absolutePath().toStdString();
    system(cmd.c_str());
    qd->cdUp();
    qd->mkdir("./.nbtemp");
    qd->cd("./.nbtemp");

    qd->mkdir("./lang");

    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
}

bool nixstbuild::save()
{
    QSettings settings("INightmare", "Nixstbuild");
    if ((!settings.contains("geninstall")) || (settings.value("geninstall")=="") || (!QFileInfo(settings.value("geninstall").toString()).exists()))
    {
        QMessageBox::warning(this, "Nixstbuild", "Please specify location for genisntall.sh");
        return false;
    }

    settings.setValue("dir", qd->absolutePath());

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
    rt_itcma = rt_insertmenu->addAction("Config menu");
    connect(rt_itcma, SIGNAL(triggered()), this, SLOT(rt_iconfig()));
    connect(rt_itcma, SIGNAL(hovered()), this, SLOT(rt_iconfigh()));
    rt_itca = rt_insertmenu->addAction("Checkbox");
    connect(rt_itca, SIGNAL(triggered()), this, SLOT(rt_icheckbox()));
    connect(rt_itca, SIGNAL(hovered()), this, SLOT(rt_icheckboxh()));
    rt_itdsa = rt_insertmenu->addAction("Directory selector");
    connect(rt_itdsa, SIGNAL(triggered()), this, SLOT(rt_idirselector()));
    connect(rt_itdsa, SIGNAL(hovered()), this, SLOT(rt_idirselectorh()));
    rt_itia = rt_insertmenu->addAction("Input Field");
    connect(rt_itia, SIGNAL(triggered()), this, SLOT(rt_iinput()));
    connect(rt_itia, SIGNAL(hovered()), this, SLOT(rt_iinputh()));
    rt_itra = rt_insertmenu->addAction("Radiobutton");
    connect(rt_itra, SIGNAL(triggered()), this, SLOT(rt_iradio()));
    connect(rt_itra, SIGNAL(hovered()), this, SLOT(rt_iradioh()));
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

void nixstbuild::addTextTabs()
{
    te_license = new QWidget;
    te_welcome = new QWidget;
    te_finish = new QWidget;

    teu_license = new Ui_TextEditW();
    teu_license->setupUi(te_license);
    connect(teu_license->saveButton, SIGNAL(clicked()), this, SLOT(saveLicense()));
    tabs->addTab(te_license, QIcon(":/script.png"), "License");

    teu_welcome = new Ui_TextEditW();
    teu_welcome->setupUi(te_welcome);
    connect(teu_welcome->saveButton, SIGNAL(clicked()), this, SLOT(saveWelcome()));
    tabs->addTab(te_welcome, QIcon(":/script.png"), "Welcome");

    teu_finish = new Ui_TextEditW();
    teu_finish->setupUi(te_finish);
    connect(teu_finish->saveButton, SIGNAL(clicked()), this, SLOT(saveFinish()));
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
    ct_hlayout->addWidget(ct_feNcurses);
    ct_hlayout->addWidget(ct_feFltk);
    ct_layout->addWidget(new QLabel("Frontends:"), 4, 0);
    ct_layout->addLayout(ct_hlayout, 4, 1);

    QHBoxLayout *ct_hlayout2 = new QHBoxLayout;
    ct_layout->addWidget(new QLabel("Intro picture:"), 5, 0);
    ct_img = new QLineEdit;
    QPushButton *ct_oip = new QPushButton(QIcon(":/cr32-mime-image.png"), "", 0);
    connect(ct_oip, SIGNAL(clicked()), this, SLOT(openIntroPic()));
    ct_hlayout2->addWidget(ct_img);
    ct_hlayout2->addWidget(ct_oip);
    ct_layout->addLayout(ct_hlayout2, 5, 1);

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
    LuaHighlighter *lh = new LuaHighlighter(rt_textedit->document());

    rt_mainlayout->addLayout(rt_btns);
    rt_mainlayout->addWidget(rt_textedit);

    tabs->addTab(rt_widget, QIcon(":/gear.png"), "Run");
}

void nixstbuild::fvCustomContext(const QPoint &pos)
{
    foldermenu->popup(folderview->mapToGlobal(pos));
}

void nixstbuild::fvDeleteFile()
{
    QString fpath = qdmodel->filePath(folderview->currentIndex());
    string cmd("rm -rf ");
    cmd += fpath.toStdString();
    system(cmd.c_str());
    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
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

    qd->cdUp();
    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
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
    qd->cdUp();
    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
}

void nixstbuild::saveLicense()
{
    QFile file(qd->absolutePath()+"/license");
    file.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&file);
    out << teu_license->textEdit->toPlainText();

    statusBar()->showMessage("License saved", 5000);
    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
}

void nixstbuild::saveWelcome()
{
    QFile file(qd->absolutePath()+"/welcome");
    file.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&file);
    out << teu_welcome->textEdit->toPlainText();

    statusBar()->showMessage("Welcome saved", 5000);
    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
}

void nixstbuild::saveFinish()
{
    QFile file(qd->absolutePath()+"/finish");
    file.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&file);
    out << teu_finish->textEdit->toPlainText();

    statusBar()->showMessage("Finish saved", 5000);
    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
}

void nixstbuild::saveConfig()
{
    QFile cfile(qd->absolutePath()+"/config.lua");
    cfile.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&cfile);
    out << "cfg.appname = \"" << ct_appName->displayText() << "\"\n";
    out << "cfg.archivetype = \"" << ct_archiveType->currentText() << "\"\n";
    out << "cfg.defaultlang = \"" << ct_defaultLang->displayText() << "\"\n";

    if ((ct_feNcurses->checkState()!=Qt::Checked) && (ct_feFltk->checkState()!=Qt::Checked))
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
        out << "\"fltk\" }\n";
    } else out << " }\n";

    if (ct_img->text()!="")
    {
        out << "cfg.intropic = \"" << ct_img->text() << "\"";
    }

    statusBar()->showMessage("Config saved", 5000);
    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
}

void nixstbuild::saveRun()
{
    QFile file(qd->absolutePath()+"/run.lua");
    file.open(QFile::WriteOnly | QFile::Text);

    QTextStream out(&file);
    out << rt_textedit->toPlainText();

    statusBar()->showMessage("Run script saved", 5000);
    qdmodel->refresh(qdmodel->index(qd->absolutePath()));
}

void nixstbuild::generateRun()
{
    NBRunGen rg(this);
    rg.exec();
    rt_textedit->setText(rg.script().c_str());
}

void nixstbuild::insertTemplate()
{
    rt_insertmenu->popup(rt_insert->mapToGlobal(QPoint(0, 28)));
}

void nixstbuild::rt_iconfig()
{
    rt_textedit->insertPlainText(templates[0]);
}

void nixstbuild::rt_icheckbox()
{
    rt_textedit->insertPlainText(templates[1]);
}

void nixstbuild::rt_idirselector()
{
    rt_textedit->insertPlainText(templates[2]);
}

void nixstbuild::rt_iinput()
{
    rt_textedit->insertPlainText(templates[3]);
}

void nixstbuild::rt_iradio()
{
    rt_textedit->insertPlainText(templates[4]);
}

void nixstbuild::rt_iconfigh()
{
    QToolTip::showText(rt_insertmenu->mapToGlobal(rt_insertmenu->actionGeometry(rt_itcma).topRight()), template_tooltips[0]);
}

void nixstbuild::rt_icheckboxh()
{
    QToolTip::showText(rt_insertmenu->mapToGlobal(rt_insertmenu->actionGeometry(rt_itca).topRight()), template_tooltips[1]);
}

void nixstbuild::rt_idirselectorh()
{
    QToolTip::showText(rt_insertmenu->mapToGlobal(rt_insertmenu->actionGeometry(rt_itdsa).topRight()), template_tooltips[2]);
}

void nixstbuild::rt_iinputh()
{
    QToolTip::showText(rt_insertmenu->mapToGlobal(rt_insertmenu->actionGeometry(rt_itia).topRight()), template_tooltips[3]);
}

void nixstbuild::rt_iradioh()
{
    QToolTip::showText(rt_insertmenu->mapToGlobal(rt_insertmenu->actionGeometry(rt_itra).topRight()), template_tooltips[4]);
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

QString nixstbuild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

nixstbuild::~nixstbuild()
{

}

