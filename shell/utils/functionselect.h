#ifndef FUNCTIONSELECT_H
#define FUNCTIONSELECT_H

#include <QList>
#include <QStringList>


typedef struct _FuncInfo : QObjectUserData
{
    int type;
    int index;
    QString nameString;
    QString namei18nString;
}FuncInfo;

Q_DECLARE_METATYPE(_FuncInfo)

class FunctionSelect
{
public:
    explicit FunctionSelect();
    ~FunctionSelect();

public:
    static QList<QStringList> funcsList;

    static QList<QList<FuncInfo>> funcinfoList;

//    static FuncInfo displayStruct;

    static void initValue();
};

#endif // FUNCTIONSELECT_H
