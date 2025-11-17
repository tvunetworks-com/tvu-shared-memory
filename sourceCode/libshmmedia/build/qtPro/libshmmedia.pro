CONFIG -= qt

DEFINES += LIBSHMMEDIA_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#use sub template
TEMPLATE = subdirs

SUBDIRS += \
    libtvufourcc \
    libshmUtil \
    libsharememory \
    libvaItemSharedMemory \
    libshmmediaProto \
    libshmmedia \


SOURCES += \



HEADERS += \


INCLUDEPATH += \


DEFINES += \
  _TVU_VIARIABLE_SHM_FEATURE_ENABLE=1 \
  TVU_LINUX = 1


