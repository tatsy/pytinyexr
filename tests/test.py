import os
import hashlib

import numpy as np
import pytest

from tinyexr import PyEXRImage


def test_2by2():
    filename = 'data/2by2.exr'
    exrImage = PyEXRImage(filename)

    assert exrImage.filename == filename
    assert exrImage.width == 2
    assert exrImage.height == 2
    print(exrImage)

    names = exrImage.names
    assert "R" in names and "G" in names and "B" in names and "A" in names
    print(names)

    # Direct access to floats
    try:
        for i in range(exrImage.width * exrImage.height):
            r = exrImage.get(i, 0)
            g = exrImage.get(i, 1)
            b = exrImage.get(i, 2)
            a = exrImage.get(i, 3)
            print(r, g, b, a)
        print('--------------------------')
    except Exception as e:
        pytest.fail(str(e))

    # Access by index x,y,channel
    try:
        for y in range(exrImage.width):
            for x in range(exrImage.height):
                r = exrImage.getPixel(x, y, 0)
                g = exrImage.getPixel(x, y, 1)
                b = exrImage.getPixel(x, y, 2)
                a = exrImage.getPixel(x, y, 3)
                print(r, g, b, a)
        print('--------------------------')
    except Exception as e:
        pytest.fail(str(e))

    # Convert to NumPy
    try:
        m = np.array(exrImage, copy=False)
        print(m)
        print('--------------------------')
    except Exception as e:
        pytest.fail(str(e))

    # Shape into x,y,channel matrix
    try:
        t = np.reshape(m, (exrImage.width, exrImage.height, 4))
        print(t)
        print('--------------------------')
    except Exception as e:
        pytest.fail(str(e))

    # Save EXR file
    try:
        exrImage.save('temp.exr')
    except Exception as e:
        pytest.fail(str(e))

    md5_src = hashlib.md5()
    with open(filename, 'rb') as f:
        md5_src.update(f.read())

    md5_dst = hashlib.md5()
    with open('temp.exr', 'rb') as f:
        md5_dst.update(f.read())

    assert md5_src.hexdigest() == md5_dst.hexdigest()
    os.remove('temp.exr')
