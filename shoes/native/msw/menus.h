#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

extern VALUE shoes_native_menubar_setup(shoes_app *app, void *something);
extern void shoes_win32_menubar_setup(shoes_app *app, HWND hwnd);
extern void shoes_native_build_menus(shoes_app *app, VALUE mbv);
//extern void shoes_native_menuitem_callback(GtkWidget *wid, gpointer extra);
//extern void shoes_native_menuitem_insert(shoes_menu *mn, shoes_menuitem *mi, int pos);
extern void shoes_win32_menu_lookup(shoes_app *app, LPARAM lp);

