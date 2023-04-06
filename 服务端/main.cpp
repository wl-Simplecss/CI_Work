#include "server.h"
#include"createDataModel.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir dir;
    if(!dir.exists("files"))
    {
        dir.mkdir("files");
    }
    if(!createDataModel()) return 1;
    Server w;
    w.show();
    return a.exec();
}
