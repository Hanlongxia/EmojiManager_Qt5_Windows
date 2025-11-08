QT += widgets
QT += core gui widgets svg

CONFIG += c++17 release
TARGET = EmojiManager
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/emojilistwidget.cpp \
    src/emojilistdelegate.cpp \
    src/previewdialog.cpp \
    src/splashiconwidget.cpp

HEADERS += \
    src/emoji_meta.h \
    src/mainwindow.h \
    src/emojilistwidget.h \
    src/emojilistdelegate.h \
    src/previewdialog.h \
    src/emoji_meta.h \
    src/splashiconwidget.h

RESOURCES += \
    resources.qrc

DISTFILES += \
    icons/invalid.png
