from setuptools import setup, Extension

module = Extension(
    'cFraglets',
    sources=[
        'moleculemultiset.cpp',
        'keymultiset.cpp',
        'fragletsToPy.cpp',
        'fraglets.cpp',
    ],
    extra_link_args=['-lgvc'],
)

setup(
    name='cFraglets',
    version='1.0',
    description='Fraglets C++ extension module',
    ext_modules=[module],
)
