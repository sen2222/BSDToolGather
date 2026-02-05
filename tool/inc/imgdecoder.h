#ifndef IMGDECODER_H
#define IMGDECODER_H

#include <QImage>
#include <cstdint>

class ImgDecoder
{
public:
    ImgDecoder() = default;
    ~ImgDecoder() = default;
    ImgDecoder(const ImgDecoder&) = delete;
    ImgDecoder& operator=(const ImgDecoder&) = delete;

    int yuv420decode(uint8_t *yuv420, int width, int height, QImage &img);

private:
    void yuv2rgb(uint8_t Y, uint8_t U, uint8_t V, uint8_t& R, uint8_t& G, uint8_t& B);
};

#endif // IMGDECODER_H