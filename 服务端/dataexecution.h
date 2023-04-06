#ifndef DATAEXECUTION_H
#define DATAEXECUTION_H
#include <stdio.h>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include<QSqlTableModel>>
#include<QMessageBox>
/*向数据库中添加用户信息*/

QSqlTableModel *model;      //数据库

void addDataModel(QString name, QString pass,QSqlDatabase db)
{


    db = QSqlDatabase::addDatabase("QODBC");
        db.setHostName("127.0.0.1");
        db.setPort(3306);
        db.setDatabaseName("mysql");
        db.setUserName("root");
        db.setPassword("wangleikuaile1");
    QString sql="insert into program.users (name, pass) values("+name+", "+pass+")";
    db.exec(sql);
    db.close();
}

/*登录时查询数据库*/
/*返回值
 * 1 -- 登录成功
 * 2 -- 登录失败，密码错误
 * 3 -- 登录失败，无此用户*/
int loginSearchDataModel(QString id, QString pass,QSqlDatabase db)
{

    QSqlQuery query(db);
    query.exec("select id,password from program.users");
    while(query.next()){
        if(query.value(0).toString() == id){
            if(query.value(1).toString() == pass){
                db.close();
                return 1;   //登录成功
            }
            else{
                db.close();
                return 2;   //密码错误
            }
        }
    }

    return 3;   //登录失败，无此用户
}



#endif // DATAEXECUTION_H
