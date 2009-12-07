/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation. Please
 * review the following information to ensure the GNU Lesser General
 * Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * As a special exception to the GNU Lesser General Public License
 * version 2.1, the object code form of a "work that uses the Library"
 * may incorporate material from a header file that is part of the
 * Library.  You may distribute such object code under terms of your
 * choice, provided that the incorporated material (i) does not exceed
 * more than 5% of the total size of the Library; and (ii) is limited to
 * numerical parameters, data structure layouts, accessors, macros,
 * inline functions and templates.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include <Python.h>
#include <limits>
#include "pyenum.h"
#include "basewrapper.h"
#include "bindingmanager.h"

// When the user adds a function with an argument unknown for the typesystem, the generator writes type checks as
// TYPENAME_Check, so this macro allows users to add PyObject arguments to their added functions.
#define PyObject_Check(X) true

namespace Shiboken
{
/**
*   This function template is used to get the PyObjectType of a C++ type T.
*   It's main usage if handle multiple inheritance casts.
*   \see SpecialCastFunction
*/
template<typename T>
inline PyTypeObject* PyType()
{
    assert(false); // This *SHOULD* never be called.
    return 0;
}

// Base Conversions ----------------------------------------------------------
template <typename T> struct Converter;

template <typename T>
struct ConverterBase
{
    static PyObject* createWrapper(const T* cppobj)
    {
        return Shiboken::PyBaseWrapper_New(PyType<T>(), cppobj);;
    }
    static T* copyCppObject(const T& cppobj) { return 0; }
    static bool isConvertible(PyObject* pyobj) { return pyobj == Py_None; }

    // Must be reimplemented.
    static PyObject* toPython(const T& cppobj);

    // Classes with implicit conversions are expected to reimplement
    // this to build T from its various implicit constructors.
    static T toCpp(PyObject* pyobj) { return *Converter<T*>::toCpp(pyobj); }
};

// Specialization meant to be used by abstract classes and object-types
// (i.e. classes with private copy constructors and = operators).
// Example: "struct Converter<AbstractClass* > : ConverterBase<AbstractClass* >"
template <typename T>
struct ConverterBase<T*> : ConverterBase<T>
{
    static PyObject* toPython(const T* cppobj)
    {
        if (!cppobj)
            Py_RETURN_NONE;
        PyObject* pyobj = BindingManager::instance().retrieveWrapper(cppobj);
        if (pyobj)
            Py_INCREF(pyobj);
        else
            pyobj = createWrapper(cppobj);
        return pyobj;
    }
    static T* toCpp(PyObject* pyobj)
    {
        if (pyobj == Py_None)
            return 0;
        ShiboTypeObject* shiboType = reinterpret_cast<ShiboTypeObject*>(pyobj->ob_type);
        if (shiboType->mi_specialcast)
            return (T*) shiboType->mi_specialcast(pyobj, reinterpret_cast<ShiboTypeObject*>(PyType<T>()));
        return (T*) ((Shiboken::PyBaseWrapper*) pyobj)->cptr;
    }
};

// Pointer Conversions
template <typename T> struct Converter : ConverterBase<T> {};
template <typename T>
struct Converter<T*> : Converter<T>
{
    static PyObject* toPython(const T* cppobj)
    {
        if (!cppobj)
            Py_RETURN_NONE;
        PyObject* pyobj = BindingManager::instance().retrieveWrapper(cppobj);
        if (pyobj)
            Py_INCREF(pyobj);
        else
            pyobj = createWrapper(cppobj);
        return pyobj;
    }
    static T* toCpp(PyObject* pyobj)
    {
        if (Shiboken_TypeCheck(pyobj, T))
            return (T*) ((Shiboken::PyBaseWrapper*) pyobj)->cptr;
        else if (Converter<T>::isConvertible(pyobj))
            return Converter<T>::copyCppObject(Converter<T>::toCpp(pyobj));
        return 0;
    }
};
template <typename T> struct Converter<const T*> : Converter<T*> {};

// PyObject* specialization to avoid converting what doesn't need to be converted.
template<>
struct Converter<PyObject*> : ConverterBase<PyObject*>
{
    inline static PyObject* toCpp(PyObject* pyobj) { return pyobj; }
};
template <> struct Converter<const PyObject*> : Converter<PyObject*> {};

// Reference Conversions
template <typename T>
struct Converter<T&> : Converter<T*>
{
    static PyObject* toPython(const T& cppobj)
    {
        return Converter<T*>::toPython(&cppobj);
    }
    static T& toCpp(PyObject* pyobj)
    {
        return *Converter<T*>::toCpp(pyobj);
    }
};
template <typename T> struct Converter<const T&> : Converter<T&> {};

// Primitive Conversions ------------------------------------------------------
template <>
struct Converter<bool>
{
    static bool isConvertible(PyObject* pyobj)
    {
        return PyInt_Check(pyobj);
    }
    static PyObject* toPython(bool cppobj)
    {
        return PyBool_FromLong(cppobj);
    }
    static bool toCpp(PyObject* pyobj)
    {
        return pyobj == Py_True;
    }
};

template <typename PyIntEquiv>
struct Converter_PyInt
{
    static PyObject* toPython(PyIntEquiv cppobj)
    {
        return PyInt_FromLong((long) cppobj);
    }
    static PyIntEquiv toCpp(PyObject* pyobj)
    {
        long result;
        if (PyFloat_Check(pyobj))
            result = (long) PyFloat_AS_DOUBLE(pyobj);
        else
            result = PyInt_AS_LONG(pyobj);
        if (result < std::numeric_limits<PyIntEquiv>::min() || result > std::numeric_limits<PyIntEquiv>::max())
            PyErr_SetObject(PyExc_OverflowError, 0);
        return (PyIntEquiv) result;
    }
};

template <> struct Converter<char> : Converter_PyInt<char> {};
template <> struct Converter<signed char> : Converter_PyInt<signed char> {};
template <> struct Converter<unsigned char> : Converter_PyInt<unsigned char> {};
template <> struct Converter<int> : Converter_PyInt<int> {};
template <> struct Converter<unsigned int> : Converter_PyInt<unsigned int> {};
template <> struct Converter<short> : Converter_PyInt<short> {};
template <> struct Converter<unsigned short> : Converter_PyInt<unsigned short> {};
template <> struct Converter<long> : Converter_PyInt<long> {};

template <>
struct Converter<unsigned long>
{
    static PyObject* toPython(unsigned long holder)
    {
        return PyLong_FromUnsignedLong(holder);
    }
    static unsigned long toCpp(PyObject* pyobj)
    {
        return PyLong_AsUnsignedLong(pyobj);
    }
};

template <>
struct Converter<PY_LONG_LONG>
{
    static PyObject* toPython(PY_LONG_LONG holder)
    {
        return PyLong_FromLongLong(holder);
    }
    static PY_LONG_LONG toCpp(PyObject* pyobj)
    {
        return (PY_LONG_LONG) PyLong_AsLongLong(pyobj);
    }
};

template <>
struct Converter<unsigned PY_LONG_LONG>
{
    static PyObject* toPython(unsigned PY_LONG_LONG holder)
    {
        return PyLong_FromUnsignedLongLong(holder);
    }
    static unsigned PY_LONG_LONG toCpp(PyObject* pyobj)
    {
        return (unsigned PY_LONG_LONG) PyLong_AsUnsignedLongLong(pyobj);
    }
};

template <typename PyFloatEquiv>
struct Converter_PyFloat
{
    static PyObject* toPython(PyFloatEquiv holder)
    {
        return PyFloat_FromDouble((double) holder);
    }
    static PyFloatEquiv toCpp(PyObject* pyobj)
    {
        if (PyInt_Check(pyobj))
            return (PyFloatEquiv) PyInt_AS_LONG(pyobj);
        return (PyFloatEquiv) PyFloat_AS_DOUBLE(pyobj);
    }
};

template <> struct Converter<float> : Converter_PyFloat<float> {};
template <> struct Converter<double> : Converter_PyFloat<double> {};

// PyEnum Conversions ---------------------------------------------------------
template <typename CppEnum>
struct Converter_CppEnum
{
    static PyObject* createWrapper(CppEnum cppobj)
    {
        return PyEnumObject_New(PyType<CppEnum>(), (long)cppobj);
    }
    static CppEnum toCpp(PyObject* pyobj)
    {
        return (CppEnum) ((Shiboken::PyEnumObject*)pyobj)->ob_ival;
    }
    static PyObject* toPython(CppEnum cppenum)
    {
        return Converter<CppEnum>::createWrapper(cppenum);
    }
};

// C Sting Types --------------------------------------------------------------
template <typename CString>
struct Converter_CString
{
    static PyObject* toPython(CString cppobj)
    {
        if (!cppobj)
            Py_RETURN_NONE;
        return PyString_FromString(cppobj);
    }
    static CString toCpp(PyObject* pyobj)
    {
        return PyString_AsString(pyobj);
    }
};

template <> struct Converter<char*> : Converter_CString<char*> {};
template <> struct Converter<const char*> : Converter_CString<const char*> {};

} // namespace Shiboken

#endif // CONVERSIONS_H

