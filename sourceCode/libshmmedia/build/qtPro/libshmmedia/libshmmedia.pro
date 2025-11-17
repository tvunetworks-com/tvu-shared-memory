CONFIG -= qt

TEMPLATE = lib subdirs
DEFINES += LIBSHMMEDIA_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
../../../prj/libshmmedia/src/libshm_media.cpp \
../../../prj/libshmmedia/src/libshm_media_protocol_internal.cpp \
../../../prj/libshmmedia/src/libshm_media_raw_data_opt.cpp \
../../../prj/libshmmedia/src/libshm_media_variable_item.cpp \
../../../prj/libshmmedia/src/buffer_ctrl.cpp \
../../../prj/libshmmedia/src/libshmmedia_control_protocol.cpp \
../../../prj/libshmmedia/src/libshmmedia_encoding_protocol.cpp \
../../../prj/libshmmedia/src/libshmmedia_tvulive_protocol.cpp \
../../../prj/libshmmedia/src/libshmmedia_variableitem_rawdata.cpp \
../../../prj/libshmmedia/test_libshmmedia/test.cpp \
../../../prj/libshmmedia/test_libshmmedia/test_data_protocol/test_pro.cpp \
../../../prj/libshmmedia/test_libshmmedia/test_raw_data/transfer_shm.cpp \
../../../prj/libshmmedia/test_libshmmedia/test_shm_tvulive/test_shm_tvulive.cpp \
../../../prj/libshmmedia/test_libshmmedia/variable_item_shm_test/test_variable_item_shm.cpp \
../../../prj/libshmmedia/test_libshmmedia/control_data_test/test_control_cmd.cpp \
../../../prj/libshmmedia/test_libshmmedia/sample_code/read_sample_code.cpp \
../../../prj/libshmmedia/test_libshmmedia/sample_code/write_sample_code.cpp \
../../../prj/libshmmedia/test_libshmmedia/test_util/test_util.cpp \


HEADERS += \
../../../prj/libshmmedia/include/libshm_data_protocol.h \
../../../prj/libshmmedia/include/libshmmedia.h \
../../../prj/libshmmedia/include/libshm_media.h \
../../../prj/libshmmedia/include/libshm_media_protocol.h \
../../../prj/libshmmedia/include/libshm_media_raw_data_opt.h \
../../../prj/libshmmedia/include/libshm_media_variable_item.h \
../../../prj/libshmmedia/include/libshmmedia.h \
../../../prj/libshmmedia/include/libshmmedia_control_protocol.h \
../../../prj/libshmmedia/include/libshmmedia_data_protocol.h \
../../../prj/libshmmedia/include/libshmmedia_encoding_protocol.h \
../../../prj/libshmmedia/include/libshmmedia_rawdata.h \
../../../prj/libshmmedia/include/libshmmedia_tvulive_protocol.h \
../../../prj/libshmmedia/include/libshmmedia_variableitem.h \
../../../prj/libshmmedia/include/libshmmedia_variableitem_rawdata.h \
../../../prj/libshmmedia/include/libtvu_media_fourcc.h \
../../../prj/libshmmedia/include/libtvu_media_info.h \
../../../prj/libshmmedia/include/libshmmedia_common.h \
../../../prj/libshmmedia/src/libshm_media_internal.h \
../../../prj/libshmmedia/src/libshm_media_protocol_internal.h \
../../../prj/libshmmedia/src/libshm_media_variable_item_internal.h \
../../../prj/libshmmedia/src/libshmmedia_control_protocol_internal.h \
../../../prj/libshmmedia/src/libshmmedia_tvulive_protocol_internal.h \


INCLUDEPATH += \
../../../include \
../../../prj/libsharememory/include \
../../../prj/libsharememory/src/include \
../../../prj/libvaItemSharedMemory/include \
../../../prj/libvaItemSharedMemory/src/include \
../../../prj/libshmmedia/include \
../../../prj/libshmmediaProto/include \
../../../prj/libshmmediaProto/src \
../../../prj/libtvuutil/include \
../../../prj/libtvufourcc/include \
../../../prj/libShmmediaActionWrap/include \
../../../prj/libtvulog/include \
../../../prj/libShmmediaActionWrap/depends/libtvulive/include \
../../../prj/libshmUtil/include \
../../../prj/libshmUtil/src/include \



DEFINES += \
  _TVU_VIARIABLE_SHM_FEATURE_ENABLE=1 \
  TVU_LINUX = 1


DISTFILES += \

