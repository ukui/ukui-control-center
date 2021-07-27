#include "netconnectwork.h"
#include <QElapsedTimer>

NetconnectWork::NetconnectWork()
{

}

NetconnectWork::~NetconnectWork() {

}

void NetconnectWork::run() {
//    qDebug()<<"-----------started-------------";
    QElapsedTimer t;
    QStringList slist;
    for(int refresh_time = 0; refresh_time < 5; refresh_time++) {
        if(!this->is_wifi_open) break;
        QProcess *wifiPro = new QProcess();
        wifiPro->start("nmcli -f signal,ssid,security device wifi");
        wifiPro->waitForFinished();
        QString shellOutput = "";
        QString output = wifiPro->readAll();
        shellOutput += output;
        slist = shellOutput.split("\n");
//        for(QString name : slist){
//            qDebug()<<name;
//        }
//        qDebug()<<refresh_time<<" "<<slist;
        if(slist.length() > 2) break;
        t.start();
        while(t.elapsed() < 1000);
//        qDebug()<<refresh_time;
    }

//    qDebug()<<"-----------finished-------------";

//    qDebug()<<"slist is--------->"<<slist<<endl;
    emit wifiGerneral(slist);
    emit workerComplete();
}
