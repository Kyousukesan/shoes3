// column chart

/*
 * Note: this draws very differently from plot_line.c - assumes a small
 * number of observations and draws across (left to right)
 */

#include "shoes/plot/plot.h"

void shoes_plot_draw_column_top(cairo_t *cr, int x, int y) {
}

void shoes_plot_column_xaxis(cairo_t *cr, shoes_plot *plot, int x, VALUE obs) {
    char *rawstr = RSTRING_PTR(obs);
    int y;
    y = plot->graph_h;
    shoes_plot_draw_label(cr, plot, x, y, rawstr, BELOW);
    // we don't need to call shoes_plot_set_cairo_default on return
    // since we know it won't be changed on us.
}

void shoes_plot_draw_columns(cairo_t *cr, shoes_plot *plot) {
    int i, num_series;
    int top,left,bottom,right;
    left = plot->graph_x;
    top = plot->graph_y;
    right = plot->graph_w;
    bottom = plot->graph_h;
    int width = right - left;
    int height = bottom - top;
    // need to compute x advance based on number of series and stroke width
    int colsw = 0;       // combined stroke width for one set of bars
    num_series = plot->seriescnt;
    int strokesw[num_series];
    double maximums[num_series];
    double minimums[num_series];
    double vScales[num_series];
    shoes_color *colors[num_series];
    int nubs[num_series];
    VALUE values[num_series];
    VALUE labels[num_series];
    int range = plot->end_idx - plot->beg_idx; // zooming adj
    // load local var arrays
    for (i = 0; i < plot->seriescnt; i++) {
        VALUE rbser = rb_ary_entry(plot->series, i);
        shoes_chart_series *cs;
        Data_Get_Struct(rbser, shoes_chart_series, cs);
        values[i] = cs->values;
        labels[i] = cs->labels;
        maximums[i] = NUM2DBL(cs->maxv);
        minimums[i] = NUM2DBL(cs->minv);
        nubs[i] = (width / range > 10) ? RTEST(cs->point_type) : 0;
#ifdef NEW_MACRO_COLOR
        colors[i] = Get_TypedStruct3(cs->color, shoes_color);
#else
        Data_Get_Struct(cs->color, shoes_color, colors[i]);
#endif
        int sw = NUM2INT(cs->strokes);
        if (sw < 4) sw = 4;
        strokesw[i] = sw;
        colsw += sw;
        vScales[i] = (height / (maximums[i] - minimums[i]));
        //values[i] = rb_ary_entry(plot->values, i);
        //VALUE rbmaxv = rb_ary_entry(plot->maxvs, i);
        //VALUE rbminv = rb_ary_entry(plot->minvs, i);
        //maximums[i] = NUM2DBL(rbmaxv);
        //minimums[i] = NUM2DBL(rbminv);
        //VALUE rbnubs = rb_ary_entry(plot->nubs, i);
        //nubs[i] = (width / range > 10) ? RTEST(rbnubs) : 0;
        //VALUE rbcolor = rb_ary_entry(plot->color, i);
        //VALUE rbstroke = rb_ary_entry(plot->strokes, i);
        //int sw = NUM2INT(rbstroke);
        //if (sw < 4) sw = 4;
        //strokesw[i] = sw;
        //colsw += sw;
        //vScales[i] = (height / (maximums[i] - minimums[i]));
        //Data_Get_Struct(rbcolor, shoes_color, colors[i]);
    }
    int ncolsw = width / (range) ; //
    int xinset = left + (ncolsw / 2); // start inside the box, half a width
    int xpos = xinset;
    //printf("ncolsw: w: %i, advw: %i, start inset %i\n", width, ncolsw, xpos);
    for (i = plot->beg_idx; i < plot->end_idx; i++) {
        int j;
        for (j = 0; j < plot->seriescnt; j++) {
            VALUE rbdp  = rb_ary_entry(values[j], i + plot->beg_idx);
            if (NIL_P(rbdp)) {
                printf("skipping nil at %i\n", i + plot->beg_idx);
                continue;
            }
            double v = NUM2DBL(rbdp);
            cairo_set_line_width(cr, strokesw[j]);
            cairo_set_source_rgba(cr, colors[j]->r / 255.0, colors[j]->g / 255.0,
                                  colors[j]->b / 255.0, colors[j]->a / 255.0);

            long y = height - roundl((v - minimums[j]) * vScales[j]);
            //printf("move i: %i, j: %i, x: %i, v: %f -> y: %i,%i \n", i, j, xpos, v, y, y+top);
            y += top;
            cairo_move_to(cr, xpos, bottom);
            cairo_line_to(cr, xpos, y);
            cairo_stroke(cr);
            xpos += strokesw[j];
        }
        // draw xaxis labels.
        shoes_plot_set_cairo_default(cr, plot); // reset to default
        VALUE obs = rb_ary_entry(labels[0], i + plot->beg_idx);
        //VALUE obs = rb_ary_entry(rbobs, i + plot->beg_idx);
        shoes_plot_column_xaxis(cr, plot, xpos-(colsw / 2), obs);
        xpos = (ncolsw * (i + 1)) + xinset;
    }
    // tell cairo to draw all lines (and points) not already drawn.
    cairo_stroke(cr);
    shoes_plot_set_cairo_default(cr, plot);
}

// called by the draw event - draw everything.
void shoes_plot_column_draw(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
    shoes_plot_util_adornments(cr, place, self_t, 50);
    if (self_t->seriescnt) {
        // draw  box, ticks and x,y labels.
        shoes_plot_draw_ticks_and_labels(cr, self_t); // FIX for v2?
        shoes_plot_draw_legend(cr, self_t); //Fix for v2?
        shoes_plot_draw_columns(cr, self_t);
    }
}
