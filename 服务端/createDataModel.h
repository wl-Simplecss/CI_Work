#ifndef CREATEDATAMODEL_H
#define CREATEDATAMODEL_H

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>


/*建立数据库*/
static bool createDataModel()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
        db.setHostName("127.0.0.1");
        db.setPort(3306);
        db.setDatabaseName("mysql");
        db.setUserName("root");
        db.setPassword("    ");

    if(!db.open()){

        QMessageBox::critical(0, "Cannot open database",
                              "Unable to establish a database connection.", QMessageBox::Cancel);
        return false;
    }else{

    }
    QSqlQuery query;
    //数据库预设信息
    db.close();

    return true;
}



#endif // CREATEDATAMODEL_H
