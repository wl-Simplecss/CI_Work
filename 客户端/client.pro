QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogre.cpp \
    logindialog.cpp \
    main.cpp \
    client.cpp

HEADERS += \
    client.h \
    dialogre.h \
    logindialog.h

QT +=network

FORMS += \
    client.ui \
    dialogre.ui \
    logindialog.ui

RC_ICONS = ico.ico

TRANSLATIONS += \
    client_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qss.qrc
