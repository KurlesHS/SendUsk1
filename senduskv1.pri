##################################################
# подключение qtserialport
INCLUDEPATH += $$PWD/..

##################################################

HEADERS += \
    ../SendUSKv1/sendusk1protocol.h \
    ../SendUSKv1/senduskv1.h \
    ../SendUSKv1/senduskv1global.h \
    ../SendUSKv1/senduskv1workingthread.h \
    ../SendUSKv1/usk1incomingcommand.h \
    ../SendUSKv1/usk1outgoingcommand.h

SOURCES += \
    ../SendUSKv1/sendusk1protocol.cpp \
    ../SendUSKv1/senduskv1.cpp \
    ../SendUSKv1/senduskv1workingthread.cpp \
    ../SendUSKv1/usk1incomingcommand.cpp \
    ../SendUSKv1/usk1outgoingcommand.cpp
