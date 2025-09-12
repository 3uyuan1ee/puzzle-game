QT += core gui widgets multimedia
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

# 简化临时目录路径
MOC_DIR         = temp/moc
RCC_DIR         = temp/rcc
UI_DIR          = temp/ui
OBJECTS_DIR     = temp/obj

# 简化输出目录设置
CONFIG(debug, debug|release) {
    DESTDIR = ../debug
} else {
    DESTDIR = ../release
}

TARGET = PuzzleGame
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11 warn_on

# macOS 特定设置
macx {
    QMAKE_CC = clang
    QMAKE_CXX = clang++
    CONFIG += x86_64
}

SOURCES += \
    src/ui/help.cpp \
    src/main.cpp \
    src/DlgMenu.cpp \
    src/play4x4.cpp \
    src/IrregularPuzzle.cpp \
    src/JigsawPuzzle.cpp \
    src/LevelSelect.cpp \
    src/CustomMode.cpp \
    src/NetworkClient.cpp \
    src/LoginDialog.cpp \
    src/RankingDialog.cpp \
    src/LevelRankingDialog.cpp \
    src/TimeRankingDialog.cpp \
    src/StepRankingDialog.cpp

HEADERS += \
    src/ui/help.h \
    src/network/protocol.h \
    src/DlgMenu.h \
    src/play4x4.h \
    src/IrregularPuzzle.h \
    src/JigsawPuzzle.h \
    src/LevelSelect.h \
    src/CustomMode.h \
    src/LevelConfig.h \
    src/NetworkClient.h \
    src/LoginDialog.h \
    src/RankingDialog.h \
    src/LevelRankingDialog.h \
    src/TimeRankingDialog.h \
    src/StepRankingDialog.h

FORMS += \
    src/ui/help.ui \
    src/DlgMenu.ui \
    src/play4x4.ui \
    src/IrregularPuzzle.ui \
    src/JigsawPuzzle.ui \
    src/LevelSelect.ui \
    src/CustomMode.ui \
    src/LoginDialog.ui \
    src/RankingDialog.ui \
    src/LevelRankingDialog.ui \
    src/TimeRankingDialog.ui \
    src/StepRankingDialog.ui

RESOURCES += \
    res/img.qrc
