/*
 * Copyright Notice for
 * Proactive Data Containers (PDC) Software Library and Utilities
 * -----------------------------------------------------------------------------

 *** Copyright Notice ***

 * Proactive Data Containers (PDC) Copyright (c) 2017, The Regents of the
 * University of California, through Lawrence Berkeley National Laboratory,
 * UChicago Argonne, LLC, operator of Argonne National Laboratory, and The HDF
 * Group (subject to receipt of any required approvals from the U.S. Dept. of
 * Energy).  All rights reserved.

 * If you have questions about your rights to use or distribute this software,
 * please contact Berkeley Lab's Innovation & Partnerships Office at  IPO@lbl.gov.

 * NOTICE.  This Software was developed under funding from the U.S. Department of
 * Energy and the U.S. Government consequently retains certain rights. As such, the
 * U.S. Government has been granted for itself and others acting on its behalf a
 * paid-up, nonexclusive, irrevocable, worldwide license in the Software to
 * reproduce, distribute copies to the public, prepare derivative works, and
 * perform publicly and display publicly, and to permit other to do so.
 */

#ifndef PDC_OBJ_H
#define PDC_OBJ_H

#include "pdc_public.h"

/*******************/
/* Public Typedefs */
/*******************/
typedef enum { PDC_NA = 0, PDC_READ = 1, PDC_WRITE = 2 } pdc_access_t;
typedef enum {
    PDC_OBJ_STATIC     = 0,
    PDC_REGION_STATIC  = 1,
    PDC_REGION_DYNAMIC = 2,
    PDC_REGION_LOCAL   = 3
} pdc_region_partition_t;
typedef enum { PDC_BLOCK = 0, PDC_NOBLOCK = 1 } pdc_lock_mode_t;
typedef enum {
    PDC_CONSISTENCY_DEFAULT  = 0,
    PDC_CONSISTENCY_POSIX    = 1,
    PDC_CONSISTENCY_COMMIT   = 2,
    PDC_CONSISTENCY_SESSION  = 3,
    PDC_CONSISTENCY_EVENTUAL = 4
} pdc_consistency_t;
typedef struct _pdc_id_info obj_handle;

/*******************/
/* Public Structs */
/*******************/
struct pdc_obj_info {
    /* Directly coped from user argument at object creation. */
    char *name;
    /* 0 for location = PDC_OBJ_LOAL.
     * When PDC_OBJ_GLOBAL = 1, use PDC_Client_send_name_recv_id to retrieve ID. */
    pdcid_t meta_id;
    /* Registered using PDC_id_register */
    pdcid_t local_id;
    /* Set to 0 at creation time. */
    int server_id;
    /* Metadata server for this object */
    uint32_t metadata_server_id;
    /* Object property. Directly copy from user argument at object creation. */
    struct pdc_obj_prop *obj_pt;
};

/*********************/
/* Public Prototypes */
/*********************/
/**
 * Create an object
 *
 * \param cont_id [IN]          ID of the container
 * \param obj_name [IN]         Name of the object
 * \param obj_create_prop [IN]  ID of object property,
 *                              returned by PDCprop_create(PDC_OBJ_CREATE)
 *
 * \return Object id on success/Zero on failure
 */
pdcid_t PDCobj_create(pdcid_t cont_id, const char *obj_name, pdcid_t obj_create_prop);

/**
 * Open an object within a container
 *
 * \param pdc_id [IN]           ID of pdc
 * \param obj_name [IN]         Name of the object
 *
 * \return Object id on success/Zero on failure
 */
pdcid_t PDCobj_open(const char *obj_name, pdcid_t pdc_id);

/**
 * Open an object within a container collectively
 *
 * \param pdc_id [IN]           ID of pdc
 * \param obj_name [IN]         Name of the object
 *
 * \return Object id on success/Zero on failure
 */
pdcid_t PDCobj_open_col(const char *obj_name, pdcid_t pdc_id);

/**

 * Force write-back of the object from its cache.
 * This function does not guarantee the finish of flushing.
 * Only useful when server cache is enabled.
 *
 * \param obj_id [IN]           ID of the object
 *
 * \return Non-negative on success/Negative on failure

 */
perr_t PDCobj_flush_start(pdcid_t obj_id);

/**


 * Force write-back of all objects from its cache.
 * This function does not guarantee the finish of flushing.
 * Only useful when server cache is enabled.
 *

 * \param obj_id [IN]           ID of the object
 *
 * \return Non-negative on success/Negative on failure

 */
perr_t PDCobj_flush_all_start();

/**
 * Close an object
 *
 * \param obj_id [IN]           ID of the object
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCobj_close(pdcid_t obj_id);

/**
 * Get object information
 *
 * \param obj_name [IN]         Name of the object
 *
 * \return Pointer to pdc_obj_info struct on success/Null on failure
 */
struct pdc_obj_info *PDCobj_get_info(pdcid_t obj_id);

/**
 * ***********
 *
 * \param tag_name [IN]         Metadta field name
 * \param tag_value [IN]        Metadta field value
 * \param value_size [IN]       *********
 * \param n_res [IN]            *********
 * \param pdc_ids [IN]          *********
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCobj_get_id(const char *tag_name, void *tag_value, int value_size, int *n_res, uint64_t **pdc_ids);

/**
 * ***********
 *
 * \param tag_name [IN]         Metadta field name
 * \param value_len [IN]        Metadta field value
 * \param value_size [IN]       *********
 * \param n_res [IN]            *********
 * \param obj_names [IN]        *********
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCobj_get_name(const char *tag_name, void *tag_value, int value_size, int *n_res, char **obj_names);

/**
 * Set object user ID
 *
 * \param obj_prop [IN]         ID of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * \param user_id [IN]          User id
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_user_id(pdcid_t obj_prop, uint32_t user_id);

/**
 * Set object data location
 *
 * \param obj_prop [IN]         ID of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * \param app_name [IN]         Data location path
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_data_loc(pdcid_t obj_prop, char *app_name);

/**
 * Set object application name
 *
 * \param obj_prop [IN]         ID of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * \param app_name [IN]         Application name
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_app_name(pdcid_t obj_prop, char *app_name);

/**
 * Set object time step
 *
 * \param obj_prop [IN]         ID of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * \param time_step [IN]        Time step
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_time_step(pdcid_t obj_prop, uint32_t time_step);

/**
 * Set object tag
 *
 * \param obj_prop [IN]         ID of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * \param tags [IN]             Tags
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_tags(pdcid_t obj_prop, char *tags);

/**
 * Set object dimension
 *
 * \param obj_prop [IN]         ID of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * \param ndim [IN]             Number of dimensions
 * \param dims [IN]             Size of each dimension
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_dims(pdcid_t obj_prop, PDC_int_t ndim, uint64_t *dims);

/**
 * Set object type
 *
 * \param obj_prop [IN]         ID of object property,
 *                              returned by PDCprop_create(PDC_OBJ_CREATE)
 * \param type [IN]             Object variable type (enum type),
 *                              i.e. PDC_int_t, PDC_float_t
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_type(pdcid_t obj_prop, pdc_var_type_t type);

/**

 * Set object transfer partitioning
 *
 * \param obj_prop [IN]         ID of object property,
 *                              returned by PDCprop_create(PDC_OBJ_CREATE)

 * \param type [IN]             Object transfer partitioning method (enum type),
 *                              i.e. PDC_OBJ_STATIC, PDC_REGION_STATIC, PDC_REGION_DYNAMIC
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_transfer_region_type(pdcid_t obj_prop, pdc_region_partition_t region_partition);

/**

 * Set object consistency semantics
 *
 * \param obj_prop [IN]         ID of object property,
 *                              returned by PDCprop_create(PDC_OBJ_CREATE)

 * \param consistency [IN]      Consistency semantics required
 *                              e.g., PDC_CONSISTENCY_DEFAULT, PDC_CONSISTENCY_POSIX, etc
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_consistency_semantics(pdcid_t obj_prop, pdc_consistency_t consistency);

/**
 * Set an object buffer
 *
 * \param obj_prop [IN]         ID of object property,
 *                              returned by PDCprop_create(PDC_OBJ_CREATE)
 * \param buf [IN]              Starting point of object storage
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_buf(pdcid_t obj_prop, void *buf);

/**
 * Reset obj dimension
 *
 * \param obj_id [IN]         ID of object,
 * \param ndim [IN]         number of dimensions, this one must match existing record.
 * \param dims [IN]         new dimensions to be set
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCobj_set_dims(pdcid_t obj_id, int ndim, uint64_t *dims);
/**
 * Get obj dimension
 *

 * \param obj_id [IN]         ID of object,
 * \param ndim [OUT]         number of dimensions, this one must match existing record.
 * \param dims [OUT]         new dimensions to be set
 * \return Non-negative on success/Negative on failure

 */
perr_t PDCobj_get_dims(pdcid_t obj_id, int *ndim, uint64_t **dims);

/**
 * get obj dimension
 *
 * \param obj_id [IN]         ID of object,
 * \param ndim [IN]         number of object dimensions
 * \param dims [IN]         object dimensions, in a newly malloced buffer.
 * \return Non-negative on success/Negative on failure
 */
void **PDCobj_buf_retrieve(pdcid_t obj_id);

/**
 * Iterate over objects in a container
 *
 * \param cont_id [IN]          Container ID, returned by
 *                              PDCobj_open(pdcid_t pdc_id, const char *cont_name)
 *
 * \return A pointer to object handle struct on success/NULL on failure
 */
obj_handle *PDCobj_iter_start(pdcid_t cont_id);

/**
 * Check if object handle is pointing to NULL
 *
 * \param ohandle [IN]          A obj_handle struct, returned by PDCobj_iter_start(pdcid_t cont_id)
 *
 * \return 1 on success/0 on failure
 */
pbool_t PDCobj_iter_null(obj_handle *ohandle);

/**
 * Iterate the next object
 *
 * \param ohandle [IN]          A obj_handle struct, returned by
 *                              PDCobj_iter_start(pdcid_t cont_id)
 * \param cont_id [IN]          ID of the container
 *
 * \return A pointer to object handle struct on success/Zero on failure
 */
obj_handle *PDCobj_iter_next(obj_handle *ohandle, pdcid_t cont_id);

/**
 * Get object information
 *
 * \param ohandle [IN]          A pointer to obj_handle struct,
 *                              returned by PDCobj_iter_start(pdcid_t cont_id)
 *
 * \return Pointer to a pdc_obj_info struct on success/NULL on failure
 */
struct pdc_obj_info *PDCobj_iter_get_info(obj_handle *ohandle);

/**
 * View query result
 *
 * \param view_id [IN]          Query ID, returned by PDCquery_obj
 *                              e.g. pdcid_t pdc_id, PDC_match_op_t match_op, ...
 *
 * \return Object handle on success/Null on failure
 */
obj_handle *PDCview_iter_start(pdcid_t view_id);

/**
 * ***********
 *
 * \param obj_name [IN]         Object name
 * \param data [IN]             *********
 * \param size [IN]             *********
 * \param n_res [IN]            *********
 * \param cont_id [IN]          *********
 *
 * \return Non-negative on success/Negative on failure
 */
pdcid_t PDCobj_put_data(const char *obj_name, void *data, uint64_t size, pdcid_t cont_id);

/**
 * ***********
 *
 * \param obj_id [IN]           Object ID
 * \param data [IN]             *********
 * \param size [IN]             *********
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCobj_get_data(pdcid_t obj_id, void *data, uint64_t size);

/**
 * ***********
 *
 * \param obj_id [IN]           Object ID
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCobj_del(pdcid_t obj_id);

/**
 * Add a tag to an object
 *
 * \param obj_id [IN]           Object ID
 * \param tag_name [IN]         Metadta field name
 * \param tag_value [IN]        Metadta field value
 * \param value_size [IN]       ******
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCobj_put_tag(pdcid_t obj_id, char *tag_name, void *tag_value, psize_t value_size);

/**
 * Get tag information
 *
 * \param obj_id [IN]           Object ID
 * \param tag_name [IN]         Metadta field name
 * \param tag_value [IN]        Metadta field value
 * \param value_size [IN]       ******
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCobj_get_tag(pdcid_t obj_id, char *tag_name, void **tag_value, psize_t *value_size);

/**
 * Delete a tag from the object
 *
 * \param obj_id [IN]           Object ID
 * \param tag_name [IN]         Metadta field name
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDCobj_del_tag(pdcid_t obj_id, char *tag_name);

#endif /* PDC_OBJ_H */
