/****************************************************************************
** Meta object code from reading C++ file 'nixstbuild.h'
**
** Created: Fri Feb 22 19:28:14 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "nixstbuild.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'nixstbuild.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_nixstbuild[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      27,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      27,   11,   22,   11, 0x08,
      34,   11,   11,   11, 0x08,
      42,   11,   11,   11, 0x08,
      64,   11,   11,   11, 0x08,
      77,   11,   11,   11, 0x08,
      89,   11,   11,   11, 0x08,
     103,   11,   11,   11, 0x08,
     117,   11,   11,   11, 0x08,
     130,   11,   11,   11, 0x08,
     144,   11,   11,   11, 0x08,
     158,   11,   11,   11, 0x08,
     171,   11,   11,   11, 0x08,
     184,   11,   11,   11, 0x08,
     194,   11,   11,   11, 0x08,
     208,   11,   11,   11, 0x08,
     222,   11,   11,   11, 0x08,
     239,   11,   11,   11, 0x08,
     254,   11,   11,   11, 0x08,
     275,  271,   11,   11, 0x08,
     299,   11,   11,   11, 0x08,
     321,  314,   11,   11, 0x08,
     342,  314,   11,   11, 0x08,
     368,  363,   11,   11, 0x08,
     391,   11,   11,   11, 0x08,
     401,   11,   11,   11, 0x08,
     432,  412,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nixstbuild[] = {
    "nixstbuild\0\0newFile()\0bool\0save()\0"
    "about()\0documentWasModified()\0"
    "ft_addFile()\0ft_addDir()\0saveLicense()\0"
    "saveWelcome()\0saveFinish()\0openLicense()\0"
    "openWelcome()\0openFinish()\0saveConfig()\0"
    "saveRun()\0savePackage()\0generateRun()\0"
    "insertTemplate()\0openIntroPic()\0"
    "settingsDialog()\0pos\0fvCustomContext(QPoint)\0"
    "fvDeleteFile()\0action\0rt_hovered(QAction*)\0"
    "rt_clicked(QAction*)\0line\0"
    "cv_appendline(QString)\0cv_read()\0"
    "cv_close()\0exitCode,exitStatus\0"
    "cv_finished(int,QProcess::ExitStatus)\0"
};

const QMetaObject nixstbuild::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_nixstbuild,
      qt_meta_data_nixstbuild, 0 }
};

const QMetaObject *nixstbuild::metaObject() const
{
    return &staticMetaObject;
}

void *nixstbuild::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nixstbuild))
	return static_cast<void*>(const_cast< nixstbuild*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int nixstbuild::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newFile(); break;
        case 1: { bool _r = save();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 2: about(); break;
        case 3: documentWasModified(); break;
        case 4: ft_addFile(); break;
        case 5: ft_addDir(); break;
        case 6: saveLicense(); break;
        case 7: saveWelcome(); break;
        case 8: saveFinish(); break;
        case 9: openLicense(); break;
        case 10: openWelcome(); break;
        case 11: openFinish(); break;
        case 12: saveConfig(); break;
        case 13: saveRun(); break;
        case 14: savePackage(); break;
        case 15: generateRun(); break;
        case 16: insertTemplate(); break;
        case 17: openIntroPic(); break;
        case 18: settingsDialog(); break;
        case 19: fvCustomContext((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 20: fvDeleteFile(); break;
        case 21: rt_hovered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 22: rt_clicked((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 23: cv_appendline((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 24: cv_read(); break;
        case 25: cv_close(); break;
        case 26: cv_finished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        }
        _id -= 27;
    }
    return _id;
}
