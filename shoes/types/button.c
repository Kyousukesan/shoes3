#include "shoes/types/native.h"
#include "shoes/types/radio.h"
#include "shoes/types/button.h"
#include "shoes/app.h"

// ruby
VALUE cButton;

FUNC_T("+button", button, -1);

void shoes_button_init() {
    cButton  = rb_define_class_under(cTypes, "Button", cNative);
    rb_define_method(cButton, "draw", CASTHOOK(shoes_button_draw), 2);
    rb_define_method(cButton, "click", CASTHOOK(shoes_control_click), -1);
    rb_define_method(cButton, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cButton, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);
    RUBY_M("+button", button, -1);
}

VALUE shoes_button_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL_T(2, 0, TRUE);

#ifdef SHOES_QUARTZ
    place.h += 8;
    place.ih += 8;
#endif
    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            self_t->ref = shoes_native_button(self, canvas, &place, self_t->attr, msg);
            shoes_control_check_styles(self_t);
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;
}

#ifdef SHOES_WIN32
/*  
 * OSX used to call this from button.m,radio.m,check.m
 * Windows native does too. 
*/
void shoes_button_send_click(VALUE control) {
    if (rb_obj_is_kind_of(control, cRadio))
        shoes_check_set_checked_m(control, Qtrue);
    shoes_control_send(control, s_click);
}
#endif

// canvas
VALUE shoes_canvas_button(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE text = Qnil, attr = Qnil, button;
    
    SETUP_CANVAS();

    switch (rb_parse_args(argc, argv, "s|h,|h", &args)) {
        case 1:
            text = args.a[0];
            attr = args.a[1];
            break;

        case 2:
            attr = args.a[0];
            break;
    }

    if (!NIL_P(text))
        ATTRSET(attr, text, text);

    if (rb_block_given_p())
        ATTRSET(attr, click, rb_block_proc());

    button = shoes_control_new(cButton, attr, self);
    shoes_add_ele(canvas, button);
    
    return button;
}

