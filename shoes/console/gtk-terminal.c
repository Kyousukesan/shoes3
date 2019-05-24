#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <signal.h>

#include "tesi.h"
#include <gdk/gdkkeysyms.h>
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"

#include "shoes/native/gtk/gtkcss.h"
extern char *colorstrings[];
extern void shoes_native_monitor_set(void *win, int monitor);
/*
 * heavily modified from https://github.com/alanszlosek/tesi/
 * for use in Shoes/Linux
 *
 * There are some restrictions - there is only one tesiObject because
 * it hooks to stdin/out/err and there's only one of those in a Shoes/Ruby/C
 * process.
 *  *
 * We can also switch the gtk_text_view buffer from log like/readline buffer
 * to a legacy buffer (with 80 * 24/25 lines of characters)
*/
static struct tesiObject *tobj;
static GtkWidget *terminal_window;
static GtkTextView *view;
static GtkTextBuffer *buffer;
static gboolean log_mode = TRUE;
static GtkTextBuffer *log_buffer = NULL;
static GtkTextBuffer *game_buffer = NULL;
static char *blank_line; // has columns number of spaces + nl & null
static GtkTextMark **begline;
static GtkTextMark **endline;
static gboolean saveRaw = FALSE;
static GString *rawBuffer;

static gboolean keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    struct tesiObject *tobj = (struct tesiObject*)data;
    char *c = ((GdkEventKey*)event)->string;
    char s = *c;
    if (event->keyval == GDK_KEY_BackSpace) {
        s = 010;
    }
    int cnt;
    cnt = write(tobj->fd_input, &s, 1);
    return (cnt > 0 ? TRUE : FALSE);
}

static gboolean clear_console(GtkWidget *widget, GdkEvent *event, gpointer data) {
    struct tesiObject *tobj = (struct tesiObject*) data;
    GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
    GtkTextBuffer *newbuf = gtk_text_buffer_new(NULL);
    gtk_text_view_set_buffer(view, newbuf);
    // set a mark to the end? get focus
    gtk_widget_grab_focus(GTK_WIDGET(view));
    // clean out rawBuffer
    g_string_free (rawBuffer,TRUE);
    rawBuffer = g_string_sized_new(4096);
    return TRUE;
}

static gboolean copy_console(GtkWidget *widget, GdkEvent *event, gpointer data) {
    struct tesiObject *tobj = (struct tesiObject*) data;

    GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    GtkTextIter iter_s, iter_e;
    gtk_text_buffer_get_bounds(buffer, &iter_s, &iter_e);
    gchar *bigstr = gtk_text_buffer_get_slice(buffer, &iter_s, &iter_e, TRUE);
    GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(primary, bigstr, strlen(bigstr));
    return TRUE;
}

static gboolean raw_copy(GtkWidget *widget, GdkEvent *event, gpointer data) {
    struct tesiObject *tobj = (struct tesiObject*) data;
    GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(primary, rawBuffer->str, rawBuffer->len );
    return TRUE;
}
/*
 * This is called to handle characters received from the pty
 * in response to a puts/printf/write from Shoes,Ruby, & C
 * It does't manage escape seq, x,y or deal with width and height.
 * Just write to the end of the buffer and let the gtk_text_view manage it.
 *
 * In 3.3.2 this won't be used.
*/
void console_haveChar(void *p, char c) {
    struct tesiObject *tobj = (struct tesiObject*)p;
    GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
    GtkTextIter iter_e;
    GtkTextMark *insert_mark;
    char in[8];

    snprintf(in, 7, "%c", c);
    if (c >= 32 && c != 127) {
        buffer = gtk_text_view_get_buffer(view);
        gtk_text_buffer_insert_at_cursor(buffer, in, 1);
        return;
    }
    switch (c) {
        case '\x1B': // begin escape sequence (aborting previous one if any)
            //tobj->partialSequence = 1;
            // possibly flush buffer
            break;

        case '\r': // carriage return ('M' - '@'). Move cursor to first column.
            // odds are high this preceeds a \n. Move to the begining of
            // last line in buffer line.  What happens if we insert
            break;

        case '\n':  // line feed ('J' - '@'). Move cursor down line and to first column.
            // just insert '\n' into the buffer.
            gtk_text_buffer_insert_at_cursor(buffer, in, 1);
            break;

        case '\t': // ht - horizontal tab, ('I' - '@')
            // textview can handle tabs - it claims.
            gtk_text_buffer_insert_at_cursor(buffer, in, 1);
            break;

        case '\a': // bell ('G' - '@')
            // do nothing for now... maybe a visual bell would be nice?
            break;

        case 8: // backspace cub1 cursor back 1 ('H' - '@')
            gtk_text_buffer_get_end_iter (buffer, &iter_e);
            gtk_text_buffer_backspace(buffer, &iter_e, 1, 1);
            break;

        default:
#ifdef DEBUG
            fprintf(stderr, "Unrecognized control char: %d (^%c)\n", c, c + '@');
#endif
            break;
    }
    // tell the view to show the newest position
    // from http://www.gtkforums.com/viewtopic.php?t=1307
    gtk_text_buffer_get_end_iter (buffer, &iter_e);
    insert_mark = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_place_cursor(buffer, &iter_e);
    gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (view),
                                  insert_mark, 0.0, TRUE, 0.0, 1.0);
}

// for more control, we implemented these callbacks in 3.3.2

void terminal_visAscii(struct tesiObject *tobj, char c, int x, int y) {
    char in[8];
    //GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
    //GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
    GtkTextIter iter_e;
    GtkTextMark *insert_mark;

    snprintf(in, 7, "%c", c);
    if (log_mode) {
        gtk_text_buffer_insert_at_cursor(buffer, in, 1);
    } else {
        // FIXME: so horribly inefficient and it doesn't work.
        // constrain x to avoid GTK wrap. does'nt always do that
        if (x >= tobj->width) {
            x = tobj->width = (tobj->width -1);
        }
        GtkTextIter iter_s;
        gtk_text_buffer_get_iter_at_line_offset(buffer, &iter_s, y, x);
        gtk_text_buffer_get_iter_at_line_offset(buffer, &iter_e, y, x+1);
        gtk_text_buffer_delete(buffer, &iter_s, &iter_e); // invalidates iters maybe
        gtk_text_buffer_get_iter_at_line_offset(buffer, &iter_s, y, x);
        gtk_text_buffer_place_cursor(buffer, &iter_s);
        gtk_text_buffer_insert_at_cursor(buffer, in, 1);
        return; // don't scroll
    }
    // update on screen
    gtk_text_buffer_get_end_iter (buffer, &iter_e);
    insert_mark = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_place_cursor(buffer, &iter_e);
    gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (view),
                                  insert_mark, 0.0, TRUE, 0.0, 1.0);
}

void terminal_return(struct tesiObject *tobj, int x, int y) {
    GtkTextIter iter;

    if (!log_mode) {
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, begline[y]);
        //gtk_text_buffer_get_iter_at_line_index(buffer, &iter, y, 0);
        gtk_text_buffer_place_cursor(buffer, &iter);
    }
}

void terminal_newline(struct tesiObject *tobj, int x, int y) {
    //GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
    //GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
    GtkTextIter iter_e;
    GtkTextMark *insert_mark;

    if (log_mode) {
        gtk_text_buffer_insert_at_cursor(buffer, "\n", 1);
    } else {
        // tesi moved y for us. double check it's in Gtk range
        if (y >= tobj->height) {
            tobj->y = y = (tobj->height - 1);
            // real terminals did this
        }
        gtk_text_buffer_get_iter_at_mark (buffer, &iter_e, begline[y]);
        //gtk_text_buffer_get_iter_at_line(buffer, &iter_e, buf_y);
        gtk_text_buffer_place_cursor(buffer, &iter_e);
        return; // don't scroll
    }
    // update on screen
    gtk_text_buffer_get_end_iter (buffer, &iter_e);
    insert_mark = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_place_cursor(buffer, &iter_e);
    gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (view),
                                  insert_mark, 0.0, TRUE, 0.0, 1.0);
}

void terminal_backspace(struct tesiObject *tobj, int x, int y) {
//	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
//	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
    GtkTextIter iter_e;
    GtkTextMark *insert_mark;

    gtk_text_buffer_get_end_iter (buffer, &iter_e);
    gtk_text_buffer_backspace(buffer, &iter_e, 1, 1);

    // update on screen
    gtk_text_buffer_get_end_iter (buffer, &iter_e);
    insert_mark = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_place_cursor(buffer, &iter_e);
    gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (view),
                                  insert_mark, 0.0, TRUE, 0.0, 1.0);
}

void terminal_tab(struct tesiObject *tobj, int x, int y) {
    return terminal_visAscii(tobj, '\t', x, y);
}

/*
 * Handle terminal attributes to Gtk Textview/buffer settings
 *
*/
struct tagcapture {
    int open;
    GtkTextTag *tag[8]; // this many tags in a Esc [ 1;2;  8m
    int begpos;         // buffer offsets - marks and iters are troublesome
    int endpos;         // Be wery, wery careful -- Elmer Fudd
} capture;

static  GtkTextTag *fgcolortag[256];
static  GtkTextTag *bgcolortag[256];
static  GtkTextTag *chartags[4];

static GdkRGBA colortable[256];


static void initattr(GtkTextBuffer *buffer) {
#if 1
    int i;
    for (i = 0; i < 256; i++) {
        char *cstr = strdup(colorstrings[i]); // can't write into code sections
        char *hyphen = strchr(cstr, '-');
        *hyphen = '\0';
        char *name = cstr;
        char *rgbstr = ++hyphen;
        gdk_rgba_parse(&colortable[i], rgbstr);
        char bgname[8], fgname[8];
        sprintf(fgname, "%s-fg", name);
        sprintf(bgname, "%s-bg", name);
        fgcolortag[i] =  gtk_text_buffer_create_tag(buffer, fgname,"foreground-rgba", &colortable[i], NULL);
        bgcolortag[i] =  gtk_text_buffer_create_tag(buffer, bgname,"background-rgba", &colortable[i], NULL);
        free(cstr);
    }
#else
    fgcolortag[0] =  gtk_text_buffer_create_tag(buffer, "blackfb","foreground", "black", NULL);
    //fgcolortag[1] =  gtk_text_buffer_create_tag(buffer, "redfb","foreground", "red", NULL);
    GdkRGBA temp;
    temp.red = 0.5;
    temp.green = 0.0;
    temp.blue =0.0;
    temp.alpha = 1.0;
    colortable[1] = temp;
    colortable[2].red = 0.0;
    colortable[2].green = 0.5;
    colortable[2].blue = 0.0;
    colortable[2].alpha = 1.0;
    fgcolortag[1] =  gtk_text_buffer_create_tag(buffer, "redfg","foreground-rgba", &colortable[1], NULL);
    //fgcolortag[2] =  gtk_text_buffer_create_tag(buffer, "greenfb","foreground", "green", NULL);
    fgcolortag[2] =  gtk_text_buffer_create_tag(buffer, "greenfb","foreground-rgba", &colortable[2], NULL);
    fgcolortag[3] =  gtk_text_buffer_create_tag(buffer, "brownfb","foreground", "brown", NULL);
    fgcolortag[4] =  gtk_text_buffer_create_tag(buffer, "bluefb","foreground", "blue", NULL);
    //fgcolortag[5] =  gtk_text_buffer_create_tag(buffer, "magentafb","foreground", "magenta", NULL);
    char *tstr = "#800080";
    gdk_rgba_parse(&colortable[5], tstr);
    fgcolortag[5] =  gtk_text_buffer_create_tag(buffer, tstr,"foreground-rgba", &colortable[5], NULL);
    fgcolortag[6] =  gtk_text_buffer_create_tag(buffer, "cyanfb","foreground", "cyan", NULL);
    fgcolortag[7] =  gtk_text_buffer_create_tag(buffer, "white","foreground", "white", NULL);

    bgcolortag[0] =  gtk_text_buffer_create_tag(buffer, "blackbg","background", "black", NULL);
    bgcolortag[1] =  gtk_text_buffer_create_tag(buffer, "redbg","background", "red", NULL);
    //bgcolortag[1] =  gtk_text_buffer_create_tag(buffer, "redbg", "background-rgba", colortable[1], NULL);
    bgcolortag[2] =  gtk_text_buffer_create_tag(buffer, "greenbg","background", "green", NULL);
    bgcolortag[3] =  gtk_text_buffer_create_tag(buffer, "brownbg","background", "brown", NULL);
    bgcolortag[4] =  gtk_text_buffer_create_tag(buffer, "bluebg","background", "blue", NULL);
    bgcolortag[5] =  gtk_text_buffer_create_tag(buffer, "magentabg","background", "magenta", NULL);
    bgcolortag[6] =  gtk_text_buffer_create_tag(buffer, "cyanbg","background", "cyan", NULL);
    bgcolortag[7] =  gtk_text_buffer_create_tag(buffer, "whitebg","background", "white", NULL);
#endif
    chartags[0] = gtk_text_buffer_create_tag(buffer, "boldtag","weight", PANGO_WEIGHT_BOLD, NULL);
    chartags[1] = gtk_text_buffer_create_tag(buffer, "underlinetag","underline", PANGO_UNDERLINE_SINGLE, NULL);
    capture.open = 0;
}

#if 0
static void initgame(int columns, int rows) {
    blank_line = malloc(columns+2);
    int i;
    for (i = 0; i < columns; i++) blank_line[i] = ' ';
    blank_line[i++] = '\n';
    blank_line[i] = 0;
    int len = strlen(blank_line);
    GtkTextIter iter;
    gtk_text_view_set_wrap_mode(view, GTK_WRAP_NONE);
    //gtk_text_view_set_overwrite (view, TRUE);
    begline = malloc(sizeof (GtkTextMark*) * rows);
    endline = malloc(sizeof (GtkTextMark*) * rows);
    gtk_text_buffer_get_iter_at_line_index(buffer, &iter, 0, 0);
    gtk_text_buffer_place_cursor (buffer, &iter);
    for (i = 0; i < rows; i++) {
        gtk_text_buffer_get_iter_at_line (buffer, &iter, i);
        begline[i] = gtk_text_buffer_create_mark(buffer, NULL, &iter, TRUE);
        gtk_text_buffer_insert (buffer, &iter, blank_line, len);
        // endline[] May not be needed
        //gtk_text_buffer_get_iter_at_line_offset(buffer, &iter, i, columns -1);
        //endline[i] = gtk_text_buffer_create_mark(buffer, NULL, &iter, TRUE);
    }
    gtk_text_buffer_get_iter_at_line_index(buffer, &iter, 0, 0);
    gtk_text_buffer_place_cursor (buffer, &iter);
    // below returns 1 line more than I inserted. Documented behaviour.
    //int nlines = gtk_text_buffer_get_line_count(buffer);
}
#endif

void terminal_attreset(struct tesiObject *tobj) {
    // reset all attibutes (color, bold,...)
    // close the tagcapture - apply the save color from saved pt to where
    // the cursor is now. These are buffer offsets converted to iters.
    if (capture.open > 0) {
        GtkTextIter start;
        GtkTextIter end;
        GtkTextMark *endmark;
        endmark = gtk_text_buffer_get_insert(buffer); // cursor 'insert' mark
        gtk_text_buffer_get_iter_at_mark(buffer, &end, endmark);
        // convert begpos (correct offset) to start iter;
        gtk_text_buffer_get_iter_at_offset(buffer, &start, capture.begpos);
        int j;
        for (j = 0; j < capture.open; j++) {
            gtk_text_buffer_apply_tag(buffer, capture.tag[j], &start, &end);
        }
        capture.open = 0;
        // should delete or unref marks and otherwise clean up memory? TODO
    }
}

void terminal_setfgcolor(struct tesiObject *tobj, int fg) {

    capture.tag[capture.open] = fgcolortag[(fg - 30)+8]; // go for the bright colors
    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer); // cursor mark named 'insert'
    GtkTextIter start;
    // convert mark to iter to pos
    gtk_text_buffer_get_iter_at_mark(buffer, &start, mark);
    capture.begpos = gtk_text_iter_get_offset(&start);
    capture.open++; // make room for the next tag
}

void terminal_setbgcolor(struct tesiObject *tobj, int bg) {

    capture.tag[capture.open] = bgcolortag[(bg - 40)];
    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer); // cursor mark named 'insert'
    GtkTextIter start;
    // convert mark to iter to pos
    gtk_text_buffer_get_iter_at_mark(buffer, &start, mark);
    capture.begpos = gtk_text_iter_get_offset(&start);
    capture.open++; // make room for the next tag
}

void terminal_setfg256(struct tesiObject *tobj, int fg) {

    capture.tag[capture.open] = fgcolortag[fg]; // go for the bright colors
    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer); // cursor mark named 'insert'
    GtkTextIter start;
    // convert mark to iter to pos
    gtk_text_buffer_get_iter_at_mark(buffer, &start, mark);
    capture.begpos = gtk_text_iter_get_offset(&start);
    capture.open++; // make room for the next tag
}

void terminal_setbg256(struct tesiObject *tobj, int bg) {

    capture.tag[capture.open] = bgcolortag[bg];
    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer); // cursor mark named 'insert'
    GtkTextIter start;
    // convert mark to iter to pos
    gtk_text_buffer_get_iter_at_mark(buffer, &start, mark);
    capture.begpos = gtk_text_iter_get_offset(&start);
    capture.open++; // make room for the next tag
}

void terminal_charattr(struct tesiObject *tobj, int attr) {
    GtkTextTag *tag = NULL;
    if (attr == 1) {
        tag = chartags[0];
    } else if (attr == 4) {
        tag = chartags[1];
    }
    if (tag != NULL) {
        //GtkWidget *view = GTK_WIDGET(tobj->pointer);
        //GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (view));
        capture.tag[capture.open] = tag;
        GtkTextMark *mark = gtk_text_buffer_get_insert(buffer); // cursor mark named 'insert'
        GtkTextIter start;
        // convert mark to iter to pos
        gtk_text_buffer_get_iter_at_mark(buffer, &start, mark);
        capture.begpos = gtk_text_iter_get_offset(&start);
        capture.open++; // make room for the next tag
    }
}

/*
 * NOTE - cursor based drawing is discouraged - assume it doesn't work
 * because if it does work, it won't be what you want in the scrollback
 * buffer.
 *
 * When a cursor based callback is called it will check the log_mode
 * and if true, call start_cursor_mode. No return to log mode is possible
 * Brutal, unholy things happen when switching to this mode.
 *
 * Very UTF-8 unfriendly. Have I mentioned that you shouldn't do this?
*/

void start_cursor_mode(struct tesiObject *tobj) {
    if (log_mode == FALSE) return;  // was called before
    log_mode = FALSE;
}

void terminal_clearscreen(struct tesiObject *tobj, int scrollback) {
    // if (scrollback) then clear it too.
    GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
}

void console_eraseCharacter(struct tesiObject *tobj, int x, int y) {
    GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
    GtkTextIter iter_e;

    //buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(p)); ?
    gtk_text_buffer_get_iter_at_line_index(buffer, &iter_e, y, x);
    gtk_text_buffer_backspace(buffer, &iter_e, 1, 1);
}


void console_scrollUp(struct tesiObject *tobj) {
    // add line to buffer, scroll up
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    gint lcnt;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tobj->pointer));
    lcnt = gtk_text_buffer_get_line_count(buffer);
    gtk_text_buffer_get_iter_at_line_index(buffer, &iter, lcnt+1, 0);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(tobj->pointer), &iter, 0.0, false, 0.0, 0.0);
}

void terminal_moveCursor(struct tesiObject *tobj, int x, int y) {
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line_offset(buffer, &iter, y, x);
    gtk_text_buffer_place_cursor (buffer, &iter);
}

void console_insertLine(struct tesiObject *tobj, int y) {
    printf("Insert Line\n");
}

void terminal_eraseLine(struct tesiObject *tobj, int startx, int endx, int y) {
    // we can ignore this in log mode - readline will do this for a \b
    if (!log_mode) {
        GtkTextIter iter, s, e;
        int len = endx - startx;
        gtk_text_buffer_get_iter_at_line_offset(buffer, &s, y, startx);
        gtk_text_buffer_get_iter_at_line_offset(buffer, &e, y, endx);
        gtk_text_buffer_delete (buffer, &s, &e);
        gtk_text_buffer_get_iter_at_line_offset(buffer, &s, y, startx);
        gtk_text_buffer_insert(buffer, &s, blank_line, len);
    }
}
// end of incomplete/untested  functions

gboolean g_tesi_handleInput(gpointer data) {
    tesi_handleInput( (struct tesiObject*) data);
    return TRUE;
}

// access to shoes_global_console
#include "shoes/app.h"

static gboolean clean(GtkWidget *widget, GdkEvent *event, gpointer data) {
    struct tesiObject *to = (struct tesiObject*)data;
    g_source_remove(to->ides); // remove g_timeout_add
    g_source_destroy(g_main_context_find_source_by_id(NULL, to->ides));

    deleteTesiObject(data); // FIXME see deleteTesiObject

    shoes_global_terminal = 0;
    // closing the console window does not restore stdin/stdout/stderr
    return FALSE;
}

#ifdef USE_PTY // Linux only. Debug only. TODO !! 
// callback for raw capture
static void terminal_raw (struct tesiObject *tobj, char *raw, int len) {
    int i;
    for (i = 0; i < len; i++) {
        g_string_append_c(rawBuffer, (gchar) raw[i]);
    }
}
#endif
 
void shoes_native_terminal(char *app_dir, int monitor, int columns, int rows,
                           int fontsize, char* fg, char *bg, char* title) {
    GtkWidget *window;
    GtkWidget *canvas;
    GtkWidget *vbox;
    GtkScrolledWindow *sw;
    PangoFontDescription *pfd;  // for terminal
    //PangoFontDescription *bpfd; // for Label in button panel

    struct tesiObject *t;
    // create the debugging capture buffer. expands as needed.
    //  Not a lot of memory all things considered.
    rawBuffer = g_string_sized_new (4096);

    terminal_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    // size set below based on font (rows*columns)
    //gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_title (GTK_WINDOW (window), title);

    // like a Shoes stack at the top.
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    // need a panel with a string (icon?), copy button and clear button
    GtkWidget *btnpnl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2); // think flow layout
    // create widgets for btnpnl - icon, title, clear, copy, copy-raw
    // icon is wicked
    char icon_path[256];
    sprintf(icon_path, "%s/static/app-icon.png", app_dir);
    GdkPixbuf *icon_pix = gdk_pixbuf_new_from_file_at_size(icon_path, 32, 32, NULL);
    GtkWidget *icon = gtk_image_new_from_pixbuf(icon_pix);
    gtk_box_pack_start (GTK_BOX(btnpnl), icon, 1, 0, 0);

    GtkWidget *announce = gtk_label_new(title? title : "Shoes Terminal");
#if 1
    // use css for Label
    char *an_css = "GtkLabel {\n font: %s;\n color: %s;\n}\n";
    shoes_css_apply_font(announce,"Sans-Serif Italic 14", an_css);
#else
    bpfd = pango_font_description_from_string ("Sans-Serif Italic 14");
    gtk_widget_override_font (announce, bpfd);
#endif
    gtk_box_pack_start(GTK_BOX(btnpnl), announce, 1, 0, 0);

    GtkWidget *clrbtn = gtk_button_new_with_label ("Clear");
    gtk_box_pack_start (GTK_BOX(btnpnl), clrbtn, 1, 0, 0);

    GtkWidget *cpybtn = gtk_button_new_with_label ("Copy");
    gtk_box_pack_start (GTK_BOX(btnpnl), cpybtn, 1, 0, 0);

    GtkWidget *rawbtn = gtk_button_new_with_label ("copy raw");
    gtk_box_pack_start (GTK_BOX(btnpnl), rawbtn, 1, 0, 0);

    gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(btnpnl), 0, 0, 0);


    // Now create  a widget/panel for the terminal contents
    sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(sw),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(sw), 1, 1, 0);

    canvas = gtk_text_view_new();
    view = GTK_TEXT_VIEW(canvas);
    gtk_container_add (GTK_CONTAINER (sw), canvas);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(canvas), TRUE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(canvas), 4);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(canvas), 4);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(canvas), GTK_WRAP_CHAR);
    //gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(canvas), GTK_WRAP_NONE);
#if 1
    // set font and background color for scrollable window using css
    char fontnm[64];
    sprintf(fontnm,"monospace %d", fontsize);
    pfd = pango_font_description_from_string (fontnm); // needed later
    char *tcss = "GtkTextView {\n font: %s;\n color:%s\n;background-color:%s\n;\n}\n";
    char *fgc = "black";
    char *bgc = "white";
    if (fg)
      fgc = fg;
    if (bg) 
      bgc = bg;
    shoes_css_apply_font_and_colors(canvas, fontnm, fgc, bgc, tcss);
#else
    // Deal with the colors of the terminal widget. Note: these functions
    // are deprecated at gtk 3.16
    GdkRGBA bg_color, fg_color;
    if (fg)
        gdk_rgba_parse(&fg_color, fg);
    else
        gdk_rgba_parse(&fg_color, "black");
    
    gtk_widget_override_color(canvas, GTK_STATE_FLAG_NORMAL, &fg_color);
    if (bg)
        gdk_rgba_parse(&bg_color, bg);
    else
        gdk_rgba_parse(&bg_color, "white");
    gtk_widget_override_background_color(canvas, GTK_STATE_FLAG_NORMAL, &bg_color);

    // set font for scrollable window
    char fontnm[64];
    sprintf(fontnm,"monospace %d", fontsize);
    pfd = pango_font_description_from_string (fontnm);
    //pfd = pango_font_description_from_string ("monospace 12");
    gtk_widget_override_font (canvas, pfd);
#endif

    // compute 'char' width, and tab settings.
    PangoLayout *playout;
    PangoTabArray *tab_array;
    gint charwidth, charheight, tabwidth;
    playout = gtk_widget_create_pango_layout(canvas, "M");
    pango_layout_set_font_description(playout, pfd);
    pango_layout_get_pixel_size(playout, &charwidth, &charheight);
    tabwidth = charwidth * 8;
    tab_array = pango_tab_array_new(1, TRUE);
    pango_tab_array_set_tab( tab_array, 0, PANGO_TAB_LEFT, tabwidth);
    gtk_text_view_set_tabs(GTK_TEXT_VIEW(canvas), tab_array);

#if 0
    // init buffers base on mode.
    if (mode == 0) {
        log_mode = 0;
        game_buffer = buffer = gtk_text_view_get_buffer(view);
        initattr(game_buffer);
        initgame(columns, rows);
    } else { // default
        log_buffer = buffer = gtk_text_view_get_buffer(view);
        initattr(log_buffer);
    }
#else
    log_buffer = buffer = gtk_text_view_get_buffer(view);
    initattr(log_buffer);
#endif
    //gtk_widget_set_size_request (GTK_WIDGET (sw), 80*charwidth, 24*charheight);
    int slop = charwidth * 3; // probably the 4 pixel margins.
    gtk_widget_set_size_request (GTK_WIDGET (sw), (columns*charwidth)+slop, rows*charheight);

    t = newTesiObject("/bin/bash", columns, rows); // first arg not used
    tobj = t;
    t->pointer = canvas;
    //t->callback_haveCharacter = &console_haveChar;
    // cjc - haveCharacter short circuts  all? of the callbacks below:
    t->callback_handleNL = &terminal_newline;
    t->callback_handleRTN = &terminal_return;
    t->callback_handleBS = &terminal_backspace;
    t->callback_handleTAB = &terminal_tab;
    t->callback_handleBEL = NULL;
    t->callback_printCharacter = &terminal_visAscii;
    t->callback_attreset = &terminal_attreset;
    t->callback_charattr = terminal_charattr;
    t->callback_setfgcolor= &terminal_setfgcolor;
    t->callback_setbgcolor = &terminal_setbgcolor;
    t->callback_setfg256 = &terminal_setfg256;
    t->callback_setbg256 = &terminal_setbg256;
    // that's the minimum set of call backs;
    t->callback_setdefcolor = NULL;
    t->callback_deleteLines = NULL;
    t->callback_insertLines = NULL;
    t->callback_attributes = NULL; // old tesi - not used?

    t->callback_clearScreen = NULL;  //&terminal_clearscreen;
    t->callback_eraseCharacter = NULL; // &console_eraseCharacter;
    t->callback_moveCursor = &terminal_moveCursor;
    t->callback_insertLines = NULL; //&console_insertLine;
    t->callback_eraseLine = &terminal_eraseLine;
    t->callback_scrollUp = NULL; // &console_scrollUp;
#ifdef USE_PTY // Linux only
    t->callback_rawCapture = &terminal_raw; //debugging -remove it later!!
#endif

    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(clean), t);

    g_signal_connect (G_OBJECT (canvas), "key-press-event", G_CALLBACK (keypress_event), t);
    g_signal_connect (G_OBJECT (clrbtn), "clicked", G_CALLBACK (clear_console), t);
    g_signal_connect (G_OBJECT (cpybtn), "clicked", G_CALLBACK (copy_console), t);
    g_signal_connect (G_OBJECT (rawbtn), "clicked", G_CALLBACK (raw_copy), t);

    gtk_widget_grab_focus(canvas);
    //unsigned int ides = g_timeout_add(100, &g_tesi_handleInput, t);
    unsigned int ides = g_timeout_add(20, &g_tesi_handleInput, t);
    t->ides = ides;

    gtk_widget_show_all (window);
    
    // TODO: needs testing
    if (monitor >= 0) {
      // user requested a specfic monitor (0 ...)
      shoes_native_monitor_set((void *)window, monitor);
    }

    // TODO: some clean up here. Complete ?
    pango_font_description_free(pfd);
    //pango_font_description_free(bpfd);
    pango_tab_array_free(tab_array);
    g_object_unref(playout);
    return;
}

