/* systray
 *  Is not really a widget with things you can modifiy
*/
#include "shoes/types/native.h"
#include "shoes/types/systray.h"
#include "shoes/app.h"

// ruby
VALUE cSystray;

#ifdef NEW_MACRO_APP
FUNC_T("+systray", systray, -1);
#else
FUNC_M("+systray", systray, -1);
#endif

void shoes_systray_init() {
#ifdef NEW_MACRO_SYSTRAY
    cSystray = rb_define_class_under(cTypes, "Systray", rb_cData); 
#else
    cSystray = rb_define_class_under(cTypes, "Systray", cNative); 
    rb_define_alloc_func(cSystray, shoes_systray_alloc);
#endif
    // no methods 
    RUBY_M("+systray", systray, -1);
}


void shoes_systray_mark(shoes_systray *handle) {
    // we don't have any Ruby objects to mark.
}

static void shoes_systray_free(shoes_systray *handle) {
    if (handle->icon_path) free(handle->icon_path);
    if (handle->title) free(handle->title);
    if (handle->message) free(handle->message);
    RUBY_CRITICAL(SHOE_FREE(handle));
}

#ifdef NEW_MACRO_SYSTRAY
TypedData_Type_New(shoes_systray);
#endif

VALUE shoes_systray_alloc(VALUE klass) {
    VALUE obj;
    shoes_systray *handle = SHOE_ALLOC(shoes_systray);
    SHOE_MEMZERO(handle, shoes_systray, 1);
#ifdef NEW_MACRO_SYSTRAY
    obj = TypedData_Wrap_Struct(klass, &shoes_systray_type, handle);
#else
    obj = Data_Wrap_Struct(klass, NULL, shoes_systray_free, handle);
#endif
    handle->icon_path = NULL;
    handle->title = NULL;
    handle->message = NULL;
    return obj;
}

VALUE shoes_systray_new(int argc, VALUE *argv, VALUE parent) {
    // Get Ruby args. 
    VALUE rbtitle, rbmessage, rbpath;
    if (argc == 1) {
        rbtitle = shoes_hash_get(argv[0], rb_intern("title"));
        rbmessage = shoes_hash_get(argv[0], rb_intern("message"));
        rbpath = shoes_hash_get(argv[0], rb_intern("icon"));
    } else if (argc == 3) {
        rbtitle = argv[0];
        rbmessage = argv[1];
        rbpath = argv[2];
    } else {
      rb_raise(rb_eArgError, "Missing an argument to systray");
    }
    char *title = NULL, *message = NULL, *path = NULL;
    
    /* Alloc the object and init. We do keep a copy of the strings
     * which will be garbage collected at some point by Ruby
     * Assumes the strings and pixbufs in the native are copied
     * out of our process memory into the Desktop's space. 
    */
    VALUE obj = shoes_systray_alloc(cSystray);
#ifdef NEW_MACRO_SYSTRAY
    Get_TypedStruct2(obj, shoes_systray, self_t);
#else
    shoes_systray *self_t;
    Data_Get_Struct(obj, shoes_systray, self_t);
#endif
    Check_Type(rbtitle, T_STRING);
    if ((!NIL_P(rbtitle)) && (RSTRING_LEN(rbtitle) > 0)) {
      title = self_t->title = strdup(RSTRING_PTR(rbtitle));
    }
    Check_Type(rbmessage, T_STRING);
    if ((!NIL_P(rbmessage)) && (RSTRING_LEN(rbmessage) > 0)) {
      message = self_t->message = strdup(RSTRING_PTR(rbmessage));
    }
    Check_Type(rbpath, T_STRING);
    if ((!NIL_P(rbpath)) && (RSTRING_LEN(rbpath) > 0)) {
      path = self_t->icon_path =strdup(RSTRING_PTR(rbpath));
    }
    if (path == NULL || message == NULL || title == NULL) {
      rb_raise(rb_eArgError, "Bad arguments to systray");
    }
    // call the native widget
    shoes_native_systray(title, message, path); 
    return Qnil;
}

// canvas 

VALUE shoes_canvas_systray(int argc, VALUE *argv, VALUE self) {
    VALUE han;
    han = shoes_systray_new(argc, argv, self);
    return han;
}

