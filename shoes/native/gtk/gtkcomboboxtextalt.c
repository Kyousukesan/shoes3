#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/types/color.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

#include "gtkcomboboxtextalt.h"
//#include <gtk/gtkcombobox.h>

#if 0 // not used - probably a copy of Gtk internals
struct _GtkComboBoxPrivate {
    GtkTreeModel *model;

    GtkCellArea *area;

    gint col_column;
    gint row_column;

    gint wrap_width;
    GtkShadowType shadow_type;

    gint active; /* Only temporary */
    GtkTreeRowReference *active_row;

    GtkWidget *tree_view;

    GtkWidget *cell_view;
    GtkWidget *cell_view_frame;

    GtkWidget *button;
    GtkWidget *box;
    GtkWidget *arrow;
    GtkWidget *separator;

    GtkWidget *popup_widget;
    GtkWidget *popup_window;
    GtkWidget *scrolled_window;

    gulong inserted_id;
    gulong deleted_id;
    gulong reordered_id;
    gulong changed_id;
    guint popup_idle_id;
    guint activate_button;
    guint32 activate_time;
    guint scroll_timer;
    guint resize_idle_id;

    /* For "has-entry" specific behavior we track
     * an automated cell renderer and text column
     */
    gint  text_column;
    GtkCellRenderer *text_renderer;

    gint id_column;

    guint popup_in_progress : 1;
    guint popup_shown : 1;
    guint add_tearoffs : 1;
    guint has_frame : 1;
    guint is_cell_renderer : 1;
    guint editing_canceled : 1;
    guint auto_scroll : 1;
    guint focus_on_click : 1;
    guint button_sensitivity : 2;
    guint has_entry : 1;
    guint popup_fixed_width : 1;

    GtkTreeViewRowSeparatorFunc row_separator_func;
    gpointer                    row_separator_data;
    GDestroyNotify              row_separator_destroy;

    GdkDevice *grab_pointer;
    GdkDevice *grab_keyboard;

    gchar *tearoff_title;
};
#endif 

/* Private class member */
#define GTK_COMBOBOXTEXT_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
  GTK_TYPE_COMBO_BOX_TEXT_ALT, GtkComboBoxText_AltPrivate))

typedef struct _GtkComboBoxText_AltPrivate GtkComboBoxText_AltPrivate;

struct _GtkComboBoxText_AltPrivate {
    /* to avoid warnings (g_type_class_add_private: assertion `private_size > 0' failed) */
    /* Shoes controls are fixed size and known in advanced. */
    int shoes_width;
    int shoes_height;
};

/* Forward declarations */
static void gtk_combo_box_text_alt_get_preferred_width(GtkWidget *widget,
        int *minimal, int *natural);
static void gtk_combo_box_text_alt_get_preferred_height(GtkWidget *widget,
        int *minimal, int *natural);
#ifdef ADVANCED_GTK
static void gtk_combo_box_text_alt_get_preferred_height_for_width(GtkWidget *widget,
        gint avail_size, gint *minimum_size, gint *natural_size);
static void gtk_combo_box_text_alt_get_preferred_width_for_height(GtkWidget *widget,
        gint avail_size, gint *minimum_size, gint *natural_size);
#endif

/* Define the GtkComboBoxText_Alt type and inherit from GtkComboBoxText */
G_DEFINE_TYPE(GtkComboBoxText_Alt, gtk_combo_box_text_alt, GTK_TYPE_COMBO_BOX_TEXT);

/* Initialize the GtkComboBoxText_Alt class */
static void gtk_combo_box_text_alt_class_init(GtkComboBoxText_AltClass *klass) {
    /* Override GtkWidget methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->get_preferred_width = gtk_combo_box_text_alt_get_preferred_width;
    widget_class->get_preferred_height = gtk_combo_box_text_alt_get_preferred_height;
#ifdef ADVANCED_GTK
    widget_class->get_preferred_height_for_width = gtk_combo_box_text_alt_get_preferred_height_for_width;
    widget_class->get_preferred_width_for_height = gtk_combo_box_text_alt_get_preferred_width_for_height;
#endif
    /* Override GtkComboBoxText methods */
    // TODO: determine whether gobject_class has any use.
    // GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    // ...

    /* Add private indirection member */
    g_type_class_add_private(klass, sizeof(GtkComboBoxText_AltPrivate));
}

/* Initialize a new GtkComboBoxText_Alt instance */
static void gtk_combo_box_text_alt_init(GtkComboBoxText_Alt *comboboxtextAlt) {
    /* This means that GtkComboBoxText_Alt doesn't supply its own GdkWindow */
    gtk_widget_set_has_window(GTK_WIDGET(comboboxtextAlt), FALSE);

    /* Initialize private members */
    // TODO: determine whether priv has any use.
    //GtkComboBoxText_AltPrivate *priv = GTK_COMBOBOXTEXT_ALT_PRIVATE(comboboxtextAlt);
}

extern VALUE cColor;
/* Return a new GtkComboBoxText_Alt cast to a GtkWidget */
GtkWidget *gtk_combo_box_text_alt_new(VALUE attribs, int bottom_margin) {
    GtkWidget *ref;
    ref = GTK_WIDGET(g_object_new(gtk_combo_box_text_alt_get_type(), NULL));

    /* emulating gtk2 defaults*/
    int w = 160, h = 30;
    if (RTEST(ATTR(attribs, width))) w = NUM2INT(ATTR(attribs, width));
    if (RTEST(ATTR(attribs, height))) h = NUM2INT(ATTR(attribs, height));
    
    GtkComboBoxText_AltPrivate *priv = GTK_COMBOBOXTEXT_ALT_PRIVATE(ref);
    priv->shoes_width = w;
    priv->shoes_height = h;


    //GtkCellArea *area = gtk_cell_layout_get_area((GtkCellLayout *)ref);
    GList *renderers = gtk_cell_layout_get_cells((GtkCellLayout *)ref);
    GtkCellRendererText *cell = g_list_first(renderers)->data;    //only one renderer
    g_list_free(renderers);
    gtk_cell_renderer_set_fixed_size((GtkCellRenderer *)cell, w, h-bottom_margin*2);
    if (RTEST(ATTR(attribs, font))) {
        char *fontnm = RSTRING_PTR(ATTR(attribs, font));
        g_object_set((GtkCellRenderer *)cell, "font", fontnm, NULL);
    }
    if (RTEST(ATTR(attribs, stroke))) {
      VALUE fgclr = ATTR(attribs, stroke);
      if (TYPE(fgclr) == T_STRING) 
          fgclr = shoes_color_parse(cColor, fgclr);  // convert string to cColor
      if (rb_obj_is_kind_of(fgclr, cColor)) { 
          Get_TypedStruct2(fgclr, shoes_color, color);
          GdkRGBA gclr; 
          gclr.red = color->r / 255.0;
          gclr.green = color->g / 255.0; 
          gclr.blue = color->b / 255.0;
          gclr.alpha = color->a / 255.0;
          g_object_set((GtkCellRenderer *)cell, "foreground-rgba", &gclr, NULL);
      }
    }
    if (RTEST(ATTR(attribs, wrap))) {
        g_object_set((GtkCellRenderer *)cell, "wrap-width", w, NULL);
        char *wrapstr = RSTRING_PTR(ATTR(attribs, wrap));
        if (strcmp(wrapstr, "char") == 0)
            g_object_set((GtkCellRenderer *)cell, "wrap-mode", PANGO_WRAP_CHAR, NULL);
        else if (strcmp(wrapstr, "word") == 0)
            g_object_set((GtkCellRenderer *)cell, "wrap-mode", PANGO_WRAP_WORD, NULL);
        else if (strcmp(wrapstr, "trim") == 0)
            g_object_set((GtkCellRenderer *)cell, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    } else {
        g_object_set((GtkCellRenderer *)cell, "ellipsize", PANGO_ELLIPSIZE_MIDDLE, NULL);
    }
    return ref;
}

static void gtk_combo_box_text_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    /*  This is how we can access private data from parent Widget
      GtkComboBox *combo_box = GTK_COMBO_BOX(widget);
      GtkComboBoxPrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE(combo_box,
                                                            GTK_TYPE_COMBO_BOX,
                                                            GtkComboBoxPrivate);
      gint box_width;
      gtk_widget_get_preferred_width(priv->box, &box_width, NULL);
    */
#ifndef ADVANCED_GTK
    GtkComboBoxText_Alt *ref = (GtkComboBoxText_Alt *)widget;
    GtkComboBoxText_AltPrivate *priv = GTK_COMBOBOXTEXT_ALT_PRIVATE(ref);
    // Enough to quiet Gtk3.20+ whines  
    *minimal = priv->shoes_width;
    *natural = priv->shoes_width;
#else
    GList *renderers = gtk_cell_layout_get_cells((GtkCellLayout *)widget);
    GtkCellRenderer *cell = g_list_first(renderers)->data;    //only one renderer
    g_list_free(renderers);
    gint cell_width, cell_height;
    gtk_cell_renderer_get_fixed_size(cell, &cell_width, &cell_height);

    *minimal = cell_width;
    *natural = cell_width;
#endif    
}

static void gtk_combo_box_text_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    /* Combo box is height-for-width only
     * (so we always just reserve enough height for the minimum width) */
    gint min_width, nat_width;
    GTK_WIDGET_GET_CLASS(widget)->get_preferred_width(widget, &min_width, &nat_width);
    GTK_WIDGET_GET_CLASS(widget)->get_preferred_height_for_width(widget, min_width, minimal, natural);

}
#ifdef ADVANCED_GTK
static void gtk_combo_box_text_alt_get_preferred_width_for_height(GtkWidget *widget,
        gint       avail_size,
        gint      *minimum_size,
        gint      *natural_size) {
    /* Combo box is height-for-width only
     * (so we assume we always reserved enough height for the minimum width) */
    GTK_WIDGET_GET_CLASS(widget)->get_preferred_width(widget, minimum_size, natural_size);
}

static void gtk_combo_box_text_alt_get_preferred_height_for_width(GtkWidget *widget,
        gint       avail_size,
        gint      *minimum_size,
        gint      *natural_size) {
    GList *renderers = gtk_cell_layout_get_cells((GtkCellLayout *)widget);
    GtkCellRenderer *cell = g_list_first(renderers)->data;    //only one renderer
    g_list_free(renderers);

    GtkRequisition min_size, nat_size;
    gtk_cell_renderer_get_preferred_size(cell, widget, &min_size, &nat_size);

    gint xpad, ypad;
    gtk_cell_renderer_get_padding(cell, &xpad, &ypad);
    gtk_cell_renderer_set_padding(cell, xpad, 0);

    *minimum_size = min_size.height;
    *natural_size = nat_size.height;

}
#endif
// end subclass fun

extern void shoes_widget_changed(GtkWidget *ref, gpointer data);

SHOES_CONTROL_REF shoes_native_list_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    /*get bottom margin : following macro gives us bmargin (also lmargin,tmargin,rmargin)*/
    ATTR_MARGINS(attr, 0, canvas);

    SHOES_CONTROL_REF ref = gtk_combo_box_text_alt_new(attr, bmargin);

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "changed",
                     G_CALLBACK(shoes_widget_changed),
                     (gpointer)self);
    return ref;
}

void shoes_native_list_box_update(SHOES_CONTROL_REF combo, VALUE ary) {
    long i;
    gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo))));
    for (i = 0; i < RARRAY_LEN(ary); i++) {
        VALUE msg = shoes_native_to_s(rb_ary_entry(ary, i));
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, _(RSTRING_PTR(msg)));
    }
}

VALUE shoes_native_list_box_get_active(SHOES_CONTROL_REF ref, VALUE items) {
    int sel = gtk_combo_box_get_active(GTK_COMBO_BOX(ref));
    if (sel >= 0)
        return rb_ary_entry(items, sel);
    return Qnil;
}

void shoes_native_list_box_set_active(SHOES_CONTROL_REF combo, VALUE ary, VALUE item) {
    int idx = rb_ary_index_of(ary, item);
    if (idx < 0) return;
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), idx);
}

