#include "pydigraphillion.h"

#include <Python.h>
#include <assert.h>

#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "digraphillion/graphset.h"
#include "py3c.h"
#include "py3c/tpflags.h"
#include "subsetting/util/IntRange.hpp"

#define PyString_AsString PyUnicode_AsUTF8

#define CHECK_OR_ERROR(obj, check, name, ret)        \
  do {                                               \
    if (!check(obj)) {                               \
      PyErr_SetString(PyExc_TypeError, "not " name); \
      return (ret);                                  \
    }                                                \
  } while (0);

#define CHECK_SETSET_OR_ERROR(obj) \
  CHECK_OR_ERROR(obj, PySetset_Check, "setset", NULL);

#define RETURN_NEW_SETSET(self, expr)                         \
  do {                                                        \
    PySetsetObject* _ret = reinterpret_cast<PySetsetObject*>( \
        Py_TYPE(self)->tp_alloc(Py_TYPE(self), 0));           \
    _ret->ss = new digraphillion::setset(expr);               \
    return reinterpret_cast<PyObject*>(_ret);                 \
  } while (0);

#define RETURN_NEW_SETSET2(self, other, _other, expr)                   \
  do {                                                                  \
    PySetsetObject*(_other) = reinterpret_cast<PySetsetObject*>(other); \
    PySetsetObject* _ret = reinterpret_cast<PySetsetObject*>(           \
        Py_TYPE(self)->tp_alloc(Py_TYPE(self), 0));                     \
    if (_ret == NULL) return NULL;                                      \
    _ret->ss = new digraphillion::setset(expr);                         \
    return reinterpret_cast<PyObject*>(_ret);                           \
  } while (0);

#define RETURN_SELF_SETSET(self, other, _other, expr)                  \
  do {                                                                 \
    PySetsetObject* _other = reinterpret_cast<PySetsetObject*>(other); \
    (expr);                                                            \
    Py_INCREF(self);                                                   \
    return reinterpret_cast<PyObject*>(self);                          \
  } while (0);

#define RETURN_TRUE_IF(self, other, _other, expr)                       \
  do {                                                                  \
    PySetsetObject*(_other) = reinterpret_cast<PySetsetObject*>(other); \
    if (expr)                                                           \
      Py_RETURN_TRUE;                                                   \
    else                                                                \
      Py_RETURN_FALSE;                                                  \
  } while (0);

#define DO_FOR_MULTI(self, others, expr)                            \
  do {                                                              \
    PyObject* _result = reinterpret_cast<PyObject*>(self);          \
    if (PyTuple_GET_SIZE(others) == 0) return setset_copy(self);    \
    Py_INCREF(self);                                                \
    for (Py_ssize_t _i = 0; _i < PyTuple_GET_SIZE(others); ++_i) {  \
      PyObject* _other = PyTuple_GET_ITEM(others, _i);              \
      PyObject* _newresult =                                        \
          expr(reinterpret_cast<PySetsetObject*>(_result), _other); \
      if (_newresult == NULL) {                                     \
        Py_DECREF(_result);                                         \
        return NULL;                                                \
      }                                                             \
      Py_DECREF(_result);                                           \
      _result = _newresult;                                         \
    }                                                               \
    return _result;                                                 \
  } while (0);

#define UPDATE_FOR_MULTI(self, others, expr)                       \
  do {                                                             \
    for (Py_ssize_t _i = 0; _i < PyTuple_GET_SIZE(others); ++_i) { \
      PyObject* _other = PyTuple_GET_ITEM(others, _i);             \
      if (expr(self, _other) == NULL) return NULL;                 \
    }                                                              \
    Py_RETURN_NONE;                                                \
  } while (0);

#if IS_PY3 == 1

#define PyString_AsString PyUnicode_AsUTF8

int PyFile_Check(PyObject* obj) { return PyObject_AsFileDescriptor(obj); }

#endif

static PyObject* setset_build_set(const std::set<int>& s) {
  PyObject* so = PySet_New(NULL);
  for (std::set<int>::const_iterator e = s.begin(); e != s.end(); ++e) {
    PyObject* eo = PyInt_FromLong(*e);
    if (eo == NULL) {
      PyErr_SetString(PyExc_TypeError, "not int set");
      Py_DECREF(eo);
      return NULL;
    }
    if (PySet_Add(so, eo) == -1) {
      PyErr_SetString(PyExc_RuntimeError, "can't add elements to a set");
      Py_DECREF(eo);
      return NULL;
    }
    Py_DECREF(eo);  // TODO: is Py_DECREF() required for PyInt_FromLong object?
  }
  return so;
}

static int setset_parse_set(PyObject* so, std::set<int>* s) {
  assert(s != NULL);
  PyObject* i = PyObject_GetIter(so);
  if (i == NULL) return -1;
  PyObject* eo;
  while ((eo = PyIter_Next(i))) {
    if (!PyInt_Check(eo)) {
      Py_DECREF(eo);
      PyErr_SetString(PyExc_TypeError, "not int set");
      return -1;
    }
    s->insert(PyInt_AsLong(eo));
    Py_DECREF(eo);
  }
  Py_DECREF(i);
  return 0;
}

static std::vector<int> intersection(
    const std::map<std::string, std::vector<int> >& m, const std::string& key1,
    const std::string& key2) {
  std::map<std::string, std::vector<int> >::const_iterator in_i = m.find(key1);
  std::map<std::string, std::vector<int> >::const_iterator ex_i = m.find(key2);
  std::vector<int> in_v = in_i != m.end() ? in_i->second : std::vector<int>();
  std::vector<int> ex_v = ex_i != m.end() ? ex_i->second : std::vector<int>();
  std::sort(in_v.begin(), in_v.end());
  std::sort(ex_v.begin(), ex_v.end());
  std::vector<int> v(std::max(in_v.size(), ex_v.size()));
  std::vector<int>::const_iterator end = std::set_intersection(
      in_v.begin(), in_v.end(), ex_v.begin(), ex_v.end(), v.begin());
  v.resize(end - v.begin());
  return v;
}

static int setset_parse_map(PyObject* dict_obj,
                            std::map<std::string, std::vector<int> >* m) {
  assert(m != NULL);
  PyObject* key_obj;
  PyObject* lo;
  Py_ssize_t pos = 0;
  while (PyDict_Next(dict_obj, &pos, &key_obj, &lo)) {
    if (!PyStr_Check(key_obj)) {
      PyErr_SetString(PyExc_TypeError, "invalid argument");
      return -1;
    }
    std::string key = PyString_AsString(key_obj);
    if (key != "include" && key != "exclude") {
      PyErr_SetString(PyExc_TypeError, "invalid dict key");
      return -1;
    }
    PyObject* i = PyObject_GetIter(lo);
    if (i == NULL) return -1;
    std::vector<int> v;
    PyObject* eo;
    while ((eo = PyIter_Next(i))) {
      if (!PyInt_Check(eo)) {
        Py_DECREF(eo);
        PyErr_SetString(PyExc_TypeError, "not int");
        return -1;
      }
      v.push_back(PyInt_AsLong(eo));
      Py_DECREF(eo);
    }
    Py_DECREF(i);
    (*m)[key] = v;
  }
  if (!intersection(*m, "include", "exclude").empty()) {
    PyErr_SetString(PyExc_TypeError, "inconsistent constraints");
    return -1;
  }
  return 0;
}

// setset::iterator

typedef struct {
  PyObject_HEAD digraphillion::setset::iterator* it;
} PySetsetIterObject;

static PyObject* setsetiter_new(PyTypeObject* type, PyObject* args,
                                PyObject* kwds) {
  PySetsetIterObject* self;
  self = reinterpret_cast<PySetsetIterObject*>(type->tp_alloc(type, 0));
  if (self == NULL) return NULL;
  return reinterpret_cast<PyObject*>(self);
}

static void setsetiter_dealloc(PySetsetIterObject* self) {
  delete self->it;
  PyObject_Del(self);
}

static PyObject* setsetiter_next(PySetsetIterObject* self) {
  if (*(self->it) == digraphillion::setset::end()) return NULL;
  std::set<int> s = *(*self->it);
  ++(*self->it);
  return setset_build_set(s);
}

static PyMethodDef setsetiter_methods[] = {
    {NULL, NULL} /* sentinel */
};

static PyTypeObject PySetsetIter_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0) "setset_iterator", /* tp_name */
    sizeof(PySetsetIterObject),                               /* tp_basicsize */
    0,                                                        /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(setsetiter_dealloc), /* tp_dealloc */
    0,                                                /* tp_print */
    0,                                                /* tp_getattr */
    0,                                                /* tp_setattr */
    0,                       /* tp_compare or *tp_reserved */
    0,                       /* tp_repr */
    0,                       /* tp_as_number */
    0,                       /* tp_as_sequence */
    0,                       /* tp_as_mapping */
    0,                       /* tp_hash */
    0,                       /* tp_call */
    0,                       /* tp_str */
    PyObject_GenericGetAttr, /* tp_getattro */
    0,                       /* tp_setattro */
    0,                       /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_ITER,                        /* tp_flags */
    0,                                               /* tp_doc */
    0,                                               /* tp_traverse */
    0,                                               /* tp_clear */
    0,                                               /* tp_richcompare */
    0,                                               /* tp_weaklistoffset */
    PyObject_SelfIter,                               /* tp_iter */
    reinterpret_cast<iternextfunc>(setsetiter_next), /* tp_iternext */
    setsetiter_methods,                              /* tp_methods */
    0,                                               /* tp_members */
    0,                                               /* tp_getset */
    0,                                               /* tp_base */
    0,                                               /* tp_dict */
    0,                                               /* tp_descr_get */
    0,                                               /* tp_descr_set */
    0,                                               /* tp_dictoffset */
    0,                                               /* tp_init */
    PyType_GenericAlloc,                             /* tp_alloc */
    setsetiter_new,                                  /* tp_new */
#if IS_PY3 == 1
    0, /* tp_free */
    0, /* tp_is_gc */
    0, /* *tp_bases */
    0, /* *tp_mro */
    0, /* *tp_cache */
    0, /* *tp_subclasses */
    0, /* *tp_weaklist */
    0, /* tp_version_tag */
    0, /* tp_finalize */
#endif
};

// setset

static PyObject* setset_new(PyTypeObject* type, PyObject* args,
                            PyObject* kwds) {
  PySetsetObject* self;
  self = reinterpret_cast<PySetsetObject*>(type->tp_alloc(type, 0));
  if (self == NULL) return NULL;
  return reinterpret_cast<PyObject*>(self);
}

static int setset_init(PySetsetObject* self, PyObject* args, PyObject* kwds) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "|O", &obj)) return -1;
  if (obj == NULL || obj == Py_None) {
    self->ss = new digraphillion::setset();
  } else if (PySetset_Check(obj)) {
    PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(obj);
    self->ss = new digraphillion::setset(*(sso->ss));
  } else if (PyList_Check(obj)) {
    PyObject* i = PyObject_GetIter(obj);
    if (i == NULL) return -1;
    std::vector<std::set<int> > vs;
    PyObject* o;
    while ((o = PyIter_Next(i))) {
      if (!PyAnySet_Check(o)) {
        PyErr_SetString(PyExc_TypeError, "not set");
        return -1;
      }
      std::set<int> s;
      if (setset_parse_set(o, &s) == -1) return -1;
      vs.push_back(s);
      Py_DECREF(o);
    }
    Py_DECREF(i);
    self->ss = new digraphillion::setset(vs);
  } else if (PyDict_Check(obj)) {
    std::map<std::string, std::vector<int> > m;
    if (setset_parse_map(obj, &m) == -1) return -1;
    self->ss = new digraphillion::setset(m);
  } else {
    PyErr_SetString(PyExc_TypeError, "invalid argumet");
    return -1;
  }
  if (PyErr_Occurred()) return -1;
  return 0;
}

static void setset_dealloc(PySetsetObject* self) {
  delete self->ss;
  Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

static PyObject* setset_copy(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, *self->ss);
}

static PyObject* setset_invert(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, ~(*self->ss));
}

static PyObject* setset_union(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) | (*_other->ss));
}

static PyObject* setset_union_multi(PySetsetObject* self, PyObject* others) {
  DO_FOR_MULTI(self, others, setset_union);
}

static PyObject* setset_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) |= (*_other->ss));
}

static PyObject* setset_update_multi(PySetsetObject* self, PyObject* others) {
  UPDATE_FOR_MULTI(self, others, setset_update);
}

static PyObject* setset_intersection(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) & (*_other->ss));
}

static PyObject* setset_intersection_multi(PySetsetObject* self,
                                           PyObject* others) {
  DO_FOR_MULTI(self, others, setset_intersection);
}

static PyObject* setset_intersection_update(PySetsetObject* self,
                                            PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) &= (*_other->ss));
}

static PyObject* setset_intersection_update_multi(PySetsetObject* self,
                                                  PyObject* others) {
  UPDATE_FOR_MULTI(self, others, setset_intersection_update);
}

static PyObject* setset_difference(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) - (*_other->ss));
}

static PyObject* setset_difference_multi(PySetsetObject* self,
                                         PyObject* others) {
  DO_FOR_MULTI(self, others, setset_difference);
}

static PyObject* setset_difference_update(PySetsetObject* self,
                                          PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) -= (*_other->ss));
}

static PyObject* setset_difference_update_multi(PySetsetObject* self,
                                                PyObject* others) {
  UPDATE_FOR_MULTI(self, others, setset_difference_update);
}

static PyObject* setset_symmetric_difference(PySetsetObject* self,
                                             PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) ^ (*_other->ss));
}

static PyObject* setset_symmetric_difference_multi(PySetsetObject* self,
                                                   PyObject* others) {
  DO_FOR_MULTI(self, others, setset_symmetric_difference);
}

static PyObject* setset_symmetric_difference_update(PySetsetObject* self,
                                                    PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) ^= (*_other->ss));
}

static PyObject* setset_symmetric_difference_update_multi(PySetsetObject* self,
                                                          PyObject* others) {
  UPDATE_FOR_MULTI(self, others, setset_symmetric_difference_update);
}

static PyObject* setset_quotient(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) / (*_other->ss));
}

static PyObject* setset_quotient_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) /= (*_other->ss));
}

static PyObject* setset_remainder(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) % (*_other->ss));
}

static PyObject* setset_remainder_update(PySetsetObject* self,
                                         PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) %= (*_other->ss));
}

static PyObject* setset_isdisjoint(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_TRUE_IF(self, other, _other, self->ss->is_disjoint(*_other->ss));
}

static PyObject* setset_issubset(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_TRUE_IF(self, other, _other, self->ss->is_subset(*_other->ss));
}

static PyObject* setset_issuperset(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_TRUE_IF(self, other, _other, self->ss->is_superset(*_other->ss));
}

static int setset_nonzero(PySetsetObject* self) { return !self->ss->empty(); }

static Py_ssize_t setset_len(PyObject* obj) {
  PySetsetObject* self = reinterpret_cast<PySetsetObject*>(obj);
  long long int len = strtoll(self->ss->size().c_str(), NULL, 0);
  if (len != LLONG_MAX) {
    return len;
  } else {
    PyErr_SetString(PyExc_OverflowError, "overflow, use obj.len()");
    return -1;
  }
}

static PyObject* setset_len2(PySetsetObject* self, PyObject* args) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "|O", &obj)) return NULL;
  if (obj == NULL || obj == Py_None) {
    std::string size = self->ss->size();
    std::vector<char> buf;
    for (std::string::const_iterator c = size.begin(); c != size.end(); ++c)
      buf.push_back(*c);
    buf.push_back('\0');
    return PyLong_FromString(buf.data(), NULL, 0);
  } else if (PyInt_Check(obj)) {
    int len = PyLong_AsLong(obj);
    RETURN_NEW_SETSET(self, self->ss->set_size(len));
  } else {
    PyErr_SetString(PyExc_TypeError, "not int");
    return NULL;
  }
}

static PyObject* setset_iter(PySetsetObject* self) {
  PySetsetIterObject* ssi =
      PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) return NULL;
  ssi->it = new digraphillion::setset::iterator(self->ss->begin());
  if (ssi->it == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

static PyObject* setset_rand_iter(PySetsetObject* self) {
  PySetsetIterObject* ssi =
      PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) return NULL;
  ssi->it =
      new digraphillion::setset::random_iterator(self->ss->begin_randomly());
  if (ssi->it == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

static PyObject* setset_optimize(PySetsetObject* self, PyObject* weights,
                                 bool is_maximizing) {
  PyObject* i = PyObject_GetIter(weights);
  if (i == NULL) return NULL;
  PyObject* eo;
  std::vector<double> w;
  while ((eo = PyIter_Next(i))) {
    if (PyFloat_Check(eo)) {
      w.push_back(PyFloat_AsDouble(eo));
    } else if (PyLong_Check(eo)) {
      w.push_back(static_cast<double>(PyLong_AsLong(eo)));
    } else if (PyInt_Check(eo)) {
      w.push_back(static_cast<double>(PyInt_AsLong(eo)));
    } else {
      PyErr_SetString(PyExc_TypeError, "not a number");
      Py_DECREF(eo);
      return NULL;
    }
    Py_DECREF(eo);
  }
  Py_DECREF(i);
  PySetsetIterObject* ssi =
      PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) return NULL;
  ssi->it = new digraphillion::setset::weighted_iterator(
      is_maximizing ? self->ss->begin_from_max(w)
                    : self->ss->begin_from_min(w));
  if (ssi->it == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

static PyObject* setset_max_iter(PySetsetObject* self, PyObject* weights) {
  return setset_optimize(self, weights, true);
}

static PyObject* setset_min_iter(PySetsetObject* self, PyObject* weights) {
  return setset_optimize(self, weights, false);
}

// If an item in o is equal to value, return 1, otherwise return 0. On error,
// return -1.
static int setset_contains(PySetsetObject* self, PyObject* obj) {
  if (PyAnySet_Check(obj)) {
    std::set<int> s;
    if (setset_parse_set(obj, &s) == -1) return -1;
    return self->ss->find(s) != self->ss->end() ? 1 : 0;
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    return self->ss->supersets(e) != digraphillion::setset() ? 1 : 0;
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return -1;
  }
}

static PyObject* setset_add(PySetsetObject* self, PyObject* obj) {
  if (PyAnySet_Check(obj)) {
    std::set<int> s;
    if (setset_parse_set(obj, &s) == -1) return NULL;
    self->ss->insert(s);
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    self->ss->insert(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_remove(PySetsetObject* self, PyObject* obj) {
  if (PyAnySet_Check(obj)) {
    std::set<int> s;
    if (setset_parse_set(obj, &s) == -1) return NULL;
    if (self->ss->erase(s) == 0) {
      PyErr_SetString(PyExc_KeyError, "not found");
      return NULL;
    }
    self->ss->erase(s);
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    if (self->ss->supersets(e).empty()) {
      PyErr_SetString(PyExc_KeyError, "not found");
      return NULL;
    }
    self->ss->erase(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_discard(PySetsetObject* self, PyObject* obj) {
  if (PyAnySet_Check(obj)) {
    std::set<int> s;
    if (setset_parse_set(obj, &s) == -1) return NULL;
    self->ss->erase(s);
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    self->ss->erase(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_pop(PySetsetObject* self) {
  digraphillion::setset::iterator i = self->ss->begin();
  if (i == self->ss->end()) {
    PyErr_SetString(PyExc_KeyError, "'pop' from an empty set");
    return NULL;
  }
  std::set<int> s = *i;
  self->ss->erase(s);
  return setset_build_set(s);
}

static PyObject* setset_clear(PySetsetObject* self) {
  self->ss->clear();
  Py_RETURN_NONE;
}

static PyObject* setset_flip(PySetsetObject* self, PyObject* args) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "|O", &obj)) return NULL;
  if (obj == NULL || obj == Py_None) {
    self->ss->flip();
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    self->ss->flip(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_minimal(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->minimal());
}

static PyObject* setset_maximal(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->maximal());
}

static PyObject* setset_hitting(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->hitting());
}

static PyObject* setset_smaller(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyInt_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_SETSET(self, self->ss->smaller(set_size));
}

static PyObject* setset_larger(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyInt_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_SETSET(self, self->ss->larger(set_size));
}

static PyObject* setset_set_size(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyInt_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_SETSET(self, self->ss->set_size(set_size));
}

static PyObject* setset_join(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->join(*_other->ss));
}

static PyObject* setset_meet(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->meet(*_other->ss));
}

static PyObject* setset_subsets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->subsets(*_other->ss));
}

static PyObject* setset_supersets(PySetsetObject* self, PyObject* obj) {
  if (PySetset_Check(obj)) {
    RETURN_NEW_SETSET2(self, obj, _obj, self->ss->supersets(*_obj->ss));
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    RETURN_NEW_SETSET(self, self->ss->supersets(e));
  } else {
    PyErr_SetString(PyExc_TypeError, "not setset nor int");
    return NULL;
  }
}

static PyObject* setset_non_subsets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->non_subsets(*_other->ss));
}

static PyObject* setset_non_supersets(PySetsetObject* self, PyObject* obj) {
  if (PySetset_Check(obj)) {
    RETURN_NEW_SETSET2(self, obj, _obj, self->ss->non_supersets(*_obj->ss));
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    RETURN_NEW_SETSET(self, self->ss->non_supersets(e));
  } else {
    PyErr_SetString(PyExc_TypeError, "not setset nor int");
    return NULL;
  }
}

static PyObject* setset_choice(PySetsetObject* self) {
  digraphillion::setset::iterator i = self->ss->begin();
  if (i == self->ss->end()) {
    PyErr_SetString(PyExc_KeyError, "'choice' from an empty set");
    return NULL;
  }
  std::set<int> s = *i;
  return setset_build_set(s);
}

static PyObject* setset_probability(PySetsetObject* self,
                                    PyObject* probabilities) {
  PyObject* i = PyObject_GetIter(probabilities);
  if (i == NULL) return NULL;
  PyObject* eo;
  std::vector<double> p;
  while ((eo = PyIter_Next(i))) {
    if (PyFloat_Check(eo)) {
      p.push_back(PyFloat_AsDouble(eo));
    } else if (PyLong_Check(eo)) {
      p.push_back(static_cast<double>(PyLong_AsLong(eo)));
    } else if (PyInt_Check(eo)) {
      p.push_back(static_cast<double>(PyInt_AsLong(eo)));
    } else {
      PyErr_SetString(PyExc_TypeError, "not a number");
      Py_DECREF(eo);
      return NULL;
    }
    Py_DECREF(eo);
  }
  Py_DECREF(i);
  return PyFloat_FromDouble(self->ss->probability(p));
}

static PyObject* setset_dump(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", NULL);
#if IS_PY3 == 1
  int fd = PyObject_AsFileDescriptor(obj);
  FILE* fp = fdopen(dup(fd), "w");
#else
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
#endif
  Py_BEGIN_ALLOW_THREADS;
  self->ss->dump(fp);
  Py_END_ALLOW_THREADS;
#if IS_PY3 == 1
  fclose(fp);
#else
  PyFile_DecUseCount(file);
#endif
  Py_RETURN_NONE;
}

static PyObject* setset_dumps(PySetsetObject* self) {
  std::stringstream sstr;
  self->ss->dump(sstr);
  return PyStr_FromString(sstr.str().c_str());
}

static PyObject* setset_load(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", NULL);
#if IS_PY3 == 1
  int fd = PyObject_AsFileDescriptor(obj);
  FILE* fp = fdopen(dup(fd), "r");
#else
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
#endif
  PySetsetObject* ret;
  Py_BEGIN_ALLOW_THREADS;
  ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new digraphillion::setset(digraphillion::setset::load(fp));
  Py_END_ALLOW_THREADS;
#if IS_PY3 == 1
  fclose(fp);
#else
  PyFile_DecUseCount(file);
#endif
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* setset_loads(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyStr_Check, "str", NULL);
  std::stringstream sstr(PyStr_AsString(obj));
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new digraphillion::setset(digraphillion::setset::load(sstr));
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* setset_enum(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", NULL);
#if IS_PY3 == 1
  int fd = PyObject_AsFileDescriptor(obj);
  FILE* fp = fdopen(fd, "r");
#else
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
#endif
  Py_BEGIN_ALLOW_THREADS;
  std::string name = Py_TYPE(self)->tp_name;
  self->ss->_enum(fp, std::make_pair((name + "([").c_str(), "])"),
                  std::make_pair("set([", "])"));
  Py_END_ALLOW_THREADS;
#if IS_PY3 == 0
  PyFile_DecUseCount(file);
#endif
  Py_RETURN_NONE;
}

static PyObject* setset_enums(PySetsetObject* self) {
  std::stringstream sstr;
  std::string name = Py_TYPE(self)->tp_name;
  self->ss->_enum(sstr, std::make_pair((name + "([").c_str(), "])"),
                  std::make_pair("set([", "])"));
  return PyStr_FromString(sstr.str().c_str());
}

static PyObject* setset_repr(PySetsetObject* self) {
  return PyStr_FromFormat("<%s object of %p>", Py_TYPE(self)->tp_name,
                          reinterpret_cast<void*>(self->ss->id()));
}
/*
static long setset_hash(PyObject* self) {
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(self);
  return sso->ss->id();
}
*/
#if IS_PY3 == 0
static int setset_nocmp(PyObject* self, PyObject* other) {
  PyErr_SetString(PyExc_TypeError, "cannot compare using cmp()");
  return -1;
}
#endif

static PyObject* setset_richcompare(PySetsetObject* self, PyObject* obj,
                                    int op) {
  PySetsetObject* sso;
  if (!PySetset_Check(obj)) {
    if (op == Py_EQ) Py_RETURN_FALSE;
    if (op == Py_NE) Py_RETURN_TRUE;
    PyErr_SetString(PyExc_TypeError, "can only compare to set of sets");
    return NULL;
  }
  sso = reinterpret_cast<PySetsetObject*>(obj);
  switch (op) {
    case Py_EQ:
      if (*self->ss == *sso->ss)
        Py_RETURN_TRUE;
      else
        Py_RETURN_FALSE;
    case Py_NE:
      if (*self->ss != *sso->ss)
        Py_RETURN_TRUE;
      else
        Py_RETURN_FALSE;
    case Py_LE:
      if (*self->ss <= *sso->ss)
        Py_RETURN_TRUE;
      else
        Py_RETURN_FALSE;
    case Py_GE:
      if (*self->ss >= *sso->ss)
        Py_RETURN_TRUE;
      else
        Py_RETURN_FALSE;
    case Py_LT:
      if (*self->ss < *sso->ss)
        Py_RETURN_TRUE;
      else
        Py_RETURN_FALSE;
    case Py_GT:
      if (*self->ss > *sso->ss)
        Py_RETURN_TRUE;
      else
        Py_RETURN_FALSE;
  }
  Py_INCREF(Py_NotImplemented);
  return Py_NotImplemented;
}

// TODO
// static PyMemberDef setset_members[] = {
//    {NULL} /* Sentinel */
//};

static PyMethodDef setset_methods[] = {
    {"copy", reinterpret_cast<PyCFunction>(setset_copy), METH_NOARGS, ""},
    {"invert", reinterpret_cast<PyCFunction>(setset_invert), METH_NOARGS, ""},
    {"union", reinterpret_cast<PyCFunction>(setset_union_multi), METH_VARARGS,
     ""},
    {"update", reinterpret_cast<PyCFunction>(setset_update_multi), METH_VARARGS,
     ""},
    {"intersection", reinterpret_cast<PyCFunction>(setset_intersection_multi),
     METH_VARARGS, ""},
    {"intersection_update",
     reinterpret_cast<PyCFunction>(setset_intersection_update_multi),
     METH_VARARGS, ""},
    {"difference", reinterpret_cast<PyCFunction>(setset_difference_multi),
     METH_VARARGS, ""},
    {"difference_update",
     reinterpret_cast<PyCFunction>(setset_difference_update_multi),
     METH_VARARGS, ""},
    {"symmetric_difference",
     reinterpret_cast<PyCFunction>(setset_symmetric_difference_multi),
     METH_VARARGS, ""},
    {"symmetric_difference_update",
     reinterpret_cast<PyCFunction>(setset_symmetric_difference_update_multi),
     METH_VARARGS, ""},
    {"quotient", reinterpret_cast<PyCFunction>(setset_quotient), METH_O, ""},
    {"quotient_update", reinterpret_cast<PyCFunction>(setset_quotient_update),
     METH_O, ""},
    {"remainder", reinterpret_cast<PyCFunction>(setset_remainder), METH_O, ""},
    {"remainder_update", reinterpret_cast<PyCFunction>(setset_remainder_update),
     METH_O, ""},
    {"isdisjoint", reinterpret_cast<PyCFunction>(setset_isdisjoint), METH_O,
     ""},
    {"issubset", reinterpret_cast<PyCFunction>(setset_issubset), METH_O, ""},
    {"issuperset", reinterpret_cast<PyCFunction>(setset_issuperset), METH_O,
     ""},
    {"len", reinterpret_cast<PyCFunction>(setset_len2), METH_VARARGS, ""},
    {"iter", reinterpret_cast<PyCFunction>(setset_iter), METH_NOARGS, ""},
    {"rand_iter", reinterpret_cast<PyCFunction>(setset_rand_iter), METH_NOARGS,
     ""},
    {"max_iter", reinterpret_cast<PyCFunction>(setset_max_iter), METH_O, ""},
    {"min_iter", reinterpret_cast<PyCFunction>(setset_min_iter), METH_O, ""},
    {"add", reinterpret_cast<PyCFunction>(setset_add), METH_O, ""},
    {"remove", reinterpret_cast<PyCFunction>(setset_remove), METH_O, ""},
    {"discard", reinterpret_cast<PyCFunction>(setset_discard), METH_O, ""},
    {"pop", reinterpret_cast<PyCFunction>(setset_pop), METH_NOARGS, ""},
    {"clear", reinterpret_cast<PyCFunction>(setset_clear), METH_NOARGS, ""},
    {"minimal", reinterpret_cast<PyCFunction>(setset_minimal), METH_NOARGS, ""},
    {"maximal", reinterpret_cast<PyCFunction>(setset_maximal), METH_NOARGS, ""},
    {"hitting", reinterpret_cast<PyCFunction>(setset_hitting), METH_NOARGS, ""},
    {"smaller", reinterpret_cast<PyCFunction>(setset_smaller), METH_O, ""},
    {"larger", reinterpret_cast<PyCFunction>(setset_larger), METH_O, ""},
    {"set_size", reinterpret_cast<PyCFunction>(setset_set_size), METH_O, ""},
    {"flip", reinterpret_cast<PyCFunction>(setset_flip), METH_VARARGS, ""},
    {"join", reinterpret_cast<PyCFunction>(setset_join), METH_O, ""},
    {"meet", reinterpret_cast<PyCFunction>(setset_meet), METH_O, ""},
    {"subsets", reinterpret_cast<PyCFunction>(setset_subsets), METH_O, ""},
    {"supersets", reinterpret_cast<PyCFunction>(setset_supersets), METH_O, ""},
    {"non_subsets", reinterpret_cast<PyCFunction>(setset_non_subsets), METH_O,
     ""},
    {"non_supersets", reinterpret_cast<PyCFunction>(setset_non_supersets),
     METH_O, ""},
    {"choice", reinterpret_cast<PyCFunction>(setset_choice), METH_NOARGS, ""},
    {"probability", reinterpret_cast<PyCFunction>(setset_probability), METH_O,
     ""},
    {"dump", reinterpret_cast<PyCFunction>(setset_dump), METH_O, ""},
    {"dumps", reinterpret_cast<PyCFunction>(setset_dumps), METH_NOARGS, ""},
    {"_enum", reinterpret_cast<PyCFunction>(setset_enum), METH_O, ""},
    {"_enums", reinterpret_cast<PyCFunction>(setset_enums), METH_NOARGS, ""},
    {NULL} /* Sentinel */
};

static PyNumberMethods setset_as_number = {
    0,                                               /*nb_add*/
    reinterpret_cast<binaryfunc>(setset_difference), /*nb_subtract*/
    0,                                               /*nb_multiply*/
#if IS_PY3 == 0
    reinterpret_cast<binaryfunc>(setset_quotient), /*nb_divide*/
#endif
    reinterpret_cast<binaryfunc>(setset_remainder),    /*nb_remainder*/
    0,                                                 /*nb_divmod*/
    0,                                                 /*nb_power*/
    0,                                                 /*nb_negative*/
    0,                                                 /*nb_positive*/
    0,                                                 /*nb_absolute*/
    reinterpret_cast<inquiry>(setset_nonzero),         /*nb_nonzero or nb_bool*/
    reinterpret_cast<unaryfunc>(setset_invert),        /*nb_invert*/
    0,                                                 /*nb_lshift*/
    0,                                                 /*nb_rshift*/
    reinterpret_cast<binaryfunc>(setset_intersection), /*nb_and*/
    reinterpret_cast<binaryfunc>(setset_symmetric_difference), /*nb_xor*/
    reinterpret_cast<binaryfunc>(setset_union),                /*nb_or*/
#if IS_PY3 == 0
    0 /*reinterpret_cast<coercion>(Py_TPFLAGS_CHECKTYPES)*/, /*nb_coerce*/
#endif
    0, /*nb_int*/
    0, /*nb_long or *nb_reserved*/
    0, /*nb_float*/
#if IS_PY3 == 0
    0, /*nb_oct*/
    0, /*nb_hex*/
#endif
    0, /*nb_inplace_add*/
    reinterpret_cast<binaryfunc>(
        setset_difference_update), /*nb_inplace_subtract*/
    0,                             /*nb_inplace_multiply*/
#if IS_PY3 == 0
    reinterpret_cast<binaryfunc>(setset_quotient_update), /*nb_inplace_divide*/
#endif
    reinterpret_cast<binaryfunc>(
        setset_remainder_update), /*nb_inplace_remainder*/
    0,                            /*nb_inplace_power*/
    0,                            /*nb_inplace_lshift*/
    0,                            /*nb_inplace_rshift*/
    reinterpret_cast<binaryfunc>(setset_intersection_update), /*nb_inplace_and*/
    reinterpret_cast<binaryfunc>(
        setset_symmetric_difference_update),     /*nb_inplace_xor*/
    reinterpret_cast<binaryfunc>(setset_update), /*nb_inplace_or*/
#if IS_PY3 == 1
    0,                                             /*nb_floor_divide*/
    reinterpret_cast<binaryfunc>(setset_quotient), /*nb_true_divide*/
    0,                                             /*nb_inplace_floor_divide*/
    reinterpret_cast<binaryfunc>(
        setset_quotient_update), /*nb_inplace_true_divide*/
    0                            /*nb_index*/
// for 3.5?
//  0, /*nb_matrix_multiply*/
//  0 /*nb_inplace_matrix_multiply*/
#endif
};

static PySequenceMethods setset_as_sequence = {
    setset_len,                                    /* sq_length */
    0,                                             /* sq_concat */
    0,                                             /* sq_repeat */
    0,                                             /* sq_item */
    0,                                             /* sq_slice */
    0,                                             /* sq_ass_item */
    0,                                             /* sq_ass_slice */
    reinterpret_cast<objobjproc>(setset_contains), /* sq_contains */
};

PyDoc_STRVAR(setset_doc,
             "Hidden class to implement digraphillion classes.\n\
\n\
A setset object stores a set of sets.  A set element must be a\n\
positive number.");

#ifdef WIN32
__declspec(dllexport)
#endif
    PyTypeObject PySetset_Type = {
        PyVarObject_HEAD_INIT(NULL, 0) "_digraphillion.setset", /*tp_name*/
        sizeof(PySetsetObject),                                 /*tp_basicsize*/
        0,                                                      /*tp_itemsize*/
        reinterpret_cast<destructor>(setset_dealloc),           /*tp_dealloc*/
        0,                                                      /*tp_print*/
        0,                                                      /*tp_getattr*/
        0,                                                      /*tp_setattr*/
#if IS_PY3 == 1
        0, /*tp_reserved at 3.4*/
#else
    setset_nocmp, /*tp_compare*/
#endif
        reinterpret_cast<reprfunc>(setset_repr), /*tp_repr*/
        &setset_as_number,                       /*tp_as_number*/
        &setset_as_sequence,                     /*tp_as_sequence*/
        0,                                       /*tp_as_mapping*/
        0,                                       /*tp_hash */
        0,                                       /*tp_call*/
        0,                                       /*tp_str*/
        0,                                       /*tp_getattro*/
        0,                                       /*tp_setattro*/
        0,                                       /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
            Py_TPFLAGS_CHECKTYPES,                         /*tp_flags*/
        setset_doc,                                        /* tp_doc */
        0,                                                 /* tp_traverse */
        0,                                                 /* tp_clear */
        reinterpret_cast<richcmpfunc>(setset_richcompare), /* tp_richcompare */
        0,              /* tp_weaklistoffset */
        0,              /* tp_iter */
        0,              /* tp_iternext */
        setset_methods, /* tp_methods */
        0,  // setset_members,                          /* tp_members */
        0,  /* tp_getset */
        0,  /* tp_base */
        0,  /* tp_dict */
        0,  /* tp_descr_get */
        0,  /* tp_descr_set */
        0,  /* tp_dictoffset */
        reinterpret_cast<initproc>(setset_init), /* tp_init */
        PyType_GenericAlloc,                     /* tp_alloc */
        setset_new,                              /* tp_new */
#ifdef IS_PY3
        0, /* tp_free */
        0, /* tp_is_gc */
        0, /* *tp_bases */
        0, /* *tp_mro */
        0, /* *tp_cache */
        0, /* *tp_subclasses */
        0, /* *tp_weaklist */
        0, /* tp_version_tag */
        0, /* tp_finalize */
#endif
};

static PyObject* setset_elem_limit(PyObject*) {
  return PyInt_FromLong(digraphillion::setset::elem_limit());
}

static PyObject* setset_num_elems(PyObject*, PyObject* args) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "|O", &obj)) return NULL;
  if (obj == NULL) {
    return PyInt_FromLong(digraphillion::setset::num_elems());
  } else {
    digraphillion::setset::num_elems(PyInt_AsLong(obj));
    Py_RETURN_NONE;
  }
}

bool input_graph(PyObject* graph_obj,
                 std::vector<std::pair<std::string, std::string> >& graph) {
  if (graph_obj == NULL || graph_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no graph");
    return false;
  }
  PyObject* i = PyObject_GetIter(graph_obj);
  if (i == NULL) return false;
  PyObject* eo;
  while ((eo = PyIter_Next(i))) {
    PyObject* j = PyObject_GetIter(eo);
    if (j == NULL) return false;
    std::vector<std::string> e;
    PyObject* vo;
    while ((vo = PyIter_Next(j))) {
      if (!PyBytes_Check(vo)) {
        PyErr_SetString(PyExc_TypeError, "invalid graph");
        return false;
      }
      std::string v = PyBytes_AsString(vo);
      if (v.find(',') != std::string::npos) {
        PyErr_SetString(PyExc_TypeError, "invalid vertex in the graph");
        return false;
      }
      e.push_back(v);
    }
    assert(e.size() == 2);
    graph.push_back(make_pair(e[0], e[1]));
  }
  return true;
}

bool input_string_list(PyObject* list_obj, std::vector<std::string>& list) {
  if (list_obj == NULL || list_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no input");
    return false;
  }

  PyObject* i = PyObject_GetIter(list_obj);
  if (i == NULL) return false;
  PyObject* vo;
  while ((vo = PyIter_Next(i))) {
    if (!PyBytes_Check(vo)) {
      PyErr_SetString(PyExc_TypeError, "invalid input");
      return false;
    }
    std::string v = PyBytes_AsString(vo);
    if (v.find(',') != std::string::npos) {
      PyErr_SetString(PyExc_TypeError, "invalid vertex in the graph");
      return false;
    }
    list.push_back(v);
  }
  return true;
}

bool input_vertex_to_range_map(
    PyObject* map_obj, std::map<std::string, tdzdd::Range>& mp) {
  PyObject* vo;
  PyObject* lo;
  Py_ssize_t pos = 0;
  while (PyDict_Next(map_obj, &pos, &vo, &lo)) {
    if (!PyBytes_Check(vo)) {
      PyErr_SetString(PyExc_TypeError, "invalid vertex in map object");
      return false;
    }
    std::string vertex = PyBytes_AsString(vo);
    PyObject* i = PyObject_GetIter(lo);
    if (i == NULL) return false;
    std::vector<int> r;
    PyObject* io;
    while ((io = PyIter_Next(i))) {
      if (!PyInt_Check(io)) {
        Py_DECREF(io);
        PyErr_SetString(PyExc_TypeError, "invalid degree in map object");
        return false;
      }
      r.push_back(PyInt_AsLong(io));
    }
    mp[vertex] = tdzdd::Range(r[0], r[1], r[2]);
  }
  return true;
}

static PyObject* graphset_directed_cycles(PyObject*, PyObject* args,
                                          PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "search_space";
  static char* kwlist[3] = {s1, s2, NULL};
  PyObject* graph_obj = NULL;
  PyObject* search_space_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &graph_obj,
                                   &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  digraphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None)
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;

  digraphillion::setset ss =
      digraphillion::SearchDirectedCycles(graph, search_space);

  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new digraphillion::setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* graphset_directed_hamiltonian_cycles(PyObject*, PyObject* args,
                                                      PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "search_space";
  static char* kwlist[3] = {s1, s2, NULL};
  PyObject* graph_obj = NULL;
  PyObject* search_space_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &graph_obj,
                                   &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  digraphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None)
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;

  digraphillion::setset ss =
      digraphillion::SearchDirectedHamiltonianCycles(graph, search_space);

  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new digraphillion::setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* graphset_directed_st_path(PyObject*, PyObject* args,
                                           PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "s";
  static char s3[] = "t";
  static char s4[] = "is_hamiltonian";
  static char s5[] = "search_space";
  static char* kwlist[] = {s1, s2, s3, s4, s5, NULL};
  PyObject* graph_obj = NULL;
  int is_hamiltonian = false;
  PyObject* s_obj = NULL;
  PyObject* t_obj = NULL;
  PyObject* search_space_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "OSSp|O", kwlist, &graph_obj,
                                   &s_obj, &t_obj, &is_hamiltonian,
                                   &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  std::string s, t;
  if (s_obj == NULL || s_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no vertex s");
    return NULL;
  }
  if (!PyBytes_Check(s_obj)) {
    PyErr_SetString(PyExc_TypeError, "invalid vertex s");
    return NULL;
  }
  s = PyBytes_AsString(s_obj);

  if (t_obj == NULL || t_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no vertex t");
    return NULL;
  }
  if (!PyBytes_Check(t_obj)) {
    PyErr_SetString(PyExc_TypeError, "invalid vertex t");
    return NULL;
  }
  t = PyBytes_AsString(t_obj);

  digraphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None)
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;

  digraphillion::setset ss = digraphillion::SearchDirectedSTPath(
      graph, is_hamiltonian, s, t, search_space);

  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new digraphillion::setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* graphset_rooted_forests(PyObject*, PyObject* args,
                                         PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "roots";
  static char s3[] = "is_spanning";
  static char s4[] = "search_space";
  static char* kwlist[] = {s1, s2, s3, s4, NULL};
  PyObject* graph_obj = NULL;
  PyObject* roots_obj = NULL;
  PyObject* search_space_obj = NULL;
  int is_spanning;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OpO", kwlist, &graph_obj,
                                   &roots_obj, &is_spanning, &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  std::vector<std::string> roots;
  if (roots_obj != NULL && roots_obj != Py_None) {
    if (!input_string_list(roots_obj, roots)) {
      return NULL;
    }
  }

  digraphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None)
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;

  digraphillion::setset ss = digraphillion::SearchDirectedForests(
      graph, roots, is_spanning, search_space);

  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new digraphillion::setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* graphset_rooted_trees(PyObject*, PyObject* args,
                                       PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "root";
  static char s3[] = "is_spanning";
  static char s4[] = "search_space";
  static char* kwlist[] = {s1, s2, s3, s4, NULL};
  PyObject* graph_obj = NULL;
  PyObject* search_space_obj = NULL;
  PyObject* root_obj = NULL;
  int is_spanning = false;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "OSp|O", kwlist, &graph_obj,
                                   &root_obj, &is_spanning, &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  std::string root;
  if (root_obj == NULL || root_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no vertex root");
    return NULL;
  }
  if (!PyBytes_Check(root_obj)) {
    PyErr_SetString(PyExc_TypeError, "invalid vertex root");
    return NULL;
  }
  root = PyBytes_AsString(root_obj);

  digraphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None)
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;

  digraphillion::setset ss =
      digraphillion::SearchRootedTrees(graph, root, is_spanning, search_space);

  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new digraphillion::setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* graphset_directed_graphs(PyObject*, PyObject* args,
                                          PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "in_degree_constraints";
  static char s3[] = "out_degree_constraints";
  static char s4[] = "search_space";
  static char* kwlist[] = {s1, s2, s3, s4, NULL};
  PyObject* graph_obj = NULL;
  PyObject* in_degree_constraints_obj = NULL;
  PyObject* out_degree_constraints_obj = NULL;
  PyObject* search_space_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(
          args, kwds, "O|OOO", kwlist, &graph_obj, &in_degree_constraints_obj,
          &out_degree_constraints_obj, &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  std::map<std::string, tdzdd::Range> in_degree_constraints_entity;
  std::map<std::string, tdzdd::Range>* in_degree_constraints = NULL;
  if (in_degree_constraints_obj != NULL &&
      in_degree_constraints_obj != Py_None) {
    in_degree_constraints = &in_degree_constraints_entity;
    if (!input_vertex_to_range_map(in_degree_constraints_obj,
                                   in_degree_constraints_entity)) {
      return NULL;
    }
  }

  std::map<std::string, tdzdd::Range> out_degree_constrains_entity;
  std::map<std::string, tdzdd::Range>* out_degree_constrains = NULL;
  if (out_degree_constraints_obj != NULL &&
      out_degree_constraints_obj != Py_None) {
    out_degree_constrains = &out_degree_constrains_entity;
    if (!input_vertex_to_range_map(out_degree_constraints_obj,
                                   out_degree_constrains_entity)) {
      return NULL;
    }
  }

  digraphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None)
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;

  digraphillion::setset ss = digraphillion::SearchDirectedGraphs(
      graph, in_degree_constraints, out_degree_constrains, search_space);

  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new digraphillion::setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* graphset_show_messages(PySetsetObject* self, PyObject* obj) {
  int ret = digraphillion::ShowMessages(PyObject_IsTrue(obj));
  if (ret)
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

static PyMethodDef module_methods[] = {
    {"load", reinterpret_cast<PyCFunction>(setset_load), METH_O, ""},
    {"loads", reinterpret_cast<PyCFunction>(setset_loads), METH_O, ""},
    {"_elem_limit", reinterpret_cast<PyCFunction>(setset_elem_limit),
     METH_NOARGS, ""},
    {"_num_elems", setset_num_elems, METH_VARARGS, ""},
    {"_directed_cycles",
     reinterpret_cast<PyCFunction>(graphset_directed_cycles),
     METH_VARARGS | METH_KEYWORDS, ""},
    {"_directed_hamiltonian_cycles",
     reinterpret_cast<PyCFunction>(graphset_directed_hamiltonian_cycles),
     METH_VARARGS | METH_KEYWORDS, ""},
    {"_directed_st_path",
     reinterpret_cast<PyCFunction>(graphset_directed_st_path),
     METH_VARARGS | METH_KEYWORDS, ""},
    {"_rooted_forests", reinterpret_cast<PyCFunction>(graphset_rooted_forests),
     METH_VARARGS | METH_KEYWORDS, ""},
    {"_rooted_trees", reinterpret_cast<PyCFunction>(graphset_rooted_trees),
     METH_VARARGS | METH_KEYWORDS, ""},
    {"_directed_graphs",
     reinterpret_cast<PyCFunction>(graphset_directed_graphs),
     METH_VARARGS | METH_KEYWORDS, ""},
    {"_show_messages", reinterpret_cast<PyCFunction>(graphset_show_messages),
     METH_O, ""},
    {NULL} /* Sentinel */
};

PyDoc_STRVAR(digraphillion_doc,
             "Hidden module to implement digraphillion classes.");

#if IS_PY3 == 1
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "_digraphillion",  /* m_name */
    digraphillion_doc, /* m_doc */
    -1,                /* m_size */
    module_methods,    /* m_methods */
    NULL,              /* m_reload */
    NULL,              /* m_traverse */
    NULL,              /* m_clear */
    NULL,              /* m_free */
};
#endif
MODULE_INIT_FUNC(_digraphillion) {
  PyObject* m;
  if (PyType_Ready(&PySetset_Type) < 0) return NULL;
  if (PyType_Ready(&PySetsetIter_Type) < 0) return NULL;
#if IS_PY3 == 1
  m = PyModule_Create(&moduledef);
#else
  m = Py_InitModule3("_digraphillion", module_methods, digraphillion_doc);
#endif
  if (m == NULL) return NULL;
  Py_INCREF(&PySetset_Type);
  Py_INCREF(&PySetsetIter_Type);
  PyModule_AddObject(m, "setset", reinterpret_cast<PyObject*>(&PySetset_Type));
  PyModule_AddObject(m, "setset_iterator",
                     reinterpret_cast<PyObject*>(&PySetsetIter_Type));
  return m;
}
