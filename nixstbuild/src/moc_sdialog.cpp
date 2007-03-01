/****************************************************************************
** Meta object code from reading C++ file 'sdialog.h'
**
** Created: Thu Mar 1 16:50:46 2007
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sdialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_NBSettingsDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x08,
      29,   17,   17,   17, 0x08,
      35,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_NBSettingsDialog[] = {
    "NBSettingsDialog\0\0showOpen()\0sOK()\0sCancel()\0"
};

const QMetaObject NBSettingsDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_NBSettingsDialog,
      qt_meta_data_NBSettingsDialog, 0 }
};

const QMetaObject *NBSettingsDialog::metaObject() const
{
    return &staticMetaObject;
}

void *NBSettingsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NBSettingsDialog))
	return static_cast<void*>(const_cast<NBSettingsDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int NBSettingsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: showOpen(); break;
        case 1: sOK(); break;
        case 2: sCancel(); break;
        }
        _id -= 3;
    }
    return _id;
}
