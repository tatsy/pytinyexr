#define TINYEXR_IMPLEMENTATION
#include "PyEXR.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdexcept>

PyEXRImage::PyEXRImage(const std::string &filename)
    : filename{ filename }
{
    load(filename);
}

PyEXRImage::PyEXRImage(int width, int height, int channels)
    : width{ width }
    , height{ height }
    , channels{ channels }
{
    images.assign(channels, std::vector<float>(width * height, 1.0f));
}

void PyEXRImage::load(const std::string &filename)
{
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    int ret;
    const char *err;

    // Load EXR version
    EXRVersion exr_version;
    ret = ParseEXRVersionFromFile(&exr_version, filename.c_str());
    if (ret != 0)
    {
        throw std::runtime_error("Invalid EXR file: " + filename);
    }

    if (exr_version.multipart)
    {
        throw std::runtime_error("Multipart flag must be false!");
    }

    // Read EXR header
    ret = ParseEXRHeaderFromFile(&header, &exr_version, filename.c_str(), &err);
    if (ret != 0)
    {
        const std::string errMsg(err);
        FreeEXRErrorMessage(err);
        throw std::runtime_error("Parse EXR err: " + errMsg);
    }

    // NOTE: Assume all data are represented by 32bit float
    for (int i = 0; i < header.num_channels; i++)
    {
        if (header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF)
        {
            header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
        }
    }

    // Read image data
    ret = LoadEXRImageFromFile(&image, &header, filename.c_str(), &err);
    if (ret != 0)
    {
        const std::string errMsg(err);
        FreeEXRErrorMessage(err);
        throw std::runtime_error("Load EXR err: " + errMsg);
    }
    width = image.width;
    height = image.height;
    channels = image.num_channels;

    // Copy image data
    images.resize(channels);
    for (int c = 0; c < channels; c++)
    {
        images[c] = std::vector<float>(width * height);
        std::memcpy(images[c].data(), image.images[c], sizeof(float) * width * height);
    }

    // Create a map from channel name to index
    name2index.clear();
    for (int i = 0; i < header.num_channels; i++)
    {
        const std::string key = header.channels[i].name;
        name2index[key] = i;
    }

    FreeEXRImage(&image);
    FreeEXRHeader(&header);
}

void PyEXRImage::save(const std::string &filename) const
{
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    int ret;
    const char *err;

    image.num_channels = channels;

    std::vector<const float *> image_ptr(channels, NULL);
    for (int c = 0; c < channels; c++)
    {
        image_ptr[c] = images[c].data();
    }

    image.images = (unsigned char **)image_ptr.data();
    image.width = width;
    image.height = height;

    header.num_channels = channels;
    header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    for (auto it = name2index.begin(); it != name2index.end(); ++it)
    {
        snprintf(header.channels[it->second].name, 512, "%s", it->first.c_str());
    }

    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    for (int c = 0; c < header.num_channels; c++)
    {
        header.pixel_types[c] = TINYEXR_PIXELTYPE_FLOAT;           // pixel type of input image
        header.requested_pixel_types[c] = TINYEXR_PIXELTYPE_HALF;  // pixel type of output image to be stored in .EXR
    }

    ret = SaveEXRImageToFile(&image, &header, filename.c_str(), &err);
    if (ret != TINYEXR_SUCCESS)
    {
        const std::string errMsg = std::string(err);
        FreeEXRErrorMessage(err);
        throw std::runtime_error("Save EXR err: " + errMsg);
    }

    free(header.pixel_types);
    free(header.requested_pixel_types);
}

float PyEXRImage::getPixel(int x, int y, int channel) const
{
    const uint32_t pixel_idx = (y * getWidth()) + x;
    return images[channel][pixel_idx];
}

void PyEXRImage::setPixel(int x, int y, int channel, float value)
{
    const uint32_t pixel_idx = (y * getWidth()) + x;
    images[channel][pixel_idx] = value;
}

float PyEXRImage::get(int i, int channel) const
{
    return images[channel][i];
}
void PyEXRImage::set(int i, int channel, float value)
{
    images[channel][i] = value;
}

/* Python Binding */
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

PYBIND11_MODULE(tinyexr, m)
{
    m.doc() = "EXR image files reader";
    py::class_<PyEXRImage>(m, "PyEXRImage", py::dynamic_attr(), py::buffer_protocol())
        .def(py::init<const std::string &>())
        .def(py::init<int, int>())
        .def("getPixel", &PyEXRImage::getPixel, "Get a pixel's RGBA values", py::arg("x"), py::arg("y"),
             py::arg("channel"))
        .def("setPixel", &PyEXRImage::setPixel, "Set a pixel's RGBA values", py::arg("x"), py::arg("y"),
             py::arg("channel"), py::arg("value"))
        .def("get", &PyEXRImage::get, "Get pixel value", py::arg("i"), py::arg("channel"))
        .def("set", &PyEXRImage::set, "Set pixel value", py::arg("i"), py::arg("channel"), py::arg("value"))
        .def("save", &PyEXRImage::save)
        .def_property("width", &PyEXRImage::getWidth, nullptr, py::return_value_policy::copy)
        .def_property("height", &PyEXRImage::getHeight, nullptr, py::return_value_policy::copy)
        .def_property("filename", &PyEXRImage::getFilename, nullptr, py::return_value_policy::copy)
        .def_property("channels", &PyEXRImage::getChannels, nullptr, py::return_value_policy::copy)
        .def_property("names", &PyEXRImage::getNames, nullptr, py::return_value_policy::move)
        .def("channel",
             [](PyEXRImage &m, const std::string &name) -> py::array_t<float>
             {
                 return py::array_t<float>({ m.getHeight(), m.getWidth() },
                                           { m.getWidth() * sizeof(float), sizeof(float) }, m.channel(name));
             })
        .def("__repr__",
             [](const PyEXRImage &m) -> std::string
             {
                 return "<EXR Image " + std::to_string(m.getWidth()) + "x" + std::to_string(m.getHeight()) + " '" +
                        m.getFilename() + "'>";
             })
        .def_buffer(
            [](PyEXRImage &m) -> py::buffer_info
            {
                return py::buffer_info(
                    m.data(),                                          /* Pointer to buffer */
                    sizeof(float),                                     /* Size of one scalar */
                    py::format_descriptor<float>::format(),            /* Python struct-style format descriptor */
                    3,                                                 /* Number of dimensions */
                    { m.getHeight(), m.getHeight(), m.getChannels() }, /* Buffer dimensions */
                    { m.getWidth() * m.getChannels() * sizeof(float), m.getChannels() * sizeof(float), sizeof(float) }
                    /* Strides (in bytes) for each index */
                );
            });
}
