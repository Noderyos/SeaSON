# SeaSON

A single-header json reader/writer

> See full documentation in [season.h](season.h)

## Short mode example

### Normal mode

```c
#define SEASON_IMPLEMENTATION
#include "season.h"

#include <stdio.h>

int main() {
    struct season object = season_object();
      season_object_add(&object, "name", season_string("Noderyos"));
      season_object_add(&object, "age", season_number(18.5));
      struct season array = season_array();
        season_array_add(&array, season_number(16));
        season_array_add(&array, season_null());
        season_array_add(&array, season_boolean(1));
        season_array_add(&array, season_number(18));
      season_object_add(&object, "scores", array);

    season_render(&object, stdout);
    season_free(&object);
}
```

### Short mode

```c
#define SEASON_SHORT
#define SEASON_IMPLEMENTATION
#include "season.h"

#include <stdio.h>

int main() {
    season object = season_obj();
      season_obj_add(&object, "name", season_str("Noderyos"));
      season_obj_add(&object, "age", season_num(18.5));
      season array = season_arr();
        season_arr_add(&array, season_num(16));
        season_arr_add(&array, season_null());
        season_arr_add(&array, season_bool(1));
        season_arr_add(&array, season_num(18));
      season_obj_add(&object, "scores", array);

    season_render(&object, stdout);
    season_free(&object);
}
```
