QT       += core gui printsupport testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    birimlistform.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    birimlistform.h \
    configuration.h \
    mainwindow.h

FORMS += \
    birimlistform.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target






INCLUDEPATH += C:/boost/boost
DEPENDPATH += C:/boost/boost



win32: LIBS += -LC:/Qt/KDReport/msvc2017_64/lib/ -lkdreports1

INCLUDEPATH += C:/Qt/KDReport/msvc2017_64/include
DEPENDPATH += C:/Qt/KDReport/msvc2017_64/include





win32: LIBS += -LC:/Mongo/msvc2022x64/lib/ -lbsoncxx

INCLUDEPATH += C:/Mongo/msvc2022x64/include/bsoncxx/v_noabi
DEPENDPATH += C:/Mongo/msvc2022x64/include/bsoncxx/v_noabi


win32: LIBS += -LC:/Mongo/msvc2022x64/lib/ -lmongocxx

INCLUDEPATH += C:/Mongo/msvc2022x64/include/mongocxx/v_noabi
DEPENDPATH += C:/Mongo/msvc2022x64/include/mongocxx/v_noabi


win32: LIBS += -LC:/SerikBLDCoreRelease/MSVC2022x64/lib/ -lSerikBLDCore

INCLUDEPATH += C:/SerikBLDCoreRelease/MSVC2022x64/include
DEPENDPATH += C:/SerikBLDCoreRelease/MSVC2022x64/include





#win32: LIBS += -LC:/Qt/Libs/LibHaru2.3/lib/ -llibhpdf

#INCLUDEPATH += C:/Qt/Libs/LibHaru2.3/include
#DEPENDPATH += C:/Qt/Libs/LibHaru2.3/include


RC_ICONS = logoico.ico

RESOURCES += \
    resource.qrc
