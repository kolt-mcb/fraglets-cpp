from distutils.core import setup, Extension

module1 = Extension('cFraglets',
                    sources = [ 'moleculemultiset.cpp', 'keymultiset.cpp','fragletsToPy.cpp','fraglets.cpp'],
                    # include_dirs=['.','/usr/include/graphviz/']
                    extra_link_args=['-lgvc']
                    )

setup (name = 'cFraglets',
       version = '1.0',
       description = 'This is a demo package',
       ext_modules = [module1])


