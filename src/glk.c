#include <gtk/gtk.h>

#include "glk.h"

void
glk_exit(void)
{
	gtk_main();
}

void
glk_select(event_t *event)
{
	gtk_main_iteration();
}



