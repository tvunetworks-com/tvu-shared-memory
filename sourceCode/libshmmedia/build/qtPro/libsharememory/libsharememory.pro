TEMPLATE = lib


SOURCES += \
    ../../../prj/libsharememory/src/file_lock.cpp \
    ../../../prj/libsharememory/src/sharememory.cpp \
    ../../../prj/libsharememory/src/shm_variable_item_ring_buff.cpp \
    ../../../prj/libsharememory/test/shmmng_test.cpp \


HEADERS += \
    ../../../prj/libsharememory/src/include/file_lock.h \
    ../../../prj/libsharememory/include/sharememory.h \
    ../../../prj/libsharememory/src/include/sharememory_internal.h \
    ../../../prj/libsharememory/src/include/shm_variable_item_ring_buff.h \
    ../../../prj/libsharememory/src/include/shmhead.h \
    ../../../prj/libsharememory/src/include/file_lock_internal.h \


INCLUDEPATH += \
    ../../../include \
    ../../../prj/libsharememory/include \
    ../../../prj/libsharememory/src/include \
    ../../../prj/libshmUtil/include \
    ../../../prj/libshmUtil/src/include \
    ../../../prj/libtvuutil/include \


DEFINES += \
    _TVU_VIARIABLE_SHM_FEATURE_ENABLE=1 \
    TVU_LINUX = 1
