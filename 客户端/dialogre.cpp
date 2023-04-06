#include "dialogre.h"
#include "ui_dialogre.h"
#include"logindialog.h"
#include <QGridLayout>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>
#include <QtNetwork>
#include <QFileDialog>
#include <QFile>


DialogRe::DialogRe(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRe)
{
    ui->setupUi(this);

    setWindowTitle("注册");
//    this->setWindowFlags(Qt::FramelessWindowHint);
}

DialogRe::~DialogRe()
{
    delete ui;
}


//------------------------------------------------------------------------跳转逻辑。
void DialogRe::on_pushButton_clicked()
{
    QString name = ui->lineEditName->text();   //获取账户
    QString pass1 = ui->lineEditPass1 ->text();    //获取密码
    QString pass2 = ui->lineEditPass2 ->text();
    ui->lineEditName->setText("") ;
    ui->lineEditPass1 ->setText("") ;
    ui->lineEditPass2 ->setText("") ;

    if(name.size()<2){
            QMessageBox::StandardButton button;
            button = QMessageBox::warning(this, tr("警告"), QString(tr("名字过短")),
                                          QMessageBox::Cancel);

    }else if(pass1.compare(pass2)){
        QMessageBox::StandardButton button;
        button = QMessageBox::warning(this, tr("警告"), QString(tr("两次输入密码不一致")),
                                      QMessageBox::Cancel);
    }else if(pass1.size()<3){
        QMessageBox::StandardButton button;
        button = QMessageBox::warning(this, tr("警告"), QString(tr("密码安全系数过低")),
                                      QMessageBox::Cancel);

    }else {
        LoginDialog* ww=new LoginDialog();
        ww->sendMessToServer(REGIREQ, name, pass1);
        ww->readMessage();
        ww->close();
        this->hide();
        emit showLogin();
    }
}

void DialogRe::reciveRe(){
    this->show();//显示这个界面
}


void DialogRe::on_pushButton_2_clicked()
{
    ui->lineEditName->setText("") ;
    ui->lineEditPass1 ->setText("") ;
    ui->lineEditPass2 ->setText("") ;
    this->hide();
    emit showLogin();
}
//------------------------------------------------------------------------


