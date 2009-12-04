/**************************************************************************
*
* Fittstool : map mouse button events on screen corners to commands.
*
* Copyright (C) 2009 Yasen Atanasov (yasen.atanasov@gmail.com)
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**************************************************************************/

/* MACROS */

#define str_defined(str) ( (strlen(str) > 0) ? 1 : 0 )
#define unless(a)        if (!a)
#define get_cmd(win,cmd) window_options[win].commands[cmd]

/* CONSTANTS/OPTIONS */

/* Screen Corners */
#define TopLeft      0
#define TopCenter    1
#define TopRight     2
#define Right        3
#define BottomRight  4
#define BottomCenter 5
#define BottomLeft   6
#define Left         7

/* Mouse button indexes for commands */
#define LeftButton    0
#define MiddleButton  1
#define RightButton   2
#define WheelUp       3
#define WheelDown     4
#define WheelUpOnce   5
#define WheelDownOnce 6

/* INCLUDES */

#include <xcb/xcb.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>   /* getenv(), etc. */

/* STRUCTS */
struct str_window_options {
  char enabled;
  int x;
  int y;
  int h;
  int w;
  char commands[7][100];
  xcb_window_t xcb_window; /* pointer to the newly created window.      */
  time_t last_time_up; /* last time a wheel event on a corner has been made */
  time_t last_time_down; /* last time a wheel event on a corner has been made */
};

/*GLOBALS*/
struct str_window_options window_options[8];

/* function prototypes */
int  can_execute (const int corner, const int direction);
void config_read ();
void config_read_file (const char *file_path);
void fill_file(const char *file_path);
void init_options (const int screen_width, const int screen_height);
void server_create_windows(xcb_connection_t *connection, xcb_screen_t *screen);
int  server_find_window(xcb_window_t win);
void server_event_loop (xcb_connection_t *connection);

/* implementations */
void
server_create_windows(xcb_connection_t *connection, xcb_screen_t *screen)
{
  int i;
  uint32_t values[2] = {1, XCB_EVENT_MASK_BUTTON_PRESS};;
 
  for (i=0; i<8; i++) {
    unless (window_options[i].enabled) continue;
    window_options[i].xcb_window = xcb_generate_id (connection);
        
    /* InputOnly window to get the focus when no other window can get it */
    printf("Created a window - x:%d y:%d width:%d height:%d \n"
      ,window_options[i].x,window_options[i].y,window_options[i].h,window_options[i].w);
    xcb_create_window (connection, 0, window_options[i].xcb_window, screen->root, window_options[i].x,
          window_options[i].y, window_options[i].w, window_options[i].h, 0,
          XCB_WINDOW_CLASS_INPUT_ONLY, screen->root_visual,
          XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, values );      

    /* Map the window on the screen */
    xcb_map_window (connection, window_options[i].xcb_window);

    xcb_flush (connection);
  }
}

int
server_find_window(xcb_window_t win)
{
  int i;
  for (i=0; i<8 ;i++) if (window_options[i].xcb_window==win) return i;
  return -1;
}

void
server_event_loop (xcb_connection_t *connection)
{
  char done = 0;
  int cur_win;
  xcb_generic_event_t *event;
  
  while ((event = xcb_wait_for_event (connection))) {
    if ((event->response_type & ~0x80)==XCB_BUTTON_PRESS) {
      xcb_button_press_event_t *bp = (xcb_button_press_event_t *)event;
      cur_win = server_find_window(bp->event);
      /* printf("this event is coming from window %d \n", cur_win); */
      switch (bp->detail) {
      case 1: /*left button */
        if (str_defined(get_cmd(cur_win,LeftButton))) 
          system(get_cmd(cur_win,LeftButton));
        break;
      case 2: /* middle button */
        if (str_defined(get_cmd(cur_win,MiddleButton))) 
          system(get_cmd(cur_win,MiddleButton));
        break;
      case 3: /* right button */
        if (str_defined(get_cmd(cur_win,RightButton))) 
          system(get_cmd(cur_win,RightButton));
        break;
      case 4: /*mouse wheel up*/
        if (str_defined(get_cmd(cur_win,WheelUp))) 
          system(get_cmd(cur_win,WheelUp));
        if ( str_defined(get_cmd(cur_win,WheelUpOnce)) && can_execute(cur_win, 0) )
          system(get_cmd(cur_win,WheelUpOnce));
        break;
      case 5: /*mouse wheel down*/
        if (str_defined(get_cmd(cur_win,WheelDown))) 
          system(get_cmd(cur_win,WheelDown));
        if (str_defined(get_cmd(cur_win,WheelDownOnce)) && can_execute(cur_win, 1) )
          system(get_cmd(cur_win,WheelDownOnce));
        break;
      }
    }
    free (event);
    if (done) break;
  }
}

int
can_execute (const int corner, int direction)
{
  time_t current_time;
  time_t* last_exec;
  long int diff;
  
  unless (direction) last_exec = &window_options[corner].last_time_up; 
  else last_exec = &window_options[corner].last_time_down;
  
  time(&current_time);
  diff = (long int) current_time - (long int) *last_exec;
  
  if ( !last_exec || ( diff > 2 ) ) {
    *last_exec = current_time;
    return 1;
  }
  
  return 0; 
}

void
init_options (const int screen_width, const int screen_height)
{
  int i;
  for (i=0; i<8; i++) {
    window_options[i].enabled = 0;
    window_options[i].last_time_down = (time_t) 0;
    window_options[i].last_time_up = (time_t) 0;
  }
  window_options[TopLeft].w = 5;
  window_options[TopLeft].h = 5;
  window_options[TopCenter].w = screen_width*0.6;
  window_options[TopCenter].h = 2;
  window_options[TopRight].w = 6;
  window_options[TopRight].h = 5;
  window_options[Right].w = 3;
  window_options[Right].h = screen_height*0.6;
  window_options[BottomRight].w = 6;
  window_options[BottomRight].h = 6;
  window_options[BottomCenter].w = screen_width*0.6;
  window_options[BottomCenter].h = 2;
  window_options[BottomLeft].w = 5;
  window_options[BottomLeft].h = 5;
  window_options[Left].w = 2;
  window_options[Left].h = screen_height*0.6;
  
  window_options[TopLeft].x = 0;
  window_options[TopLeft].y = 0;
  window_options[TopCenter].x = (screen_width - window_options[TopCenter].w)/2;
  window_options[TopCenter].y = 0;
  window_options[TopRight].x = screen_width-5;
  window_options[TopRight].y = 0;
  window_options[Right].x = screen_width-2;
  window_options[Right].y = (screen_height - window_options[Right].h)/2;
  window_options[BottomRight].x = screen_width-5;
  window_options[BottomRight].y = screen_height-5;
  window_options[BottomCenter].x = (screen_width - window_options[BottomCenter].w)/2;
  window_options[BottomCenter].y = screen_height-2;
  window_options[BottomLeft].x = 0;
  window_options[BottomLeft].y = screen_height-5;
  window_options[Left].x = 0;
  window_options[Left].y = (screen_height - window_options[Left].h)/2;
}

void 
config_read_file (const char *file_path)
{
  GKeyFile* config_file;
  gchar* groups[] = {"TopLeft", "TopCenter", "TopRight", "Right", "BottomRight", "BottomCenter", "BottomLeft", "Left"};
  gchar* events[] = {"LeftButton", "MiddleButton", "RightButton", "WheelUp", "WheelDown", "WheelUpOnce", "WheelDownOnce"};
  gchar* current_value;
  int i,j;

  config_file = g_key_file_new();
  
  unless (g_key_file_load_from_file(config_file, file_path, G_KEY_FILE_NONE, NULL)) return;
  
  for (i=0; i<8; i++)
    if ( g_key_file_has_group(config_file, groups[i]) ) {
      window_options[i].enabled = 1;
      for (j=0; j<7; j++)
        if ( (current_value = g_key_file_get_value(config_file, groups[i], events[j], NULL)) ) {
          unless (g_str_has_suffix(current_value, "&"))
            current_value = g_strdup_printf("%s &", current_value);
            
          strcpy(window_options[i].commands[j], current_value);
          printf("%s %s : %s \n", groups[i], events[j], current_value);
        }
    }
}

void 
fill_file(const char *file_path)
{
  FILE *fp;
  int  i;
   
  char* lines[] = {
    "#fittstoolrc example\n",
    "#volume control in the top right corner:\n\n",
    "[TopRight]\n",
    "WheelUp=amixer -q sset Master 2+\n",
    "WheelDown=amixer -q sset Master 2-\n",
    "RightButton=amixer -q sset Master toggle\n",
    "LeftButton=xterm -C alsamixer\n\n\n",
    "#Available positions: Left, TopLeft, etc, TopCenter, BottomCenter, Right, TopRight, BottomRight, etc...\n",
    "#Available events: LeftButton, RightButton, MiddleButton, WheelUp, WheelDown, WheelUpOnce, WheelDownOnce \n"
  };

  fp = fopen(file_path, "wb");
  if (fp == NULL) return;

  for (i=0; i<9; i++) fputs(lines[i], fp);

  fclose (fp);
}

void 
config_read ()
{
  char *path;
  
  /* check fittstoolrc in user directory */
  path = g_build_filename (g_get_user_config_dir(), "fittstool", "fittstoolrc", NULL);
  if (g_file_test (path, G_FILE_TEST_EXISTS)) {
    config_read_file(path);
    g_free(path);
    return;
  }

  char *dir = g_build_filename (g_get_user_config_dir(), "fittstool", NULL);
  if (!g_file_test (dir, G_FILE_TEST_IS_DIR)) g_mkdir(dir, 0777);
  g_free(dir);

  path = g_build_filename (g_get_user_config_dir(), "fittstool", "fittstoolrc", NULL);
  fill_file(path);
  printf("Created a sample fittstoolrc for you in %s \n", path);

  config_read_file (path);
  g_free(path);
}

int
main(int argc, char* argv[])
{
  xcb_connection_t *connection; /* pointer to XCB connection*/
  xcb_screen_t *screen; /* number of screen to place the window on.  */
   /* open connection to X server. */
  connection = xcb_connect (NULL, NULL);
  
  /* get screen*/
  screen = xcb_setup_roots_iterator (xcb_get_setup (connection)).data;

  init_options(screen->width_in_pixels, screen->height_in_pixels);
  config_read();
  
  /* create window */
  server_create_windows(connection, screen);
  
  /*event loop */
  server_event_loop(connection);
  
  /*close connection to server */
  xcb_disconnect(connection);
  
  return 0;
}
