/* Copyright / licensing information here. */

#ifndef __CHIMARA_GLK_H__
#define __CHIMARA_GLK_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CHIMARA_TYPE_GLK            (chimara_glk_get_type())
#define CHIMARA_GLK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                                     CHIMARA_TYPE_GLK, ChimaraGlk))
#define CHIMARA_GLK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), \
                                     CHIMARA_TYPE_GLK, ChimaraGlkClass))
#define CHIMARA_IS_GLK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
                                     CHIMARA_TYPE_GLK))
#define CHIMARA_IS_GLK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), \
                                     CHIMARA_TYPE_GLK))
#define CHIMARA_GLK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), \
                                     CHIMARA_TYPE_GLK, ChimaraGlkClass))

typedef struct _ChimaraGlk ChimaraGlk;
typedef struct _ChimaraGlkClass ChimaraGlkClass;

struct _ChimaraGlk {
    GtkContainer parent_instance;
    
    /* Public members */
};

struct _ChimaraGlkClass {
    GtkContainerClass parent_class;
    
    /* Signals */
	void(* stopped) (ChimaraGlk *self);
	void(* started) (ChimaraGlk *self);
};

GType chimara_glk_get_type(void) G_GNUC_CONST;
GtkWidget *chimara_glk_new(void);
void chimara_glk_set_interactive(ChimaraGlk *glk, gboolean interactive);
gboolean chimara_glk_get_interactive(ChimaraGlk *glk);
void chimara_glk_set_protect(ChimaraGlk *glk, gboolean protect);
gboolean chimara_glk_get_protect(ChimaraGlk *glk);
gboolean chimara_glk_run(ChimaraGlk *glk, GError **error);
void chimara_glk_stop(ChimaraGlk *glk);
void chimara_glk_wait(ChimaraGlk *glk);

G_END_DECLS

#endif /* __CHIMARA_GLK_H__ */