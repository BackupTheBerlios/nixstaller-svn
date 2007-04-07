/****************************************************************************
** Meta object code from reading C++ file 'nixstbuild.h'
**
** Created: Sun Mar 25 20:22:06 2007
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "nixstbuild.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'nixstbuild.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.0. It"
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
     143,   11,   11,   11, 0x08,
     153,   11,   11,   11, 0x08,
     167,   11,   11,   11, 0x08,
     184,   11,   11,   11, 0x08,
     199,   11,   11,   11, 0x08,
     220,  216,   11,   11, 0x08,
     244,   11,   11,   11, 0x08,
     259,   11,   11,   11, 0x08,
     274,   11,   11,   11, 0x08,
     286,   11,   11,   11, 0x08,
     299,   11,   11,   11, 0x08,
     317,   11,   11,   11, 0x08,
     329,   11,   11,   11, 0x08,
     345,   11,   11,   11, 0x08,
     358,   11,   11,   11, 0x08,
     372,   11,   11,   11, 0x08,
     391,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nixstbuild[] = {
    "nixstbuild\0\0newFile()\0bool\0save()\0about()\0documentWasModified()\0"
    "ft_addFile()\0ft_addDir()\0saveLicense()\0saveWelcome()\0saveFinish()\0"
    "saveConfig()\0saveRun()\0generateRun()\0insertTemplate()\0"
    "openIntroPic()\0settingsDialog()\0pos\0fvCustomContext(QPoint)\0"
    "fvDeleteFile()\0rt_icheckbox()\0rt_iradio()\0rt_iconfig()\0"
    "rt_idirselector()\0rt_iinput()\0rt_icheckboxh()\0rt_iradioh()\0"
    "rt_iconfigh()\0rt_idirselectorh()\0rt_iinputh()\0"
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
	return static_cast<void*>(const_cast<nixstbuild*>(this));
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
        case 9: saveConfig(); break;
        case 10: saveRun(); break;
        case 11: generateRun(); break;
        case 12: insertTemplate(); break;
        case 13: openIntroPic(); break;
        case 14: settingsDialog(); break;
        case 15: fvCustomContext((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 16: fvDeleteFile(); break;
        case 17: rt_icheckbox(); break;
        case 18: rt_iradio(); break;
        case 19: rt_iconfig(); break;
        case 20: rt_idirselector(); break;
        case 21: rt_iinput(); break;
        case 22: rt_icheckboxh(); break;
        case 23: rt_iradioh(); break;
        case 24: rt_iconfigh(); break;
        case 25: rt_idirselectorh(); break;
        case 26: rt_iinputh(); break;
        }
        _id -= 27;
    }
    return _id;
}
