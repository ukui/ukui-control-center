#ifndef OOBE_PLUGIN_INFACE_H
#define OOBE_PLUGIN_INFACE_H

#include <QtPlugin>
#include <functional>
#include <QString>
#include <QObject>
#include <functional>

#define Successful 0
#define Loading 1
#define ParameterErr 2
#define NetWorkError 3
#define InvalidTime 4
#define UnKnowErr 5
#define ReplyUrlErr 6


class QWidget;
namespace hp {
  //typedef void (*CallBack)(int status , QString msg) ;
     // std::function<void(int ,QString)>  CallBack;
    class QRCodeInterface
    {
    public:
        virtual ~QRCodeInterface() {}
        virtual const QString name() = 0;
        virtual const QString description() = 0;
        virtual uint32_t getPluginHints() = 0;
        virtual QWidget *createWidget(QWidget *parent) = 0;
        virtual void refreshWidget(QWidget* wQRCode) = 0;
       // virtual void registerCallBack(CallBack cbfun)=0;
        virtual void registerCallBack(std::function<void(int, QString)>)=0;
        int getWidth() const
        {
            return m_width;
        }
        int getHeight() const
        {
            return m_height;
        }

    protected:
        int m_width;  // width
        int m_height; // height
        //CallBack m_callBackFun=nullptr;
        std::function<void(int ,QString)> m_callBackFun=nullptr;
    };
}

Q_DECLARE_INTERFACE (hp::QRCodeInterface, "com.hp.QRCodeInterface")
#endif
