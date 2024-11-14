/***************************************************************
 *
 * MODULE:       v.rmarea
 *
 * AUTHOR(S):    Markus Metz
 *               based on v.clean
 *
 * PURPOSE:      Remove small areas with identical attributes
 *
 * COPYRIGHT:    (C) 2024 by the GRASS Development Team
 *
 *               This program is free software under the
 *               GNU General Public License (>=v2).
 *               Read the file COPYING that comes with GRASS
 *               for details.
 *
 **************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <grass/gis.h>
#include <grass/vector.h>
#include <grass/glocale.h>

#include "proto.h"

static void error_handler_err(void *p);

int main(int argc, char *argv[])
{
    struct Map_info In, Out, Err, *pErr;
    int with_z, native;
    struct GModule *module;
    struct {
        struct Option *in, *field, *out, *thresh, *err, *cols;
    } opt;
    struct {
        struct Flag *no_build;
    } flag;
    double thresh;
    int count, count_total;
    double size;
    int layer;
    int ncols, ncols_table, col, nrec, i, j;
    struct field_info *Fi = NULL;
    dbDriver *driver = NULL;
    dbString table_name;
    dbTable *table;
    dbCatValArray *cvarr;
    char *catcol;
    char **columns;
    const char *colname;


    G_gisinit(argv[0]);

    module = G_define_module();
    G_add_keyword(_("vector"));
    G_add_keyword(_("topology"));
    G_add_keyword(_("geometry"));
    G_add_keyword(_("snapping"));
    module->description = _("Toolset for cleaning topology of vector map.");

    opt.in = G_define_standard_option(G_OPT_V_INPUT);

    opt.field = G_define_standard_option(G_OPT_V_FIELD);
    opt.field->answer = "1";
    opt.field->guisection = _("Selection");

    opt.cols = G_define_standard_option(G_OPT_DB_COLUMNS);
    opt.cols->required = YES;
    opt.cols->guisection = _("Selection");

    opt.out = G_define_standard_option(G_OPT_V_OUTPUT);

    opt.err = G_define_standard_option(G_OPT_V_OUTPUT);
    opt.err->key = "error";
    opt.err->description = _("Name of output map where errors are written");
    opt.err->required = NO;

    opt.thresh = G_define_option();
    opt.thresh->key = "threshold";
    opt.thresh->type = TYPE_DOUBLE;
    opt.thresh->required = YES;
    opt.thresh->multiple = NO;
    opt.thresh->label = _("Minimum area size in square meters");

    flag.no_build = G_define_flag();
    flag.no_build->key = 'b';
    flag.no_build->description =
        _("Do not build topology for the output vector");

    if (G_parser(argc, argv))
        exit(EXIT_FAILURE);

    Vect_check_input_output_name(opt.in->answer, opt.out->answer, G_FATAL_EXIT);
    if (opt.err->answer) {
        Vect_check_input_output_name(opt.in->answer, opt.err->answer,
                                     G_FATAL_EXIT);
    }

    /* Read threshold */
    thresh = atof(opt.thresh->answer);
    G_message(_("Tool: Threshold"));

    G_message("%s: %.15g", _("Remove small areas"), thresh);

    G_message(SEP);

    /* Input vector may be both on level 1 and 2. Level 2 is necessary for
     * virtual centroids (shapefile/OGR) and level 1 is better if input is too
     * big and build in previous module (like v.in.ogr or other call to v.clean)
     * would take a long time */
    if (Vect_open_old2(&In, opt.in->answer, "", opt.field->answer) < 0)
        G_fatal_error(_("Unable to open vector map <%s>"), opt.in->answer);

    with_z = Vect_is_3d(&In);

    if (Vect_open_new(&Out, opt.out->answer, with_z) < 0)
        G_fatal_error(_("Unable to create vector map <%s>"), opt.out->answer);

    Vect_set_error_handler_io(&In, &Out);

    if (opt.err->answer) {
        Vect_set_open_level(2);
        if (Vect_open_new(&Err, opt.err->answer, with_z) < 0)
            G_fatal_error(_("Unable to create vector map <%s>"),
                          opt.err->answer);
        G_add_error_handler(error_handler_err, &Err);
        pErr = &Err;
    }
    else {
        pErr = NULL;
    }

    /* Copy input to output */
    Vect_copy_head_data(&In, &Out);
    Vect_hist_copy(&In, &Out);
    Vect_hist_command(&Out);

    native = Vect_maptype(&Out) == GV_FORMAT_NATIVE;

    /* columns */
    layer = Vect_get_field_number(&Out, opt.field->answer);
    ncols = 0;
    columns = opt.cols->answers;
    while (columns[ncols]) {
        ncols++;
    }

    G_debug(1, "Number of columns to check: %d", ncols);

    Fi = Vect_get_field(&In, layer);
    if (Fi == NULL)
        G_fatal_error(_("Database connection not defined for layer %d"), layer);
    catcol = Fi->key;
    driver = db_start_driver_open_database(Fi->driver, Fi->database);
    db_init_string(&table_name);
    db_set_string(&table_name, Fi->table);
    if (db_describe_table(driver, &table_name, &table) != DB_OK)
        G_fatal_error(_("Unable to describe table <%s>"), Fi->table);

    ncols_table = db_get_table_number_of_columns(table);
    cvarr = G_malloc(sizeof(dbCatValArray) * ncols);

    G_debug(1, "Number of columns in table: %d", ncols_table);

    G_message("Copy attributes for %d columns ...", ncols);

    i = 0;
    for (col = 0; col < ncols_table; col++) {
        int use_col = 0;

        colname = db_get_column_name(db_get_table_column(table, col));
        for (j = 0; j < ncols; j++) {
            if (strcmp(colname, columns[j]) == 0) {
                use_col = 1;
                break;
            }
        }

        if (use_col) {
            db_CatValArray_init(&cvarr[i]);
            nrec = db_select_CatValArray(driver, Fi->table, catcol, colname,
                                         NULL, &cvarr[i]);
            i++;
        }
    }
    db_close_database_shutdown_driver(driver);
    driver = NULL;

    /* This works for both level 1 and 2 */
    Vect_copy_map_lines_field(
        &In, Vect_get_field_number(&In, opt.field->answer), &Out);

    Vect_set_release_support(&In);
    Vect_close(&In);

    if (Vect_get_built(&Out) >= GV_BUILD_CENTROIDS) {
        Vect_build_partial(&Out, GV_BUILD_CENTROIDS);
        G_message(SEP);
    }
    else {
        G_important_message(_("Rebuilding parts of topology..."));
        Vect_build_partial(&Out, GV_BUILD_CENTROIDS);
        G_message(SEP);
    }

    G_message(_("Tool: Remove small areas"));
    /* new function to also consider attributes */
    count_total = 0;
    count = 1;
    while (count > 0) {
        count = remove_small_areas(&Out, thresh, pErr, &size, layer, cvarr,
                                   ncols);
        if (count > 0) {
            count_total += count;
            
            Vect_build_partial(&Out, GV_BUILD_NONE);
            Vect_build_partial(&Out, GV_BUILD_CENTROIDS);
        }
    }

    if (count_total > 0) {
        Vect_build_partial(&Out, GV_BUILD_BASE);
        G_message(SEP);
        G_message(_("Tool: Merge boundaries"));
        Vect_merge_lines(&Out, GV_BOUNDARY, NULL, pErr);
    }

    G_message(SEP);

    if (!flag.no_build->answer) {
        G_important_message(_("Rebuilding topology for output vector map..."));
        Vect_build_partial(&Out, GV_BUILD_NONE);
        Vect_build(&Out);
    }
    else {
        Vect_build_partial(&Out, GV_BUILD_NONE); /* -> topo not saved */
    }

    if (Vect_open_old2(&In, opt.in->answer, "", opt.field->answer) < 0)
        G_fatal_error(_("Unable to open vector map <%s>"), opt.in->answer);

    copy_tabs(&In, &Out);

    Vect_close(&In);
    Vect_close(&Out);

    if (pErr) {
        G_message(SEP);
        G_important_message(_("Building topology for error vector map..."));
        Vect_build(pErr);
        Vect_close(pErr);
    }

    exit(EXIT_SUCCESS);
}

void error_handler_err(void *p)
{
    char *name;
    struct Map_info *Err;

    Err = (struct Map_info *)p;

    if (Err && Err->open == VECT_OPEN_CODE) {
        name = G_store(Err->name);
        Vect_delete(name);
        G_free(name);
    }
}
