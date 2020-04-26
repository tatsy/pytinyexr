project (pytinyexr)
include_directories(tinyexr)
add_subdirectory(pybind11)
pybind11_add_module(pytinyexr PyEXR.cpp)
