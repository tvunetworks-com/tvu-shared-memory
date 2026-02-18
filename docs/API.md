# API Documentation - tvu-shared-memory

## Service Information

| Property | Value |
|----------|-------|
| **Service** | tvu-shared-memory |
| **Language** | C++ |
| **Framework** | C++ |


## API Endpoints

The following endpoints were detected in the codebase:

```
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:15: *      used to declare some APIs about shmmedia protocol,
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:67:typedef int (*libshm_media_process_handle_t)(uint8_t *data, int len, void *userdata);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:109:     *     API is LibshmmediaAudioChannelLayoutSerializeToBinary.
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:126:    int         i_totalLen;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:128:    int         i_vLen;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:133:    int         i_aLen;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:138:    int         i_sLen;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:143:    int         i_CCLen;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:145:    int         i_timeCode;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:149:    int         i_userDataLen;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:151:    int         i_userDataType;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:170:    void                                *p_opaq;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:234:typedef int (*libshmmedia_item_checking_fn_t)(void *user, const libshm_media_head_param_t *, const libshm_media_item_param_t*);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:239:extern "C" {
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:264:unsigned int LibShmMediaProtoGetHeadVersion(const uint8_t *headBuf);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:279:int  LibShmMediaProtoReadItemBufferLayout(/*OUT*/libshm_media_head_param_t *pmh
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:296:int  LibShmMediaProtoReadItemBufferLayoutWithHeadVer(/*OUT*/libshm_media_head_param_t *pmh
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:315:int  LibShmMediaProtoGetWriteItemBufferLayout(/*IN*/const libshm_media_item_param_t *pmi
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:328:int  LibShmMediaProtoRequireWriteItemBufferLength(/*IN*/const libshm_media_item_param_t *pmi);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:336: *      @pItemAddr[OUT]  : item address point which would be written.
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:343:int LibShmMediaProtoWriteItemBuffer(
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:358:int LibShmMediaHeadParamInit(libshm_media_head_param_t *p, uint32_t structSize);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:369:void LibShmMediaHeadParamRelease(libshm_media_head_param_t *p);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:381:int LibShmMediaItemParamInit(libshm_media_item_param_t *p, uint32_t structSize);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:392:void LibShmMediaItemParamRelease(libshm_media_item_param_t *p);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:421:int LibShmMediaRawDataParamInit(libshmmedia_raw_data_param_t *p, uint32_t structSize);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:432://void LibShmMediaRawDataParamRelease(libshmmedia_raw_data_param_t *p);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:445:int LibShmMediaRawHeadParamInit(libshmmedia_raw_head_param_t *p, uint32_t structSize);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_protocol.h:456://void LibShmMediaRawHeadParamRelease(libshmmedia_raw_head_param_t *p);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:21:    extern "C" {
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:77:    int                 i_uuid_length;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:79:    int                 i_cc608_cdp_length;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:81:    int                 i_caption_text_length;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:83:    int                 i_producer_stream_info_length;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:85:    int                 i_receiver_info_length;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:87:    int                 i_scte104_data_len;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:89:    int                 i_scte35_data_len;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:92:    int                 i_timecode_index_length;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:94:    int                 i_start_timecode_length;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:97:    int                 i_hdr_metadata;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:100:    int                 i_timecode_fps_index;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:103:    int                 i_pic_struct;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:105:    int                 i_source_timestamp;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:108:    int                 i_timecode;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:113:    int                 i_metaDataPts;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:116:    int                 i_source_timebase;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:119:    int                 i_smpte_afd_data;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:123:    int                 i_source_action_timestamp;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:128:    int                 i_vanc_smpte2038;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:133:    int                 i_gop_poc;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:139:    int                 i_subtitle;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:144:    int                 i_smpte336m;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:151:    bool                bHasColorPrimariesVal_;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:158:    bool                bHasColorTransferCharacteristicVal_;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:165:    bool                bHasColorSpaceVal_;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:172:    bool                bHasVideoFullRangeFlagVal_;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:179:    bool                bGotTvutimestamp;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:189: *      @pShmUserData[IN]   :   external buffer point.
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:191: *      @pExtendData[OUT]   :   destination external data structure.
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:213://old deprecated API for read v1 user data,use libShmReadExtendDataV2 to instead
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:215://void libShmReadExtendData(Lib_shm_extend_data_t* pExtendData,const uint8_t* pShmUserData,int dataSize);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:218:typedef void * libshmmedia_extended_data_context_t;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:226: *      used to read out extended data, the data point was not at @pShmUserData but at the internal buffer of @v2DataCtx
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:229:int LibShmMediaReadExtendData(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize, int userDataType, libshmmedia_extended_data_context_t v2DataCtx);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:233: *      used to read out extended data, the data point was not at @pShmUserData but at the internal buffer of @v2DataCtx
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:236:int libShmReadExtendDataV2(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize, int userDataType, libshmmedia_extended_data_context_t v2DataCtx);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:240: *      used to read out extended data, the data point was at @pShmUserData
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:245:int LibShmMeidaParseExtendData(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize, int userDataType);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:247:int LibShmMeidaParseExtendDataV2(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:254: *      @pExtendData[IN]    :   src external data information structure.
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:261:int LibShmMediaEstimateExtendDataSize(/*IN*/const libshmmedia_extend_data_info_t* pExtendData);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:265: *      a func to create V2 user data buffer API
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:269: *      @pExtendData[IN]    :   src external data information structure.
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:275:int libShmWriteExtendData(/*OUT*/uint8_t dataBuffer[], /*IN*/int bufferSize, /*IN*/const libshmmedia_extend_data_info_t* pExtendData);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:277:int LibShmMediaWriteExtendData(/*OUT*/uint8_t dataBuffer[], /*IN*/int bufferSize, /*IN*/const libshmmedia_extend_data_info_t* pExtendData);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:284:void LibshmMediaExtDataResetEntry(libshmmedia_extended_data_context_t h);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:291:unsigned int LibshmMediaExtDataGetEntryBuffSize(libshmmedia_extended_data_context_t h);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:299:int LibshmMediaExtDataParseBuff(libshmmedia_extended_data_context_t h, const uint8_t *pbuf, uint32_t ibuflen);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:306:unsigned int LibshmMediaExtDataGetEntryCounts(libshmmedia_extended_data_context_t h);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_extension_protocol.h:309:void LibshmMediaExtDataDestroyHandle(libshmmedia_extended_data_context_t *ph);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h:23:typedef void * libshmmedia_bin_concat_proto_handle_t;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h:32:    extern "C" {
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h:52:    bool LibshmmediaBinConcatProtoReset(libshmmedia_bin_concat_proto_handle_t);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h:66:    bool LibshmmediaBinConcatProtoConcatSegment(libshmmedia_bin_concat_proto_handle_t pctx, /*IN*/ const libshmmedia_bin_concat_proto_seg_t *pSeg);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h:80:    bool LibshmmediaBinConcatProtoFlushBinary(libshmmedia_bin_concat_proto_handle_t pctx, /*OUT*/const uint8_t **ppBin, /*OUT*/uint32_t *pnBin);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h:95:    bool LibshmmediaBinConcatProtoSplitBinary(libshmmedia_bin_concat_proto_handle_t pctx
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h:96:                                              , /*IN*/const uint8_t *pBin, /*IN*/uint32_t nBin, /*IN*/bool bCreateBuffer
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h:112:    bool LibshmmediaBinConcatProtoParseBinary(libshmmedia_bin_concat_proto_handle_t pctx
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_bin_concat_protocol.h:126:    void LibshmmediaBinConcatProtoDestroy(libshmmedia_bin_concat_proto_handle_t pctx);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:23:typedef void libshmmedia_audio_channel_layout_object_t;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:24:typedef void *libshmmedia_audio_channel_layout_handle_t;
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:27:    extern "C" {
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:42://    bool LibshmmediaAudioChannelLayoutReset(libshmmedia_audio_channel_layout_t *pctx);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:49:    bool LibshmmediaAudioChannelLayoutIsPlanar(const libshmmedia_audio_channel_layout_object_t *pctx);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:63:    bool LibshmmediaAudioChannelLayoutGetChannelArr(const libshmmedia_audio_channel_layout_object_t *pctx, /*OUT*/const uint16_t **ppDest, /*OUT*/uint16_t *pnDest);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:70:    bool LibshmmediaAudioChannelLayoutGetBinaryAddr(const libshmmedia_audio_channel_layout_object_t *pctx, /*OUT*/const uint8_t **ppDest, /*OUT*/uint32_t *pnDest);
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:77:    bool LibshmmediaAudioChannelLayoutSerializeToBinary(libshmmedia_audio_channel_layout_object_t *pctx
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:78:                                                       , const uint16_t *pChan, uint16_t nChan, bool bPlanar
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:86:    bool LibshmmediaAudioChannelLayoutParseFromBinary(libshmmedia_audio_channel_layout_object_t *pctx
./sourceCode/libshmmedia/prj/libshmmediaProto/include/libshm_media_audio_track_channel_protocol.h:95:    void LibshmmediaAudioChannelLayoutDestroy(libshmmedia_audio_channel_layout_object_t *pctx);
```


---
*Documentation auto-generated by TVU AI Documentation System*
