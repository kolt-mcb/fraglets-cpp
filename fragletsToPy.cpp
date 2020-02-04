#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "fraglets.h"


static PyObject* construct(PyObject* self, PyObject* args)
{

    fraglets* fraglet = new fraglets();

    PyObject* fragletsCapsule = PyCapsule_New((void *)fraglet, "fragletsPtr", NULL);
    PyCapsule_SetPointer(fragletsCapsule, (void *)fraglet);

    return Py_BuildValue("O", fragletsCapsule); 
}


PyObject* parse(PyObject* self, PyObject* args)
{

    PyObject* fragletsCapsule_;

    const char* str;
    // Process arguments
    PyArg_ParseTuple(args, "Os",
                     &fragletsCapsule_,
                     &str);
    symbol line(str);
    fraglets* frag = (fraglets*)PyCapsule_GetPointer(fragletsCapsule_, "fragletsPtr");
    frag->parse(line);

    // Return nothing
    return Py_BuildValue("");
}

PyObject* drawGraphViz(PyObject* self, PyObject* args)
{

    PyObject* fragletsCapsule_;


    // Process arguments
    PyArg_ParseTuple(args, "O",
                     &fragletsCapsule_);
    fraglets* frag = (fraglets*)PyCapsule_GetPointer(fragletsCapsule_, "fragletsPtr");
    frag->drawGraphViz();

    // Return nothing
    return Py_BuildValue("");
}



PyObject* run(PyObject* self, PyObject* args)
{
    PyObject* fragletsCapsule_;
    int iter_;
    int size_;
    bool quiet_;
    // Process arguments
    PyArg_ParseTuple(args, "Oiib",
                     &fragletsCapsule_,
                     &iter_,
                     &size_,
                     &quiet_);
    fraglets* frag = (fraglets*)PyCapsule_GetPointer(fragletsCapsule_, "fragletsPtr");

    frag->run(iter_,size_,quiet_);

    // Return nothing
    return Py_BuildValue("");
}

PyObject* getIter(PyObject* self, PyObject* args)
{
    PyObject* fragletsCapsule_;

    // Process arguments
    PyArg_ParseTuple(args, "O",
                     &fragletsCapsule_);
    fraglets* frag = (fraglets*)PyCapsule_GetPointer(fragletsCapsule_, "fragletsPtr");
    
    PyObject* iter = PyLong_FromLong(frag->iter);

    // Return nothing
    return iter;
}



PyObject* getUnimolTags(PyObject* self, PyObject* args)
{
    PyObject* fragletsCapsule_;

    // Process arguments
    PyArg_ParseTuple(args, "O",
                     &fragletsCapsule_);
    fraglets* frag = (fraglets*)PyCapsule_GetPointer(fragletsCapsule_, "fragletsPtr");

    PyObject* tags = PyList_New(unimolTags.size());
    int c=0;
    for (auto tag : unimolTags ){
        PyObject* python_string = Py_BuildValue("s", tag.c_str());
        PyList_SetItem(tags,c,python_string);
        c++;
    }

    return tags;
}


PyObject* delete_object(PyObject* self, PyObject* args)
{

    PyObject* fragletsCapsule_;  


    PyArg_ParseTuple(args, "O",
                     &fragletsCapsule_);

    fraglets* frag = (fraglets*)PyCapsule_GetPointer(fragletsCapsule_, "fragletsPtr");

    delete frag;

    return Py_BuildValue("");
}



static PyMethodDef fragletsFunctions[] =
{
/*
 *  Structures which define functions ("methods") provided by the module.
 */
    {"construct",                   // C++/Py Constructor
      construct, METH_VARARGS,
     "Create `fraglets` object"},
    {"run",run, METH_VARARGS,
    "runs vessel"},
    {"parse",parse, METH_VARARGS,
    "parses string and injects mol"},
    {"getUnimolTags",getUnimolTags,METH_VARARGS,
    "gets the unimol tags"},
    {"getIter",getIter,METH_VARARGS,
    "gets the current number of iterations"},
    {"drawGraphViz",drawGraphViz,METH_VARARGS,
    "draws graph"},
    {"delete_object",               // C++/Py Destructor
      delete_object, METH_VARARGS,
     "Delete `fraglets` object"},

    {NULL, NULL, 0, NULL}      // Last function description must be empty.
                               // Otherwise, it will create seg fault while
                               // importing the module.
};


static struct PyModuleDef fragletsModule =
{

   PyModuleDef_HEAD_INIT,
   "cFraglets",               // Name of the module.

   NULL,                 // Docstring for the module - in this case empty.

   -1,                   // Used by sub-interpreters, if you do not know what
                         // it is then you do not need it, keep -1 .

   fragletsFunctions         // Structures of type `PyMethodDef` with functions
                         // (or "methods") provided by the module.
};


PyMODINIT_FUNC PyInit_cFraglets(void)
{

    return PyModule_Create(&fragletsModule);
}