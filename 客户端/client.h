#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QAbstractSocket>
#include<QStandardItem>
QT_BEGIN_NAMESPACE

class QTcpSocket;
class QFile;

namespace Ui { class Client; }
QT_END_NAMESPACE


class Client : public QWidget
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();

private slots:
    void reciveMain();
    void readFileMessage();
    void on_ButtonLogout_clicked();
    void disPlayError(QAbstractSocket::SocketError);
    void sendFileMessToServer(QString Mess, QString str1, QString str2);

    void on_openButton_clicked();

    void on_upButton_clicked();
    void startUpload();
    void updateUploadProgress(qint64 numBytes);

    void on_searchButton_clicked();

    void on_downButton_clicked();
    void on_unDownButton_clicked();

    void on_treeView_clicked(const QModelIndex &index);
    void on_getFile_clicked();


    void addItemsRecursive(const QJsonObject& jsonObject, QStandardItem* parentItem);
    void createModelFromJson();



    void on_treeView_pressed(const QModelIndex &index);

signals:
    void showLoginPage();
private:
    Ui::Client *ui;
    QFile *qssFile;




    void openFile();
    int stateful;
    const QString LOGOUT = "0";     //登出标记
    const QString UPDATE = "20";        //上传文件请求
    const QString UPSTART = "21";       //开始上传消息
    const QString UPREJ = "22";         //文件已存在，拒绝上传
    const QString SEARCH = "30";        //查找文件
    const QString FILEEXIST = "31";     //文件存在
    const QString FILENONEXIST = "32";  //文件不存在
    const QString FILENAMENON = "33";   //查找文件名为空
    const QString DOWNSTART = "34";     //开始下载
    const QString EXITAPP = "40";       //退出应用程序
    const QString MESSNULL = "-1";      //空信息
    const QString FILEINDEX = "41";       //获取文件索引
    QString currentFile;

    //打开要上传的文件
    //网络文件变量
    QTcpSocket *tcpClientFile;
    qint64 messageSize;     //接收到的信息大小
    QString upFileName;       //要上传文件名
    qint64 upPerSize;       //传送文件时每次传送的大小
    qint64 upTotalBytes;    //上传的文件总大小
    qint64 upBytesWritten;  //已上传大小
    qint64 upBytesToWrite;  //要上传的大小
    bool isUploadingFile;   //上传标记
    QFile *fileToUpdate;    //上传的文件
    QByteArray  upBlock;    //上传缓冲区

    QString downFileName;   //下载文件的文件名
    QString showDownFileName;   //下载文件的文件名
    bool isDownloadingFile; //下载状态
    qint64 downTotalBytes;  //总共下载大小
    qint64 downBytesReceived;   //已下载大小
    qint64 downFileNameSize;    //下载文件名大小
    QFile *downLocalFile;   //下载文件名
    QByteArray downBlock;   //下载缓冲区

signals:
    void startUploadSignal();

};
#endif // CLIENT_H
