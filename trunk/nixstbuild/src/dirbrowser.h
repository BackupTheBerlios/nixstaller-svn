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
#include <QWidget>

#include "main/main.h"

class QProgressDialog;
class QTreeView;

class CDirModel;
class CDirSortProxy;

class CDirBrowser: public QWidget
{
    Q_OBJECT
    
    CDirModel *browserModel;
    CDirSortProxy *proxyModel;
    QTreeView *browserView;

private slots:
    void copyAction(void);
    void cutAction(void);
    void pasteAction(void);
    
public:
    CDirBrowser(const QString &d = QString(), QWidget *parent = 0, Qt::WindowFlags flags = 0);
};

class CDirSortProxy: public QSortFilterProxyModel
{
protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

public:
    CDirSortProxy(QObject *parent = 0) : QSortFilterProxyModel(parent) {}
};

class CDirModel: public QFileSystemModel
{
public:
    enum optype { COPY, MOVE, DELETE };

private:
    enum asktype { DO_ALL, DO_NONE, DO_ASK };

    QProgressDialog *progressDialog;
    long statUpdateTime;
    int statWritten, statSizeFact;
    asktype handleOverWrite;
    bool multipleFiles;

    int sizeUnitFact(qint64 size);
    void safeFileOp(const std::string &src, const std::string &dest, optype opType);
    void safeMKDirRec(const std::string &dir);
    bool getAllSubPaths(const std::string &dir, TStringVec &paths);
    bool askOverWrite(const QString &t, const QString &l);
    bool verifyExistance(const std::string &src, std::string dest);

public:
    CDirModel(QObject *parent = 0);

    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row,
                                int column, const QModelIndex &parent);

    void operateOnFiles(const QMimeData *data, const QModelIndex &parent, optype opType);

    static void opWritten(int bytes, void *data);
};

#endif
