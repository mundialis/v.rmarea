#define SEP           "--------------------------------------------------"

int remove_small_areas(struct Map_info *Map, double thresh,
                       struct Map_info *Err, double *removed_area,
                       int layer, dbCatValArray *cvarr, int nols,
                       struct cat_list *, int);

void copy_tabs(struct Map_info *In, struct Map_info *Out);
