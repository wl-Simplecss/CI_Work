#include "logindialog.h"
#include "ui_logindialog.h"
#include <QGridLayout>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>
#include <QtNetwork>
#include <QFileDialog>
#include <QFile>
#include<iostream>
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    qssFile = new QFile(":/qss/style.qss",this);
    qssFile->open(QFile::ReadOnly);
    QString styleSheet = tr(qssFile->readAll());
    qApp->setStyleSheet(styleSheet);
    qssFile->close();
    setWindowTitle("登录");



    tcpClient = new QTcpSocket(this);
    tcpClient->connectToHost(QHostAddress::LocalHost,6666);

    //当有可读数据时，发射readyread信号
    connect(tcpClient, &QTcpSocket::readyRead, this, &LoginDialog::readMessage);
    //连接错误
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(disPlayError(QAbstractSocket::SocketError)));//发生错误时error信号
    //发射开始上传信息好后开始上传




}

LoginDialog::~LoginDialog()
{
    delete ui;
}


//------------------------------------------------------------------------跳转逻辑。

//登录
void LoginDialog::on_loginButton_clicked()
{
    QString id = ui->idLineEdit->text();        //获取账户
    QString pass = ui->passLineEdit->text();    //获取密码
    ui->idLineEdit->setText("");
    ui->passLineEdit->setText("");
    sendMessToServer(LOGINREQ, id, pass);
}

void LoginDialog::reciveLogin(){
    this->show();
}

void LoginDialog::reciveLoginPage(){
    this->show();
}


/*点击注册按钮*/
void LoginDialog::on_regiButton_clicked()
{
     this->hide();
    emit showRe();
}
//-----------------------------------------------------------------------------具体操作


void LoginDialog::sendMessToServer(QString Mess, QString str1, QString str2){
    QByteArray requestBlock;    //用来存放请求信息
    QDataStream out(&requestBlock, QIODevice::WriteOnly);
    //设置数据流的版本，客户端和服务器使用的版本要相同
    out.setVersion(QDataStream::Qt_5_6);//版本
    out << (quint64)0;//因为在写入数据以前可能不知道实际数据的大小，所以要先在数据块的最前面留四个字节位置，以便以后填写数据大小
    out << Mess << str1 << str2;        //输入实际数据
    out.device()->seek(0);//跳转到数据块头部
    out << (quint64)(requestBlock.size() - sizeof(quint64));//填写信息大小
    tcpClient->write(requestBlock); //发送
    qDebug() <<"send" << Mess << str1 << str2;
    requestBlock.resize(0);
}




void LoginDialog::readMessage()
{

    QString getMess;       //接收到的消息
    messageSize = 0;    //初始化信息大小
    QDataStream in(tcpClient);
    in.setVersion(QDataStream::Qt_5_6);
    if(messageSize == 0){   //如果是刚开始接收数据
        //判断接收的数据是否大于四字节，也就是文件的大小信息所占的空间
        //如果是则保存到blockSize变量中，否则直接返回，继续接收数据
        if(tcpClient->bytesAvailable() < (int)sizeof(quint64)) return;
        in >> messageSize;
    }
    //如果没有得到全部的数据，则返回，继续接收数据
    if(tcpClient->bytesAvailable() < messageSize) return;
    //将接收到的数据存放在变量中
    in >> getMess;
    std::cout << "receive:" << getMess.toStdString()<<std::endl;

    //根据接收信息进行处理
    /*登录处理*/
    if(getMess == LOGINPASSERROR){ //登录密码错误
        ui->passLineEdit->setText("");  //密码框重置
        ui->contentLabel->setText(tr("密码错误，请重新输入"));
    }
    else if(getMess == LOGINIDNON){   //登录用户不存在
        ui->idLineEdit->setText("");    //输入框清空
        ui->passLineEdit->setText("");
        ui->contentLabel->setText(tr("用户不存在，请重新输入"));
    }
    else if(getMess == REGIIDBLANKERROR){   //ID为空错误
        ui->idLineEdit->setText("");    //输入框清空
        ui->passLineEdit->setText("");
        ui->contentLabel->setText(tr("用户名不能为空"));
    }
    else if(getMess == REGIPASSBLANKERROR){ //密码为空错误
        ui->passLineEdit->setText("");
        ui->contentLabel->setText(tr("密码不能为空"));
    }else  if(getMess == LOGINSUCCESS){    //登录成功
        this->hide();
        emit showMain();     //登陆成功后显示处理界面
    }
    else if(getMess.toInt() >=10000){      //注册成功
        std::cout << "receive:" << getMess.toStdString();
        QString statusShow = "用户id为：" + getMess;
        ui->contentLabel->setText(statusShow);  //显示用户登录信息
        QMessageBox::StandardButton button;
        button = QMessageBox::information(this, tr("注册id为"), QString(statusShow),
                                      QMessageBox::Close);
    }


}

/*接收信息错误处理*/
void  LoginDialog::disPlayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpClient->errorString();
}







