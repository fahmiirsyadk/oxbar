#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("date", NULL, 60.0, update_cmd, NULL); }
void plugin_init_date(void) { plugin_register("date", create); }
