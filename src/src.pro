TARGET = harbour-cover-miner

CONFIG += sailfishapp

LIBS += -L../taglib -ltaglib

taglib.files += ../taglib/libtaglib.so.1
taglib.path = /usr/share/$${TARGET}/lib
INSTALLS += taglib

app.files = ./harbour-cover-miner
taglib.path = /usr/share/$${TARGET}/lib

INCLUDEPATH += ../taglib \
    ../taglib/taglib/taglib \
    ../taglib/taglib/taglib/toolkit \
    ../taglib/taglib/taglib/ape \
    ../taglib/taglib/taglib/asf \
    ../taglib/taglib/taglib/flac \
    ../taglib/taglib/taglib/it \
    ../taglib/taglib/taglib/mod \
    ../taglib/taglib/taglib/mp4 \
    ../taglib/taglib/taglib/mpc \
    ../taglib/taglib/taglib/mpeg \
    ../taglib/taglib/taglib/mpeg/id3v1 \
    ../taglib/taglib/taglib/mpeg/id3v2 \
    ../taglib/taglib/taglib/mpeg/id3v2/frames \
    ../taglib/taglib/taglib/ogg \
    ../taglib/taglib/taglib/ogg/flac \
    ../taglib/taglib/taglib/ogg/opus \
    ../taglib/taglib/taglib/ogg/speex \
    ../taglib/taglib/taglib/ogg/vorbis \
    ../taglib/taglib/taglib/riff \
    ../taglib/taglib/taglib/riff/aiff \
    ../taglib/taglib/taglib/riff/wav \
    ../taglib/taglib/taglib/s3m \
    ../taglib/taglib/taglib/trueaudio \
    ../taglib/taglib/taglib/wavpack \
    ../taglib/taglib/taglib/xm

SOURCES += \
    harbour-cover-miner.cpp \
    covergenerator.cpp

DISTFILES += \
    qml/harbour-cover-miner.qml \
    qml/Starter.qml \
    qml/about.qml \
    qml/cover/CoverPage.qml

HEADERS += \
    safe_file_names.hpp \
    covergenerator.h
