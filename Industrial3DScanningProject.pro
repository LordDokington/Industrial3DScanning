QT += qml quick
CONFIG += c++14

SOURCES += main.cpp \
    scenerenderer.cpp

RESOURCES += qml.qrc

OTHER_FILES += \
    main.qml \
    bunny.xyz

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

#DESTDIR = ./bin

#installfiles.path = $$DESTDIR
#installfiles.files = bunny.xyz

#INSTALLS += installfiles

HEADERS += \
    scenerenderer.h \
    vertexfileloader.h
