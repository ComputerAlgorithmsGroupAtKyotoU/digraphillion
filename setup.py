from ntpath import join
from setuptools import setup, Extension
import os
import sys

sys.path.insert(0, 'digraphillion')
import release

root_dir = os.path.normpath(os.path.join(__file__, os.pardir))
ext_module_sources = [
    os.path.join(root_dir, 'src', 'pydigraphillion.cc'),
    os.path.join(root_dir, 'src', 'digraphillion', 'graphset.cc'),
    os.path.join(root_dir, 'src', 'digraphillion', 'setset.cc'),
    os.path.join(root_dir, 'src', 'digraphillion', 'zdd.cc'),
    os.path.join(root_dir, 'src', 'SAPPOROBDD', 'bddc.c'),
    os.path.join(root_dir, 'src', 'SAPPOROBDD', 'BDD.cc'),
    os.path.join(root_dir, 'src', 'SAPPOROBDD', 'ZBDD.cc')
]

setup(
    ext_modules=[Extension(
        '_digraphillion',
        sources=ext_module_sources,
        include_dirs=['src', 'src/SAPPOROBDD'],
        libraries=[],
        define_macros=[('B_64', None)],
        extra_compile_args=['-march=native'],
        extra_link_args=[],
    ), ],
    author=release.authors[0][0],
    author_email=release.authors[0][1],
    test_suite='digraphillion.test'
)
