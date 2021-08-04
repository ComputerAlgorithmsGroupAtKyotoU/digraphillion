from ntpath import join
from setuptools import setup, Extension
import os
import sys

sys.path.insert(0, 'digraphillion')
import release

ext_module_sources = [
    os.path.join('src', 'pydigraphillion.cc'),
    os.path.join('src', 'digraphillion', 'graphset.cc'),
    os.path.join('src', 'digraphillion', 'setset.cc'),
    os.path.join('src', 'digraphillion', 'zdd.cc'),
    os.path.join('src', 'SAPPOROBDD', 'bddc.c'),
    os.path.join('src', 'SAPPOROBDD', 'BDD.cc'),
    os.path.join('src', 'SAPPOROBDD', 'ZBDD.cc')
]

setup(
    name='DiGraphillion',
    version=release.version,
    keywords=['graph', 'set', 'math', 'network'],
    license=release.license,
    packages=['digraphillion'],
    ext_modules=[Extension(
        '_digraphillion',
        sources=ext_module_sources,
        include_dirs=['src', 'src/SAPPOROBDD'],
        libraries=[],
        define_macros=[('B_64', None)],
        extra_compile_args=['-O3', '-march=native'],
        extra_link_args=[],
    ), ],
)
