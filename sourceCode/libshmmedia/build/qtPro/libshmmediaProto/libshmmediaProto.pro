CONFIG -= qt

TEMPLATE = lib subdirs
DEFINES += LIBSHMMEDIA_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../../prj/libshmmediaProto/src/libshm_media_audio_track_channel_protocol.cpp \
    ../../../prj/libshmmediaProto/src/libshm_media_bin_concat_protocol.cpp \
../../../prj/libshmmediaProto/src/libshm_media_extension_protocol.cpp \
    ../../../prj/libshmmediaProto/src/libshm_media_key_value_proto.cpp \
    ../../../prj/libshmmediaProto/src/libshm_media_media_head_protocol.cpp \
../../../prj/libshmmediaProto/src/libshm_media_protocol_internal.cpp \
../../../prj/libshmmediaProto/src/libshm_media_protocol_log_internal.cpp \
../../../prj/libshmmediaProto/src/libshm_media_subtitle_private_protocol_internal.cpp \
../../../prj/libshmmediaProto/src/libshm_media_struct.cpp \
    ../../../prj/libshmmediaProto/test/test_shm_proto.cpp


HEADERS += \
    ../../../prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h \
    ../../../prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h \
../../../prj/libshmmediaProto/include/libshm_media_extension_protocol.h \
    ../../../prj/libshmmediaProto/include/libshm_media_media_head_protocol.h \
    ../../../prj/libshmmediaProto/include/libshm_media_proto_common.h \
../../../prj/libshmmediaProto/include/libshm_media_protocol.h \
../../../prj/libshmmediaProto/include/libshm_media_protocol_log.h \
../../../prj/libshmmediaProto/include/libshm_media_subtitle_private_protocol.h \
    ../../../prj/libshmmediaProto/src/libshm_media_audio_track_channel_proto_internal.h \
    ../../../prj/libshmmediaProto/src/libshm_media_bin_concat_protocol_internal.h \
../../../prj/libshmmediaProto/src/libshm_media_extension_protocol_internal.h \
    ../../../prj/libshmmediaProto/src/libshm_media_key_value_proto_internal.h \
    ../../../prj/libshmmediaProto/src/libshm_media_media_head_proto_internal.h \
../../../prj/libshmmediaProto/src/libshm_media_protocol_internal.h \
../../../prj/libshmmediaProto/src/libshm_media_protocol_log_internal.h \
../../../prj/libshmmediaProto/src/libshm_media_struct.h \
../../../prj/libshmmediaProto/src/libshm_media_subtitle_private_protocol_internal.h \

INCLUDEPATH += \
../../../prj/libshmmediaProto/include \
../../../prj/libshmmediaProto/src \
../../../prj/libsharememory/include \
../../../prj/libsharememory/src/include \
../../../prj/libshmUtil/include \
../../../prj/libshmUtil/src/include \
../../../prj/libtvuutil/include \


DEFINES += \
  _TVU_VIARIABLE_SHM_FEATURE_ENABLE=1 \
  TVU_LINUX = 1


DISTFILES += \

