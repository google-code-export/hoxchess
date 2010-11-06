/****************************************************************************
** Meta object code from reading C++ file 'tableui.h'
**
** Created: Sun Apr 11 22:15:56 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "tableui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tableui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TableUI[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x08,
      38,    8,    8,    8, 0x08,
      69,    8,    8,    8, 0x08,
      99,    8,    8,    8, 0x08,
     129,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_TableUI[] = {
    "TableUI\0\0on_replayEndButton_clicked()\0"
    "on_replayBeginButton_clicked()\0"
    "on_replayNextButton_clicked()\0"
    "on_replayPrevButton_clicked()\0"
    "on_resetButton_clicked()\0"
};

const QMetaObject TableUI::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_TableUI,
      qt_meta_data_TableUI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TableUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TableUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TableUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TableUI))
        return static_cast<void*>(const_cast< TableUI*>(this));
    return QWidget::qt_metacast(_clname);
}

int TableUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_replayEndButton_clicked(); break;
        case 1: on_replayBeginButton_clicked(); break;
        case 2: on_replayNextButton_clicked(); break;
        case 3: on_replayPrevButton_clicked(); break;
        case 4: on_resetButton_clicked(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
