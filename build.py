import platform

from setuptools.command.build_ext import build_ext

from pybind11.setup_helpers import Pybind11Extension

sources = [
    'pytinyexr/PyEXR.cpp',
    'tinyexr/deps/miniz/miniz.c',
]
include_dirs = [
    './pytinyexr/',
    './tinyexr/',
    './tinyexr/deps/miniz',
]

extra_compile_args = []
extra_link_args = []
define_macros = []
if platform.system() == 'Windows':
    pass
elif platform.system() == 'Darwin':
    extra_compile_args.extend([
        '-mmacosx-version-min=10.9',
    ])
    extra_link_args.extend([])
else:
    extra_compile_args.extend([
        '-std=c++11',
    ])
    extra_link_args.extend([])

ext_modules = [
    Pybind11Extension('pytinyexr',
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
