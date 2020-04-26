import setuptools
import platform

fh = open("README.md", "r")
long_description = fh.read()

# Adapted from https://github.com/pybind/python_example/blob/master/setup.py
class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False, pep517=False):
        self.user = user
        self.pep517 = pep517

    def __str__(self):
        import os
        import pybind11

        interpreter_include_path = pybind11.get_include(self.user)

        if self.pep517:
            # When pybind11 is installed permanently in site packages, the headers
            # will be in the interpreter include path above. PEP 517 provides an
            # experimental feature for build system dependencies. When installing
            # a package from a source distribvution, first its build dependencies
            # are installed in a temporary location. pybind11 does not return the
            # correct path for this condition, so we glom together a second path,
            # and ultimately specify them _both_ in the include search path.
            # https://github.com/pybind/pybind11/issues/1067
            return os.path.abspath(
                os.path.join(
                    os.path.dirname(pybind11.__file__),
                    "..",
                    "..",
                    "..",
                    "..",
                    "include",
                    os.path.basename(interpreter_include_path),
                )
            )
        else:
            return interpreter_include_path

#ext_compile_args = []
ext_compile_args = ["-std=c++11"]
ext_link_args = []

# Raise macosx version.
if platform.system() == "Darwin":
   # XCode10 or later does not support libstdc++, so we need to use libc++.
   # macosx-version 10.6 does not support libc++, so we require min macosx version 10.9.
   ext_compile_args.append("-stdlib=libc++")
   ext_compile_args.append("-mmacosx-version-min=10.9")
   ext_link_args.append("-stdlib=libc++")
   ext_link_args.append("-mmacosx-version-min=10.9")

m = setuptools.Extension(
    "pytinyexr",
    extra_compile_args=ext_compile_args,
    extra_link_args=ext_link_args,
    sources=["PyEXR.cpp"],
    include_dirs=[
        "./tinyexr/",
        # Support `build_ext` finding pybind 11 (provided it's permanently
        # installed).
        get_pybind_include(),
        get_pybind_include(user=True),
        # Support building from a source distribution finding pybind11 from
        # a PEP 517 temporary install.
        get_pybind_include(pep517=True),
    ],
    language="c++",
)

setuptools.setup(
    name="pytinyexr",
    version="0.9.0",
    description="Python bindings for TinyEXR(OpenEXR loader/saver)",
    long_description=long_description,
    long_description_content_type="text/markdown",
    author="Syoyo Fujita",
    author_email="",
    url="https://github.com/syoyo/PyEXR",
    project_urls={
        "Issue Tracker": "https://github.com/ialhashim/PyEXR/issues",
    },
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Intended Audience :: Manufacturing",
        "Topic :: Artistic Software",
        "Topic :: Multimedia :: Graphics :: 3D Modeling",
        "Topic :: Scientific/Engineering :: Visualization",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
    ],
    packages=setuptools.find_packages(),
    ext_modules=[m],
)


