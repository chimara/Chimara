#ifndef __CHIMARA_IF_H__
#define __CHIMARA_IF_H__

#include <glib.h>
#include "chimara-glk.h"

G_BEGIN_DECLS

#define CHIMARA_TYPE_IF            (chimara_if_get_type())
#define CHIMARA_IF(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), CHIMARA_TYPE_IF, ChimaraIF))
#define CHIMARA_IF_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), CHIMARA_TYPE_IF, ChimaraIFClass))
#define CHIMARA_IS_IF(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHIMARA_TYPE_IF))
#define CHIMARA_IS_IF_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CHIMARA_TYPE_IF))
#define CHIMARA_IF_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), CHIMARA_TYPE_IF, ChimaraIFClass))

typedef enum _ChimaraIFFormat {
	CHIMARA_IF_FORMAT_Z5,
	CHIMARA_IF_FORMAT_Z6,
	CHIMARA_IF_FORMAT_Z8,
	CHIMARA_IF_FORMAT_Z_BLORB,
	CHIMARA_IF_FORMAT_GLULX,
	CHIMARA_IF_FORMAT_GLULX_BLORB,
	CHIMARA_IF_NUM_FORMATS
} ChimaraIFFormat;

typedef enum _ChimaraIFInterpreter {
	CHIMARA_IF_INTERPRETER_FROTZ,
	CHIMARA_IF_INTERPRETER_NITFOL,
	CHIMARA_IF_INTERPRETER_GLULXE,
	CHIMARA_IF_INTERPRETER_GIT,
	CHIMARA_IF_NUM_INTERPRETERS
} ChimaraIFInterpreter;

typedef struct _ChimaraIF {
	ChimaraGlk parent_instance;
} ChimaraIF;

typedef struct _ChimaraIFClass {
	ChimaraGlkClass parent_class;
	/* Signals */
	void(* command) (ChimaraIF *self, gchar *input, gchar *response);
} ChimaraIFClass;

GType chimara_if_get_type(void) G_GNUC_CONST;
GtkWidget *chimara_if_new(void);
void chimara_if_set_preferred_interpreter(ChimaraIF *self, ChimaraIFFormat format, ChimaraIFInterpreter interpreter);
ChimaraIFInterpreter chimara_if_get_preferred_interpreter(ChimaraIF *self, ChimaraIFFormat format);
gboolean chimara_if_run_game(ChimaraIF *self, gchar *gamefile, GError **error);

G_END_DECLS

#endif /* __CHIMARA_IF_H__ */
