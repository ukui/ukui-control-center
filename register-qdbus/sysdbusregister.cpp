#include "sysdbusregister.h"

#include <QDebug>

SysdbusRegister::SysdbusRegister()
{
}

SysdbusRegister::~SysdbusRegister()
{
}

//QString SysdbusRegister::name () const{
//    return m_name;
//}

//void SysdbusRegister::SetName(QString name){
//    m_name = name;
//}

void SysdbusRegister::exitService(){
    qApp->exit(0);
}

QString SysdbusRegister::GetComputerInfo(){
    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    sprintf(cmd, "dmidecode -t system");
    fp = popen(cmd, "r");

    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        while (!feof(fp)) {
            fgets(buf, sizeof (buf), fp);
            ba.append(buf);
        }
        pclose(fp);
        fp = NULL;
    }
    return QString(ba);
}
