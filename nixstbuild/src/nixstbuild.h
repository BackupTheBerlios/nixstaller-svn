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


#ifndef NIXSTBUILD_H
#define NIXSTBUILD_H

#include <QMainWindow>
#include <QCloseEvent>

class QAction;
class QMenu;
class QTextEdit;
class QDir;
class QDirModel;
class QTreeView;
class QListView;
class QSplitter;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QTabWidget;
class QBoxLayout;
class QGridLayout;
class QPushButton;
class Ui_TextEditW;


class nixstbuild:public QMainWindow
{
      Q_OBJECT

public:
      nixstbuild();
      ~nixstbuild();

protected:
      void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    bool save();
    void about();
    void documentWasModified();
    void ft_addFile();
    void ft_addDir();
    void saveLicense();
    void saveWelcome();
    void saveFinish();
    void saveConfig();
    void saveRun();
    void generateRun();
    void openIntroPic();
    void settingsDialog();
    void fvCustomContext(const QPoint &pos);
    void fvDeleteFile();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void initMainControls();
    void addFileTab();
    void addLanguageTab();
    void addTextTabs();
    void addConfigTab();
    void addRunTab();
    void readSettings();
    void writeSettings();
    QString strippedName(const QString &fullFileName);

    QTextEdit *textEdit;
    QString curFile;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;
    QMenu *settingsMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *settingsAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    QDir *qd;
    QDirModel *qdmodel;
    QTreeView *folderview;
    QMenu *foldermenu;
    QSplitter *msplitter;
    QTabWidget *tabs;

    // Add files tab
    QGridLayout *ft_layout;
    QWidget *ft_parent;
    QComboBox *ft_arch;
    QComboBox *ft_os;
    QPushButton *ft_addfile;
    QPushButton *ft_adddir;

    // License textEdits
    QWidget *te_license;
    QWidget *te_welcome;
    QWidget *te_finish;
    Ui_TextEditW *teu_license;
    Ui_TextEditW *teu_welcome;
    Ui_TextEditW *teu_finish;

    // Config tab
    QWidget *ct_widget;
    QLineEdit *ct_appName;
    QComboBox *ct_archiveType;
    QLineEdit *ct_defaultLang;
    QCheckBox *ct_feNcurses;
    QCheckBox *ct_feFltk;
    QLineEdit *ct_img;

    // Run tab
    QTextEdit *rt_textedit;
};

#endif
