#ifndef DIGRAPHILLION_DIGRAPHILLION_H_
#define DIGRAPHILLION_DIGRAPHILLION_H_

#include <Python.h>

#include "digraphillion/setset.h"

typedef struct {
  PyObject_HEAD digraphillion::setset* ss;
} PySetsetObject;

PyAPI_DATA(PyTypeObject) PySetset_Type;

#define PySetset_Check(ob)          \
  (Py_TYPE(ob) == &PySetset_Type || \
   PyType_IsSubtype(Py_TYPE(ob), &PySetset_Type))

#endif
