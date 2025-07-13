#
#  fraglets.py:
#
#  TOP INTERFACE: DEFINITION OF PYTHON WRAPPING CLASS FOR C++ `Fraglets` CLASS.
#
#
try:
    import cFraglets
except:
    strErr = "\n\n`cFraglets` module not found, "
    strErr += "run `$ make car_py_module`! \n"
    raise RuntimeError(strErr)

class fraglets():

    def __init__(self):

        # Construct the `Car` object and store a Python capsule with
        # a C++ pointer to the object
        #
        # Take a look at function `construct` in `car2py.cpp` file
        self.cfraglets = cFraglets.construct()
        self.unimolTags = cFraglets.getUnimolTags(self.cfraglets)
        self.iter = 0

    def run(self, iter,size,quiet=False):
        cFraglets.run(self.cfraglets,iter,size,quiet)
        self.iter = cFraglets.getIter(self.cfraglets)

    def run_threads(self, iter, size, threads, quiet=False):
        cFraglets.run_threads(self.cfraglets, iter, size, threads, quiet)
        self.iter = cFraglets.getIter(self.cfraglets)

    def parse(self, line):
        cFraglets.parse(self.cfraglets,line)


    def drawGraphViz(self):
        cFraglets.drawGraphViz(self.cfraglets)

    def get_sorted(self):
        return cFraglets.getSorted(self.cfraglets)


    def __delete__(self):
        cFraglets.delete_object(self.cfraglets)

