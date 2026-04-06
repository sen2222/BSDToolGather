#ifndef IMGDECODER_H
#define IMGDECODER_H

#include <QImage>
#include <cstdint>


typedef enum
{
    I2D_ANGLE_0 = 0,
    I2D_ANGLE_90 = 90,
    I2D_ANGLE_180 = 180,
    I2D_ANGLE_270 = 270,
}I2D_ANGLE_E;
class ImgDecoder
{
public:
    ImgDecoder() = default;
    ~ImgDecoder() = default;
    ImgDecoder(const ImgDecoder&) = delete;
    ImgDecoder& operator=(const ImgDecoder&) = delete;

    int yuv420decode(uint8_t *yuv420, uint32_t width, uint32_t height, QImage &img, I2D_ANGLE_E angle);
private:
    void yuv2rgb(uint8_t Y, uint8_t U, uint8_t V, uint8_t& R, uint8_t& G, uint8_t& B);
};

#endif // IMGDECODER_H