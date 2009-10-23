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

/**
 * ChimaraIFFormat:
 * 
 * Constants representing all game formats supported by the Chimara system.
 */
typedef enum _ChimaraIFFormat {
	CHIMARA_IF_FORMAT_NONE = -1,
	CHIMARA_IF_FORMAT_Z5,
	CHIMARA_IF_FORMAT_Z6,
	CHIMARA_IF_FORMAT_Z8,
	CHIMARA_IF_FORMAT_Z_BLORB,
	CHIMARA_IF_FORMAT_GLULX,
	CHIMARA_IF_FORMAT_GLULX_BLORB,
	CHIMARA_IF_NUM_FORMATS
} ChimaraIFFormat;

/**
 * ChimaraIFInterpreter:
 * 
 * Constants representing the available interpreter plugins.
 */
typedef enum _ChimaraIFInterpreter {
	CHIMARA_IF_INTERPRETER_NONE = -1,
	CHIMARA_IF_INTERPRETER_FROTZ,
	CHIMARA_IF_INTERPRETER_NITFOL,
	CHIMARA_IF_INTERPRETER_GLULXE,
	CHIMARA_IF_INTERPRETER_GIT,
	CHIMARA_IF_NUM_INTERPRETERS
} ChimaraIFInterpreter;

/**
 * ChimaraIFZmachineVersion:
 * 
 * Allowed values for the #ChimaraIF:interpreter-number property.
 */
typedef enum _ChimaraIFZmachineVersion {
	CHIMARA_IF_ZMACHINE_DEFAULT = 0,
	CHIMARA_IF_ZMACHINE_DECSYSTEM_20,
	CHIMARA_IF_ZMACHINE_APPLE_IIE,
	CHIMARA_IF_ZMACHINE_MACINTOSH,
	CHIMARA_IF_ZMACHINE_AMIGA,
	CHIMARA_IF_ZMACHINE_ATARI_ST,
	CHIMARA_IF_ZMACHINE_IBM_PC,
	CHIMARA_IF_ZMACHINE_COMMODORE_128,
	CHIMARA_IF_ZMACHINE_COMMODORE_64,
	CHIMARA_IF_ZMACHINE_APPLE_IIC,
	CHIMARA_IF_ZMACHINE_APPLE_IIGS,
	CHIMARA_IF_ZMACHINE_TANDY_COLOR,
	CHIMARA_IF_ZMACHINE_MAXVAL = CHIMARA_IF_ZMACHINE_TANDY_COLOR
} ChimaraIFZmachineVersion;

/**
 * ChimaraIF:
 * 
 * This structure contains no public members.
 */
typedef struct _ChimaraIF {
	ChimaraGlk parent_instance;
	
	/*< public >*/
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
ChimaraIFFormat chimara_if_get_format(ChimaraIF *self);
ChimaraIFInterpreter chimara_if_get_interpreter(ChimaraIF *self);

G_END_DECLS

#endif /* __CHIMARA_IF_H__ */
