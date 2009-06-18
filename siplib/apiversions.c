/*
 * The implementation of the supprt for setting API versions.
 *
 * @BS_LICENSE@
 */


#include <Python.h>

#include <string.h>

#include "sip.h"
#include "sipint.h"


/*
 * The structure that defines the version number of an API.
 */
typedef struct _apiVersionDef {
    /* The name of the API. */
    const char *api_name;

    /*
     * The version number of the API.  This will either be set explicitly via
     * a call to sip.setapi() or implicitly by an imported module.
     */
    int version_nr;

    /* The next in the list of APIs. */
    struct _apiVersionDef *next;
} apiVersionDef;


/*
 * The list of API versions.
 */
static apiVersionDef *api_versions = NULL;


/*
 * Forward declarations.
 */
static int add_api(const char *api, int version_nr);
static apiVersionDef *find_api(const char *api);
static int is_range_enabled(sipExportedModuleDef *em, int range_index);


/*
 * See if a range of versions of a particular API is enabled.
 */
int sip_api_is_api_enabled(const char *name, int from, int to)
{
    const apiVersionDef *avd;

    if ((avd = find_api(name)) == NULL)
        return FALSE;

    if (from > 0 && avd->version_nr < from)
        return FALSE;

    if (to > 0 && avd->version_nr >= to)
        return FALSE;

    return TRUE;
}


/*
 * Initialise the the API for a module and return a negative value on error.
 */
int sipInitAPI(sipExportedModuleDef *em, PyObject *mod_dict)
{
    int *apis;
    sipVersionedFunctionDef *vf;

    /* See if the module defines any APIs. */
    if ((apis = em->em_versions) != NULL)
    {
        while (apis[0] >= 0)
        {
            /*
             * See if it is an API definition rather than a range
             * definition.
             */
            if (apis[2] < 0)
            {
                const char *api_name;
                const apiVersionDef *avd;

                api_name = sipNameFromPool(em, apis[0]);

                /* Use the default version if not already set explicitly. */
                if ((avd = find_api(api_name)) == NULL)
                    if (add_api(api_name, apis[1]) < 0)
                        return -1;
            }

            apis += 3;
        }
    }

    /* Add any versioned global functions to the module dictionary. */
    if ((vf = em->em_versioned_functions) != NULL)
    {
        while (vf->vf_name >= 0)
        {
            if (is_range_enabled(em, vf->vf_api_range))
            {
                const char *func_name = sipNameFromPool(em, vf->vf_name);
                PyMethodDef *pmd;
                PyObject *py_func;

                if ((pmd = sip_api_malloc(sizeof (PyMethodDef))) == NULL)
                    return -1;

                pmd->ml_name = SIP_MLNAME_CAST(func_name);
                pmd->ml_meth = vf->vf_function;
                pmd->ml_flags = vf->vf_flags;
                pmd->ml_doc = NULL;

                if ((py_func = PyCFunction_New(pmd, NULL)) == NULL)
                    return -1;

                if (PyDict_SetItemString(mod_dict, func_name, py_func) < 0)
                {
                    Py_DECREF(py_func);
                    return -1;
                }

                Py_DECREF(py_func);
            }

            ++vf;
        }
    }

    /* TODO: Update the types table according to any version information. */

    return 0;
}


/*
 * Get the version number for an API.
 */
PyObject *sipGetAPI(PyObject *self, PyObject *args)
{
    const char *api;
    const apiVersionDef *avd;

    if (!PyArg_ParseTuple(args, "s:getapi", &api))
        return NULL;

    if ((avd = find_api(api)) == NULL)
    {
        PyErr_Format(PyExc_ValueError, "unknown API '%s'", api);
        return NULL;
    }

#if PY_MAJOR_VERSION >= 3
    return PyLong_FromLong(avd->version_nr);
#else
    return PyInt_FromLong(avd->version_nr);
#endif
}


/*
 * Set the version number for an API.
 */
PyObject *sipSetAPI(PyObject *self, PyObject *args)
{
    const char *api;
    int version_nr;
    const apiVersionDef *avd;

    if (!PyArg_ParseTuple(args, "si:setapi", &api, &version_nr))
        return NULL;

    if (version_nr < 1)
    {
        PyErr_Format(PyExc_ValueError,
                "API version numbers must be greater or equal to 1, not %d",
                version_nr);
        return NULL;
    }

    if ((avd = find_api(api)) == NULL)
    {
        const char *api_copy;

        /* Make a deep copy of the name. */
        if ((api_copy = sip_api_malloc(strlen(api))) == NULL)
            return NULL;

        strcpy(api_copy, api);

        if (add_api(api_copy, version_nr) < 0)
            return NULL;
    }
    else if (avd->version_nr != version_nr)
    {
        PyErr_Format(PyExc_ValueError,
                "API '%s' has already been set to version %d", api,
                avd->version_nr);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


/*
 * Add a new API to the global list returning a negative value on error.
 */
static int add_api(const char *api, int version_nr)
{
    apiVersionDef *avd;

    if ((avd = sip_api_malloc(sizeof (apiVersionDef))) == NULL)
        return -1;

    avd->api_name = api;
    avd->version_nr = version_nr;
    avd->next = api_versions;

    api_versions = avd;

    return 0;
}


/*
 * Return the definition for the given API, or NULL if there was none.
 */
static apiVersionDef *find_api(const char *api)
{
    apiVersionDef *avd;

    for (avd = api_versions; avd != NULL; avd = avd->next)
        if (strcmp(avd->api_name, api) == 0)
            break;

    return avd;
}


/*
 * Return TRUE if a range defined by a range index is enabled.
 */
static int is_range_enabled(sipExportedModuleDef *em, int range_index)
{
    int *range = &em->em_versions[range_index * 3];
    const char *api_name = sipNameFromPool(em, range[0]);

    return sip_api_is_api_enabled(api_name, range[1], range[2]);
}
