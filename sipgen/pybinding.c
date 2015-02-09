/*
 * The transitional Python bindings for the C parts of the sip5 code generator.
 *
 * Copyright (c) 2015 Riverbank Computing Limited <info@riverbankcomputing.com>
 *
 * This file is part of SIP.
 *
 * This copy of SIP is licensed for use under the terms of the SIP License
 * Agreement.  See the file LICENSE for more details.
 *
 * This copy of SIP may also used under the terms of the GNU General Public
 * License v2 or v3 as published by the Free Software Foundation which can be
 * found in the files LICENSE-GPL2 and LICENSE-GPL3 included in this package.
 *
 * SIP is supplied WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <Python.h>

#include "sip.h"


/* Global variables - see sip.h for their meaning. */
unsigned sipVersion;
const char *sipVersionStr;
stringList *includeDirList;

/* This will go when warning() goes. */
static int warnings = FALSE;


/* Forward declarations. */
static PyObject *py_parse(PyObject *self, PyObject *args);
static PyObject *py_transform(PyObject *self, PyObject *args);
static PyObject *py_generateCode(PyObject *self, PyObject *args);
static PyObject *py_generateExtracts(PyObject *self, PyObject *args);
static PyObject *py_generateAPI(PyObject *self, PyObject *args);
static PyObject *py_generateXML(PyObject *self, PyObject *args);

static int fs_convertor(PyObject *obj, char **fsp);
static int KwArgs_convertor(PyObject *obj, KwArgs *kap);
static int sipSpec_convertor(PyObject *obj, sipSpec **ptp);
static int stringList_convertor(PyObject *obj, stringList **slp);


/*
 * The _sip5 module initialisation function.
 */
PyMODINIT_FUNC PyInit__sip(void)
{
    static PyMethodDef methods[] = {
        {"parse", py_parse, METH_VARARGS, NULL},
        {"transform", py_transform, METH_VARARGS, NULL},
        {"generateCode", py_generateCode, METH_VARARGS, NULL},
        {"generateExtracts", py_generateExtracts, METH_VARARGS, NULL},
        {"generateAPI", py_generateAPI, METH_VARARGS, NULL},
        {"generateXML", py_generateXML, METH_VARARGS, NULL},
        {NULL, NULL, 0, NULL},
    };

    static PyModuleDef module_def = {
        PyModuleDef_HEAD_INIT,
        "sip5._sip",            /* m_name */
        NULL,                   /* m_doc */
        -1,                     /* m_size */
        methods,                /* m_methods */
        NULL,                   /* m_reload */
        NULL,                   /* m_traverse */
        NULL,                   /* m_clear */
        NULL,                   /* m_free */
    };

    return PyModule_Create(&module_def);
}


/*
 * Append a string to a list of them.
 */
void appendString(stringList **headp, const char *s)
{
    stringList *sl;

    /* Create the new entry. */

    sl = sipMalloc(sizeof (stringList));

    sl -> s = s;
    sl -> next = NULL;

    /* Append it to the list. */

    while (*headp != NULL)
        headp = &(*headp) -> next;

    *headp = sl;
}


/*
 * Display a warning message.
 */
void warning(Warning w, const char *fmt, ...)
{
    static int start = TRUE;

    va_list ap;

    /* Don't allow deprecation warnings to be suppressed. */
    if (!warnings && w != DeprecationWarning)
        return;

    if (start)
    {
        const char *wstr;

        switch (w)
        {
        case ParserWarning:
            wstr = "Parser warning";
            break;

        case DeprecationWarning:
            wstr = "Deprecation warning";
            break;
        }

        fprintf(stderr, "sip5: %s: ", wstr);
        start = FALSE;
    }

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (strchr(fmt, '\n') != NULL)
        start = TRUE;
}


/*
 * Display all or part of a one line error message describing a fatal error.
 * If the message is complete (it has a newline) then the program exits.
 */
void fatal(char *fmt,...)
{
    static int start = TRUE;

    va_list ap;

    if (start)
    {
        fprintf(stderr,"sip5: ");
        start = FALSE;
    }

    va_start(ap,fmt);
    vfprintf(stderr,fmt,ap);
    va_end(ap);

    if (strchr(fmt,'\n') != NULL)
        exit(1);
}


/*
 * Wrapper around parse().
 */
static PyObject *py_parse(PyObject *self, PyObject *args)
{
    sipSpec *pt;
    FILE *file;
    char *filename;
    stringList *versions, *backstops, *xfeatures;
    KwArgs kwArgs;
    int protHack;

    if (!PyArg_ParseTuple(args, "IsO&O&O&O&O&O&p",
            &sipVersion,
            &sipVersionStr,
            fs_convertor, &filename,
            stringList_convertor, &includeDirList,
            stringList_convertor, &versions,
            stringList_convertor, &backstops,
            stringList_convertor, &xfeatures,
            KwArgs_convertor, &kwArgs,
            &protHack))
        return NULL;

    pt = sipMalloc(sizeof (sipSpec));

    if (filename != NULL)
    {
        file = NULL;
    }
    else
    {
        file = stdin;
        filename = "stdin";
    }

    parse(pt, file, filename, versions, backstops, xfeatures, kwArgs, protHack);

    return PyCapsule_New(pt, NULL, NULL);
}


/*
 * Wrapper around transform().
 */
static PyObject *py_transform(PyObject *self, PyObject *args)
{
    sipSpec *pt;

    if (!PyArg_ParseTuple(args, "O&", sipSpec_convertor, &pt))
        return NULL;

    transform(pt);

    Py_RETURN_NONE;
}


/*
 * Wrapper around generateCode().
 */
static PyObject *py_generateCode(PyObject *self, PyObject *args)
{
    sipSpec *pt;
    char *codeDir, *docFile, *srcSuffix, consModule;
    int exceptions, tracing, releaseGIL, parts, docs, timestamp;
    stringList *versions, *xfeatures;

    if (!PyArg_ParseTuple(args, "O&O&O&O&pppiO&O&O&pp",
            sipSpec_convertor, &pt,
            fs_convertor, &codeDir,
            fs_convertor, &docFile,
            fs_convertor, &srcSuffix,
            &exceptions,
            &tracing,
            &releaseGIL,
            &parts,
            stringList_convertor, &versions,
            stringList_convertor, &xfeatures,
            fs_convertor, &consModule,
            &docs,
            &timestamp))
        return NULL;

    generateCode(pt, codeDir, docFile, srcSuffix, exceptions, tracing,
            releaseGIL, parts, versions, xfeatures, consModule, docs,
            timestamp);

    Py_RETURN_NONE;
}


/*
 * Wrapper around generateExtracts().
 */
static PyObject *py_generateExtracts(PyObject *self, PyObject *args)
{
    sipSpec *pt;
    stringList *extracts;

    if (!PyArg_ParseTuple(args, "O&O&",
            sipSpec_convertor, &pt,
            stringList_convertor, &extracts))
        return NULL;

    generateExtracts(pt, extracts);

    Py_RETURN_NONE;
}


/*
 * Wrapper around generateAPI().
 */
static PyObject *py_generateAPI(PyObject *self, PyObject *args)
{
    sipSpec *pt;
    char *apiFile;

    if (!PyArg_ParseTuple(args, "O&O&",
            sipSpec_convertor, &pt,
            fs_convertor, &apiFile))
        return NULL;

    generateAPI(pt, pt->module, apiFile);

    Py_RETURN_NONE;
}


/*
 * Wrapper around generateXML().
 */
static PyObject *py_generateXML(PyObject *self, PyObject *args)
{
    sipSpec *pt;
    char *xmlFile;

    if (!PyArg_ParseTuple(args, "O&O&",
            sipSpec_convertor, &pt,
            fs_convertor, &xmlFile))
        return NULL;

    generateXML(pt, pt->module, xmlFile);

    Py_RETURN_NONE;
}


/*
 * Convert a callable argument to a filesystem name.
 */
static int fs_convertor(PyObject *obj, char **fsp)
{
    PyObject *bytes;

    if (obj == Py_None)
    {
        *fsp = NULL;
        return 1;
    }

    if ((bytes = PyUnicode_EncodeFSDefault(obj)) == NULL)
        return 0;

    /* Leak the bytes object rather than strdup() its contents. */
    *fsp = PyBytes_AS_STRING(bytes);

    return 1;
}


/*
 * Convert a callable argument to a KwArgs.
 */
static int KwArgs_convertor(PyObject *obj, KwArgs *kap)
{
    long ka = PyLong_AsLong(obj);

    if (ka < 0 || ka > 2)
    {
        PyErr_SetString(PyExc_ValueError, "int in range 0 to 2 expected");
        return 0;
    }

    *kap = (KwArgs)ka;

    return 1;
}


/*
 * Convert a callable argument to a sipSpec.
 */
static int sipSpec_convertor(PyObject *obj, sipSpec **ptp)
{
    if (!PyCapsule_CheckExact(obj))
    {
        PyErr_SetString(PyExc_TypeError, "parse tree expected");
        return 0;
    }

    if ((*ptp = (sipSpec *)PyCapsule_GetPointer(obj, NULL)) == NULL)
        return 0;

    return 1;
}


/*
 * Convert a callable argument to a stringList.
 */
static int stringList_convertor(PyObject *obj, stringList **slp)
{
    Py_ssize_t i;

    *slp = NULL;

    if (obj == Py_None)
        return 1;

    if (!PyList_Check(obj))
    {
        PyErr_SetString(PyExc_TypeError, "list of str expected");
        return 0;
    }

    for (i = 0; i < PyList_GET_SIZE(obj); ++i)
    {
        PyObject *el;

        if ((el = PyUnicode_EncodeLocale(PyList_GET_ITEM(obj, i), NULL)) == NULL)
            return 0;

        /* Leak the bytes object rather than strdup() its contents. */
        appendString(slp, PyBytes_AS_STRING(el));
    }

    return 1;
}
