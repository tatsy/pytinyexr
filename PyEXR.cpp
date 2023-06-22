#define TINYEXR_IMPLEMENTATION
#include "PyEXR.h"

#include <stdexcept>

PyEXRImage::PyEXRImage(const std::string &filename)
    : width(-1), height(-1), channels(-1), out_rgba(nullptr), filename(filename), isLoaded(false)
{
    load(filename);
}

PyEXRImage::PyEXRImage(int width, int height, int channels)
    : width(width), height(height), channels(channels), out_rgba(nullptr)
{
    auto size = 4 * static_cast<size_t>(width) * static_cast<size_t>(height);
    out_rgba = reinterpret_cast<float *>(malloc(sizeof(float) * size));

    for (size_t i = 0; i < size; ++i)
    {
        out_rgba[i] = 1.0f;
    }
}

PyEXRImage::~PyEXRImage()
{
    release();
}

void PyEXRImage::load(const std::string &filename)
{
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
    InitEXRHeader(&header);
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
    InitEXRImage(&image);
    ret = LoadEXRImageFromFile(&image, &header, filename.c_str(), &err);
    if (ret != 0)
    {
        const std::string errMsg(err);
        FreeEXRErrorMessage(err);
        throw std::runtime_error("Load EXR err: " + errMsg);
    }

    // Create a map from channel name to index
    name2index.clear();
    for (int i = 0; i < header.num_channels; i++)
    {
        const std::string key = header.channels[i].name;
        name2index[key] = i;
    }

    width = image.width;
    height = image.height;
    channels = image.num_channels;
    isLoaded = true;
}

void PyEXRImage::release()
{
    if (!isLoaded)
    {
        return;
    }

    FreeEXRImage(&image);
    FreeEXRHeader(&header);
    isLoaded = false;
}

void PyEXRImage::save(const std::string &filename)
{
    // SaveEXR(out_rgba, width, height, 4, 1, filename.c_str());
}

float PyEXRImage::getPixel(int x, int y, int channel)
{
    auto line_width = width * 4;
    auto pixel_idx = (y * line_width) + (x * 4);
    return out_rgba[pixel_idx + channel];
}

float PyEXRImage::get(int i)
{
    return out_rgba[i];
}

void PyEXRImage::set(int i, int channel, float value)
{
    out_rgba[i * 4 + channel] = value;
}

/* Python Binding */
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

PYBIND11_MODULE(pytinyexr, m)
{
    m.doc() = "EXR image files reader";
    py::class_<PyEXRImage>(m, "PyEXRImage", py::dynamic_attr(), py::buffer_protocol())
        .def(py::init<const std::string &>())
        .def(py::init<int, int>())
        .def("getPixel", &PyEXRImage::getPixel, "Get a pixel's RGBA values", py::arg("x"), py::arg("y"), py::arg("channel"))
        .def("get", &PyEXRImage::get)
        .def("set", &PyEXRImage::set, "Set pixel value", py::arg("i"), py::arg("channel"), py::arg("value"))
        .def("save", &PyEXRImage::save)
        .def_property("width", &PyEXRImage::getWidth, nullptr, py::return_value_policy::copy)
        .def_property("height", &PyEXRImage::getHeight, nullptr, py::return_value_policy::copy)
        .def_property("filename", &PyEXRImage::getFilename, nullptr, py::return_value_policy::copy)
        .def_property("channels", &PyEXRImage::getChannels, nullptr, py::return_value_policy::copy)
        .def("channel", [](PyEXRImage &m, const std::string &name) -> py::array_t<float>
             { return py::array_t<float>(
                   {m.getHeight(), m.getWidth()},
                   {m.getWidth() * sizeof(float), sizeof(float)},
                   m.channel(name)); })
        // .def_readwrite("ret", &PyEXRImage::ret)
        // .def_readwrite("err", &PyEXRImage::err)
        .def("__repr__",
             [](const PyEXRImage &m) -> std::string
             {
                 return "<EXR Image " + std::to_string(m.getWidth()) + "x" + std::to_string(m.getHeight()) + " '" + m.getFilename() + "'>";
             })
        .def_buffer([](PyEXRImage &m) -> py::buffer_info
                    { return py::buffer_info(
                          m.data(),                                                                                        /* Pointer to buffer */
                          sizeof(float),                                                                                   /* Size of one scalar */
                          py::format_descriptor<float>::format(),                                                          /* Python struct-style format descriptor */
                          3,                                                                                               /* Number of dimensions */
                          {m.getHeight(), m.getHeight(), m.getChannels()},                                                 /* Buffer dimensions */
                          {m.getWidth() * m.getChannels() * sizeof(float), m.getChannels() * sizeof(float), sizeof(float)} /* Strides (in bytes) for each index */
                      ); });
}
