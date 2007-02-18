/****************************************************************************
** Meta object code from reading C++ file 'rungen.h'
**
** Created: Sun Feb 18 13:14:01 2007
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "rungen.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rungen.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_NBRunGen[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x08,
      16,    9,    9,    9, 0x08,
      30,   26,    9,    9, 0x08,
      41,    9,    9,    9, 0x08,
      48,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_NBRunGen[] = {
    "NBRunGen\0\0sOK()\0sCancel()\0row\0sShow(int)\0sBUp()\0sBDown()\0"
};

const QMetaObject NBRunGen::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_NBRunGen,
      qt_meta_data_NBRunGen, 0 }
};

const QMetaObject *NBRunGen::metaObject() const
{
    return &staticMetaObject;
}

void *NBRunGen::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NBRunGen))
	return static_cast<void*>(const_cast<NBRunGen*>(this));
    return QDialog::qt_metacast(_clname);
}

int NBRunGen::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: sOK(); break;
        case 1: sCancel(); break;
        case 2: sShow((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: sBUp(); break;
        case 4: sBDown(); break;
        }
        _id -= 5;
    }
    return _id;
}
