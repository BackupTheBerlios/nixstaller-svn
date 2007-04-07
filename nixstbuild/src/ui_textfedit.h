/********************************************************************************
** Form generated from reading ui file 'textfedit.ui'
**
** Created: Sun Mar 25 20:21:35 2007
**      by: Qt User Interface Compiler version 4.2.0
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
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_TextEditW
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *vboxLayout;
    QPushButton *saveButton;
    QTextEdit *textEdit;

    void setupUi(QWidget *TextEditW)
    {
    TextEditW->setObjectName(QString::fromUtf8("TextEditW"));
    gridLayout = new QGridLayout(TextEditW);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(4);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    saveButton = new QPushButton(TextEditW);
    saveButton->setObjectName(QString::fromUtf8("saveButton"));
    saveButton->setMaximumSize(QSize(128, 24));
    saveButton->setBaseSize(QSize(128, 24));
    saveButton->setIcon(QIcon(QString::fromUtf8(":/filesave.xpm")));

    vboxLayout->addWidget(saveButton);

    textEdit = new QTextEdit(TextEditW);
    textEdit->setObjectName(QString::fromUtf8("textEdit"));

    vboxLayout->addWidget(textEdit);


    gridLayout->addLayout(vboxLayout, 0, 0, 1, 1);


    retranslateUi(TextEditW);

    QSize size(400, 300);
    size = size.expandedTo(TextEditW->minimumSizeHint());
    TextEditW->resize(size);


    QMetaObject::connectSlotsByName(TextEditW);
    } // setupUi

    void retranslateUi(QWidget *TextEditW)
    {
    TextEditW->setWindowTitle(QApplication::translate("TextEditW", "Form", 0, QApplication::UnicodeUTF8));
    saveButton->setToolTip(QApplication::translate("TextEditW", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Save file</p></body></html>", 0, QApplication::UnicodeUTF8));
    saveButton->setText(QApplication::translate("TextEditW", "Save", 0, QApplication::UnicodeUTF8));
    textEdit->setHtml(QApplication::translate("TextEditW", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p></body></html>", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(TextEditW);
    } // retranslateUi

};

namespace Ui {
    class TextEditW: public Ui_TextEditW {};
} // namespace Ui

#endif // UI_TEXTFEDIT_H
