#include "client.h"
#include"logindialog.h"
#include <QApplication>
#include <QLocale>
#include<QDir>
#include <QTranslator>
#include"dialogre.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QTranslator translator;
//    const QStringList uiLanguages = QLocale::system().uiLanguages();
//    for (const QString &locale : uiLanguages) {
//        const QString baseName = "client_" + QLocale(locale).name();
//        if (translator.load(":/i18n/" + baseName)) {
//            a.installTranslator(&translator);
//            break;
//        }
//    }

    QDir dir;
    if(!dir.exists("files"))
    {
        dir.mkdir("files");
    }

    Client w;
    LoginDialog dlg;
    DialogRe w2;
    dlg.show();
    QObject::connect(&dlg, SIGNAL(showRe()),&w2,SLOT(reciveRe()));
    QObject::connect(&w2, SIGNAL(showLogin()),&dlg,SLOT(reciveLogin()));
    QObject::connect(&dlg, SIGNAL(showMain()),&w,SLOT(reciveMain()));
    QObject::connect(&w, SIGNAL(showLoginPage()),&dlg,SLOT(reciveLoginPage()));

    return a.exec();
}
