#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <inttypes.h>
#include <unistd.h>

#include "pdc.h"
#include "pdc_client_connect.h"

int
main(void)
{
    uint64_t        i;
    pdc_metadata_t *energy_meta;
    pdcid_t         pdc, energy_id;

    // Construct query constraints
    float           energy_lo0 = 3.75, energy_hi0 = 4.0;
    pdc_query_t *   q0l, *q0h, *q;
    pdc_selection_t sel;

    struct timeval pdc_timer_start;
    struct timeval pdc_timer_end;

    pdc = PDCinit("pdc");

    PDC_Client_query_metadata_name_timestep("Energy", 0, &energy_meta);
    if (energy_meta == NULL || energy_meta->obj_id == 0) {
        printf("Error with energy metadata!\n");
        goto done;
    }
    energy_id = energy_meta->obj_id;

    q0l = PDCquery_create(energy_id, PDC_GTE, PDC_FLOAT, &energy_lo0);
    q0h = PDCquery_create(energy_id, PDC_LTE, PDC_FLOAT, &energy_hi0);
    q   = PDCquery_and(q0l, q0h);

    // Get selection
    gettimeofday(&pdc_timer_start, 0);

    PDCquery_get_selection(q, &sel);

    gettimeofday(&pdc_timer_end, 0);
    double get_sel_time = PDC_get_elapsed_time_double(&pdc_timer_start, &pdc_timer_end);
    printf("Get selection time: %.5e\n", get_sel_time);

    printf("  Query results:\n");
    if (sel.nhits < 500)
        PDCselection_print_all(&sel);
    else
        PDCselection_print(&sel);

    float *energy_data;
    if (sel.nhits > 0)
        energy_data = (float *)calloc(sel.nhits, sizeof(float));

    // Get data
    gettimeofday(&pdc_timer_start, 0);

    PDCquery_get_data(energy_id, &sel, energy_data);

    gettimeofday(&pdc_timer_end, 0);
    double get_data_time = PDC_get_elapsed_time_double(&pdc_timer_start, &pdc_timer_end);
    printf("Get data time: %.5e\n", get_data_time);

    printf("Query result energy data (%" PRIu64 " hits):\n", sel.nhits);
    for (i = 0; i < sel.nhits; i++) {
        if (energy_data[i] > energy_hi0 || energy_data[i] < energy_lo0) {
            printf("Error with result %" PRIu64 ": %.5e\n", i, energy_data[i]);
        }
    }
    printf("Verified: all correct!\n");

    PDCquery_free_all(q);
    PDCselection_free(&sel);

done:
    if (PDCclose(pdc) < 0)
        printf("fail to close PDC\n");

    return 0;
}
