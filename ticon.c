/** @file
 * small/ticon/ticon.c - Lua-configurable GTK tray icon
 *
 * @author Piotr S. Staszewski
 * @date 2014
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

// Configurable defines

#define ICON_KIND     "s"             // "s" for stock, "f" for file
#define ICON_PATH     "gtk-stop"      // GTK_STOCK string, or path to file
#define ICON_TOOLTIP  "Lua Tray Icon" // default tool-tip

// Internal defines

#define IDX_KIND    0
#define IDX_PATH    1
#define IDX_TOOLTIP 2

#define GLUE_NX(a, b) a##b
#define GLUE(a, b)    GLUE_NX(a, b)

#ifdef DEBUG
  #define dbg(code) code;
#else
  #define dbg(code)
#endif

// lua helpers (note: lua state has to be in var L)
#define LUA_CHKGLOBAL(key, type) \
  lua_getglobal(L, key); \
  if (GLUE(!lua_is, type)(L, -1))

#define LUA_CHKKEY(key, type) \
  lua_pushstring(L, key); \
  lua_gettable(L, -2); \
  if (GLUE(!lua_is, type)(L, -1))

// Private variables

static GtkWidget *menu, *item;
static GtkStatusIcon *icon;

// Public variables

lua_State *L;

// Private routines

static void die(const char *msg) {
  if (errno) {
    perror(msg);
    exit(2);
  } else {
    fprintf(stderr, "ABORT: %s\n", msg);
    exit(1);
  }
}

// tray icon click handler
static void handle_icon(GtkWidget *widget, GdkEvent *event, gpointer data) {
  GdkEventButton *evt = (GdkEventButton *)event;
  dbg(printf("DEBUG: handle icon, button = %d\n", evt->button));

  switch (evt->button) {
    case 1:
      gtk_menu_popup(GTK_MENU(menu), NULL, NULL, gtk_status_icon_position_menu, \
          icon, evt->button, evt->time);
      break;
    case 3:
      gtk_main_quit();
      break;
  }
}

// menu item click handler
static void handle_menu(gpointer data) {
  dbg(printf("DEBUG: handle menu, data = %d\n", GPOINTER_TO_INT(data)));
  lua_rawgeti(L, LUA_REGISTRYINDEX, GPOINTER_TO_INT(data));
  lua_pcall(L, 0, 0, 0);
}

// Main routine

int main(int argc, char *argv[]) {
  const char *strings[3];

  if (argc != 2) {
    fprintf(stderr, "Usage: %s script.lua\n", argv[0]);
    exit(1);
  }

  // setup GTK
  gtk_init(&argc, &argv);

  // setup Lua
  L = luaL_newstate();
  luaL_openlibs(L);
  luaL_dofile(L, argv[1]);

  // get tray icon config
  LUA_CHKGLOBAL("icon", table) {
    dbg(printf("WARN: No icon defined, using built-in\n"));
    strings[IDX_KIND] = ICON_KIND;
    strings[IDX_PATH] = ICON_PATH;
    strings[IDX_TOOLTIP] = ICON_TOOLTIP;
  } else {
    LUA_CHKKEY("kind", string) {
      dbg(printf("WARN: No icon kind, using built-in\n"));
      strings[IDX_KIND] = ICON_KIND;
      strings[IDX_PATH] = ICON_PATH;
    } else {
      strings[IDX_KIND] = lua_tostring(L, -1);
      if ((strcmp(strings[IDX_KIND], "stock") == 0) || \
          (strcmp(strings[IDX_KIND], "file") == 0)) {
        lua_pop(L, 1);

        LUA_CHKKEY("path", string) {
          dbg(printf("WARN: No icon path, using built-in\n"));
          strings[IDX_PATH] = ICON_PATH;
          strings[IDX_PATH] = ICON_PATH;
        } else strings[IDX_PATH] = lua_tostring(L, -1);
        lua_pop(L, 1);
      } else {
        dbg(printf("WARN: Unknown icon kind, using built-in\n"));
        strings[IDX_KIND] = "s";
        strings[IDX_PATH] = ICON_PATH;

        lua_pop(L, 1);
      }
    }

    LUA_CHKKEY("title", string) {
      dbg(printf("WARN: No title given, using built-in\n"));
      strings[IDX_TOOLTIP] = ICON_TOOLTIP;
    } else strings[IDX_TOOLTIP] = lua_tostring(L, -1);
  }
  lua_pop(L, 2);

  // setup tray icon
  if (strings[IDX_KIND][0] == 's')
    icon = gtk_status_icon_new_from_stock(strings[IDX_PATH]);
  else {
    if (access(strings[IDX_PATH], R_OK) != 0) die("Can't open icon file");
    icon = gtk_status_icon_new_from_file(strings[IDX_PATH]);
  }

  gtk_status_icon_set_tooltip_text(icon, strings[IDX_TOOLTIP]);
  gtk_status_icon_set_visible(icon, TRUE);
  g_signal_connect(icon, "button-release-event", G_CALLBACK(handle_icon), NULL);

  // parse config for and setup menu
  menu = gtk_menu_new();

  LUA_CHKGLOBAL("menu", table)
    die("No menu defined");
  else {
    lua_pushnil(L);
    while (lua_next(L, -2)) {
      if (!lua_isnumber(L, -2) || !lua_istable(L, -1)) {
        dbg(printf("WARN: Wrong menu format at element (1)"));
        lua_pop(L, 1);
      } else {
        lua_pushnil(L);
        if (lua_next(L, -2)) {
          if (!lua_isnumber(L, -2) || !lua_isstring(L, -1)) {
            dbg(printf("WARN: Wrong menu format at element (2)"));
            lua_pop(L, 3);
          } else {
            strings[0] = lua_tostring(L, -1);
            item = gtk_menu_item_new_with_label(strings[0]);
            gtk_widget_show(GTK_WIDGET(item));
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
            lua_pop(L, 1);

            if (lua_next(L, -2)) {
              if (!lua_isnumber(L, -2) || !lua_isfunction(L, -1)) {
                dbg(printf("WARN: Wrong menu format at element (3)"));
                lua_pop(L, 1);
              } else {
                g_signal_connect_swapped(item, "activate", G_CALLBACK(handle_menu), \
                    GINT_TO_POINTER(luaL_ref(L, LUA_REGISTRYINDEX)));

                if (lua_next(L, -2)) {
                  dbg(printf("WARN: Ignoring rest at element\n"));
                  lua_pop(L, 2);
                }

                lua_pop(L, 1);
              }
            }
          }
        }
      }
    }
  }

  gtk_main();
  lua_close(L);
  exit(0);
}

// vim: ts=2 et sw=2 sts=2
