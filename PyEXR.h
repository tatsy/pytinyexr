#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "tinyexr.h"

class PyEXRImage
{
public:
    PyEXRImage() = default;
    explicit PyEXRImage(const std::string &filename);
    PyEXRImage(int width, int height, int channels = 4);
    virtual ~PyEXRImage() = default;

    float get(int i, int channel) const;
    void set(int i, int channel, float value);
    float getPixel(int x, int y, int channel) const;
    void setPixel(int x, int y, int channel, float value);

    float *data()
    {
        const uint32_t width = getWidth();
        const uint32_t height = getHeight();
        const uint32_t channels = getChannels();
        float *pixels = new float[width * height * channels];
        for (uint32_t y = 0; y < height; y++)
        {
            for (uint32_t x = 0; x < width; x++)
            {
                for (uint32_t c = 0; c < channels; c++)
                {
                    pixels[(y * width + x) * channels + c] = images[c][y * width + x];
                }
            }
        }
        return pixels;
    }

    float *channel(const std::string &name)
    {
        return images[name2index[name]].data();
    }

    void load(const std::string &filename);
    void save(const std::string &filename) const;

    inline std::string getFilename() const
    {
        return filename;
    }
    inline int getWidth() const
    {
        return width;
    }
    inline int getHeight() const
    {
        return height;
    }
    inline int getChannels() const
    {
        return channels;
    }
    inline std::vector<std::string> getNames() const
    {
        using P = std::pair<std::string, uint32_t>;
        std::vector<std::string> ret(name2index.size());
        std::transform(name2index.begin(), name2index.end(), ret.begin(),
                       [](const P &it) -> std::string { return it.first; });
        return ret;
    }

private:
    int width = -1;
    int height = -1;
    int channels = -1;
    std::string filename = "";

    std::vector<std::vector<float>> images;
    std::unordered_map<std::string, uint32_t> name2index;
};
