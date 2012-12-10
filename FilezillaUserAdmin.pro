#-------------------------------------------------
#
# Project created by QtCreator 2012-12-04T08:53:11
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FilezillaUserAdmin
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    filezillaserverdesc.cpp \
    filezillaadminconnection.cpp \
    md5.cpp \
    filezillaaccounts.cpp \
    filezillausermodel.cpp \
    editdirectoriesdialog.cpp

HEADERS  += mainwindow.h \
    filezillaserverdesc.h \
    filezillaadminconnection.h \
    md5.h \
    filezillaaccounts.h \
    filezillausermodel.h \
    editdirectoriesdialog.h

FORMS    += mainwindow.ui \
    editdirectoriesdialog.ui
