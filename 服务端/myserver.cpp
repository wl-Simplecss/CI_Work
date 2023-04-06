#include "myserver.h"
#include "socketthread.h"
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>


MyServer::MyServer()
{


    NewIndex();


}


void MyServer::NewIndex(){
    QString folderPath = "    ";
    QJsonObject folderJson = traverseFolder(folderPath);
    QString outputFilePath = "    ";
    if (saveJsonToFile(folderJson, outputFilePath))
        qDebug() << "JSON file saved successfully.";
    else
        qDebug() << "Failed to save JSON file.";


}




// 递归遍历文件夹，生成JSON对象
QJsonObject MyServer::traverseFolder(const QString& folderPath)
{
    QDir dir(folderPath);
    QJsonObject folderJson;
    folderJson["name"] = dir.dirName();
    folderJson["type"] = "folder";
    QJsonArray filesArray;

    // 遍历所有文件和子文件夹
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot))
    {
        if (fileInfo.isFile())
        {
            // 添加文件信息到文件数组中
            QJsonObject fileJson;
            fileJson["name"] = fileInfo.fileName();
            fileJson["type"] = "file";
            fileJson["size"] = fileInfo.size();
            fileJson["path"] = fileInfo.absoluteFilePath();
            filesArray.append(fileJson);
        }
        else if (fileInfo.isDir())
        {
            filesArray.append(traverseFolder(fileInfo.filePath()));
        }
    }

    // 将文件数组添加到文件夹JSON对象中
    folderJson["files"] = filesArray;
    return folderJson;
}

// 将JSON对象保存到文件中
bool MyServer::saveJsonToFile(const QJsonObject& jsonObj, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QJsonDocument jsonDoc(jsonObj);
    file.write(jsonDoc.toJson());
    file.close();
    return true;
}


/*新的连接到来时调用*/
void MyServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() <<"new connect" << socketDescriptor;
    SocketThread *thread = new SocketThread();  //每来一个连接建立一个线程

    thread->write_ptr(socketDescriptor);
    thread->moveToThread(thread);
    thread->start();
    //动态传递的信号与槽连接
    connect(thread, SIGNAL(showMessThreadToServer(QString)), this, SLOT(getMessThreadToServer(QString)));
}

/*槽：从线程接收信息到服务器*/
void MyServer::getMessThreadToServer(QString message)
{
    //抛出从服务器到界面传递的信号
    emit showMessServerToWidget(message);
}
