/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef BASEWRAPPER_H
#define BASEWRAPPER_H

#include "sbkpython.h"
#include "shibokenmacros.h"

#include <vector>
#include <string>

extern "C"
{

struct SbkConverter;
struct SbkObjectPrivate;

/// Base Python object for all the wrapped C++ classes.
struct LIBSHIBOKEN_API SbkObject
{
    PyObject_HEAD
    /// Instance dictionary.
    PyObject* ob_dict;
    /// List of weak references
    PyObject* weakreflist;
    SbkObjectPrivate* d;
};


/// Dealloc the python object \p pyObj and the C++ object represented by it.
LIBSHIBOKEN_API void SbkDeallocWrapper(PyObject* pyObj);
LIBSHIBOKEN_API void SbkDeallocQAppWrapper(PyObject* pyObj);
LIBSHIBOKEN_API void SbkDeallocWrapperWithPrivateDtor(PyObject* self);

struct SbkObjectType;

/// Function signature for the multiple inheritance information initializers that should be provided by classes with multiple inheritance.
typedef int* (*MultipleInheritanceInitFunction)(const void*);

/**
 *   Special cast function is used to correctly cast an object when it's
 *   part of a multiple inheritance hierarchy.
 *   The implementation of this function is auto generated by the generator and you don't need to care about it.
 */
typedef void* (*SpecialCastFunction)(void*, SbkObjectType*);
typedef SbkObjectType* (*TypeDiscoveryFunc)(void*, SbkObjectType*);
typedef void* (*TypeDiscoveryFuncV2)(void*, SbkObjectType*);

typedef void* (*ExtendedToCppFunc)(PyObject*);        // DEPRECATED.
typedef bool (*ExtendedIsConvertibleFunc)(PyObject*); // DEPRECATED.

// Used in userdata dealloc function
typedef void (*DeleteUserDataFunc)(void*);

typedef void (*ObjectDestructor)(void*);

typedef void (*SubTypeInitHook)(SbkObjectType*, PyObject*, PyObject*);

extern LIBSHIBOKEN_API PyTypeObject *SbkObjectType_TypeF(void);
extern LIBSHIBOKEN_API SbkObjectType *SbkObject_TypeF(void);


struct SbkObjectTypePrivate;
/// PyTypeObject extended with C++ multiple inheritance information.
struct LIBSHIBOKEN_API SbkObjectType
{
    PyTypeObject type;
};

LIBSHIBOKEN_API PyObject* SbkObjectTpNew(PyTypeObject* subtype, PyObject*, PyObject*);
// the special case of a switchable singleton
LIBSHIBOKEN_API PyObject* SbkQAppTpNew(PyTypeObject *subtype, PyObject *args, PyObject *kwds);

/**
 *  PYSIDE-595: Use a null deallocator instead of nullptr.
 *
 *  When moving to heaptypes, we were struck by a special default behavior of
 *  PyType_FromSpecWithBases that inserts subtype_dealloc when tp_dealloc is
 *  nullptr. To prevent inserting this, we use a null deallocator that is there
 *  as a placeholder.
 *
 *  The same holds for a null tp_new. We use one that raises the right error.
 */
LIBSHIBOKEN_API void SbkDummyDealloc(PyObject*);
LIBSHIBOKEN_API PyObject *SbkDummyNew(PyTypeObject *type, PyObject*, PyObject*);

} // extern "C"

namespace Shiboken
{

/**
*   Init shiboken library.
*/
LIBSHIBOKEN_API void init();


/// Delete the class T allocated on \p cptr.
template<typename T>
void callCppDestructor(void* cptr)
{
    delete reinterpret_cast<T*>(cptr);
}

/**
 *  Shiboken::importModule is DEPRECATED. Use Shiboken::Module::import() instead.
 */
SBK_DEPRECATED(LIBSHIBOKEN_API bool importModule(const char* moduleName, PyTypeObject*** cppApiPtr));
LIBSHIBOKEN_API void        setErrorAboutWrongArguments(PyObject* args, const char* funcName, const char** cppOverloads);

namespace ObjectType {

/**
*   Returns true if the object is an instance of a type created by the Shiboken generator.
*/
LIBSHIBOKEN_API bool        checkType(PyTypeObject* pyObj);

/**
*   Returns true if this object is an instance of an user defined type derived from an Shiboken type.
*/
LIBSHIBOKEN_API bool        isUserType(PyTypeObject* pyObj);

/**
*   Returns true if the constructor of \p ctorType can be called for a instance of type \p myType.
*   \note This function set a python error when returning false.
*/
LIBSHIBOKEN_API bool        canCallConstructor(PyTypeObject* myType, PyTypeObject* ctorType);

/**
 *  Tells if the \p type represents an object of a class with multiple inheritance in C++.
 *  When this occurs, the C++ pointer held by the Python wrapper will need to be cast when
 *  passed as a parameter that expects a type of its ancestry.
 *  \returns    true if a call to ObjectType::cast() is needed to obtain the correct
 *              C++ pointer for Python objects of type \p type.
 */
LIBSHIBOKEN_API bool        hasCast(SbkObjectType* type);
/**
 *  Cast the C++ pointer held by a Python object \p obj of type \p sourceType,
 *  to a C++ pointer of a C++ class indicated by type \p targetType.
 *  \returns    The cast C++ pointer.
 */
LIBSHIBOKEN_API void*       cast(SbkObjectType* sourceType, SbkObject* obj, PyTypeObject* targetType);
/// Set the C++ cast function for \p type.
LIBSHIBOKEN_API void        setCastFunction(SbkObjectType* type, SpecialCastFunction func);

LIBSHIBOKEN_API void        setOriginalName(SbkObjectType* self, const char* name);
LIBSHIBOKEN_API const char* getOriginalName(SbkObjectType* self);

LIBSHIBOKEN_API void setTypeDiscoveryFunctionV2(SbkObjectType* self, TypeDiscoveryFuncV2 func);
LIBSHIBOKEN_API void        copyMultimpleheritance(SbkObjectType* self, SbkObjectType* other);
LIBSHIBOKEN_API void        setMultipleInheritanceFunction(SbkObjectType* self, MultipleInheritanceInitFunction func);
LIBSHIBOKEN_API MultipleInheritanceInitFunction getMultipleIheritanceFunction(SbkObjectType* self);

LIBSHIBOKEN_API void        setDestructorFunction(SbkObjectType* self, ObjectDestructor func);

LIBSHIBOKEN_API void        initPrivateData(SbkObjectType* self);

/**
 *  Initializes a Shiboken wrapper type and adds it to the module,
 *  or to the enclosing class if the type is an inner class.
 *  This function also calls initPrivateData and setDestructorFunction.
 *  \param enclosingObject  The module or enclosing class to where the new \p type will be added.
 *  \param typeName         Name by which the type will be known in Python.
 *  \param originalName     Original C++ name of the type.
 *  \param type             The new type to be initialized and added to the module.
 *  \param cppObjDtor       Memory deallocation function for the C++ object held by \p type.
 *                          Should not be used if the underlying C++ class has a private destructor.
 *  \param baseType         Base type from whom the new \p type inherits.
 *  \param baseTypes        Other base types from whom the new \p type inherits.
 *  \param isInnerClass     Tells if the new \p type is an inner class (the default is that it isn't).
 *                          If false then the \p enclosingObject is a module, otherwise it is another
 *                          wrapper type.
 *  \returns                true if the initialization went fine, false otherwise.
 */
LIBSHIBOKEN_API SbkObjectType *introduceWrapperType(PyObject *enclosingObject,
                                                    const char *typeName,
                                                    const char *originalName,
                                                    PyType_Spec *typeSpec,
                                                    const char *signaturesString,
                                                    ObjectDestructor cppObjDtor,
                                                    SbkObjectType *baseType,
                                                    PyObject *baseTypes,
                                                    bool isInnerClass);

/**
 *  Set the subtype init hook for a type.
 *
 *  This hook will be invoked every time the user creates a sub-type inherited from a Shiboken based type.
 *  The hook gets 3 params, they are: The new type being created, args and kwds. The last two are the very
 *  same got from tp_new.
 */
LIBSHIBOKEN_API void        setSubTypeInitHook(SbkObjectType* self, SubTypeInitHook func);

/**
 *  Get the user data previously set by Shiboken::Object::setTypeUserData
 */
LIBSHIBOKEN_API void*       getTypeUserData(SbkObjectType* self);
LIBSHIBOKEN_API void        setTypeUserData(SbkObjectType* self, void* userData, DeleteUserDataFunc d_func);

}

namespace Object {

/**
 *  Returns a string with information about the internal state of the instance object, useful for debug purposes.
 */
LIBSHIBOKEN_API std::string info(SbkObject* self);

/**
*   Returns true if the object is an instance of a type created by the Shiboken generator.
*/
LIBSHIBOKEN_API bool        checkType(PyObject* pyObj);

/**
 *  Returns true if this object type is an instance of an user defined type derived from an Shiboken type.
 *  \see Shiboken::ObjectType::isUserType
 */
LIBSHIBOKEN_API bool        isUserType(PyObject* pyObj);

/**
 *  Generic function used to make ObjectType hashable, the C++ pointer is used as hash value.
 */
LIBSHIBOKEN_API Py_hash_t   hash(PyObject* pyObj);

/**
 * Find a child of given wrapper having same address having the specified type.
 */
LIBSHIBOKEN_API SbkObject *findColocatedChild(SbkObject *wrapper,
                                              const SbkObjectType *instanceType);

/**
 *  Bind a C++ object to Python.
 * \param instanceType equivalent Python type for the C++ object.
 * \param hasOwnership if true, Python will try to delete the underlying C++ object when there's no more refs.
 * \param isExactType if false, Shiboken will use some heuristics to detect the correct Python type of this C++
 *                    object, in any case you must provide \p instanceType, it'll be used as search starting point
 *                    and as fallback.
 * \param typeName    If non-null, this will be used as helper to find the correct Python type for this object.
 */
LIBSHIBOKEN_API PyObject*   newObject(SbkObjectType* instanceType,
                                      void* cptr,
                                      bool hasOwnership = true,
                                      bool isExactType = false,
                                      const char* typeName = 0);

/**
 *  Changes the valid flag of a PyObject, invalid objects will raise an exception when someone tries to access it.
 */
LIBSHIBOKEN_API void        setValidCpp(SbkObject* pyObj, bool value);
/**
 *  Tells shiboken the Python object \p pyObj has a C++ wrapper used to intercept virtual method calls.
 */
LIBSHIBOKEN_API void        setHasCppWrapper(SbkObject* pyObj, bool value);
/**
 *  Return true if the Python object \p pyObj has a C++ wrapper used to intercept virtual method calls.
 */
LIBSHIBOKEN_API bool        hasCppWrapper(SbkObject* pyObj);

/**
 *  Return true if the Python object was created by Python, false otherwise.
 *  \note This function was added to libshiboken only to be used by shiboken.wasCreatedByPython()
 */
LIBSHIBOKEN_API bool        wasCreatedByPython(SbkObject* pyObj);

/**
 *  Call the C++ object destructor and invalidates the Python object.
 *  \note This function was added to libshiboken only to be used by shiboken.delete()
 */
LIBSHIBOKEN_API void        callCppDestructors(SbkObject* pyObj);

/**
 *  Return true if the Python is responsible for deleting the underlying C++ object.
 */
LIBSHIBOKEN_API bool        hasOwnership(SbkObject* pyObj);

/**
 *  Sets python as responsible to delete the underlying C++ object.
 *  \note You this overload only when the PyObject can be a sequence and you want to
 *  call this function for every item in the sequence.
 *  \see getOwnership(SbkObject*)
 */
LIBSHIBOKEN_API void        getOwnership(PyObject* pyObj);

/**
 *  Sets python as responsible to delete the underlying C++ object.
 */
LIBSHIBOKEN_API void        getOwnership(SbkObject* pyObj);

/**
 *  Release the ownership, so Python will not delete the underlying C++ object.
 *  \note You this overload only when the PyObject can be a sequence and you want to
 *  call this function for every item in the sequence.
 *  \see releaseOwnership(SbkObject*)
 */
LIBSHIBOKEN_API void        releaseOwnership(PyObject* pyObj);
/**
 *  Release the ownership, so Python will not delete the underlying C++ object.
 */
LIBSHIBOKEN_API void        releaseOwnership(SbkObject* pyObj);

/**
 *   Get the C++ pointer of type \p desiredType from a Python object.
 */
LIBSHIBOKEN_API void*       cppPointer(SbkObject* pyObj, PyTypeObject* desiredType);

/**
 *   Return a list with all C++ pointers held from a Python object.
 *   \note This function was added to libshiboken only to be used by shiboken.getCppPointer()
 */
LIBSHIBOKEN_API std::vector<void*> cppPointers(SbkObject* pyObj);

/**
 *   Set the C++ pointer of type \p desiredType of a Python object.
 */
LIBSHIBOKEN_API bool        setCppPointer(SbkObject* sbkObj, PyTypeObject* desiredType, void* cptr);

/**
 * Returns false and sets a Python RuntimeError if the Python wrapper is not marked as valid.
 */
LIBSHIBOKEN_API bool        isValid(PyObject* pyObj);

/**
 * Returns false if the Python wrapper is not marked as valid.
 * \param pyObj the object.
 * \param throwPyError sets a Python RuntimeError when the object isn't valid.
 */
LIBSHIBOKEN_API bool        isValid(SbkObject* pyObj, bool throwPyError = true);

/**
 * Returns false if the Python wrapper is not marked as valid.
 * \param pyObj the object.
 * \param throwPyError sets a Python RuntimeError when the object isn't valid.
 */
LIBSHIBOKEN_API bool        isValid(PyObject* pyObj, bool throwPyError);

/**
*   Set the parent of \p child to \p parent.
*   When an object dies, all their children, grandchildren, etc, are tagged as invalid.
*   \param parent the parent object, if null, the child will have no parents.
*   \param child the child.
*/
LIBSHIBOKEN_API void        setParent(PyObject* parent, PyObject* child);

/**
*   Remove this child from their parent, if any.
*   \param child the child.
*/
LIBSHIBOKEN_API void        removeParent(SbkObject* child, bool giveOwnershipBack = true, bool keepReferenc = false);

/**
 * Mark the object as invalid
 */
LIBSHIBOKEN_API void        invalidate(SbkObject* self);

/**
 * Help function can be used to invalidate a sequence of object
 **/
LIBSHIBOKEN_API void        invalidate(PyObject* pyobj);

/**
 * Make the object valid again
 */
LIBSHIBOKEN_API void        makeValid(SbkObject* self);

/// \deprecated Use destroy(SbkObject*, void*)
SBK_DEPRECATED(LIBSHIBOKEN_API void destroy(SbkObject* self));

/**
 * Destroy any data in Shiboken structure and c++ pointer if the pyboject has the ownership
 */
LIBSHIBOKEN_API void        destroy(SbkObject* self, void* cppData);

/**
 *  Set user data on type of \p wrapper.
 *  \param wrapper instance object, the user data will be set on his type
 *  \param userData the user data
 *  \param d_func a function used to delete the user data
 */
LIBSHIBOKEN_API void        setTypeUserData(SbkObject* wrapper, void* userData, DeleteUserDataFunc d_func);
/**
 *  Get the user data previously set by Shiboken::Object::setTypeUserData
 */
LIBSHIBOKEN_API void*       getTypeUserData(SbkObject* wrapper);

/**
 *   Increments the reference count of the referred Python object.
 *   A previous Python object in the same position identified by the 'key' parameter
 *   will have its reference counter decremented automatically when replaced.
 *   All the kept references should be decremented when the Python wrapper indicated by
 *   'self' dies.
 *   No checking is done for any of the passed arguments, since it is meant to be used
 *   by generated code it is supposed that the generator is correct.
 *   \param self            the wrapper instance that keeps references to other objects.
 *   \param key             a key that identifies the C++ method signature and argument where the referred Object came from.
 *   \param referredObject  the object whose reference is used by the self object.
 */
LIBSHIBOKEN_API void        keepReference(SbkObject* self, const char* key, PyObject* referredObject, bool append = false);

/**
 *   Removes any reference previously added by keepReference function
 *   \param self            the wrapper instance that keeps references to other objects.
 *   \param key             a key that identifies the C++ method signature and argument from where the referred Object came.
 *   \param referredObject  the object whose reference is used by the self object.
 */
LIBSHIBOKEN_API void        removeReference(SbkObject* self, const char* key, PyObject* referredObject);

} // namespace Object

} // namespace Shiboken

#endif // BASEWRAPPER_H
