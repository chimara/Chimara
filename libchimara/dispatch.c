#include <libchimara/glk.h>
#include "chimara-glk-private.h"
#include "window.h"
#include "stream.h"
#include "fileref.h"

extern GPrivate *glk_data_key;

void 
gidispatch_set_object_registry(gidispatch_rock_t (*regi)(void *obj, glui32 objclass), void (*unregi)(void *obj, glui32 objclass, gidispatch_rock_t objrock))
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	winid_t win;
    strid_t str;
    frefid_t fref;
    
    glk_data->register_obj = regi;
    glk_data->unregister_obj = unregi;
    
    if(glk_data->register_obj) 
	{
        /* It's now necessary to go through all existing objects, and register them. */
        for(win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL))
            win->disprock = (*glk_data->register_obj)(win, gidisp_Class_Window);
        for(str = glk_stream_iterate(NULL, NULL); str; str = glk_stream_iterate(str, NULL))
            str->disprock = (*glk_data->register_obj)(str, gidisp_Class_Stream);
        for(fref = glk_fileref_iterate(NULL, NULL); fref; fref = glk_fileref_iterate(fref, NULL))
            fref->disprock = (*glk_data->register_obj)(fref, gidisp_Class_Fileref);
    }
}

gidispatch_rock_t 
gidispatch_get_objrock(void *obj, glui32 objclass)
{
	switch(objclass) 
	{
		case gidisp_Class_Window:
			return ((winid_t)obj)->disprock;
		case gidisp_Class_Stream:
			return ((strid_t)obj)->disprock;
		case gidisp_Class_Fileref:
			return ((frefid_t)obj)->disprock;
		default: 
		{
			gidispatch_rock_t dummy;
			dummy.num = 0;
			return dummy;
		}
	}
}

void 
gidispatch_set_retained_registry(gidispatch_rock_t (*regi)(void *array, glui32 len, char *typecode), void (*unregi)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock))
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	glk_data->register_arr = regi;
	glk_data->unregister_arr = unregi;
}
