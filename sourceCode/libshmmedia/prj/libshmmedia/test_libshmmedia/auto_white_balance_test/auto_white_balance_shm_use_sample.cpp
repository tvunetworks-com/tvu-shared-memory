#include "libshmmedia.h"
#include "libshm_media_raw_data_opt.h"
#include "tvu_fourcc.h"  // This contains the FourCC definitions
#include <string.h>

#include <iostream>
#include <algorithm>
// Hypothetical color space conversion functions
void YUYV422_to_RGB(const uint8_t *YUYV_data, uint8_t *RGB_data, int size) {
    int i;
    for (i = 0; i < size; i += 4) {
        unsigned char Y0 = YUYV_data[i];
        unsigned char U = YUYV_data[i + 1];
        unsigned char Y1 = YUYV_data[i + 2];
        unsigned char V = YUYV_data[i + 3];

        int R0, G0, B0, R1, G1, B1;

        // 计算RGB值
        // 请根据实际的YUV转RGB公式进行计算
        R0 = Y0 + 1.402 * (V - 128);
        G0 = Y0 - 0.344 * (U - 128) - 0.714 * (V - 128);
        B0 = Y0 + 1.772 * (U - 128);

        R1 = Y1 + 1.402 * (V - 128);
        G1 = Y1 - 0.344 * (U - 128) - 0.714 * (V - 128);
        B1 = Y1 + 1.772 * (U - 128);

        // 存储RGB值到RGB_data数组中
        RGB_data[i * 3 / 2] = R0;
        RGB_data[i * 3 / 2 + 1] = G0;
        RGB_data[i * 3 / 2 + 2] = B0;

        RGB_data[i * 3 / 2 + 3] = R1;
        RGB_data[i * 3 / 2 + 4] = G1;
        RGB_data[i * 3 / 2 + 5] = B1;
    }
}


void convert_YUYV_to_RGB(const uint8_t* src, uint8_t* dest, size_t width, size_t height)
{
    YUYV422_to_RGB(src, dest, width*height*2);
}

void YUV420_to_RGB(const uint8_t  *Y, const uint8_t *U, const uint8_t  *V, unsigned char *RGB_data, int width, int height) {
    int i, j, index;
    unsigned char R, G, B;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            index = i * width + j;
            int Y_index = i * width + j;
            int UV_index = (i/2) * (width/2) + (j/2);

            int C = Y[Y_index] - 16;
            int D = U[UV_index] - 128;
            int E = V[UV_index] - 128;

            R = (298 * C + 409 * E + 128) >> 8;
            G = (298 * C - 100 * D - 208 * E + 128) >> 8;
            B = (298 * C + 516 * D + 128) >> 8;

            // 确保RGB值在0-255的范围内
            R = R < 0 ? 0 : (R > 255 ? 255 : R);
            G = G < 0 ? 0 : (G > 255 ? 255 : G);
            B = B < 0 ? 0 : (B > 255 ? 255 : B);

            // 存储RGB值到RGB_data数组中
            index *= 3;
            RGB_data[index] = R;
            RGB_data[index + 1] = G;
            RGB_data[index + 2] = B;
        }
    }
}

void convert_I420_to_RGB(const uint8_t* src, uint8_t* dest, size_t width, size_t height)
{
    YUV420_to_RGB(src, src+width*height, src+width*height/4, dest, width, height);
}


void convert_uyvy_to_RGB(unsigned char *UYVY_data, unsigned char *RGB_data, int width, int height) {
    int i, j;
    unsigned char Y1, U, Y2, V;
    unsigned char R, G, B;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j += 2) {
            int index = i * width + j * 2;

            Y1 = UYVY_data[index];
            U = UYVY_data[index + 1];
            Y2 = UYVY_data[index + 2];
            V = UYVY_data[index + 3];

            int C1 = Y1 - 16;
            int C2 = Y2 - 16;
            int D = U - 128;
            int E = V - 128;

            // Calculate R, G, B values
            R = (298 * C1 + 409 * E + 128) >> 8;
            G = (298 * C1 - 100 * D - 208 * E + 128) >> 8;
            B = (298 * C1 + 516 * D + 128) >> 8;

            // Clamp RGB values
            R = R < 0 ? 0 : (R > 255 ? 255 : R);
            G = G < 0 ? 0 : (G > 255 ? 255 : G);
            B = B < 0 ? 0 : (B > 255 ? 255 : B);

            // Store RGB values in RGB_data array
            int rgbIndex = i * width * 3 + j * 3;
            RGB_data[rgbIndex] = R;
            RGB_data[rgbIndex + 1] = G;
            RGB_data[rgbIndex + 2] = B;

            // Repeat the process for the second pixel
            R = (298 * C2 + 409 * E + 128) >> 8;
            G = (298 * C2 - 100 * D - 208 * E + 128) >> 8;
            B = (298 * C2 + 516 * D + 128) >> 8;

            R = R < 0 ? 0 : (R > 255 ? 255 : R);
            G = G < 0 ? 0 : (G > 255 ? 255 : G);
            B = B < 0 ? 0 : (B > 255 ? 255 : B);

            RGB_data[rgbIndex + 3] = R;
            RGB_data[rgbIndex + 4] = G;
            RGB_data[rgbIndex + 5] = B;
        }
    }
}


// Simple auto white balance adjustment function
void auto_white_balance(uint8_t* image_data, size_t width, size_t height, int channels) {
    long long sumR = 0, sumG = 0, sumB = 0;
    size_t totalPixels = width * height;
    for (size_t i = 0; i < totalPixels * channels; i += channels) {
        sumR += image_data[i];
        sumG += image_data[i + 1];
        sumB += image_data[i + 2];
    }
    unsigned char avgR = static_cast<unsigned char>(sumR / totalPixels);
    unsigned char avgG = static_cast<unsigned char>(sumG / totalPixels);
    unsigned char avgB = static_cast<unsigned char>(sumB / totalPixels);
    // Calculate adjustment factors
    float balanceR = avgG / static_cast<float>(avgR);
    float balanceG = 1.0;
    float balanceB = avgG / static_cast<float>(avgB);
    for (size_t i = 0; i < totalPixels * channels; i += channels) {
        image_data[i] = std::min(255, static_cast<int>(image_data[i] * balanceR));
        image_data[i + 1] = std::min(255, static_cast<int>(image_data[i + 1] * balanceG));
        image_data[i + 2] = std::min(255, static_cast<int>(image_data[i + 2] * balanceB));
    }
}

void apply_color_conversion(const uint8_t* image_data, uint8_t *dest, uint32_t fourcc, size_t width, size_t height) {

    // Auto white balance should be applied here, assuming RGB format
    switch (fourcc) {
        case K_TVU_PIXFMT_VIDEO_FOURCC_YUYV422:
            convert_YUYV_to_RGB(image_data, dest, width, height);
            break;
        case K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P:
            convert_I420_to_RGB(image_data, dest, width, height);
            break;
        case K_TVU_PIXFMT_VIDEO_FOURCC_UYVY422:
            convert_I420_to_RGB(image_data, dest, width, height);
            break;
        default:
            std::cerr << "Unsupported video format."
                      <<"fourcc:"<< fourcc << std::endl;
            break;
    }

    auto_white_balance(dest, width, height, 3);
}

int ConvertWriteShm(libshm_media_handle_t h,
                    const libshm_media_head_param_t &oh,
                    const libshm_media_item_param_t &oi)
{
    int ret = 0;



    const uint8_t * image_data = oi.p_vData;

    if (!image_data)
    {
        std::cerr << "Failed to acquire image buffer" << std::endl;
        return -1;
    }

    int width = oh.i_dstw;
    int height = oh.i_dsth;
    libshm_media_head_param_t ohp = {0};
    libshm_media_item_param_t ohi = {0};
    {
        ohp.i_dstw = width;
        ohp.i_dsth = height;
        ohp.u_videofourcc = K_TVU_PIXFMT_VIDEO_FOURCC_RGB24;
    }

    {
        ohi.i_vLen = width * height * 3;
    }

    libshm_media_item_addr_layout_t olayout;
    {
        memset(&olayout, 0, sizeof(olayout));
    }

    ret = LibShmMediaItemApplyBuffer(h, &ohi, &olayout);

    if (ret <= 0)
    {
        printf("apply item buffer failed, ret %d\n", ret);
        return -1;
    }

    {
        apply_color_conversion(image_data, olayout.p_vData, oh.u_videofourcc, width, height);
        ohi.p_vData = olayout.p_vData;
        ohp.i_duration = oh.i_duration;
        ohp.i_scale = oh.i_scale;
        ohi.i64_vpts = oi.i64_vpts;
        ohi.i64_vdts = oi.i64_vdts;
        ret = LibShmMediaItemCommitBuffer(h, &ohp, &ohi);
        if (ret <= 0) {
            std::cerr
                    << "Failed to commit modified image data."
                    << "ret:" << ret << std::endl;
            return -1;
        }
    }

    return ret;
}

int shm_log_hook(int levlel, const char *fmt, va_list ap)
{
    vprintf(fmt, ap);
    return 0;
}

int main() {

    LibShmMediaSetLogCallback(shm_log_hook);

    libshm_media_handle_t handle = LibShmMediaOpen("ImageSharedMemory", NULL, NULL);
    if (!handle) {
        std::cerr << "Failed to open shared memory" << std::endl;
        return -1;
    }

    libshm_media_handle_t outputShm = NULL;

    while (1)
    {
        uint32_t fourcc = 0;  // Hypothetical function to get the FourCC code
        size_t width = 1920;
        size_t height = 1080;
        const uint8_t* image_data = NULL;
        libshm_media_head_param_t oh = {0};
        libshm_media_item_param_t oi = {0};

        int ret = LibShmMediaPollReadable(handle, 0);

        if (ret < 0)
        {
            std::cerr << "shm readable I/O error, need break loop" << std::endl;
            break;
        }
        else if (ret == 0)
        {
            continue;
        }

        ret = LibShmMediaPollReadData(handle, &oh, &oi, 0);
        if (ret < 0)
        {
            std::cerr << "shm read I/O error, need break loop" << std::endl;
            break;
        }
        else if (ret == 0)
        {
            continue;
        }

        fourcc = oh.u_videofourcc;
        image_data = oi.p_vData;
        width = oh.i_dstw;
        height = oh.i_dsth;

        if (!outputShm)
        {
            outputShm = LibShmMediaCreate("banlanced_shm_out", 1024, 100, width*height*3);
        }

        if (!outputShm)
        {
            break;
        }
        
        {
            ConvertWriteShm(outputShm, oh, oi);
        }

    }

    LibShmMediaDestroy(handle);
    LibShmMediaDestroy(outputShm);
    return 0;
}
