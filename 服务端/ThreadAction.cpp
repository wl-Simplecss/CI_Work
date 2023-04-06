#include <QSqlDatabase>
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include<QFileSystemModel>
#include"myserver.h"
#include"ThreadAction.cpp"
#include "socketthread.h"
#include "server.h"


/*登出处理*/
void SocketThread::logoutExecution(QString str1)
{
    QString id = str1;
    //抛出界面输出信息的信号
    QString showMess = "[-登出-]" + id + "登出成功";
    emit showMessThreadToServer(showMess);
}

/*处理登录请求*/
void SocketThread::loginExecution(QString str1, QString str2)
{
    QString id = str1;
    QString pass = str2;
    if(id == ""){   //ID为空
        sendMessToClient(REGIIDBLANKERROR);     //向客户端发送ID为空信息
        return ;
    }else if(pass == ""){    //密码为空
        sendMessToClient(REGIPASSBLANKERROR);   //向客户端发送密码为空的信息
        return ;
    }
    int result=1;
    QString passwd;
    if(result){
        //在线程中打开数据库的操作

        QString str=QString::number((long long )QThread::currentThreadId(), 10);
        QSqlQuery query(db);
        QString sql="select password from program.users where id="+id+";";
        query.exec(sql);
        while(query.next()){
            passwd=query.value(0).toString();
            qDebug()<<query.value(0).toString()<<"@@@@@@@@@";
        }
    }
    qDebug() << pass <<"  "<<passwd;
    if(pass==passwd&&pass!=""){
        result=1;
    }else{
        result=2;
    }
    if(result == 1){        //登录成功
        qDebug() << id <<"登录成功";

        //抛出界面输出信息的信号
        QString showMess = "[-登录-]" + id + "登录成功";
        emit showMessThreadToServer(showMess);
        sendMessToClient(LOGINSUCCESS);     //向客户端发送登录成功信息
    }
    else if(result == 2){   //登录失败，密码错误
        qDebug() << id <<"密码错误";
        sendMessToClient(LOGINPASSERROR);   //向客户端发送登录密码错误的信息
    }
    else{                   //登录失败，查无此人
        qDebug() << id <<"查无此人";
        sendMessToClient(LOGINIDNON);       //向客户端发送查无此人的信息
    }
}

/*处理注册请求*/
void SocketThread::registerExecution(QString str1, QString str2)
{
    QString name = str1;
    QString pass = str2;
    int id;
    if(name == ""){   //ID为空
        sendMessToClient(REGIIDBLANKERROR);     //向客户端发送ID为空信息
    }else if(pass == ""){    //密码为空
        sendMessToClient(REGIPASSBLANKERROR);   //向客户端发送密码为空的信息
    }else{       //注册成功
        if(true){


            QSqlQuery query(db);
            QString sql="insert into  program.users(name,password)   values('"+name+"','"+pass+"')  ;";
            query.exec(sql);
            sql="select MAX(id) from program.users ;";
            query.exec(sql);
            while(query.next()){
              id=query.value(0).toInt();
              qDebug()<<id<<"hhhhhhhhhhhhhhh";
            }
            db.close();
        };
        QString showMess = "[-注册-]" + QString::number(id) + "注册成功";   //抛出界面输出信息的信号
        emit showMessThreadToServer(showMess);
        sendMessToClient(QString::number(id));

    }

}

/*处理上传请求*/
void SocketThread::updateExecution(QString str1, QString str2)
{
    QString id = str1;
//    QString upName = str2;
    int realNameIndex = str2.lastIndexOf("/");
    QString realName = str2.right(str2.length ()-realNameIndex-1);  //取真正文件名
    QString showMess = "[-上传文件-]" + id + "上传" + realName;
    emit showMessThreadToServer(showMess);
    isGettingFile = true;   //设置为接收文件标记
    sendMessToClient(UPSTART);  //向客户端发送开始上传信息
}

/*文件已存在，拒绝上传请求*/
void SocketThread::updateReject()
{
    sendMessToClient(UPREJ);    //向客户端发送拒绝上传信息
}





/*查找请求处理*/
void SocketThread::searchExecution(QString str1, QString str2)
{
    QString id = str1;
    if(str2 == ""){     //如果查找的文件名为空
        sendMessToClient(FILENAMENON);  //向客户端发送文件名为空信息
    }
    else{       //查找的文件名不为空
        QString searchFileName = "./files/" + str2;
        QFile file(searchFileName);
        if(file.exists()){      //文件存在
            qDebug() << searchFileName << tr("存在");
            sendMessToClient(FILEEXIST);    //向客户端发送文件训在信息
        }
        else{       //文件不存在
            qDebug() << searchFileName << tr("不存在");
            sendMessToClient(FILENONEXIST); //向客户端发送文件不存在信息
        }
    }
}

/*开始下载处理*/
void SocketThread::downStartExecution(QString str1, QString str2)
{
    QString id = str1;
    QString dFileName = str2;
    if(str2 == ""){     //如果查找的文件名为空
        qDebug()<<"asdasdasdadasda";
        sendMessToClient(FILENAMENON);  //向客户端发送文件名为空信息
        return ;
    }
    downFileName = "./files/" + dFileName;
    QString showMess = "[-下载-]" + id + "下载" + dFileName;
    emit showMessThreadToServer(showMess);

    qDebug() << "start download" << dFileName;
    downPerSize = 64*1024;   //64kb
    downTotalBytes = 0;
    downBytesWritten = 0;
    downBytesToWrite = 0;
    emit startDownloadSignal(); //发送上传文件信号
    qDebug() << "here";
}



void SocketThread::downStartIndex(QString str1)
{
    QString id = str1;
    QString dFileName = "./index/index.json";
    downFileName =  dFileName;

 //   dFileName = "./index/"+dFileName;
    QString showMess = "[-下载-]" + id + "下载" + dFileName;
    emit showMessThreadToServer(showMess);
    qDebug() << "start download" << dFileName;
    downPerSize = 64*1024;   //64kb
    downTotalBytes = 0;
    downBytesWritten = 0;
    downBytesToWrite = 0;
    emit startDownloadSignal(); //发送上传文件信号
    qDebug() << "here";
}





/*开始下载文件时服务器开始传输*/
void SocketThread::startDownload()
{
    fileToDownload = new QFile(downFileName);
    if(!fileToDownload->open(QFile::ReadOnly)){
        qDebug() <<"server: open file error!";
        return;
    }
    //获取文件大小
    downTotalBytes = fileToDownload->size();
    QDataStream sendOut(&downBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_6);
    QString currentFileName = downFileName.right(downFileName.size() - downFileName.lastIndexOf('/') - 1);
    //保留总大小信息空间、文件名大小信息空间，然后输入文件名
    sendOut << qint64(0) << qint64(0) << currentFileName;
    //这里的总大小是总大小信息、文件名大小信息、文件名和实际文件大小的总和
    downTotalBytes += downBlock.size();  //要发送的整个数据的大小（文件头结构+实际文件大小）  放在数据流最开始，占用第一个qint(64)的空间
    sendOut.device()->seek(0);
    //返回outBlock的开始，用实际的大小信息代替两个qint(0)空间
    sendOut << downTotalBytes <<qint64((downBlock.size() - sizeof(qint64) * 2));
    //发送完文件头结构后剩余的数据大小

    downBytesToWrite = downTotalBytes - socket->write(downBlock);
    downBlock.resize(0);//outBlock是暂存数据的，最后要将其清零
    qDebug() <<"trans";
    isDownloadingFile = true;
}



/*文件下载时服务器传输过程*/
void SocketThread::updateDownloadProgress(qint64 numBytes)
{
    if(isDownloadingFile){
        //已经发送数据的大小
        downBytesWritten += (int)numBytes;
        //如果已经发送了数据
        if(downBytesToWrite > 0){
            //每次发送payloadSize大小的数据，这里设置为64KB，如果剩余的数据不足64KB就发送剩余数据的大小
            downBlock = fileToDownload->read(qMin(downBytesToWrite, downPerSize));
            //发送完一次数据后还剩余数据的大小

            downBytesToWrite -= (int)socket->write(downBlock);
            //emit hasWritten(haswrite);

            //socket->waitForBytesWritten();
            //清空发送缓冲区
            downBlock.resize(0);
        }
        else{   //如果没有发送任何数据，则关闭文件
            fileToDownload->close();
        }
        //如果发送完毕
        if(downBytesWritten == downTotalBytes){
            fileToDownload->close();
            isDownloadingFile = false;    //退出文件传输状态
        }
    }
}

