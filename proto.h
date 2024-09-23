#define SEP           "--------------------------------------------------"

int remove_small_areas(struct Map_info *Map, double thresh,
                       struct Map_info *Err, double *removed_area,
                       int layer, char **columns);
