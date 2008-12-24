#include "glk.h"

void glk_main(void)
{
    winid_t mainwin = glk_window_open(0, 0, 0, wintype_TextGrid, 0);
    if(!mainwin)
        return;
    
    glk_set_window(mainwin);
    glk_put_string("Philip en Marijn zijn vet goed.\n");
    glk_put_string("A veeeeeeeeeeeeeeeeeeeeeeeeeeeery looooooooooooooooooooooooong striiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiing\n");
}