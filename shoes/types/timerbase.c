#include "shoes/types/timerbase.h"
#include "shoes/app.h"

// ruby
VALUE cTimerBase, cTimer, cEvery, cAnim;

#ifdef NEW_MACRO_APP
FUNC_T("+animate", animate, -1);
FUNC_T("+every", every, -1);
FUNC_T("+timer", timer, -1);
#else
FUNC_M("+animate", animate, -1);
FUNC_M("+every", every, -1);
FUNC_M("+timer", timer, -1);
#endif

void shoes_timerbase_init() {
#ifdef NEW_MACRO_TIMER
    cTimerBase   = rb_define_class_under(cTypes, "TimerBase", rb_cData);
#else
    cTimerBase   = rb_define_class_under(cTypes, "TimerBase", rb_cObject);
    rb_define_alloc_func(cTimerBase, shoes_timer_alloc);
#endif
    rb_define_method(cTimerBase, "draw", CASTHOOK(shoes_timer_draw), 2);
    rb_define_method(cTimerBase, "remove", CASTHOOK(shoes_timer_remove), 0);
    rb_define_method(cTimerBase, "start", CASTHOOK(shoes_timer_start), 0);
    rb_define_method(cTimerBase, "stop", CASTHOOK(shoes_timer_stop), 0);
    rb_define_method(cTimerBase, "toggle", CASTHOOK(shoes_timer_toggle), 0);

    cAnim    = rb_define_class_under(cTypes, "Animation", cTimerBase);
    cEvery   = rb_define_class_under(cTypes, "Every", cTimerBase);
    cTimer   = rb_define_class_under(cTypes, "Timer", cTimerBase);

    RUBY_M("+animate", animate, -1);
    RUBY_M("+every", every, -1);
    RUBY_M("+timer", timer, -1);
}

// ruby
void shoes_timer_mark(shoes_timer *timer) {
    rb_gc_mark_maybe(timer->block);
    rb_gc_mark_maybe(timer->parent);
}

void shoes_timer_free(shoes_timer *timer) {
    RUBY_CRITICAL(free(timer));
}

#ifdef NEW_MACRO_TIMER
// creates struct shoes_timer_type
TypedData_Type_New(shoes_timer);
#endif

VALUE shoes_timer_alloc(VALUE klass) {
    VALUE obj;
    shoes_timer *timer = SHOE_ALLOC(shoes_timer);
    SHOE_MEMZERO(timer, shoes_timer, 1);
#ifdef NEW_MACRO_TIMER
    obj = TypedData_Wrap_Struct(klass, &shoes_timer_type, timer);
#else
    obj = Data_Wrap_Struct(klass, shoes_timer_mark, shoes_timer_free, timer);
#endif
    timer->block = Qnil;
    timer->rate = 1000 / 12;  // 12 frames per second
    timer->parent = Qnil;
    timer->started = ANIM_NADA;
    return obj;
}

VALUE shoes_timer_new(VALUE klass, VALUE rate, VALUE block, VALUE parent) {
    VALUE obj = shoes_timer_alloc(klass);
#ifdef NEW_MACRO_TIMER
    Get_TypedStruct2(obj, shoes_timer, timer);
#else
    shoes_timer *timer;
    Data_Get_Struct(obj, shoes_timer, timer);
#endif
    timer->block = block;
    if (!NIL_P(rate)) {
        if (klass == cAnim)
            timer->rate = 1000 / NUM2INT(rate);
        else
            timer->rate = (int)(1000. * NUM2DBL(rate));
    }
    if (timer->rate < 1) timer->rate = 1;
    timer->parent = parent;
    return obj;
}

VALUE shoes_timer_draw(VALUE self, VALUE c, VALUE actual) {
#ifdef NEW_MACRO_TIMER
    Get_TypedStruct2(self, shoes_timer, self_t);
#else
    shoes_timer *self_t;
    Data_Get_Struct(self, shoes_timer, self_t);
#endif
    shoes_canvas *canvas;
    Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
    if (RTEST(actual) && self_t->started == ANIM_NADA) {
        self_t->frame = 0;
        shoes_timer_start(self);
    }
    return self;
}

void shoes_timer_call(VALUE self) {
#ifdef NEW_MACRO_TIMER
    Get_TypedStruct2(self, shoes_timer, timer);
#else
    shoes_timer *timer;
    Data_Get_Struct(self, shoes_timer, timer);
#endif
    shoes_safe_block(timer->parent, timer->block, rb_ary_new3(1, INT2NUM(timer->frame)));
    timer->frame++;

    if (rb_obj_is_kind_of(self, cTimer)) {
        timer->ref = 0;
        timer->started = ANIM_STOPPED;
    }
}


VALUE shoes_timer_remove(VALUE self) {
#ifdef NEW_MACRO_TIMER
    Get_TypedStruct2(self, shoes_timer, self_t);
#else
    shoes_timer *self_t;
    Data_Get_Struct(self, shoes_timer, self_t);
#endif
    shoes_timer_stop(self);
    shoes_canvas_remove_item(self_t->parent, self, 0, 1);
    return self;
}

VALUE shoes_timer_stop(VALUE self) {
#ifdef NEW_MACRO_TIMER
    Get_TypedStruct2(self, shoes_timer, self_t);
#else
    shoes_timer *self_t;
    Data_Get_Struct(self, shoes_timer, self_t);
#endif
    if (self_t->started == ANIM_STARTED) {
        shoes_canvas *canvas;
        Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
        shoes_native_timer_remove(canvas, self_t->ref);
        self_t->started = ANIM_PAUSED;
    }
    return self;
}

VALUE shoes_timer_start(VALUE self) {
#ifdef NEW_MACRO_TIMER
    Get_TypedStruct2(self, shoes_timer, self_t);
#else
    shoes_timer *self_t;
    Data_Get_Struct(self, shoes_timer, self_t);
#endif
    unsigned int interval = self_t->rate;
    if (self_t->started != ANIM_STARTED) {
        shoes_canvas *canvas;
        Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
        self_t->ref = shoes_native_timer_start(self, canvas, interval);
        self_t->started = ANIM_STARTED;
    }
    return self;
}

VALUE shoes_timer_toggle(VALUE self) {
#ifdef NEW_MACRO_TIMER
    Get_TypedStruct2(self, shoes_timer, self_t);
#else
    shoes_timer *self_t;
    Data_Get_Struct(self, shoes_timer, self_t);
#endif
    return self_t->started == ANIM_STARTED ? shoes_timer_stop(self) : shoes_timer_start(self);
}

// canvas
VALUE shoes_canvas_animate(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE anim;
    SETUP_CANVAS();

    rb_parse_args(argc, argv, "|I&", &args);
    anim = shoes_timer_new(cAnim, args.a[0], args.a[1], self);
    rb_ary_push(canvas->app->extras, anim);
    return anim;
}

VALUE shoes_canvas_every(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE ev;
    SETUP_CANVAS();

    rb_parse_args(argc, argv, "|F&", &args);
    ev = shoes_timer_new(cEvery, args.a[0], args.a[1], self);
    rb_ary_push(canvas->app->extras, ev);
    return ev;
}

VALUE shoes_canvas_timer(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE timer;
    SETUP_CANVAS();

    rb_parse_args(argc, argv, "|I&", &args);
    timer = shoes_timer_new(cTimer, args.a[0], args.a[1], self);
    rb_ary_push(canvas->app->extras, timer);
    return timer;
}
