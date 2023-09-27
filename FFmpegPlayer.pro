QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DESTDIR = $$PWD/bin

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$PWD/ffmpeg-n4.4.4-6-gd5fa6e3a91-win64-lgpl-shared-4.4/ffmpeg.pri)

SOURCES += \
    LabelVideo.cpp \
    MyFFmpeg.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    LabelVideo.h \
    MainWindow.h \
    MyFFmpeg.h

FORMS += \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
