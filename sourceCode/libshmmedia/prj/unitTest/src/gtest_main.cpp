#include <gtest/gtest.h>
#include "libshmmedia_common.h"

static int get_shm_log(int level, const char *fmt, ...)
{
#if 0
    char    slog[1024]  = {0};
    va_list ap;
    va_start(ap,fmt);

    switch (level) {
    case 'i':
    {
        snprintf(slog, 1024, "info --");
        vsnprintf(slog + 7, 1024 - 7, fmt, ap);
    }
    break;
    case 'w':
    {
        snprintf(slog, 1024, "warn --");
        vsnprintf(slog + 7, 1024 - 7, fmt, ap);
    }
    break;
    case 'e':
    {
        snprintf(slog, 1024, "error--");
        vsnprintf(slog + 7, 1024 - 7, fmt, ap);
    }
    break;
    default:
        break;
    }

    fprintf(stderr, "%s", slog);
    fflush(stderr);
    va_end(ap);
#endif
    return 0;
}

int main(int argc, char **argv)
{
    LibShmMediaSetLogCb(get_shm_log);
    ::testing::InitGoogleTest(&argc, argv);
    srand((unsigned)time(NULL) ^ getpid());
    return RUN_ALL_TESTS();
}


