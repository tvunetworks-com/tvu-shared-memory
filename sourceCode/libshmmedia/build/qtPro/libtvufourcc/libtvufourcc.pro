CONFIG -= qt

TEMPLATE = lib subdirs
DEFINES += LIBSHMMEDIA_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../../prj/libtvufourcc/src/libtvu_media_info.cpp \
    ../../../prj/libtvufourcc/src/tvu_fourcc.cpp \
    ../../../prj/libtvufourcc/test/test_libtvufourcc.cpp \


HEADERS += \
    ../../../prj/libtvufourcc/include/tvu_fourcc.h \


INCLUDEPATH += \
../../../prj/libtvufourcc/include \


DEFINES += \
  _TVU_VIARIABLE_SHM_FEATURE_ENABLE=1 \
  TVU_LINUX = 1


DISTFILES += \

