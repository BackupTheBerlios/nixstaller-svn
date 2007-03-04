/********************************************************************************
** Form generated from reading ui file 'screeninput.ui'
**
** Created: Sat Mar 3 20:43:45 2007
**      by: Qt User Interface Compiler version 4.2.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SCREENINPUT_H
#define UI_SCREENINPUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QWidget>

class Ui_ScreenInputDialog
{
public:
    QPushButton *btnCancel;
    QPushButton *btnOK;
    QGroupBox *groupBox;
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QLineEdit *cScreenBox;
    QRadioButton *rDefault;
    QComboBox *dScreenBox;
    QRadioButton *rCustom;
    QHBoxLayout *hboxLayout;
    QLabel *label;
    QLineEdit *screenText;

    void setupUi(QDialog *ScreenInputDialog)
    {
    ScreenInputDialog->setObjectName(QString::fromUtf8("ScreenInputDialog"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ScreenInputDialog->sizePolicy().hasHeightForWidth());
    ScreenInputDialog->setSizePolicy(sizePolicy);
    btnCancel = new QPushButton(ScreenInputDialog);
    btnCancel->setObjectName(QString::fromUtf8("btnCancel"));
    btnCancel->setGeometry(QRect(320, 180, 80, 27));
    btnOK = new QPushButton(ScreenInputDialog);
    btnOK->setObjectName(QString::fromUtf8("btnOK"));
    btnOK->setGeometry(QRect(230, 180, 80, 27));
    groupBox = new QGroupBox(ScreenInputDialog);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    groupBox->setGeometry(QRect(10, 0, 391, 171));
    layoutWidget = new QWidget(groupBox);
    layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
    layoutWidget->setGeometry(QRect(20, 20, 361, 146));
    gridLayout = new QGridLayout(layoutWidget);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    cScreenBox = new QLineEdit(layoutWidget);
    cScreenBox->setObjectName(QString::fromUtf8("cScreenBox"));
    cScreenBox->setEnabled(false);

    gridLayout->addWidget(cScreenBox, 3, 0, 1, 1);

    rDefault = new QRadioButton(layoutWidget);
    rDefault->setObjectName(QString::fromUtf8("rDefault"));

    gridLayout->addWidget(rDefault, 0, 0, 1, 1);

    dScreenBox = new QComboBox(layoutWidget);
    dScreenBox->setObjectName(QString::fromUtf8("dScreenBox"));
    dScreenBox->setEnabled(false);

    gridLayout->addWidget(dScreenBox, 1, 0, 1, 1);

    rCustom = new QRadioButton(layoutWidget);
    rCustom->setObjectName(QString::fromUtf8("rCustom"));

    gridLayout->addWidget(rCustom, 2, 0, 1, 1);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    label = new QLabel(layoutWidget);
    label->setObjectName(QString::fromUtf8("label"));

    hboxLayout->addWidget(label);

    screenText = new QLineEdit(layoutWidget);
    screenText->setObjectName(QString::fromUtf8("screenText"));
    screenText->setEnabled(false);

    hboxLayout->addWidget(screenText);


    gridLayout->addLayout(hboxLayout, 4, 0, 1, 1);


    retranslateUi(ScreenInputDialog);

    QSize size(409, 210);
    size = size.expandedTo(ScreenInputDialog->minimumSizeHint());
    ScreenInputDialog->resize(size);


    QMetaObject::connectSlotsByName(ScreenInputDialog);
    } // setupUi

    void retranslateUi(QDialog *ScreenInputDialog)
    {
    ScreenInputDialog->setWindowTitle(QApplication::translate("ScreenInputDialog", "Add a Screen", 0, QApplication::UnicodeUTF8));
    btnCancel->setText(QApplication::translate("ScreenInputDialog", "Cancel", 0, QApplication::UnicodeUTF8));
    btnOK->setText(QApplication::translate("ScreenInputDialog", "OK", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("ScreenInputDialog", "Screen", 0, QApplication::UnicodeUTF8));
    rDefault->setText(QApplication::translate("ScreenInputDialog", "&Default", 0, QApplication::UnicodeUTF8));
    rCustom->setText(QApplication::translate("ScreenInputDialog", "&Custom", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("ScreenInputDialog", "Screen text:", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(ScreenInputDialog);
    } // retranslateUi

};

namespace Ui {
    class ScreenInputDialog: public Ui_ScreenInputDialog {};
} // namespace Ui

#endif // UI_SCREENINPUT_H
