
#include "libshm_data_protocol.h"
#include "libshm_media_subtitle_private_protocol.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

static int _trans_raw_data_to_hex_string(const uint8_t *data, int len, char hx_str[], int hx_str_len)
{
    int ix = 0;

    for (int i = 0; i < len; i++)
    {
        if (i == 0)
        {
            snprintf(hx_str + ix, hx_str_len - ix, "%02hhx", data[i]);
        }
        else
        {
            snprintf(hx_str + ix, hx_str_len - ix, ",%02hhx", data[i]);
        }
        ix = strlen(hx_str);
    }

    return ix;
}

void print_encoding_data(const libtvumedia_encoding_data_t *p)
{
    char hx_str[256] = {0};
    _trans_raw_data_to_hex_string(p->p_data, (p->i_data>16)?16:p->i_data, hx_str, sizeof(hx_str));

    printf("codec=>0x%x, stream index %u, frame index %d, pts  %" PRId64 ", dts %" PRId64 ", data len %u, data ptr [%s, ...]\n"
           , p->u_codec_tag
           , p->u_stream_index
           , p->u_frame_index
           , p->i64_pts
           , p->i64_dts
           , p->i_data
           , hx_str
    );
}

int test_encoding_data_protocol()
{
    int ret = 0;
    const uint8_t sample_data[17] = "0123456789abcdef";
    const int n_sample_data = sizeof(sample_data) - 1;

    uint32_t wanted_buffer_len = LibTvuMediaEncodingDataGetBufferSize(n_sample_data);

    if (!wanted_buffer_len) {
        return -1;
    }

    uint8_t *buffer = (uint8_t *)malloc(wanted_buffer_len);
    libtvumedia_encoding_data_t osrc;
    {
        memset((void *)&osrc, 0, sizeof(libtvumedia_encoding_data_t));
    }
    libtvumedia_encoding_data_t odst;
    {
        memset((void *)&odst, 0, sizeof(libtvumedia_encoding_data_t));
    }

    osrc.u_codec_tag = 0x12345678;
    osrc.u_stream_index = 10;
    osrc.u_frame_index = 100;
    osrc.i64_dts = 1001;
    osrc.i64_pts = 2001;
    osrc.i_data = n_sample_data;
    osrc.p_data = sample_data;

    int ret_w = LibTvuMediaEncodingDataWrite(&osrc, buffer, wanted_buffer_len);

    if (ret_w <= 0 || ret_w > (int)wanted_buffer_len)
    {
        printf("write encoding data to protocol buffer failed, ret %d, buffer size %u\n", ret_w, wanted_buffer_len);
        return -1;
    }

    printf("write encoding data ret %d, buffer size %u\n", ret_w, wanted_buffer_len);

    int ret_r = LibTvuMediaEncodingDataRead(&odst, buffer, ret_w);

    if (ret_r <= 0 || ret_r > ret_w)
    {
        printf("read encoding data to protocol buffer failed, ret %d, buffer size %d\n", ret_r, ret_w);
        return -1;
    }

    printf("read encoding data ret %d, buffer size %u\n", ret_r, ret_w);

    if (memcmp(odst.p_data, sample_data, n_sample_data))
    {
        printf("data was not equal");
        return -1;
    }

    printf("src => {\n");
    print_encoding_data(&osrc);
    printf("}\n");

    printf("dst => {\n");
    print_encoding_data(&odst);
    printf("}\n");

    free(buffer);

    return ret;
}

int test_subtitle_private_protocol()
{
    int ret = 0;
    LibShmmediaSubtitlePrivateProtocolEntries entry = {};
    {
        const int counts = 3;
        entry.head.counts = counts;
        for (int i = 0; i < counts; i++)
        {
            libshmmedia_subtitle_private_proto_entry_item_t &item = entry.entries[i];

            if (i == 0)
            {
                item.strucSize = sizeof (libshmmedia_subtitle_private_proto_entry_item_t);
                item.type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_WEBVTT;
                item.timestamp = 323412341;
                item.duration = 2000;
                item.dataLen = 4;
                item.data = (const uint8_t *)"4444";
            }
            else if (i == 1)
            {
                item.strucSize = sizeof (libshmmedia_subtitle_private_proto_entry_item_t);
                item.type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SRT;
                item.timestamp = 666666;
                item.duration = 2000;
                item.dataLen = 5;
                item.data = (const uint8_t *)"11111";
            }
            else if (i == 2)
            {
                item.strucSize = sizeof (libshmmedia_subtitle_private_proto_entry_item_t);
                item.type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBRIP;
                item.timestamp = 77777;
                item.duration = 3000;
                item.dataLen = 3;
                item.data = (const uint8_t *)"22222";
            }
        }


        int prelen = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entry);
        uint8_t *buffer = NULL;

        if (prelen < 0)
        {
            return -1;
        }

        buffer = (uint8_t *)malloc(prelen);
        if (!buffer)
        {
            return -1;
        }

        int len = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entry, buffer, prelen);

        if (len>prelen)
        {
            return -1;
        }


        {
            int errCode = LibshmmediaSubtitlePrivateProtoTuneTimestampOffSet(buffer, len, -11);
            if (errCode < 0)
            {
                printf("timestamp tune failed\n");
            }
        }

        LibShmmediaSubtitlePrivateProtocolEntries out = {};
        int errCode = LibshmmediaSubtitlePrivateProtoParseBufferSize(&out, buffer, len);
        if (!errCode)
        {
            int xcount=out.head.counts;
            printf(
                        "counts=>%d"
                        "\n"
                        , out.head.counts
                        );
            for (int i = 0; i < xcount; i++)
            {
                const libshmmedia_subtitle_private_proto_entry_item_t &item = out.entries[i];
                char data[16] = {0};
                memcpy(data, item.data, item.dataLen);
                printf(
                            "entry:{\n"
                            "    idx:%d,\n"
                            "    type:0x%x,\n"
                            "    timestamp:%" PRId64 ",\n"
                            "    duration:%d,\n"
                            "    len:%d,\n"
                            "    data:%s,\n"
                            "}n"
                            , i
                            , item.type
                            , item.timestamp
                            , item.duration
                            , item.dataLen
                            , data
                            );
            }
        }

        free(buffer);
    }

    return ret;
}

void print_tvulive_data(const libtvumedia_tvulive_data_t *p)
{
    char hx_str[256] = {0};
    _trans_raw_data_to_hex_string(p->p_data, (p->i_data>16)?16:p->i_data, hx_str, sizeof(hx_str));

    printf("itype=>%c, stream index %u, frame index %d, timestamp %lx, data len %u, data ptr [%s, ...]\n"
           , p->o_info.i_type
           , p->o_info.u_stream_index
           , p->o_info.u_frame_index
           , p->o_info.u_frame_timestamp_ms
           , p->i_data
           , hx_str
    );
}

int test_tvulive_data_protocol()
{
    int ret = 0;
    const uint8_t sample_data[17] = "0123456789abcdef";
    const int n_sample_data = sizeof(sample_data) - 1;

    uint32_t wanted_buffer_len = LibTvuMediaTvulivePreferBufferSize(n_sample_data);

    if (!wanted_buffer_len) {
        return -1;
    }

    uint8_t *buffer = (uint8_t *)malloc(wanted_buffer_len);
    libtvumedia_tvulive_data_t osrc;
    {
        memset((void *)&osrc, 0, sizeof(libtvumedia_tvulive_data_t));
    }
    libtvumedia_tvulive_data_t odst;
    {
        memset((void *)&odst, 0, sizeof(libtvumedia_tvulive_data_t));
    }

    osrc.u_struct_size = sizeof(libtvumedia_tvulive_data_t);
    osrc.o_info.i_type = 'v';
    osrc.o_info.u_stream_index = 10;
    osrc.o_info.u_frame_index = 100;
    osrc.o_info.u_frame_timestamp_ms = 0x0123456789abcdef;
    osrc.i_data = n_sample_data;
    osrc.p_data = sample_data;

    int ret_w = LibTvuMediaTvuliveWriteData(&osrc, buffer, wanted_buffer_len);

    if (ret_w <= 0 || ret_w > (int)wanted_buffer_len)
    {
        printf("write tvulive data to protocol buffer failed, ret %d, buffer size %u\n", ret_w, wanted_buffer_len);
        return -1;
    }

    printf("write tvulive data ret %d, buffer size %u\n", ret_w, wanted_buffer_len);

    odst.u_struct_size = sizeof(libtvumedia_tvulive_data_t);
    int ret_r = LibTvuMediaTvuliveReadData(&odst, buffer, ret_w);

    if (ret_r <= 0 || ret_r > ret_w)
    {
        printf("read tvulive data to protocol buffer failed, ret %d, buffer size %d\n", ret_r, ret_w);
        return -1;
    }

    printf("read tvulive data ret %d, buffer size %u\n", ret_r, ret_w);

    if (memcmp(odst.p_data, sample_data, n_sample_data))
    {
        printf("data was not equal");
        return -1;
    }

    printf("src => {\n");
    print_tvulive_data(&osrc);
    printf("}\n");

    printf("dst => {\n");
    print_tvulive_data(&odst);
    printf("}\n");

    free(buffer);

    return ret;
}

int main()
{
//    int ret = test_tvulive_data_protocol();
//    int ret1 = test_encoding_data_protocol();
    int ret2 = test_subtitle_private_protocol();

    return (ret2);
}
