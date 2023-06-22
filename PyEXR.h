#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "tinyexr.h"

class PyEXRImage
{
public:
    PyEXRImage(const std::string &filename);
    PyEXRImage(int width, int height, int channels = 4);
    virtual ~PyEXRImage();

    float getPixel(int x, int y, int channel);
    float get(int i);
    float *data() { return out_rgba; }
    float *channel(const std::string &name)
    {
        return reinterpret_cast<float *>(image.images[name2index[name]]);
    }

    void load(const std::string &filename);
    void release();
    void set(int i, int channel, float value);
    void save(const std::string &filename);

    inline std::string getFilename() const { return filename; }
    inline int getWidth() const { return width; }
    inline int getHeight() const { return height; }
    inline int getChannels() const { return channels; }

private:
    float *out_rgba; // width * height * RGBA
    int width;
    int height;
    int channels;
    std::string filename;

    int ret;
    const char *err;

    bool isLoaded;
    EXRImage image;
    EXRHeader header;
    std::unordered_map<std::string, uint32_t> name2index;
};
