#ifndef SERVE_H
#define SERVE_H

#include <QWidget>
#include <QtNetwork>
#include <QGridLayout>
#include "myserver.h"
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QTableView>
#include <QDebug>
#include<QFileSystemModel>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

class QFile;
class QSqlTableModel;
class Server : public QWidget
{
    Q_OBJECT

public:
    Server(QWidget *parent = nullptr);
    ~Server();

private slots:


    void on_submitButton_clicked();
    void on_rollbackButton_clicked();
    void on_addButton_clicked();
    void on_deleteButton_clicked();
    void showMessage(QString message);  //显示状态信息的槽


    void on_mkdirButton_clicked();

    void on_rmButton_clicked();

private:
    Ui::Server *ui;
    QFile *qssFile;
    MyServer *server;
    QSqlTableModel *model;      //数据库
    QFileSystemModel *Filemodel;
    void setMyLayout();
};
#endif // SERVE_H
