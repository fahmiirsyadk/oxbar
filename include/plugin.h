#ifndef PLUGIN_H
#define PLUGIN_H

#include "oxbar.h"

typedef Widget *(*WidgetCtor)(void);

void plugin_register(const char *name, WidgetCtor ctor);
WidgetCtor plugin_find(const char *name);
void plugin_init_all(void);

#endif
