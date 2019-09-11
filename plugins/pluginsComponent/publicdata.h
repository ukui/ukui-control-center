#ifndef PUBLICDATA_H
#define PUBLICDATA_H


#include<QList>
#include <QObject>

class PublicData
{

public:
    explicit PublicData();
    ~PublicData();

    QList<QStringList> subfuncList;

};

#endif // PUBLICDATA_H
