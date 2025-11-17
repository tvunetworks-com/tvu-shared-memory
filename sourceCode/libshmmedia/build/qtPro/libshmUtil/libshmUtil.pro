TEMPLATE = lib


SOURCES += \
    ../../../prj/libshmUtil/src/buffer_ctrl.cpp \
    ../../../prj/libshmUtil/src/libshm_cache_buffer.cpp \
    ../../../prj/libshmUtil/src/libshm_key_value.cpp \
    ../../../prj/libshmUtil/src/libshm_uint128.cpp \
    ../../../prj/libshmUtil/src/libshm_variant.cpp \
    ../../../prj/libshmUtil/src/libshm_zlib_wrap.cpp \
    ../../../prj/libshmUtil/test/test_util.cpp


HEADERS += \
    ../../../prj/libshmUtil/src/buffer_controller_internal.h \
    ../../../prj/libshmUtil/src/include/buffer_controller.h \
    ../../../prj/libshmUtil/src/include/buffer_ctrl.h \
    ../../../prj/libshmUtil/src/include/common_define.h \
    ../../../prj/libshmUtil/src/include/libshm_cache_buffer.h \
    ../../../prj/libshmUtil/src/include/libshm_key_value.h \
    ../../../prj/libshmUtil/src/include/libshm_time_internal.h \
    ../../../prj/libshmUtil/src/include/libshm_uint128.h \
    ../../../prj/libshmUtil/src/include/libshm_util_common_internal.h \
    ../../../prj/libshmUtil/src/include/libshm_util_endian.h \
    ../../../prj/libshmUtil/src/include/libshm_variant.h \
    ../../../prj/libshmUtil/src/include/libshm_zlib_wrap_internal.h \
    ../../../prj/libshmUtil/src/include/uint128_t.h


INCLUDEPATH += \
    ../../../include \
    ../../../prj/libshmUtil/include \
    ../../../prj/libshmUtil/src/include \
    ../../../prj/libtvuutil/include \

DEFINES += \
    _TVU_VIARIABLE_SHM_FEATURE_ENABLE=1 \
    TVU_LINUX = 1
