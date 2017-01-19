QT += qml quick
CONFIG += c++14

SOURCES += main.cpp \
    scenerenderer.cpp \
    kdtree.cpp \
    SVD.cpp

RESOURCES += qml.qrc

OTHER_FILES += \
    main.qml \
    bunny.xyz

QMAKE_CXXFLAGS+= -fopenmp
QMAKE_LFLAGS +=  -fopenmp
LIBS += -fopenmp

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

#DESTDIR = ./bin

#installfiles.path = $$DESTDIR
#installfiles.files = bunny.xyz

#INSTALLS += installfiles

HEADERS += \
    utils.h \
    scenerenderer.h \
    vertexfileloader.h \
    scenerendererqmlwrapper.h \
    kdtree.h \
    vertexarrayobject.h \
    vertex.h \
    Matrix.h \
    SVD.h
