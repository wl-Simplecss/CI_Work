#ifndef MYSERVER_H
#define MYSERVER_H
#include <QtNetwork>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

class MyServer : public QTcpServer
{
    Q_OBJECT
public:

    MyServer();
     void NewIndex();

signals:
    void showMessServerToWidget(QString);   //状态信息传递的信号

private slots:
    bool saveJsonToFile(const QJsonObject& jsonObj, const QString& filePath);
    QJsonObject traverseFolder(const QString& folderPath);
    void getMessThreadToServer(QString);    //状态信息穿的槽


protected:
    virtual void incomingConnection(qintptr socketDescriptor);
};

#endif // MYSERVER_H
