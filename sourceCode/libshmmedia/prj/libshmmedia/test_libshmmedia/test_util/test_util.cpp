#include "buffer_ctrl.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>

#define TVUUTIL_GET_SYS_MS64()                  _xxtvuutil_get_sys_ms64()
#define VIDEO_FRAME_SIZE                        10240
#define TVU_LINUX 1
namespace tvushm {
    static inline
    int64_t _xxtvuutil_get_sys_ms64()
    {
    #ifdef TVU_WINDOWS
        int64_t tmNow = 0;
        struct _timeb timebuffer;
        _ftime_s(&timebuffer);
        tmNow = timebuffer.time;
        tmNow *= 1000;
        tmNow += timebuffer.millitm;
        return tmNow;
    #elif defined(TVU_MINGW)
        int64_t tmNow = 0;
        struct _timeb timebuffer;
        _ftime(&timebuffer);
        tmNow = timebuffer.time;
        tmNow *= 1000;
        tmNow += timebuffer.millitm;
        return tmNow;
    #elif defined(TVU_LINUX)
        int64_t tmNow = 0;
        struct timeval  tv;
        gettimeofday(&tv, NULL);
        tmNow = tv.tv_sec * 1000 + tv.tv_usec/1000;
        return tmNow;
    #endif
    }

    static void Sleep(uint32_t ms)
    {
        usleep(ms*1000);
    }

    static bool g_exit = false;

    static int ReadUtil(const uint8_t *p, uint32_t len)
    {
        int ret = 0;
        BufferController_t r;
        BufferCtrlInit(&r);
        {
            BufferCtrlPushData(&r, p, len);
            BufferCtrlSeek(&r, 0, SEEK_SET);
        }

        uint32_t v1 = BufferCtrlRLe32(&r);
        uint32_t v2 = BufferCtrlRLe16(&r);
        uint16_t v3 = 0;
        uint32_t v4 = 0;
        FAILED_READ_DATA((ret=BufferCtrlCompactDecodeValueU16(&r, v3)));
        FAILED_READ_DATA((ret=BufferCtrlCompactDecodeValueU32(&r, v4)));

        printf("parse result."
               "v1:%x"
               ",v2:%x"
               ",v3:%hx"
               ",v4:%x"
               "\n"
               , v1, v2, v3, v4
               );

        return 0;
    }

    static int TestUtil()
    {
        int ret = 0;
        uint32_t offset = 0;
        BufferController_t o;
        ret = BufferCtrlInit(&o);

        if (ret < 0)
        {
            return ret;
        }

        FAILED_PUSH_DATA((ret=BufferCtrlWLe32(&o, 0x12345678)));
        offset += ret;


        FAILED_PUSH_DATA((ret=BufferCtrlWLe16(&o, 0xFF)));
        offset += ret;

        FAILED_PUSH_DATA((ret=BufferCtrlCompactEncodeValueU16(&o, 0x12)));
        offset += ret;

        FAILED_PUSH_DATA((ret=BufferCtrlCompactEncodeValueU32(&o, 0xabcd)));
        offset += ret;

        int buflen = BufferCtrlTellSize(&o);

        if (offset != (uint32_t)buflen)
        {
            printf("check the program for invalid offset/bufferlen."
                   "o:%u, l:%d"
                   "\n"
                   , offset, buflen
                   );
        }

        printf("xxx."
               "o:%u, l:%d"
               "\n"
               , offset, buflen
               );
        uint8_t *pout = (uint8_t *)malloc(offset);

        BufferCtrlSeek(&o, 0, SEEK_SET);
        FAILED_READ_DATA((ret = BufferCtrlPopData(&o, pout, offset)));

        printf("popout."
               "ret:%d"
               "\n"
               , ret
               );

        {
            ReadUtil(pout, ret);
        }

        if (pout)
        {
            free(pout);
        }

        BufferCtrlRelease(&o);

        return 0;
    }

    static void help(const char *prog)
    {
        printf("Usage:\n"
               "    %s\n"
               "ex:%s"
               "\n"
               , prog
               , prog
               );
        return;
    }

}

int main(int argc, const char *argv[])
{
    if (argc < 1)
    {
        tvushm::help(argv[0]);
        return -1;
    }

    if (argc > 1 && (!strcmp(argv[1],"-h") || !strcmp(argv[1],"--help")))
    {
        tvushm::help(argv[0]);
        return -1;
    }

    return tvushm::TestUtil();
}


