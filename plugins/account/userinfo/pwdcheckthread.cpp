#include "pwdcheckthread.h"

#include <QDebug>

PwdCheckThread::PwdCheckThread(QObject *parent):QThread(parent)
{
}

PwdCheckThread::~PwdCheckThread()
{
}

void PwdCheckThread::setArgs(const QString &userName, const QString &userPwd){
    uname = userName;
    upwd = userPwd;
}

void PwdCheckThread::run(){

    FILE * stream;
    char command[128];
    char output[256];

    QString result;;

    QByteArray ba1 = uname.toLatin1();

    //
    if (upwd.contains("'")){
        snprintf(command, 128, "/usr/bin/checkUserPwd %s \"%s\"", ba1.data(), upwd.toLatin1().data());
    } else {

        snprintf(command, 128, "/usr/bin/checkUserPwd %s '%s'", ba1.data(), upwd.toLatin1().data());
    }

    if ((stream = popen(command, "r")) != NULL){

        while(fgets(output, 256, stream) != NULL){
            result = QString(output).simplified();
//            if (QString::compare(QString(output).simplified(), "Succes!") == 0){
//                result = true;
//            }

        }
        pclose(stream);
    }

    emit complete(result);
}
