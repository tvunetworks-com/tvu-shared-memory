CONFIG -= qt

TEMPLATE = lib subdirs
DEFINES += LIBSHMMEDIA_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
../../../prj/libvaItemSharedMemory/src/TvuFormatUtils.cpp \
../../../prj/libvaItemSharedMemory/src/TvuLog.cpp \
../../../prj/libvaItemSharedMemory/src/TvuMemUtils.cpp \
../../../prj/libvaItemSharedMemory/src/TvuSharedMemory.cpp \
../../../prj/libvaItemSharedMemory/src/TvuShmSharedCompactRingBuffer.cpp \
../../../prj/libvaItemSharedMemory/src/TvuTimeUtils.cpp \


HEADERS += \
../../../prj/libvaItemSharedMemory/include/TvuShmSharedCompactRingBuffer.h \
../../../prj/libvaItemSharedMemory/include/TvuVaItemSharedMemory.h \
../../../prj/libvaItemSharedMemory/src/include/TvuCommonDefines.h \
../../../prj/libvaItemSharedMemory/src/include/TvuCrossPlatformDefines.h \
../../../prj/libvaItemSharedMemory/src/include/TvuFormatUtils.h \
../../../prj/libvaItemSharedMemory/src/include/TvuFoundationErrorCodes.h \
../../../prj/libvaItemSharedMemory/src/include/TvuLog.h \
../../../prj/libvaItemSharedMemory/src/include/TvuMemUtils.h \
../../../prj/libvaItemSharedMemory/include/TvuShmCrossPlatformExtDefines.h \
../../../prj/libvaItemSharedMemory/src/include/TvuTimeUtils.h \
../../../prj/libvaItemSharedMemory/src/include/TvuShmCrossPlatformExtDefines.h \


INCLUDEPATH += \
../../../prj/libvaItemSharedMemory/include \
../../../prj/libvaItemSharedMemory/src/include \


DEFINES += \
  _TVU_VIARIABLE_SHM_FEATURE_ENABLE=1 \
  TVU_LINUX = 1


DISTFILES += \

