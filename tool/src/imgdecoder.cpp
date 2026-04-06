#include "imgdecoder.h"
#include "libyuv.h"
int ImgDecoder::yuv420decode(uint8_t *yuv420, uint32_t width, uint32_t height, QImage &img, I2D_ANGLE_E angle)
{
    if (!yuv420 || width == 0 || height == 0 || (width % 2 != 0) || (height % 2 != 0))
        return -1;

    int w = (int)width;
    int h = (int)height;
    int dst_w = w;
    int dst_h = h;
    libyuv::RotationMode rot_mode = libyuv::kRotate0;
    switch (angle)
    {
        case I2D_ANGLE_90:
        {
            rot_mode = libyuv::kRotate90;
            dst_w = h;
            dst_h = w;
        }break;
        case I2D_ANGLE_180:
        {
            rot_mode = libyuv::kRotate180;
            dst_w = w;
            dst_h = h;
        }break;
        case I2D_ANGLE_270:
        {
            rot_mode = libyuv::kRotate270;
            dst_w = h;
            dst_h = w;
        }break;
        default:
        {
            rot_mode = libyuv::kRotate0;
        }break;
    }
    uint8_t *y = yuv420;
    uint8_t *uv = yuv420 + w * h;
    QImage argb_img(w, h, QImage::Format_ARGB32);
    libyuv::NV12ToARGB(y, w, uv, w, (uint8_t *)argb_img.bits(), w * 4, w, h);
    if (rot_mode == libyuv::kRotate0)
    {
        img = std::move(argb_img);
        return 0;
    }
    img = QImage(dst_w, dst_h, QImage::Format_ARGB32);
    libyuv::ARGBRotate((const uint8_t *)argb_img.constBits(), w * 4, (uint8_t *)img.bits(), dst_w * 4, w, h, rot_mode);

    return 0;
}

void ImgDecoder::yuv2rgb(uint8_t Y, uint8_t U, uint8_t V, uint8_t &R, uint8_t &G, uint8_t &B)
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
