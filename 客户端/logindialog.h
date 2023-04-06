#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QAbstractSocket>

class QTcpSocket;
class QFile;
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
     void sendMessToServer(QString Mess, QString str1, QString str2);
     void readMessage();

    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_loginButton_clicked();
    void reciveLogin();
    void reciveLoginPage();
    void on_regiButton_clicked();

    void  disPlayError(QAbstractSocket::SocketError);



signals:
    void showRe();
    void showMain();

private:
    Ui::LoginDialog *ui;
    QFile *qssFile;
    QTcpSocket *tcpClient;
    qint64 messageSize;     //接收到的信息大小
    const QString LOGOUT = "0";     //登出标记
    const QString LOGINREQ = "1";        //登录标记
    const QString REGIREQ = "2";         //注册标记
    const QString LOGINSUCCESS = "01";  //登录成功
    const QString LOGINPASSERROR = "02";    //登录密码错误
    const QString LOGINIDNON = "03";    //登录用户名不存在
    const QString REGISUCCESS = "11";   //注册成功
    const QString REGIIDBLANKERROR = "12"; //注册时ID为空错误
    const QString REGIPASSBLANKERROR = "13";    //注册时密码为空错误
    const QString REGIIDEXIT = "14";    //注册用户名已存在
};

#endif // LOGINDIALOG_H
