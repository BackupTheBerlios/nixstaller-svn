/********************************************************************************
** Form generated from reading ui file 'textfedit.ui'
**
** Created: Sun Jun 24 23:24:18 2007
**      by: Qt User Interface Compiler version 4.3.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_TEXTFEDIT_H
#define UI_TEXTFEDIT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_TextEditW
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QPushButton *openButton;
    QPushButton *saveButton;
    QSpacerItem *spacerItem;
    QTextEdit *textEdit;

    void setupUi(QWidget *TextEditW)
    {
    if (TextEditW->objectName().isEmpty())
        TextEditW->setObjectName(QString::fromUtf8("TextEditW"));
    QSize size(400, 300);
    size = size.expandedTo(TextEditW->minimumSizeHint());
    TextEditW->resize(size);
    gridLayout = new QGridLayout(TextEditW);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(4);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(4);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    openButton = new QPushButton(TextEditW);
    openButton->setObjectName(QString::fromUtf8("openButton"));
    openButton->setMinimumSize(QSize(124, 0));
    openButton->setMaximumSize(QSize(124, 24));
    openButton->setBaseSize(QSize(128, 24));
    openButton->setIcon(QIcon(QString::fromUtf8(":/cr32-action-fileopen.png")));

    hboxLayout->addWidget(openButton);

    saveButton = new QPushButton(TextEditW);
    saveButton->setObjectName(QString::fromUtf8("saveButton"));
    saveButton->setMinimumSize(QSize(124, 0));
    saveButton->setMaximumSize(QSize(128, 24));
    saveButton->setBaseSize(QSize(128, 24));
    saveButton->setIcon(QIcon(QString::fromUtf8(":/cr32-action-filesave.png")));

    hboxLayout->addWidget(saveButton);

    spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);


    vboxLayout->addLayout(hboxLayout);

    textEdit = new QTextEdit(TextEditW);
    textEdit->setObjectName(QString::fromUtf8("textEdit"));

    vboxLayout->addWidget(textEdit);


    gridLayout->addLayout(vboxLayout, 0, 0, 1, 1);


    retranslateUi(TextEditW);

    QMetaObject::connectSlotsByName(TextEditW);
    } // setupUi

    void retranslateUi(QWidget *TextEditW)
    {
    TextEditW->setWindowTitle(QApplication::translate("TextEditW", "Form", 0, QApplication::UnicodeUTF8));
    openButton->setToolTip(QString());
    openButton->setText(QApplication::translate("TextEditW", "Open", 0, QApplication::UnicodeUTF8));
    saveButton->setToolTip(QString());
    saveButton->setText(QApplication::translate("TextEditW", "Save", 0, QApplication::UnicodeUTF8));
    textEdit->setHtml(QApplication::translate("TextEditW", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p></body></html>", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(TextEditW);
    } // retranslateUi

};

namespace Ui {
    class TextEditW: public Ui_TextEditW {};
} // namespace Ui

#endif // UI_TEXTFEDIT_H
