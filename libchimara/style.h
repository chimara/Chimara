#ifndef STYLE_H
#define STYLE_H

#include <glib.h>

#include "chimara-glk.h"

G_GNUC_INTERNAL GScanner *create_css_file_scanner(void);
G_GNUC_INTERNAL void scan_css_file(GScanner *scanner, ChimaraGlk *glk);

#endif
