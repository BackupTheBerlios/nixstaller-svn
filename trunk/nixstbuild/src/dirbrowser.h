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

#ifndef DIRBROWSER_H
#define DIRBROWSER_H

#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QWidget>

#include "main/main.h"

class QProgressDialog;

class CDirModel;
class CDirSortProxy;
class CDirView;

class CDirBrowser: public QWidget
{
    Q_OBJECT
    
    QAction *homeTool, *favTool, *addDirTool;
    CDirModel *browserModel;
    CDirSortProxy *proxyModel;
    CDirView *browserView;
    QAction *copyAction, *cutAction, *pasteAction, *deleteAction, *renameAction;
    QAction *viewHidden;

    bool permissionsOK(const QString &path, bool onlyread);

private slots:
    void copyActionCB(void);
    void cutActionCB(void);
    void pasteActionCB(void);
    void deleteActionCB(void);
    void renameActionCB(void);
    void viewHiddenCB(bool e);
    void updateActions(void);

public:
    CDirBrowser(const QString &d = QString(), QWidget *parent = 0, Qt::WindowFlags flags = 0);

    void setRootDir(QString dir);
};

class CDirView: public QTreeView
{
    Q_OBJECT
    
protected:
    virtual void mousePressEvent(QMouseEvent *event);

public:
    CDirView(QWidget *parent = 0);

signals:
    void mouseClicked(void);
};

class CDirSortProxy: public QSortFilterProxyModel
{
    bool hide;
    
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

public:
    CDirSortProxy(QObject *parent = 0) : QSortFilterProxyModel(parent), hide(false) {}

    void setHide(bool h) { hide = h; invalidateFilter(); }
};

class CDirModel: public QFileSystemModel
{
    enum asktype { DO_ALL, DO_NONE, DO_ASK };

    QProgressDialog *progressDialog;
    long statUpdateTime;
    int statWritten, statSizeFact;
    asktype handleOverWrite;
    bool multipleFiles;

    int sizeUnitFact(qint64 size);
    void safeCopy(const std::string &src, const std::string &dest, bool move);
    void safeMKDirRec(const std::string &dir);
    bool getAllSubPaths(const std::string &dir, TStringVec &paths);
    bool askOverWrite(const QString &t, const QString &l);
    bool verifyExistance(const std::string &src, std::string dest);
    void RmDir(const std::string &dir);
    void safeUnlink(const std::string &file);

public:
    CDirModel(QObject *parent = 0);

    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int,
                              int, const QModelIndex &parent);

    void copyFiles(const QMimeData *data, const QModelIndex &parent, bool move);
    void removeFiles(const QMimeData *data);
    void updateLayout(void) { emit rootPathChanged("/"); }

    static void copyWritten(int bytes, void *data);
};

#endif
