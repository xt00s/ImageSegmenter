QT += core gui widgets

TARGET = ImageSegmenter
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    src/Category.h \
    src/MainWindow.h \
    src/Scheme.h \
    src/SchemeTree.h \
    src/Helper.h \
    src/SegmentationScene.h \
    src/SegmentationView.h \
    src/ZoomSlider.h \
    src/GraphicsItems.h \
    src/UndoCommands.h \
    src/BrushToolBar.h \
    src/Drawables.h \
    src/OpenFolderDialog.h \
    src/ImageList.h \
    src/NewSchemeDialog.h \
    src/AboutDialog.h \
    src/Slider.h \
    src/Selection.h \
    src/Tool.h \
    src/PolygonTool.h \
    src/BrushTool.h \
    src/MagicWandTool.h \
    src/HandlessSlider.h \
    src/MagicWandToolBar.h

SOURCES += \
    src/Category.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/Scheme.cpp \
    src/SchemeTree.cpp \
    src/Helper.cpp \
    src/SegmentationScene.cpp \
    src/SegmentationView.cpp \
    src/ZoomSlider.cpp \
    src/GraphicsItems.cpp \
    src/UndoCommands.cpp \
    src/BrushToolBar.cpp \
    src/Drawables.cpp \
    src/OpenFolderDialog.cpp \
    src/ImageList.cpp \
    src/NewSchemeDialog.cpp \
    src/AboutDialog.cpp \
    src/Slider.cpp \
    src/Selection.cpp \
    src/Tool.cpp \
    src/PolygonTool.cpp \
    src/BrushTool.cpp \
    src/MagicWandTool.cpp \
    src/HandlessSlider.cpp \
    src/MagicWandToolBar.cpp

FORMS += \
    ui/MainWindow.ui \
    ui/OpenFolderDialog.ui \
    ui/NewSchemeDialog.ui \
    ui/AboutDialog.ui

RESOURCES += \
    ImageSegmenter.qrc

DISTFILES += \
    ImageSegmenter.qss

win32:RC_FILE = ImageSegmenter.rc
win32:QMAKE_CXXFLAGS_WARN_ON -= -w34100

!exists(version.h) {
    DEFINES += NO_VERSION_HEADER
    DEFINES += BUILD_MAJOR_NUMBER=1
    DEFINES += BUILD_MINOR_NUMBER=0
    DEFINES += BUILD_REVISION_NUMBER=0
    DEFINES += BUILD_NUMBER=\\\"1.0.0\\\"
    DEFINES += BUILD_GENERAL_NUMBER=\\\"1.0\\\"
    DEFINES += BUILD_DATE_TIME=\\\"\\\"
}
