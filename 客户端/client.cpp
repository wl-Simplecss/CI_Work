#include "client.h"
#include "ui_client.h"
#include"logindialog.h""
#include <QGridLayout>
#include <QCloseEvent>
#include <QMessageBox>
#include <QTcpSocket>
#include<iostream>
#include <QDebug>
#include <QtNetwork>
#include <QFileDialog>
#include<QFileSystemModel>
#include<QStandardItem>
#include <QFile>
Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
 //   this->setWindowFlags(Qt::FramelessWindowHint);
    setWindowTitle("传输文件");

    tcpClientFile = new QTcpSocket(this);
    tcpClientFile->connectToHost(QHostAddress::LocalHost,6666);


    //先不进行文件传输
    isUploadingFile = false;

    //接收文件初始化
    isDownloadingFile = false;
    downTotalBytes = 0;
    downBytesReceived = 0;
    downFileNameSize = 0;

    connect(tcpClientFile, &QTcpSocket::readyRead, this, &Client::readFileMessage);
    //连接错误
    connect(tcpClientFile, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(disPlayError(QAbstractSocket::SocketError)));//发生错误时error信号
    //发射开始上传信息好后开始上传
    connect(this, SIGNAL(startUploadSignal()), this, SLOT(startUpload()));
    //上传过程中更新上传进度
    connect(tcpClientFile, SIGNAL(bytesWritten(qint64)), this, SLOT(updateUploadProgress(qint64)));

    qDebug() <<"ok";

    this->stateful=1;

}

Client::~Client()
{
    delete ui;
}


//------------------------------------------------------------------------跳转逻辑。

void Client::reciveMain(){
    this->show();
}


//------------------------------------------------------------------------
void Client::on_ButtonLogout_clicked()
{
    this->hide();
    emit showLoginPage();
}



void Client::sendFileMessToServer(QString Mess, QString str1, QString str2){
    QByteArray requestBlock;    //用来存放请求信息
    QDataStream out(&requestBlock, QIODevice::WriteOnly);
    //设置数据流的版本，客户端和服务器使用的版本要相同
    out.setVersion(QDataStream::Qt_5_6);//版本
    out << (quint64)0;//写入数据以前可能不知道实际数据的大小，所以要先在数据块的最前面留四个字节位置，以便以后填写数据大小
    out << Mess << str1 << str2;        //输入实际数据
    out.device()->seek(0);//跳转到数据块头部
    out << (quint64)(requestBlock.size() - sizeof(quint64));
    tcpClientFile->write(requestBlock); //发送
    qDebug() <<"send" << Mess << str1 << str2;
    requestBlock.resize(0);
}


/*接收信息*/
void Client::readFileMessage()
{


  if(!isDownloadingFile){     //不是接收文件状态

        QString getMess;       //接收到的消息
        messageSize = 0;    //初始化信息大小
        QDataStream in(tcpClientFile);
        in.setVersion(QDataStream::Qt_5_6);
        if(messageSize == 0){   //如果是刚开始接收数据
                                                   //如果是则保存到blockSize变量中，否则直接返回，继续接收数据
            if(tcpClientFile->bytesAvailable() < (int)sizeof(quint64)) return; //判断接收的数据是否大于四字节，也就是文件的大小信息所占的空间
            in >> messageSize;
        }
        //如果没有得到全部的数据，则返回，继续接收数据
        if(tcpClientFile->bytesAvailable() < messageSize) return;
        //将接收到的数据存放在变量中
        in >> getMess;
        qDebug() << "receive" << getMess;

        //根据接收信息进行处理
        if(getMess == UPSTART){    //开始上传文件
            qDebug() << "start update" << upFileName;
            upPerSize = 64*1024;   //64kb
            upTotalBytes = 0;
            upBytesWritten = 0;
            upBytesToWrite = 0;
            emit startUploadSignal(); //发送上传文件信号
        }
        else if(getMess == UPREJ){      //文件已存在，重新上传
            ui->filenameLabel->setText(tr("此文件已存在，请重新上传"));
            ui->openButton->setEnabled(true);
            ui->searchButton->setEnabled(true);
            ui->ButtonLogout->setEnabled(true);
        }
        else if(getMess == FILEEXIST){  //查询的文件存在
            QString searchFileName = ui->searchEdit->text();
            if(searchFileName!=""){
                showDownFileName = searchFileName;
            }

            int realNameIndex = searchFileName.lastIndexOf("/");             //要下载的文件名是查找到的文件名
            QString realName = searchFileName.right(searchFileName.length ()-realNameIndex-1);
            QString labelShow = realName + " 存在";
            ui->downLabel->setText(labelShow);      //显示文件存在，接下来选择是否下载
            ui->downButton->setEnabled(true);
            ui->unDownButton->setEnabled(true);     //选择下载或不进行下载
            ui->ButtonLogout->setEnabled(false);
            ui->openButton->setEnabled(false);
            ui->searchButton->setEnabled(false);
        }
        else if(getMess == FILENONEXIST){   //查询的文件不存在
            QString searchFileName = ui->searchEdit->text();    //获取要查找的文件名
            int realNameIndex = searchFileName.lastIndexOf("/");
            QString realName = searchFileName.right(searchFileName.length ()-realNameIndex-1);  //取真正文件名
            QString labelShow = realName + " 不存在";
            ui->downLabel->setText(labelShow);
        }
        else if(getMess == FILENAMENON){    //查询的文件名为空

            ui->downLabel->setText(tr("查询的文件名不能为空"));

        }
    }

    //接收文件传输状态
    else{

      if(this->stateful==-1){

          QDataStream in(tcpClientFile);
          in.setVersion(QDataStream::Qt_5_6);
          this->stateful=0;//  结束索引状态。
          //如果接收到的数据小于16个字节，保存到来的文件头结构
          if(downBytesReceived <= sizeof(qint64) * 2){
              if((tcpClientFile->bytesAvailable() >= sizeof(qint64) * 2) && (downFileNameSize == 0)){

                  in >> downTotalBytes >> downFileNameSize;//接收数据总大小信息和文件名大小信息
                  qDebug() << "downTotalBytes" << downTotalBytes << "downFileNameSize" << downFileNameSize;
                  downBytesReceived += sizeof(qint64) * 2;
              }
              if((tcpClientFile->bytesAvailable() >= downFileNameSize) && (downFileNameSize != 0)){
                  //接收文件名，并建立文件
                  in >> downFileName;
                  ui->downLabel->setText(tr("下载文件 %1").arg(downFileName));
                  downFileName = "./index/" + downFileName;   //将下载文件装入files文件夹中
                  qDebug() << downFileName;
                  downBytesReceived += downFileNameSize;
                  downLocalFile = new QFile(downFileName);
                  if(!downLocalFile->open(QFile::WriteOnly)){   //若文件不存在，会自动创建一个
                      qDebug() << "Client: open file error!";
                      return;
                  }
              }
              else{
                  return;
              }
          }
          //文件未接收完时继续接收
          if(downBytesReceived < downTotalBytes){
              downBytesReceived += tcpClientFile->bytesAvailable();
              downBlock = tcpClientFile->readAll();
              downLocalFile->write(downBlock);    //写入文件
              downBlock.resize(0);
          }

          if(downBytesReceived == downTotalBytes){
              downLocalFile->close();
              downTotalBytes = 0;
              downBytesReceived = 0;
              downFileNameSize = 0;
              isDownloadingFile = false;  //状态设置为不接收文件
          }



          return ;
      }


        QDataStream in(tcpClientFile);
        in.setVersion(QDataStream::Qt_5_6);
        //如果接收到的数据小于16个字节，保存到来的文件头结构
        if(downBytesReceived <= sizeof(qint64) * 2){
            if((tcpClientFile->bytesAvailable() >= sizeof(qint64) * 2) && (downFileNameSize == 0)){
                //接收数据总大小信息和文件名大小信息
                in >> downTotalBytes >> downFileNameSize;
                qDebug() << "downTotalBytes" << downTotalBytes << "downFileNameSize" << downFileNameSize;
                downBytesReceived += sizeof(qint64) * 2;
            }
            if((tcpClientFile->bytesAvailable() >= downFileNameSize) && (downFileNameSize != 0)){
                //接收文件名，并建立文件
                in >> downFileName;
                ui->downLabel->setText(tr("下载文件 %1").arg(downFileName));
                downFileName = "./files/" + downFileName;   //将下载文件装入files文件夹中
                qDebug() << downFileName;
                downBytesReceived += downFileNameSize;
                downLocalFile = new QFile(downFileName);
                if(!downLocalFile->open(QFile::WriteOnly)){   //若文件不存在，会自动创建一个
                    qDebug() << "Client: open file error!";
                    return;
                }
            }
            else{
                return;
            }
        }
        //文件未接收完时继续接收
        if(downBytesReceived < downTotalBytes){
            downBytesReceived += tcpClientFile->bytesAvailable();
            downBlock = tcpClientFile->readAll();
            downLocalFile->write(downBlock);    //写入文件
            downBlock.resize(0);
        }

        //更新进度条
        ui->downBar->setMaximum(downTotalBytes);
        ui->downBar->setValue(downBytesReceived);

        //接收数据完成时
        if(downBytesReceived == downTotalBytes){
            downLocalFile->close();
            downTotalBytes = 0;
            downBytesReceived = 0;
            downFileNameSize = 0;
            QString labelShow = "已完成下载 " + showDownFileName;
            ui->downLabel->setText(labelShow);
            ui->ButtonLogout->setEnabled(true);
            ui->openButton->setEnabled(true);
            ui->searchButton->setEnabled(true);
            ui->searchEdit->setText(tr(""));
            isDownloadingFile = false;  //状态设置为不接收文件
        }
    }


}

/*接收信息错误处理*/
void Client::disPlayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpClientFile->errorString();
}



/*点击打开文件按钮*/
void Client::on_openButton_clicked()
{
    ui->upBar->reset();
    ui->filenameLabel->setText(tr("状态：等待打开文件！"));
    openFile();     //打开文件
}

/*打开要上传的文件*/
void Client::openFile()
{
    upFileName = QFileDialog::getOpenFileName(this);
    if(!upFileName.isEmpty()){
        ui->upButton->setEnabled(true);     //上传按钮生效
        int realNameIndex = upFileName.lastIndexOf("/");
        QString realName = upFileName.right(upFileName.length ()-realNameIndex-1);  //取真正文件名
        ui->filenameLabel->setText(tr("文件名： %1").arg(realName));
        qDebug() << upFileName;
    }
}

/*开始上传按钮*/
void Client::on_upButton_clicked()
{
    //给服务器发送上传请求
    QString id = ui->idLineEdit->text();
    sendFileMessToServer(UPDATE, id, upFileName);  //向服务器发送开始上传请求

    ui->upButton->setEnabled(false);    //文件开始传输时上传按钮失效
    ui->openButton->setEnabled(false);  //打开文件按钮失效
    ui->ButtonLogout->setEnabled(false);  //退出登录按钮失效
    ui->searchButton->setEnabled(false);    //查找按钮失效
    ui->downButton->setEnabled(false);  //下载按钮失效
}

/*开始上传文件*/
void Client::startUpload()
{
    fileToUpdate = new QFile(upFileName);
    if(!fileToUpdate->open(QFile::ReadOnly)){
        qDebug() <<"client: open file error!";
        return;
    }
    //获取文件大小
    upTotalBytes = fileToUpdate->size();
    QDataStream sendOut(&upBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_6);
    QString currentFileName = upFileName.right(upFileName.size() - upFileName.lastIndexOf('/') - 1);
    //保留总大小信息空间、文件名大小信息空间，然后输入文件名
    sendOut << qint64(0) << qint64(0) << currentFileName;
    //总大小是总大小信息、文件名大小信息、文件名和实际文件大小的总和
    upTotalBytes += upBlock.size();  //要发送的整个数据的大小（文件头结构+实际文件大小）  放在数据流最开始，占用第一个qint(64)的空间
    sendOut.device()->seek(0);
    //返回outBlock的开始，用实际的大小信息代替两个qint(0)空间
    sendOut << upTotalBytes <<qint64((upBlock.size() - sizeof(qint64) * 2));
    //发送完文件头结构后剩余的数据大小
    upBytesToWrite = upTotalBytes - tcpClientFile->write(upBlock);
    upBlock.resize(0);  //outBlock是暂存数据的，最后要将其清零
    qDebug() <<"transenddd";
    isUploadingFile = true;     //修改正在上传文件标记
    int realNameIndex = upFileName.lastIndexOf("/");
    QString realName = upFileName.right(upFileName.length ()-realNameIndex-1);  //取文件名
    ui->filenameLabel->setText(tr("正在上传： %1").arg(realName));
}

/*文件传输过程*/
void Client::updateUploadProgress(qint64 numBytes)
{
    if(isUploadingFile){
        //已经发送数据的大小
        upBytesWritten += (int)numBytes;
        //如果已经发送了数据
        if(upBytesToWrite > 0){
            //每次发送payloadSize大小的数据，这里设置为64KB，如果剩余的数据不足64KB就发送剩余数据的大小
            upBlock = fileToUpdate->read(qMin(upBytesToWrite, upPerSize));
            //发送完一次数据后还剩余数据的大小
            upBytesToWrite -= (int)tcpClientFile->write(upBlock);
            //清空发送缓冲区
            upBlock.resize(0);
        }
        else{   //如果没有发送任何数据，则关闭文件
            fileToUpdate->close();
        }
        //更新进度条
        ui->upBar->setMaximum(upTotalBytes);
        ui->upBar->setValue(upBytesWritten);
        //如果发送完毕
        if(upBytesWritten == upTotalBytes){
            int realNameIndex = upFileName.lastIndexOf("/");
            QString realName = upFileName.right(upFileName.length ()-realNameIndex-1);  //取真正文件名
            ui->filenameLabel->setText(tr("传送文件 %1 成功").arg(realName));
            fileToUpdate->close();
            isUploadingFile = false;    //退出文件传输状态
            ui->openButton->setEnabled(true);   //文件传输完成后打开文件按钮有效
            ui->ButtonLogout->setEnabled(true);   //退出登录按钮有效
            ui->searchButton->setEnabled(true); //查找按钮有效
        }
    }
}

/*------------------------------上传模块结束----------------------------------*/

/*------------------------------下载模块-------------------------------------*/

/*查找服务器文件*/
void Client::on_searchButton_clicked()
{

    QString id = ui->idLineEdit->text();
    QString searchFileName = ui->searchEdit->text();    //获取要查找的文件名
    ui->downBar->reset();
    if(searchFileName==""){
     QModelIndex temp = ui->treeView->currentIndex();//当前选中
     temp = temp.sibling(temp.row(),0);//0 就是第一列元素，1就是第二列元素，依此类推
     QString t = tr("");
     t = ui->treeView->model()->itemData(temp).values()[0].toString();
     QString Filepath=t.mid(t.indexOf("path:")+5, -1);
     QStringList strList = t.split("                    ");
     QChar* tt=Filepath.begin();
     for(int i=0;i<Filepath.size();i++){
         if(*(tt+i)==' '){
             searchFileName=Filepath.mid(0, i);        
             showDownFileName=searchFileName;
             break;
         }
     }
    }
    std::cout<<showDownFileName.toStdString()<<"aasasasas";
    sendFileMessToServer(SEARCH, id, searchFileName);       //给服务器发送查找请求

}

/*查找到后进行下载的按钮*/
void Client::on_downButton_clicked()
{

    QString id = ui->idLineEdit->text();
    sendFileMessToServer(DOWNSTART, id, showDownFileName);  //给服务器发送下载请求
    isDownloadingFile = true;       //客户端进入接收下载文件状态
    ui->unDownButton->setEnabled(false);
    ui->downButton->setEnabled(false);
}

/*查找到后不进行下载的按钮*/
void Client::on_unDownButton_clicked()
{
    QString labelShow = "已取消下载 " + showDownFileName;
    ui->downLabel->setText(labelShow);
    ui->downButton->setEnabled(false);
    ui->unDownButton->setEnabled(false);
    ui->ButtonLogout->setEnabled(true);
    ui->openButton->setEnabled(true);
    ui->searchButton->setEnabled(true);
    ui->searchEdit->setText(tr(""));
}


void Client::on_treeView_clicked(const QModelIndex &index)
{
//    sendFileMessToServer(FILEINDEX, " ", "");

}


void Client::on_getFile_clicked()
{
    QString id = ui->idLineEdit->text();
    this->stateful=-1;//-1代表文件索引传输。
    isDownloadingFile = true;
    sendFileMessToServer(FILEINDEX, id, "../index.json");
    createModelFromJson();

}






void Client::addItemsRecursive(const QJsonObject& jsonObject, QStandardItem* parentItem) {
    // 遍历JSON对象中的所有子项
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {


        QStandardItem* item=new QStandardItem("Folder: "+jsonObject.find("name")->toString());
        QString fileName="";
        QString fileSize=" ";
        if (it.value().isArray()) {

            // 如果子项是数组，递归添加子节点
            QJsonArray array = it.value().toArray();
            for (int i = 0; i < array.size(); ++i) {
                QJsonObject obj = array[i].toObject();
                addItemsRecursive(obj, item);
            }
         }else if(it.key()=="name"){
            fileName="File: "+it.value().toString();
            it++;
            if(it.key()=="type"){
                fileName=fileName+"     type:"+it.value().toString();
                return ;
            }else if(it.key()=="path"){

                QString filePath=it.value().toString();
                fileName=fileName+"         path:"+filePath.mid(61,-1);
                it++;
                fileSize= QString::number(it.value().toDouble(),'f' ,2);
                fileName=fileName+"                    size:"+fileSize;
                it++;
                fileName=fileName+"                    type:"+it.value().toString();
            }

            item=new QStandardItem(fileName);
        }else{
            item=new QStandardItem("###########################");
        }


        // 添加子节点到父节点
        parentItem->appendRow(item);
    }
};

void Client::createModelFromJson() {

    QFile file("./index/index.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

        // 读取JSON数据
    QByteArray jsonData = file.readAll();
    // 解析JSON
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    QJsonObject rootObject = jsonDoc.object();

    // 创建根节点
    QStandardItemModel* model = new QStandardItemModel();
    QStandardItem* rootNode = model->invisibleRootItem();

    // 递归函数，用于添加子节点
    // 开始添加子节点
    addItemsRecursive(rootObject, rootNode);
    for(int row=0; row<model->rowCount(); row++){
        for(int col=0; col<model->columnCount(); col++){
            QStandardItem *item = model->item(row, col);
            QString text = item ? item->text() : "";
            qDebug() << "Row:" << row << ", Col:" << col << ", Text:" << text;
        }
    }
    model->setHorizontalHeaderLabels(QStringList()<<"----名称-------路径----------------大小------------------类型");
    ui->treeView->setModel(model);
    ui->treeView->expandAll();
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}





void Client::on_treeView_pressed(const QModelIndex &index)
{

}

