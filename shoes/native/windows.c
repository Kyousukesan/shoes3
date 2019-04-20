//
// Windows-specific code for Shoes.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/internal.h"
#include "shoes/appwin32.h"
#include "shoes/native/windows.h"


int win_current_tmo = 10; // TODO: settings can poke this. May not be needed/used 

// called from canvas.c 
void shoes_native_get_time(SHOES_TIME *ts) {
 *ts = GetTickCount();
}

// calls from ruby.c - stubbed out until written for windows - see subsys.rb
void shoes_svg_init() {
}

void shoes_video_init() {
}


shoes_code shoes_classex_init();
LRESULT CALLBACK shoes_app_win32proc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK shoes_slot_win32proc(HWND, UINT, WPARAM, LPARAM);

WCHAR *
shoes_wchar(char *utf8)
{
  WCHAR *buffer = NULL;
  LONG wlen = 0;
  if (utf8 == NULL) return NULL;
  wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
  if (!wlen) return NULL;
  buffer = SHOE_ALLOC_N(WCHAR, wlen);
  if (!buffer) return NULL;
  MultiByteToWideChar(CP_UTF8, 0, utf8, -1, buffer, wlen);
  return buffer;
}

char *
shoes_utf8(WCHAR *buffer)
{
  char *utf8 = NULL;
  LONG i8 = 0;
  if (buffer == NULL) return NULL;
  i8 = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
  if (!i8) return NULL;
  utf8 = SHOE_ALLOC_N(char, i8);
  if (!utf8) return NULL;
  WideCharToMultiByte(CP_UTF8, 0, buffer, -1, utf8, i8, NULL, NULL);
  return utf8;
}

void shoes_win32_control_font(int id, HWND hwnd)
{
  SendDlgItemMessage(hwnd, id, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(true, 0));
}

void
shoes_win32_center(HWND hwnd)
{
  RECT rc;
  
  GetWindowRect(hwnd, &rc);
  
  SetWindowPos(hwnd, 0, 
    (GetSystemMetrics(SM_CXSCREEN) - rc.right)/2,
    (GetSystemMetrics(SM_CYSCREEN) - rc.bottom)/2,
     0, 0, SWP_NOZORDER|SWP_NOSIZE );
}

// TODO - not used? renamed/removed in ruby 2.2.0
int shoes_win32_cmdvector(const char *cmdline, char ***argv)
{
  //return rb_w32_cmdvector(cmdline, argv);
  return 0;
}


VALUE
shoes_native_load_font(const char *filename)
{
  VALUE allfonts, newfonts, oldfonts;
  int fonts = AddFontResourceEx(filename, FR_PRIVATE, 0);
  if (!fonts) return Qnil;
  allfonts = shoes_native_font_list();
  oldfonts = rb_const_get(cShoes, rb_intern("FONTS"));
  newfonts = rb_funcall(allfonts, rb_intern("-"), 1, oldfonts);
  shoes_update_fonts(allfonts);
  return newfonts;
}

int CALLBACK
shoes_font_list_iter(const LOGFONTA *font, const TEXTMETRICA *pfont, DWORD type, LPARAM l)
{
  VALUE ary = (VALUE)l;
  rb_ary_push(l, rb_str_new2(font->lfFaceName));
  return 1;
}

VALUE
shoes_native_font_list()
{
  LOGFONT font;
  VALUE ary = rb_ary_new();
  HDC dc = GetDC(shoes_world->os.hidden);
  SHOE_MEMZERO(&font, LOGFONT, 1);
  font.lfCharSet = DEFAULT_CHARSET;
  EnumFontFamiliesEx(dc, &font, shoes_font_list_iter, (LPARAM)ary, 0);
  ReleaseDC(shoes_world->os.hidden, dc);
  rb_funcall(ary, rb_intern("uniq!"), 0);
  rb_funcall(ary, rb_intern("sort!"), 0);
  return ary;
}

void shoes_native_init()
{
  INITCOMMONCONTROLSEX InitCtrlEx;
  InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
  InitCtrlEx.dwICC = ICC_PROGRESS_CLASS;
  InitCommonControlsEx(&InitCtrlEx);
  shoes_classex_init();
  shoes_world->os.hidden = CreateWindow(SHOES_HIDDENCLS, SHOES_HIDDENCLS, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, shoes_world->os.instance, NULL);
}

void shoes_native_cleanup(shoes_world_t *world)
{
}

void shoes_native_quit()
{
  PostQuitMessage(0);
}

int shoes_native_throw_message(unsigned int name, VALUE obj, void *data)
{
  return SendMessage(shoes_world->os.hidden, SHOES_WM_MESSAGE + name, obj, (LPARAM)data);
}

void shoes_native_slot_mark(SHOES_SLOT_OS *slot)
{
  rb_gc_mark_maybe(slot->controls);
  rb_gc_mark_maybe(slot->focus);
}

void shoes_native_slot_reset(SHOES_SLOT_OS *slot)
{
  slot->controls = rb_ary_new();
  rb_gc_register_address(&slot->controls);
}

void shoes_native_slot_clear(shoes_canvas *canvas)
{
  rb_ary_clear(canvas->slot->controls);
}

void shoes_native_slot_paint(SHOES_SLOT_OS *slot)
{
  RedrawWindow(slot->window, NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN);
}

void shoes_native_slot_lengthen(SHOES_SLOT_OS *slot, int height, int endy)
{
  SCROLLINFO si;
  si.cbSize = sizeof(SCROLLINFO);
  si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  si.nMin = 0;
  si.nMax = endy - 1; 
  si.nPage = height;
  si.nPos = slot->scrolly;
  INFO("SetScrollInfo(%d, nMin: %d, nMax: %d, nPage: %d)\n", 
    si.nPos, si.nMin, si.nMax, si.nPage);
  SetScrollInfo(slot->window, SB_VERT, &si, TRUE);
}

void shoes_native_slot_scroll_top(SHOES_SLOT_OS *slot)
{
  SetScrollPos(slot->window, SB_VERT, slot->scrolly, TRUE);
}

int shoes_native_slot_gutter(SHOES_SLOT_OS *slot)
{
  return GetSystemMetrics(SM_CXVSCROLL);
}

void shoes_native_remove_item(SHOES_SLOT_OS *slot, VALUE item, char c)
{
  if (c) {
    long i = rb_ary_index_of(slot->controls, item);
    if (i >= 0)
      rb_ary_insert_at(slot->controls, i, 1, Qnil);
  }
}

//
// Window-level events
//
#define WINDOW_STYLE WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX

#define WM_POINTS() \
  POINT p; \
  p.x = LOWORD(l); \
  p.y = HIWORD(l)
#define WM_POINTS2() \
  WM_POINTS(); \
  ClientToScreen(win, &p); \
  ScreenToClient(canvas->app->slot->window, &p); \

#define KEY_SYM(sym)  shoes_app_keypress(app, ID2SYM(rb_intern("" # sym)))
#define KEYPRESS(name, sym) \
  else if (w == VK_##name) { \
    VALUE v = ID2SYM(rb_intern("" # sym)); \
    if (app->os.altkey) \
      KEY_STATE(alt); \
    if (app->os.shiftkey) \
      KEY_STATE(shift); \
    if (app->os.ctrlkey) \
      KEY_STATE(control); \
    shoes_app_keypress(app, v); \
  }

static void
shoes_canvas_win32_vscroll(shoes_canvas *canvas, int code, int pos)
{
  SCROLLINFO si;
  SHOE_MEMZERO(&si, SCROLLINFO, 1);
  si.cbSize = sizeof(SCROLLINFO);
  si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
  GetScrollInfo(canvas->slot->window, SB_VERT, &si);

  switch (code) {
    case SB_LINEUP:
      si.nPos -= 16;
    break;
    case SB_LINEDOWN:
      si.nPos += 16;
    break;
    case SB_PAGEUP:
      si.nPos -= si.nPage - 32;
    break;
    case SB_PAGEDOWN:
      si.nPos += si.nPage - 32;
    break;
    case SB_THUMBTRACK:
      si.nPos = pos;
    break;
    default:
      return;
  }

  if (si.nPos < 0)
    si.nPos = 0;
  else if (si.nPos > (si.nMax - si.nPage))
    si.nPos = si.nMax - si.nPage;

  SetScrollInfo(canvas->slot->window, SB_VERT, &si, TRUE);
  canvas->slot->scrolly = si.nPos;
  if (DC(canvas->app->slot) == DC(canvas->slot))
    canvas->app->slot->scrolly = si.nPos;
  InvalidateRect(canvas->slot->window, NULL, TRUE);
}

LRESULT CALLBACK
shoes_slot_win32proc(
  HWND win,
  UINT msg,
  WPARAM w,
  LPARAM l)
{
  shoes_canvas *canvas;
  int mods = 0;         // key state for mouse events
  VALUE c = (VALUE)GetWindowLong(win, GWL_USERDATA);

  if (c != NULL) {
    TypedData_Get_Struct(c, shoes_canvas, &shoes_canvas_type, canvas);
    //Data_Get_Struct(c, shoes_canvas, canvas);
    int x = 0, y = 0;

    switch (msg) {
      case WM_ERASEBKGND:
        return 1;

      case WM_PAINT:
        INFO("WM_PAINT(slot, %lu)\n", win);
        if (c != canvas->app->canvas)
          shoes_canvas_paint(c);
        return 1;

      case WM_VSCROLL:
        shoes_canvas_win32_vscroll(canvas, LOWORD(w), HIWORD(w));
        break;
// TODO: events wants at key flag for clicks
      case WM_LBUTTONDOWN:
      {
        WM_POINTS2();
        shoes_app_click(canvas->app, 1, p.x, p.y + canvas->app->slot->scrolly, mods);
      }
      break;

      case WM_RBUTTONDOWN:
      {
        WM_POINTS2();
        shoes_app_click(canvas->app, 2, p.x, p.y + canvas->app->slot->scrolly, mods);
      }
      break;

      case WM_MBUTTONDOWN:
      {
        WM_POINTS2();
        shoes_app_click(canvas->app, 3, p.x, p.y + canvas->app->slot->scrolly, mods);
      }
      break;

      case WM_LBUTTONUP:
      {
        WM_POINTS2();
        shoes_app_release(canvas->app, 1, p.x, p.y + canvas->app->slot->scrolly, mods);
      }
      break;

      case WM_RBUTTONUP:
      {
        WM_POINTS2();
        shoes_app_release(canvas->app, 2, p.x, p.y + canvas->app->slot->scrolly, mods);
      }
      break;

      case WM_MBUTTONUP:
      {
        WM_POINTS2();
        shoes_app_release(canvas->app, 3, p.x, p.y + canvas->app->slot->scrolly, mods);
      }
      break;

      case WM_MOUSEMOVE:
      {
        WM_POINTS2();
        shoes_app_motion(canvas->app, p.x, p.y + canvas->app->slot->scrolly, mods);
      }
      return 1;

      case WM_ACTIVATE:
        if (LOWORD(w) == WA_INACTIVE) {
          int i;
          HWND newFocus = GetFocus();
          for (i = 0; i < RARRAY_LEN(canvas->slot->controls); i++) {
            VALUE ctrl = rb_ary_entry(canvas->slot->controls, i);
            if (rb_obj_is_kind_of(ctrl, cNative)) {
              shoes_control *self_t;
              TypedData_Get_Struct(ctrl, shoes_control, &shoes_control_type, self_t);
              //Data_Get_Struct(ctrl, shoes_control, self_t);
              if (self_t->ref == newFocus) {
                canvas->slot->focus = ctrl;
                break;
              }
            }
          }
        }
        break;

      case WM_SETFOCUS:
        if (!NIL_P(canvas->slot->focus)) {
          shoes_control_focus(canvas->slot->focus);
        }
        break;

      case WM_COMMAND:
        if ((HWND)l) {
          switch (HIWORD(w)) {
            case BN_CLICKED:
            {
              int id = LOWORD(w);
              VALUE control = rb_ary_entry(canvas->slot->controls, id - SHOES_CONTROL1);
              if (!NIL_P(control))
                shoes_button_send_click(control);
            }
            break;

            case CBN_SELCHANGE:
            case EN_CHANGE:
            {
              int id = LOWORD(w);
              VALUE control = rb_ary_entry(canvas->slot->controls, id - SHOES_CONTROL1);
              if (!NIL_P(control))
                shoes_control_send(control, s_change);
            }
            break;
          }
        }
      break;
    }
  }
  return DefWindowProc(win, msg, w, l);
}

LRESULT CALLBACK
shoes_hidden_win32proc(HWND win, UINT msg, WPARAM w, LPARAM l)
{
  if (msg > SHOES_WM_MESSAGE && msg < SHOES_WM_MESSAGE + SHOES_MAX_MESSAGE)
    return shoes_catch_message(msg - SHOES_WM_MESSAGE, (VALUE)w, (void *)l);
  return DefWindowProc(win, msg, w, l);
}

LRESULT CALLBACK
shoes_app_win32proc(
  HWND win,
  UINT msg,
  WPARAM w,
  LPARAM l)
{
  shoes_app *app = (shoes_app *)GetWindowLong(win, GWL_USERDATA);
  int x = 0, y = 0;
  int mods = 0;   // TODO: keys pressed for motion events
  switch (msg) {
    case WM_DESTROY:
      if (shoes_app_remove(app))
        PostQuitMessage(0);
    return 0; 

    case WM_ERASEBKGND:
      return 1;

    //
    // On Windows, I have to ensure the scrollbar's width is added
    // to the client area width.  In Shoes, the toplevel slot size is
    // always obscured by the scrollbar when it appears, rather than
    // resizing the width of the slot->
    //
    case WM_PAINT:
    {
      RECT rect, wrect;
      int scrollwidth = GetSystemMetrics(SM_CXVSCROLL);
      GetClientRect(app->slot->window, &rect);
      GetWindowRect(app->slot->window, &wrect);
      if (wrect.right - wrect.left > rect.right + scrollwidth)
        rect.right += scrollwidth;
      app->width = rect.right;
      app->height = rect.bottom;
      shoes_canvas_size(app->canvas, app->width, app->height);
      INFO("WM_PAINT(app, %lu)\n", win);
      shoes_app_paint(app);
    }
    break;

    case WM_LBUTTONDOWN:
    {
      shoes_canvas *canvas;
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
      //Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_click(app, 1, p.x, p.y + canvas->slot->scrolly, mods);
    }
    break;

    case WM_RBUTTONDOWN:
    {
      shoes_canvas *canvas;
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
      //Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_click(app, 2, p.x, p.y + canvas->slot->scrolly, mods);
    }
    break;

    case WM_MBUTTONDOWN:
    {
      shoes_canvas *canvas;
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
      //Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_click(app, 3, p.x, p.y + canvas->slot->scrolly, mods);
    }
    break;

    case WM_LBUTTONUP:
    {
      shoes_canvas *canvas;
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
      //Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_release(app, 1, p.x, p.y + canvas->slot->scrolly, mods);
    }
    break;

    case WM_RBUTTONUP:
    {
      shoes_canvas *canvas;
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
      //Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_release(app, 2, p.x, p.y + canvas->slot->scrolly, mods);
    }
    break;

    case WM_MBUTTONUP:
    {
      shoes_canvas *canvas;
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
      //Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_release(app, 3, p.x, p.y + canvas->slot->scrolly, mods);
    }
    break;

    case WM_MOUSEMOVE:
    {
      shoes_canvas *canvas;
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
      //Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_motion(app, p.x, p.y + canvas->slot->scrolly, mods);
    }
    return 1;

    case WM_CHAR:
      switch(w) {
        case 0x08:
          KEY_SYM(backspace);
        break;

        case 0x09:
          KEY_SYM(tab);
        break;

        case 0x0D:
          shoes_app_keypress(app, rb_str_new2("\n"));
        break;

        default:
        {
          VALUE v;
          WCHAR _str = w;
          CHAR str[10];
          DWORD len = WideCharToMultiByte(CP_UTF8, 0, &_str, 1, (LPSTR)str, 10, NULL, NULL);
          str[len] = '\0';
          v = rb_str_new(str, len);
          shoes_app_keypress(app, v);
        }
      }
    break; // WM_CHAR

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      if (w == VK_CONTROL)
        app->os.ctrlkey = true;
      else if (w == VK_MENU)
        app->os.altkey = true;
      else if (w == VK_SHIFT)
        app->os.shiftkey = true;
      KEYPRESS(PRIOR, page_up)
      KEYPRESS(NEXT, page_down)
      KEYPRESS(HOME, home)
      KEYPRESS(END, end)
      KEYPRESS(LEFT, left)
      KEYPRESS(UP, up)
      KEYPRESS(RIGHT, right)
      KEYPRESS(DOWN, down)
      KEYPRESS(F1, f1)
      KEYPRESS(F2, f2)
      KEYPRESS(F3, f3)
      KEYPRESS(F4, f4)
      KEYPRESS(F5, f5)
      KEYPRESS(F6, f6)
      KEYPRESS(F7, f7)
      KEYPRESS(F8, f8)
      KEYPRESS(F9, f9)
      KEYPRESS(F10, f10)
      KEYPRESS(F11, f11)
      KEYPRESS(F12, f12)
      else if ((w >= 'A' && w <= 'Z') || w == 191 || w == 190) {
        VALUE v;
        char letter = w;
        if (w == 191) {
          if (app->os.shiftkey)
            letter = '?';
          else
            letter = '/';
        } else if (w == 190) {
          if (app->os.shiftkey)
            letter = '>';
          else
            letter = '.';
        } else {
          if (!app->os.shiftkey)
            letter += 32;
        }
        v = rb_str_new(&letter, 1);
        if (app->os.altkey) {
          KEY_STATE(alt);
          shoes_app_keypress(app, v);
        }
      }
    break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
      if (w == VK_CONTROL)
        app->os.ctrlkey = false;
      else if (w == VK_MENU)
        app->os.altkey = false;
      else if (w == VK_SHIFT)
        app->os.shiftkey = false;
    break;

    case WM_MOUSEWHEEL:
    {
      shoes_canvas *canvas;
      int lines = 0, scode = 0;
      int notch = ((int)w >> 16) / WHEEL_DELTA;
      SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines, 0);
      if (lines == WHEEL_PAGESCROLL)
        scode = (int)w < 0 ? SB_PAGEDOWN : SB_PAGEUP;
      else {
        scode = (int)w < 0 ? SB_LINEDOWN : SB_LINEUP;
        notch *= lines;
      }

      INFO("WM_MOUSEWHEEL: %d (%d, %d) %lu\n", w, scode, notch, lines);
      notch = abs(notch);
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
      //Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      while (notch--)
        shoes_canvas_win32_vscroll(canvas, scode, 0);
    }
    break;

    case WM_VSCROLL:
    {
      shoes_canvas *canvas;
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
      //Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      shoes_canvas_win32_vscroll(canvas, LOWORD(w), HIWORD(w));
    }
    break;

    case WM_TIMER:
    {
      int id = LOWORD(w);
      VALUE timer = rb_ary_entry(app->extras, id - SHOES_CONTROL1);
      if (!NIL_P(timer)) {
        if (rb_obj_is_kind_of(timer, cTimer))
          KillTimer(win, id);
        shoes_timer_call(timer);
      }
    }
    break;

    case WM_ACTIVATE:
      if (LOWORD(w) == WA_INACTIVE) {
        int i;
        HWND newFocus = GetFocus();
        for (i = 0; i < RARRAY_LEN(app->slot->controls); i++) {
          VALUE ctrl = rb_ary_entry(app->slot->controls, i);
          if (rb_obj_is_kind_of(ctrl, cNative)) {
            shoes_control *self_t;
            TypedData_Get_Struct(ctrl, shoes_control, &shoes_control_type, self_t);
            //Data_Get_Struct(ctrl, shoes_control, self_t);
            if (self_t->ref == newFocus) {
              app->slot->focus = ctrl;
              break;
            }
          }
        }
      }
    break;

    case WM_SETFOCUS:
      if (!NIL_P(app->slot->focus)) {
        shoes_control_focus(app->slot->focus);
      }
      break;

    case WM_COMMAND:
      if ((HWND)l) {
        switch (HIWORD(w)) {
          case BN_CLICKED:
          {
            int id = LOWORD(w);
            VALUE control = rb_ary_entry(app->slot->controls, id - SHOES_CONTROL1);
            if (!NIL_P(control))
              shoes_button_send_click(control);
          }
          break;

          case CBN_SELCHANGE:
          case EN_CHANGE:
          {
            int id = LOWORD(w);
            VALUE control = rb_ary_entry(app->slot->controls, id - SHOES_CONTROL1);
            if (!NIL_P(control))
              shoes_control_send(control, s_change);
          }
          break;
        }
      }
    break;
  }

  return DefWindowProc(win, msg, w, l);
}

shoes_code
shoes_native_app_cursor(shoes_app *app, ID cursor)
{
  HCURSOR c;
  if (cursor == s_hand_cursor|| cursor == s_link) {
    c = LoadCursor(NULL, IDC_HAND);
  } else if (cursor == s_arrow) {
    c = LoadCursor(NULL, IDC_ARROW);
  } else
    goto done;

  SetCursor(c);

  app->cursor = cursor;

done:
  return SHOES_OK;
}

shoes_code
shoes_classex_init()
{
  shoes_code code = SHOES_OK;

  shoes_world->os.hiddenex.cbSize = sizeof(WNDCLASSEX);
  shoes_world->os.hiddenex.style = 0;
  shoes_world->os.hiddenex.lpfnWndProc = (WNDPROC)shoes_hidden_win32proc;
  shoes_world->os.hiddenex.cbClsExtra = 0;
  shoes_world->os.hiddenex.cbWndExtra = 0;
  shoes_world->os.hiddenex.hInstance = shoes_world->os.instance;
  shoes_world->os.hiddenex.hIcon = NULL;
  shoes_world->os.hiddenex.hCursor = NULL;
  shoes_world->os.hiddenex.hbrBackground = NULL;
  shoes_world->os.hiddenex.lpszMenuName = NULL;
  shoes_world->os.hiddenex.lpszClassName = SHOES_HIDDENCLS;
  shoes_world->os.hiddenex.hIconSm = NULL;

  if (!RegisterClassEx(&shoes_world->os.hiddenex)) {
    QUIT("Couldn't register Shoes hidden window class.");
  }

  shoes_world->os.classex.hInstance = shoes_world->os.instance;
  shoes_world->os.classex.lpszClassName = SHOES_SHORTNAME;
  shoes_world->os.classex.lpfnWndProc = shoes_app_win32proc;
  shoes_world->os.classex.style = CS_HREDRAW | CS_VREDRAW;
  shoes_world->os.classex.cbSize = sizeof(WNDCLASSEX);
  shoes_world->os.classex.hIcon = LoadIcon(shoes_world->os.instance, IDI_APPLICATION);
  shoes_world->os.classex.hIconSm = LoadIcon(shoes_world->os.instance, IDI_APPLICATION);
  shoes_world->os.classex.hCursor = LoadCursor(NULL, IDC_ARROW);
  shoes_world->os.classex.lpszMenuName = NULL;
  shoes_world->os.classex.cbClsExtra = 0;
  shoes_world->os.classex.cbWndExtra = 0;
  shoes_world->os.classex.hbrBackground = (HBRUSH)COLOR_WINDOW;

  shoes_world->os.classatom = RegisterClassEx(&shoes_world->os.classex);
  if (!shoes_world->os.classatom) {
    QUIT("Couldn't register WIN32 window class.");
  }

  shoes_world->os.vlclassex.hInstance = shoes_world->os.slotex.hInstance = shoes_world->os.instance;
  shoes_world->os.vlclassex.lpszClassName = SHOES_VLCLASS;
  shoes_world->os.slotex.lpszClassName = SHOES_SLOTCLASS;
  shoes_world->os.vlclassex.style = shoes_world->os.slotex.style = CS_NOCLOSE;
  shoes_world->os.vlclassex.lpfnWndProc = DefWindowProc;
  shoes_world->os.slotex.lpfnWndProc = shoes_slot_win32proc;
  shoes_world->os.vlclassex.cbSize = shoes_world->os.slotex.cbSize = sizeof(WNDCLASSEX);
  shoes_world->os.vlclassex.hIcon = shoes_world->os.slotex.hIcon = NULL;
  shoes_world->os.vlclassex.hIconSm = shoes_world->os.slotex.hIconSm = NULL;
  shoes_world->os.vlclassex.hCursor = shoes_world->os.slotex.hCursor = LoadCursor(NULL, IDC_ARROW);
  shoes_world->os.vlclassex.lpszMenuName = shoes_world->os.slotex.lpszMenuName = NULL;
  shoes_world->os.vlclassex.cbClsExtra = shoes_world->os.slotex.cbClsExtra = 0;
  shoes_world->os.vlclassex.cbWndExtra = shoes_world->os.slotex.cbWndExtra = 0;
  shoes_world->os.vlclassex.hbrBackground = shoes_world->os.slotex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

  if (!RegisterClassEx(&shoes_world->os.slotex) || !RegisterClassEx(&shoes_world->os.vlclassex))
  {
    QUIT("Couldn't register VLC window class.");
  }

quit:
  return code;
}

void
shoes_native_app_resized(shoes_app *app)
{
  if (app->slot->window != NULL) {
    RECT r;
    GetWindowRect(app->slot->window, &r);
    r.right = r.left + app->width;
    r.bottom = r.top + app->height;
    AdjustWindowRect(&r, WINDOW_STYLE, FALSE);
    MoveWindow(app->slot->window, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
  }
}

void
shoes_native_app_title(shoes_app *app, char *msg)
{
  WCHAR *buffer = shoes_wchar(msg);
  if (buffer != NULL) {
    SetWindowTextW(app->slot->window, buffer);
    SHOE_FREE(buffer);
  }
}

// TODO: settings and SHOES_APPNAME
#define SHOES_APPNAME "Shoes"
shoes_code
shoes_native_app_open(shoes_app *app, char *path, int dialog, shoes_settings *st)
{
  shoes_code code = SHOES_OK;
  RECT rect;
  DWORD exStyle = dialog ? WS_EX_WINDOWEDGE : WS_EX_CLIENTEDGE;

  app->slot->controls = Qnil;
  app->slot->focus = Qnil;
  app->os.ctrlkey = false;
  app->os.altkey = false;
  app->os.shiftkey = false;

  // remove the menu
  rect.left = 0;
  rect.top = 0;
  rect.right = app->width;
  rect.bottom = app->height;
  AdjustWindowRectEx(&rect, WINDOW_STYLE, FALSE, exStyle);

  app->slot->window = CreateWindowEx(
      exStyle, SHOES_SHORTNAME, SHOES_APPNAME,
      WINDOW_STYLE | WS_CLIPCHILDREN |
        (app->resizable ? (WS_THICKFRAME | WS_MAXIMIZEBOX) : WS_DLGFRAME) |
        WS_VSCROLL | ES_AUTOVSCROLL,
      CW_USEDEFAULT, CW_USEDEFAULT,
      rect.right-rect.left, rect.bottom-rect.top,
      HWND_DESKTOP,
      NULL,
      shoes_world->os.instance,
      NULL
  );

  SetWindowLong(app->slot->window, GWL_USERDATA, (long)app);
  shoes_win32_center(app->slot->window);

  SCROLLINFO si;
  si.cbSize = sizeof(SCROLLINFO);
  si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  si.nMin = 0;
  si.nMax = 0; 
  si.nPage = 0;
  si.nPos = 0;
  SetScrollInfo(app->slot->window, SB_VERT, &si, TRUE);

quit:
  return code;
}

void
shoes_native_app_show(shoes_app *app)
{
  // TODO: disable parent windows of dialogs
  // if (dialog && !NIL_P(app->owner))
  // {
  //   shoes_app *owner;
  //   Data_Get_Struct(app->owner, shoes_app, owner);
  //   EnableWindow(owner->slot->window, FALSE);
  // }
  ShowWindow(app->slot->window, SW_SHOWNORMAL);
}

void
shoes_native_loop() {
  MSG msgs;
  while (msgs.message != WM_QUIT) {
    BOOL msg = PeekMessage(&msgs, NULL, 0, 0, PM_REMOVE);
    if (msg) {
      HWND focused = GetForegroundWindow();
      if (msgs.message == WM_KEYDOWN || msgs.message == WM_KEYUP) {
        shoes_app *appk = (shoes_app *)GetWindowLong(focused, GWL_USERDATA);
        ATOM wndatom = GetClassLong(focused, GCW_ATOM);
        if (appk != NULL && wndatom == shoes_world->os.classatom && RARRAY_LEN(appk->slot->controls) > 0) {
          switch (msgs.wParam) {
            case VK_TAB: case VK_UP: case VK_LEFT: case VK_DOWN:
            case VK_RIGHT: case VK_PRIOR: case VK_NEXT:
              break;
            default:
              msg = false;
          }
        } else msg = false;
      } else if (msgs.message == WM_SYSCHAR || msgs.message == WM_CHAR)
        msg = false;
      if (msg)
        msg = IsDialogMessage(focused, &msgs);

      if (!msg) {
        TranslateMessage(&msgs);
        DispatchMessage(&msgs);
      }
    } else { // no message TODO: run the next ruby thread that's ready
      rb_eval_string("sleep(0.001)");
    }
  }
}

void
shoes_native_app_close(shoes_app *app)
{
  SendMessage(APP_WINDOW(app), WM_CLOSE, 0, 0);
}

void
shoes_native_browser_open(char *url)
{
  ShellExecute(0, "open", url, 0, 0, 0);
}

void
shoes_native_slot_init(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int scrolls, int toplevel)
{
  shoes_canvas *canvas;
  SHOES_SLOT_OS *slot;
  TypedData_Get_Struct(c, shoes_canvas, &shoes_canvas_type, canvas);
  //Data_Get_Struct(c, shoes_canvas, canvas);
  slot = shoes_slot_alloc(canvas, parent, toplevel);
  slot->vscroll = scrolls;

  if (toplevel) {
    slot->dc = parent->dc;
    slot->window = parent->window;
    slot->controls = parent->controls;
  } else {
    slot->controls = rb_ary_new();
    slot->dc = NULL;
    slot->window = CreateWindowEx(0, SHOES_SLOTCLASS, "Shoes Slot Window",
      WS_CHILD | WS_CLIPCHILDREN | WS_TABSTOP | WS_VISIBLE,
      x, y, width, height, parent->window, NULL, 
      (HINSTANCE)GetWindowLong(parent->window, GWL_HINSTANCE), NULL);
    SetWindowLong(slot->window, GWL_USERDATA, (long)c);
  }
  if (toplevel)
    shoes_canvas_size(c, width, height);
}

void
shoes_native_slot_destroy(shoes_canvas *canvas, shoes_canvas *pc)
{
  if (canvas->slot->dc != NULL) {
    DeleteObject(GetCurrentObject(canvas->slot->dc, OBJ_BITMAP));
    DeleteDC(canvas->slot->dc);
  }
  DestroyWindow(canvas->slot->window);
}

cairo_t *
shoes_native_cairo_create(shoes_canvas *canvas)
{
  if (canvas->slot->surface != NULL)
    return NULL;

  HBITMAP bitmap, bitold;
  if (DC(canvas->slot) != DC(canvas->app->slot))
    canvas->slot->dc2 = BeginPaint(canvas->slot->window, &canvas->slot->ps);
  else
    canvas->slot->dc2 = GetDC(canvas->slot->window);
  if (canvas->slot->dc != NULL) {
    DeleteObject(GetCurrentObject(canvas->slot->dc, OBJ_BITMAP));
    DeleteDC(canvas->slot->dc);
  }
  canvas->slot->dc = CreateCompatibleDC(canvas->slot->dc2);
  bitmap = CreateCompatibleBitmap(canvas->slot->dc2, canvas->width, canvas->height);
  bitold = (HBITMAP)SelectObject(canvas->slot->dc, bitmap);
  DeleteObject(bitold);
  if (DC(canvas->slot) == DC(canvas->app->slot)) {
    RECT rc;
    HBRUSH bg = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    GetClientRect(canvas->slot->window, &rc);
    FillRect(canvas->slot->dc, &rc, bg);
    DeleteObject(bg);
  } else {
    shoes_canvas *parent;
    TypedData_Get_Struct(canvas->parent, shoes_canvas, &shoes_canvas_type, parent);
    //Data_Get_Struct(canvas->parent, shoes_canvas, parent);

    if (parent != NULL && parent->slot->dc != NULL) {
      RECT r;
      GetClientRect(canvas->slot->window, &r);
      BitBlt(canvas->slot->dc, 0, 0, r.right - r.left, r.bottom - r.top,
        parent->slot->dc, canvas->place.ix, canvas->place.iy, SRCCOPY);
    }
  }

  canvas->slot->surface = cairo_win32_surface_create(canvas->slot->dc);

  cairo_t *cr = cairo_create(canvas->slot->surface);
  cairo_translate(cr, 0, -canvas->slot->scrolly);
  return cr;
}

void shoes_native_cairo_destroy(shoes_canvas *canvas)
{
  BitBlt(canvas->slot->dc2, 0, 0, canvas->width, canvas->height, canvas->slot->dc, 0, 0, SRCCOPY);
  cairo_surface_destroy(canvas->slot->surface);
  canvas->slot->surface = NULL;
  if (DC(canvas->slot) != DC(canvas->app->slot))
    EndPaint(canvas->slot->window, &canvas->slot->ps);
  else
    ReleaseDC(canvas->slot->window, canvas->slot->dc2);
}

void
shoes_native_group_clear(SHOES_GROUP_OS *group)
{
}

void
shoes_native_canvas_place(shoes_canvas *self_t, shoes_canvas *pc)
{
  RECT r;
  GetWindowRect(self_t->slot->window, &r);
  if (r.left != self_t->place.ix + self_t->place.dx || 
      r.top != (self_t->place.iy + self_t->place.dy) - pc->slot->scrolly ||
      r.right - r.left != self_t->place.iw ||
      r.bottom - r.top != self_t->place.ih)
  {
    MoveWindow(self_t->slot->window, self_t->place.ix + self_t->place.dx, 
      (self_t->place.iy + self_t->place.dy) - pc->slot->scrolly, self_t->place.iw, 
      self_t->place.ih, TRUE);
  }
}

VALUE
shoes_native_clipboard_get(shoes_app *app)
{
  VALUE paste = Qnil;
  if (OpenClipboard(app->slot->window)) {
    HANDLE hclip = GetClipboardData(CF_UNICODETEXT);
    WCHAR *buffer = (WCHAR *)GlobalLock(hclip);
    char *utf8 = shoes_utf8(buffer);
    paste = rb_str_new2(utf8);
    GlobalUnlock(hclip);
    CloseClipboard();
    SHOE_FREE(utf8);
  }
  return paste;
}

void
shoes_native_clipboard_set(shoes_app *app, VALUE string)
{
  if (OpenClipboard(app->slot->window)) {
    WCHAR *buffer = shoes_wchar(RSTRING_PTR(string));
    LONG buflen = wcslen(buffer);
    HGLOBAL hclip;
    EmptyClipboard();
    hclip = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, (buflen + 1) * sizeof(WCHAR));
    wcsncpy((WCHAR *)GlobalLock(hclip), buffer, buflen + 1);
    GlobalUnlock(hclip);
    SetClipboardData(CF_UNICODETEXT, hclip);
    CloseClipboard();
    SHOE_FREE(buffer);
  }
}

VALUE
shoes_native_to_s(VALUE text)
{
  text = rb_funcall(text, s_to_s, 0);
  text = rb_funcall(text, s_gsub, 2, reLF, rb_str_new2("\r\n"));
  return text;
}

VALUE
shoes_native_window_color(shoes_app *app)
{
  DWORD winc = GetSysColor(COLOR_WINDOW);
  return shoes_color_new(GetRValue(winc), GetGValue(winc), GetBValue(winc), SHOES_COLOR_OPAQUE);
}

/* 
 *  TODO: the new functions since r1157 ;-)
 */

// calls from app.c functions. 

// Is this needed for winodws?
shoes_code shoes_native_app_open_menu(shoes_app *app, char *path, int dialog, shoes_settings *st) {
  fprintf(stderr, "shoes_native_app_open_menu called\n");
  return shoes_native_app_open(app, path, dialog, st);
}

// Is this needed for winodws?
void shoes_slot_init_menu(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width,
    int height, int scrolls, int toplevel) {
  fprintf(stderr, "shoes_slot_init_menu called\n");
  shoes_native_slot_init(c, parent, x, y, width, height, scrolls, toplevel);
}

void shoes_native_app_resize_window(shoes_app *app) {
}

void shoes_native_app_get_window_position(shoes_app *app) {
}

void shoes_native_app_window_move(shoes_app *app, int x, int y) {
}

void shoes_native_app_set_icon(shoes_app *app, char *icon_path) {
}

VALUE shoes_native_get_resizable(shoes_app *app) {
    return Qnil;
}

void shoes_native_set_resizable(shoes_app *app, int resizable) {
}

void shoes_native_app_fullscreen(shoes_app *app, char yn) {
}

void shoes_native_app_set_opacity(shoes_app *app, double opacity) {
}

double shoes_native_app_get_opacity(shoes_app *app) {
  return 1.0;
}

void shoes_native_app_set_decoration(shoes_app *app, gboolean decorated) {
}

int shoes_native_app_get_decoration(shoes_app *app) {
  return false;
}



