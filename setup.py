# -*- coding: utf-8 -*-
from setuptools import setup

packages = \
['tinyexr']

package_data = \
{'': ['*']}

install_requires = \
['pybind11>=2.10.1,<3.0.0', 'setuptools>=57.4.0,<58.0.0']

setup_kwargs = {
    'name': 'tinyexr',
    'version': '0.9.1',
    'description': 'Python binding for tinyexr',
    'long_description': "# pytinyexr\n\n[![Build Status](https://travis-ci.org/syoyo/PyEXR.svg?branch=master)](https://travis-ci.org/syoyo/PyEXR)\n\n![screenshot](screenshot.png)\n\nLoading OpenEXR (.exr) images using Python.\n\npytinyexr is a forked version of PyEXR(https://github.com/ialhashim/PyEXR) and maintained by TinyEXR author(s).\n\nIt is basically a Python binding for tinyexr. Use CMake to build the module (uses pybind11). Installation script is not there, you can simply copy the resulting python module files. Supports loading functionality, saving can be easily added (pull requests welcome!).\n\n# Usage\n```python\nfrom pytinyexr import PyEXRImage\n\n# Load an EXR image (tinyexr backend)\nimg = PyEXRImage('2by2.exr')\n\n# Print basic details\nprint(img)\n\n# Pixel values access\nr = img.getPixel(x,y,0)\ng = img.getPixel(x,y,1)\nb = img.getPixel(x,y,2)\na = img.getPixel(x,y,3)\n\n# Numpy:\nm = np.array(img, copy = False)\n# or\nrgb = np.reshape(np.array(rgb_img, copy = False), (rgb_img.height, rgb_img.width, 4))\n# a matrix of (height x width x channels)\n\n# Display\nfrom PIL import Image\nImage.fromarray(np.clip(np.uint8(rgb*255.0), 0, 255)).show()\n```\n\n# PyPI package\n\nPyPI package is registered as pytinyexr: https://pypi.org/project/pytinyexr/\n\n```\n$ pip install pytinyexr\n```\n\n### For developer\n\nFor each release, upload source distribution from local.\n\n```\n$ rm -rf dist && python setup.py sdist\n$ twine upload dist/*.tar.gz\n```\n\n## License\n\nEven though original PyEXR has unclear license, I'd like to use MIT license for pytinyexr(only applicable to python binding codes).\n\n### Third party licenses\n\nTinyEXR is licensed under BSD license.\n\n\n## Notice.\n\nPython2.7 binary wheel is not provided.\n",
    'author': 'Syoyo Fujita',
    'author_email': 'syoyo@lighttransport.com',
    'maintainer': 'None',
    'maintainer_email': 'None',
    'url': 'https://github.com/syoyo/pytinyexr',
    'packages': packages,
    'package_data': package_data,
    'install_requires': install_requires,
    'python_requires': '>=3.8.1,<3.11',
}
from build import *
build(setup_kwargs)

setup(**setup_kwargs)
