#ifndef _CONFIGW_H
#define _CONFIGW_H

#include <list>
#include <vector>

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>

using namespace std;


class CVariableProvider: public QWidget
{
    Q_OBJECT
protected:
    string varname;
public:
    virtual string toString() = 0;
    string getName();
};

class CStringProvider: public CVariableProvider
{
protected:
    QLineEdit *lineEdit;
public:
    CStringProvider(string name);
    virtual string toString();
};

class CFileProvider: public CVariableProvider
{
protected:
    QLineEdit *lineEdit;
    QPushButton *openButton;
    string fdest;
public:
    CFileProvider(string name, string destination);
    virtual string toString();
};

class CChoiceProvider: public CVariableProvider
{
protected:
    vector<string> choicevec;
    QButtonGroup *buttonGroup;
public:
    CChoiceProvider(string name, vector<string> choices);
    virtual string toString();
};

class CBooleanProvider: public CVariableProvider
{
protected:
    QCheckBox *checkBox;
public:
    CBooleanProvider(string name);
    virtual string toString();
};

class CMultiChoiceProvider: public CVariableProvider
{
protected:
    vector<QCheckBox*> choicevec;
    QButtonGroup *buttonGroup;
public:
    CMultiChoiceProvider(string name, vector<string> choices);
    virtual string toString();
};

class QConfigWidget: public QWidget
{
    Q_OBJECT

    std::string outputfile;
    vector<CVariableProvider*> providers;
    QGridLayout *grid;
public:
    QConfigWidget(QWidget *parent, const char *lua_descrption, const char *property_table, const char *output);
    ~QConfigWidget();

    void buildConfig();
};


#endif
