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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "pdc.h"
#include "pdc_client_connect.h"

int
assign_work_to_rank(int rank, int size, int nwork, int *my_count, int *my_start)
{
    if (rank > size || my_count == NULL || my_start == NULL) {
        printf("assign_work_to_rank(): Error with input!\n");
        return -1;
    }
    if (nwork < size) {
        if (rank < nwork)
            *my_count = 1;
        else
            *my_count = 0;
        (*my_start) = rank * (*my_count);
    }
    else {
        (*my_count) = nwork / size;
        (*my_start) = rank * (*my_count);

        // Last few ranks may have extra work
        if (rank >= size - nwork % size) {
            (*my_count)++;
            (*my_start) += (rank - (size - nwork % size));
        }
    }

    return 1;
}

void
print_usage(char *name)
{
    printf("%s n_obj n_add_tag n_query\n", name);
}

int
main(int argc, char *argv[])
{
    pdcid_t     pdc, cont_prop, cont, obj_prop;
    pdcid_t *   obj_ids;
    int         n_obj, n_add_tag, n_query, my_obj, my_obj_s, my_add_tag, my_query, my_add_tag_s, my_query_s;
    int         obj_1percent = 0, tag_1percent = 0, query_1percent = 0;
    int         proc_num, my_rank, i, v;
    char        obj_name[128];
    double      stime, total_time, percent_time;
    pdc_kvtag_t kvtag;
    void **     values;
    size_t      value_size;
#ifdef ENABLE_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
#endif
    if (argc < 4) {
        if (my_rank == 0)
            print_usage(argv[0]);
        goto done;
    }
    n_obj     = atoi(argv[1]);
    n_add_tag = atoi(argv[2]);
    n_query   = atoi(argv[3]);

    if (n_add_tag > n_obj || n_query > n_obj) {
        if (my_rank == 0)
            printf("n_add_tag or n_query larger than n_obj! Exiting...\n");
        goto done;
    }

    assign_work_to_rank(my_rank, proc_num, n_add_tag, &my_add_tag, &my_add_tag_s);
    assign_work_to_rank(my_rank, proc_num, n_query, &my_query, &my_query_s);
    assign_work_to_rank(my_rank, proc_num, n_obj, &my_obj, &my_obj_s);

    obj_1percent   = my_obj / 100;
    tag_1percent   = my_add_tag / 100;
    query_1percent = my_query / 100;

    if (my_rank == 0)
        printf("Create %d obj, %d tags, query %d\n", my_obj, my_add_tag, my_query);

    // create a pdc
    pdc = PDCinit("pdc");

    // create a container property
    cont_prop = PDCprop_create(PDC_CONT_CREATE, pdc);
    if (cont_prop <= 0)
        printf("Fail to create container property @ line  %d!\n", __LINE__);

    // create a container
    cont = PDCcont_create("c1", cont_prop);
    if (cont <= 0)
        printf("Fail to create container @ line  %d!\n", __LINE__);

    // create an object property
    obj_prop = PDCprop_create(PDC_OBJ_CREATE, pdc);
    if (obj_prop <= 0)
        printf("Fail to create object property @ line  %d!\n", __LINE__);

    // Create a number of objects, add at least one tag to that object
    obj_ids = (pdcid_t *)calloc(my_obj, sizeof(pdcid_t));

#ifdef ENABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    stime = MPI_Wtime();
#endif

    for (i = 0; i < my_obj; i++) {
        sprintf(obj_name, "obj%d", my_obj_s + i);
        obj_ids[i] = PDCobj_create(cont, obj_name, obj_prop);
        if (obj_ids[i] <= 0)
            printf("Fail to create object @ line  %d!\n", __LINE__);

        if (i > 0 && i % obj_1percent == 0) {
#ifdef ENABLE_MPI
            MPI_Barrier(MPI_COMM_WORLD);
            percent_time = MPI_Wtime() - stime;
            if (my_rank == 0) {
                int    current_percentage              = i / obj_1percent;
                int    estimated_current_object_number = n_obj / 100 * current_percentage;
                double tps                             = estimated_current_object_number / percent_time;
                printf("[OBJ PROGRESS %3d%% ] %11d objects, %7.2f seconds, TPS: %10.2f \n",
                       current_percentage, estimated_current_object_number, percent_time, tps);
            }
#endif
        }
    }

#ifdef ENABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    total_time = MPI_Wtime() - stime;
#endif

    if (my_rank == 0)
        printf("Total time to create %11d objects: %7.2f , throughput %10.2f \n", n_obj, total_time,
               n_obj / total_time);

    // Add tags
    kvtag.name  = "Group";
    kvtag.value = (void *)&v;
    kvtag.size  = sizeof(int);

#ifdef ENABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    stime = MPI_Wtime();
#endif
    for (i = 0; i < my_add_tag; i++) {
        v = i + my_add_tag_s;
        if (PDCobj_put_tag(obj_ids[i], kvtag.name, kvtag.value, kvtag.size) < 0)
            printf("fail to add a kvtag to o%d\n", i + my_obj_s);

        if (i % tag_1percent == 0) {
#ifdef ENABLE_MPI
            MPI_Barrier(MPI_COMM_WORLD);
            percent_time = MPI_Wtime() - stime;
            if (my_rank == 0) {
                int    current_percentage           = i / tag_1percent;
                int    estimated_current_tag_number = n_obj / 100 * current_percentage;
                double tps                          = estimated_current_tag_number / percent_time;
                printf("[TAG PROGRESS %3d%% ] %11d tags, %7.2f seconds, TPS: %10.2f \n", current_percentage,
                       estimated_current_tag_number, percent_time, tps);
            }
#endif
        }
    }

#ifdef ENABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    total_time = MPI_Wtime() - stime;
#endif
    if (my_rank == 0)
        printf("Total time to add tags to %11d objects: %7.2f , throughput %10.2f \n", n_add_tag, total_time,
               n_add_tag / total_time);

    values = (void **)calloc(my_query, sizeof(void *));

#ifdef ENABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    stime = MPI_Wtime();
#endif
    for (i = 0; i < my_query; i++) {
        if (PDCobj_get_tag(obj_ids[i], kvtag.name, (void *)&values[i], (void *)&value_size) < 0)
            printf("fail to get a kvtag from o%d\n", i + my_query_s);

        if (i % query_1percent == 0) {
#ifdef ENABLE_MPI
            MPI_Barrier(MPI_COMM_WORLD);
            percent_time = MPI_Wtime() - stime;
            if (my_rank == 0) {
                int    current_percentage             = i / query_1percent;
                int    estimated_current_query_number = n_obj / 100 * current_percentage;
                double tps                            = estimated_current_query_number / percent_time;
                printf("[QRY PROGRESS %3d%% ] %11d queries, %7.2f seconds, TPS: %10.2f \n",
                       current_percentage, estimated_current_query_number, percent_time, tps);
            }
#endif
        }
    }

#ifdef ENABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    total_time = MPI_Wtime() - stime;
#endif
    if (my_rank == 0)
        printf("Total time to retrieve 1 tag from %11d objects: %7.2f , throughput %10.2f \n", n_query,
               total_time, n_query / total_time);

    fflush(stdout);

    for (i = 0; i < my_query; i++) {
        if (*(int *)(values[i]) != i + my_add_tag_s)
            printf("Error with retrieved tag from o%d\n", i + my_query_s);
        free(values[i]);
    }
    free(values);
    if (my_rank == 0) {
        printf("Done checking values\n");
        fflush(stdout);
    }

    /* // close first object */
    /* for (i = 0; i < my_obj; i++) { */
    /*     if (PDCobj_close(obj_ids[i]) < 0) */
    /*         printf("fail to close object o%d\n", i + my_obj_s); */
    /* } */

    /* // close a container */
    /* if (PDCcont_close(cont) < 0) */
    /*     printf("fail to close container c1\n"); */

    /* // close a container property */
    /* if (PDCprop_close(obj_prop) < 0) */
    /*     printf("Fail to close property @ line %d\n", __LINE__); */

    /* if (PDCprop_close(cont_prop) < 0) */
    /*     printf("Fail to close property @ line %d\n", __LINE__); */

    /* // close pdc */
    /* if (PDCclose(pdc) < 0) */
    /*     printf("fail to close PDC\n"); */

    /* if (my_rank == 0) { */
    /*     printf("Done closing PDC\n"); */
    /*     fflush(stdout); */
    /* } */

done:
#ifdef ENABLE_MPI
    MPI_Finalize();
#endif

    return 0;
}
