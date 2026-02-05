#include "imgdecoder.h"

void ImgDecoder::yuv2rgb(uint8_t Y, uint8_t U, uint8_t V, uint8_t& R, uint8_t& G, uint8_t& B)
{
    int u = U - 128;
    int v = V - 128;
    int r = Y + (359 * v) / 256;
    int g = Y - (88 * u) / 256 - (183 * v) / 256;
    int b = Y + (454 * u) / 256;
    R = static_cast<uint8_t>(qBound(0, r, 255));
    G = static_cast<uint8_t>(qBound(0, g, 255));
    B = static_cast<uint8_t>(qBound(0, b, 255));
}

int ImgDecoder::yuv420decode(uint8_t *yuv420, int width, int height, QImage &img)
{
    int y_size = 0;
    int uv_size = 0;
    int u_offset = 0;
    int v_offset = 0;
    uint8_t* v_ptr = nullptr; // 改名为v_ptr，标识是指针
    uchar* img_data = nullptr;
    int img_step = 0;
    int uv_y = 0;
    int uv_x = 0;
    int uv_idx = 0;
    uint8_t Y = 0;
    uint8_t U = 0;
    uint8_t V = 0; // 普通变量V，和指针区分
    uint8_t R = 0;
    uint8_t G = 0;
    uint8_t B = 0;
    uchar* pixel = nullptr;

    if (yuv420 == nullptr || width <= 0 || height <= 0 || (width % 2 != 0) || (height % 2 != 0))
    {
        return -1;
    }

    y_size = width * height;
    uv_size = y_size / 4;
    u_offset = y_size;
    v_offset = y_size + uv_size;
    v_ptr = yuv420 + v_offset; // 指针用v_ptr
    if (v_ptr + uv_size > yuv420 + y_size * 3 / 2)
    {
        return -2;
    }

    img = QImage(width, height, QImage::Format_RGB888);
    img_data = img.bits();
    img_step = img.bytesPerLine();

    for (int y = 0; y < height; y++)
    {
        uv_y = y / 2;
        for (int x = 0; x < width; x++)
        {
            uv_x = x / 2;
            uv_idx = uv_y * (width / 2) + uv_x;

            Y = yuv420[y * width + x];
            U = yuv420[u_offset + uv_idx];
            V = yuv420[v_offset + uv_idx]; // 普通变量V赋值

            yuv2rgb(Y, U, V, R, G, B); // 传普通变量V，类型匹配

            pixel = img_data + y * img_step + x * 3;
            pixel[0] = B;
            pixel[1] = G;
            pixel[2] = R;
        }
    }

    return 0;
}