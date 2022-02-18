#include "ukccbluetoothconfig.h"

bool ukccbluetoothconfig::m_isIntel = false;

ukccbluetoothconfig::ukccbluetoothconfig()
{

}

void ukccbluetoothconfig::launchBluetoothServiceStart(const QString &processName)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    QProcess *process = new QProcess();
    QString cmd = processName;
    //cmd.append(" -o");
    qDebug () << Q_FUNC_INFO << cmd << __LINE__;
    process->startDetached(cmd);
}

void ukccbluetoothconfig::killAppProcess(const quint64 &processId)
{
//    QString strCommand = "ps -ef|grep " + processName + " |grep -v grep |awk '{print $2}'";
//    QList <quint64> processId;
//    checkProcessRunning(processName,processId);
//    for(quint64 tempId : processId)
//    {
    qDebug () << Q_FUNC_INFO << "========================" << processId;
    QProcess *process = new QProcess();
    QString cmd = QString("kill -9 %1").arg(processId);
    process->startDetached(cmd);
//    }
}

bool ukccbluetoothconfig::checkProcessRunning(const QString &processName, QList<quint64> &listProcessId)
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    bool res(false);
    QString strCommand;
    //if(processName == BluetoothServiceExePath)
    //    strCommand = "ps -ef|grep '" + processName + " -o' |grep -v grep |awk '{print $2}'";
    //else
        strCommand = "ps -ef|grep '" + processName + "' |grep -v grep |awk '{print $2}'";

    QByteArray ba = strCommand.toLatin1();
    const char* strFind_ComName = ba.data();
    //const char* strFind_ComName = convertQString2char(strCommand);
    FILE * pPipe = popen(strFind_ComName, "r");
    if (pPipe)
    {
        std::string com;
        char name[512] = { 0 };
        while (fgets(name, sizeof(name), pPipe) != NULL)
        {
            int nLen = strlen(name);
            if (nLen > 0
                && name[nLen - 1] == '\n')
                //&& name[0] == '/')
            {
                name[nLen - 1] = '\0';
                listProcessId.append(atoi(name));
                res = true;
                break;
            }
        }
        pclose(pPipe);
    }
    return res;
}
