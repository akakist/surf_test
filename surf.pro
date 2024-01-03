QT -= gui

CONFIG += c++14 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH+=jsoncpp-0.5.0/include
SOURCES += \
        jsoncpp-0.5.0/src/lib_json/json_reader.cpp \
        jsoncpp-0.5.0/src/lib_json/json_value.cpp \
        jsoncpp-0.5.0/src/lib_json/json_writer.cpp \
        main.cpp \
        surf.cpp \
        surf_old.cpp \
        utils.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    point.h \
    real.h \
    surf.h \
    utils.h
