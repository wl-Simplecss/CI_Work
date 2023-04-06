#include "server.h"
#include "ui_server.h"
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QTableView>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include<QFileSystemModel>
#include<QInputDialog>
#include<stdio.h>
#include"myserver.h"
Server::Server(QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);



    qssFile = new QFile(":/qss/style.qss",this);
    qssFile->open(QFile::ReadOnly);
    QString styleSheet = tr(qssFile->readAll());
    qApp->setStyleSheet(styleSheet);
    qssFile->close();


    this->setWindowTitle("用户信息");

    model = new QSqlTableModel(this);
    model->setTable("users");
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->tableView->setModel(model);

    Filemodel = new QFileSystemModel();
    Filemodel->setRootPath(QDir::currentPath()+"\file");

    QString site="               ";
    ui->treeView->setModel(Filemodel);
    ui->treeView->setRootIndex(Filemodel->index(site));



    //建立服务器
    server = new MyServer();
    server->listen(QHostAddress::LocalHost,6666);   //开始监听

     //动态传递的信号与槽连接
    connect(server, SIGNAL(showMessServerToWidget(QString)), this, SLOT(showMessage(QString)));
}


Server::~Server()
{
    delete server;
    delete model;
    delete ui;
}






/*服务器输出信息*/
void Server::showMessage(QString message)
{
    ui->textBrowser->append(message);
}






/*提交修改按钮*/
void Server::on_submitButton_clicked()
{
    model->database().transaction();
    if(model->submitAll()){
        if(model->database().commit()){
            QMessageBox::information(this, tr("tableModel"), tr("数据修改成功！"));
        }
    }
    else{
        model->database().rollback();
        QMessageBox::warning(this, tr("tableModel"), tr("数据库错误：%1").arg(model->lastError().text()), QMessageBox::Ok);
    }
}

/*撤销修改(已提交的不能修改)*/
void Server::on_rollbackButton_clicked()
{
    model->revertAll();
}

/*添加记录按钮*/
void Server::on_addButton_clicked()
{
    int rowNum = model->rowCount();
    QString id = "";
    model->insertRow(rowNum);
    model->setData(model->index(rowNum,0), id);
}

/*删除选中行按钮*/
void Server::on_deleteButton_clicked()
{
    int curRow = ui->tableView->currentIndex().row();
    model->removeRow(curRow);
    int ok = QMessageBox::warning(this, tr("删除当前行！"), tr("你确定删除当前行吗？"), QMessageBox::Yes, QMessageBox::No);
    if(ok == QMessageBox::No){
        model->revert();
    }
    else{
        model->submit();
    }
}

void Server::on_mkdirButton_clicked()
{
    QModelIndex index = ui->treeView->currentIndex();
        if (!index.isValid()) {
            return;
        }
        QString dirName = QInputDialog::getText(this,
                                                tr("Create Directory"),
                                                tr("Directory name"));
        if (!dirName.isEmpty()) {
            if (Filemodel->mkdir(index, dirName).isValid()) {
                MyServer* my=new MyServer();//更新索引
                my->NewIndex();
                delete my;
            }else{
                QMessageBox::information(this,
                                         tr("Create Directory"),
                                         tr("Failed to create the directory"));
            }
        }
}


void Server::on_rmButton_clicked()
{
    QModelIndex index = ui->treeView->currentIndex();
        if (!index.isValid()) {
            return;
        }
        bool ok;
        if (Filemodel->fileInfo(index).isDir()) {
            ok = Filemodel->rmdir(index);
            MyServer* my=new MyServer();
            my->NewIndex();//更新索引
            delete my;
        } else {
            ok = Filemodel->remove(index);
            MyServer* my=new MyServer();
            my->NewIndex();//更新索引
            delete my;
        }
        if (!ok) {
            QMessageBox::information(this,
                             tr("Remove"),
                             tr("Failed to remove %1").arg(Filemodel->fileName(index)));
        }
}

