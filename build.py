import platform

from pybind11.setup_helpers import Pybind11Extension
from setuptools.command.build_ext import build_ext

sources = [
    'PyEXR.cpp',
]

include_dirs = [
    './tinyexr/',
    './tinyexr/examples/common/',
]

extra_compile_args = []
extra_link_args = []
define_macros = [('TINYEXR_USE_STB_ZLIB', 1), ('TINYEXR_USE_MINIZ', 0)]
if platform.system() == 'Windows':
    pass
elif platform.system() == 'Darwin':
    extra_compile_args.extend([
        '-std=c++14',
        '-mmacosx-version-min=10.9',
    ])
    extra_link_args.extend([
        '-stdlib=libc++',
    ])
else:
    extra_compile_args.extend([
        '-std=c++14',
    ])
    extra_link_args.extend([])

ext_modules = [
    Pybind11Extension('tinyexr',
                      sources,
                      language='c++',
                      include_dirs=include_dirs,
                      extra_compile_args=extra_compile_args,
                      extra_link_args=extra_link_args,
                      define_macros=define_macros)
]


def build(setup_kwargs):
    with open('README.md', 'r') as fh:
        long_description = fh.read()

    setup_kwargs.update({
        'long_description': long_description,
        'long_description_content_type': 'text/markdown',
        'ext_modules': ext_modules,
        'cmdclass': {
            'build_ext': build_ext
        },
    })
