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
#define MouseLeft          0
#define MouseMiddle        1
#define MouseRight         2
#define MouseWheelUp       3
#define MouseWheelDown     4
#define MouseWheelUpOnce   5
#define MouseWheelDownOnce 6

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
};

/*GLOBALS*/

int done = 0; /* for event loop */

xcb_connection_t *connection; /* pointer to XCB connection*/
xcb_screen_t *screen; /* number of screen to place the window on.  */
int scr_w;
int scr_h;
time_t last_time_of_execution_up[8]; /* last time a wheel event on a corner has been made */
time_t last_time_of_execution_down[8]; /* last time a wheel event on a corner has been made */
xcb_window_t windows[8];  /* pointer to the newly created window.      */
struct str_window_options window_options[8];

/* function prototypes */
int  can_execute (const int corner, time_t last_exec_array[]);
int  defined (const char str[]);
void config_add_entry (char* key, char* value);
int  config_parse_line (const char* line);
int  config_read ();
int  config_read_file (const char *file_path);
void fill_file(const char *file_path);
void init_options ();
void server_create_windows();
int  server_find_window(xcb_window_t win);
void server_event_loop ();
int  str_eq (const char* s1, const char* s2);


/* implementations */
void
server_create_windows()
{
  int i;
  uint32_t values[2] = {1, XCB_EVENT_MASK_BUTTON_PRESS};;
 
  for (i=0; i<8; i++) {
    if (!window_options[i].enabled) continue;
    windows[i] = xcb_generate_id (connection);
        
    /* InputOnly window to get the focus when no other window can get it */
    printf("Created a window - x:%d y:%d width:%d height:%d \n"
      ,window_options[i].x,window_options[i].y,window_options[i].h,window_options[i].w);
    xcb_create_window (connection, 0, windows[i], screen->root, window_options[i].x,
          window_options[i].y, window_options[i].w, window_options[i].h, 0,
          XCB_WINDOW_CLASS_INPUT_ONLY, screen->root_visual,
          XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, values );      

    /* Map the window on the screen */
    xcb_map_window (connection, windows[i]);

    xcb_flush (connection);
  }
}

int
server_find_window(xcb_window_t win)
{
  int i;
  for (i=0; i<8 ;i++) if (windows[i]==win) return i;
  return -1;
}

void
server_event_loop ()
{
  int cur_win;
  xcb_generic_event_t *event;
  
  while ((event = xcb_wait_for_event (connection))) {
    if ((event->response_type & ~0x80)==XCB_BUTTON_PRESS) {
      xcb_button_press_event_t *bp = (xcb_button_press_event_t *)event;
      cur_win = server_find_window(bp->event);
      /* printf("this event is coming from window %d \n", cur_win); */
      switch (bp->detail) {
      case 1: /*left button */
        if (defined(window_options[cur_win].commands[MouseLeft])) 
          system(window_options[cur_win].commands[MouseLeft]);
        break;
      case 2: /* middle button */
        if (defined(window_options[cur_win].commands[MouseMiddle])) 
          system(window_options[cur_win].commands[MouseMiddle]);
        break;
      case 3: /* right button */
        if (defined(window_options[cur_win].commands[MouseRight])) 
          system(window_options[cur_win].commands[MouseRight]);
        break;
      case 4: /*mouse wheel up*/
        if (defined(window_options[cur_win].commands[MouseWheelUp])) 
          system(window_options[cur_win].commands[MouseWheelUp]);
        if ( defined(window_options[cur_win].commands[MouseWheelUpOnce]) && can_execute(cur_win, last_time_of_execution_up) )
          system(window_options[cur_win].commands[MouseWheelUpOnce]);
        break;
      case 5: /*mouse wheel down*/
        if (defined(window_options[cur_win].commands[MouseWheelDown])) 
          system(window_options[cur_win].commands[MouseWheelDown]);
        if (defined(window_options[cur_win].commands[MouseWheelDownOnce]) && can_execute(cur_win, last_time_of_execution_down) )
          system(window_options[cur_win].commands[MouseWheelDownOnce]);
        break;
      }
    }
    free (event);
    if (done) break;
  }
}

int  defined (const char str[])
{
  if (strlen(str) > 0) return 1;
  return 0;
}

int
can_execute (const int corner, time_t last_exec_array[])
{
  time_t current_time;
  long int diff;
  
  time(&current_time);
  diff = (long int) current_time - (long int) last_exec_array[corner];
  
  if ( !last_exec_array[corner] || ( diff > 2 ) ) {
    last_exec_array[corner] = current_time;
    return 1;
  }
  
  return 0; 
}

void
init_options ()
{
  int i;
  for (i=0; i<8; i++) {
    window_options[i].enabled = 0;
    last_time_of_execution_up[i] = (time_t) 0;
    last_time_of_execution_down[i] = (time_t) 0;
  }
  window_options[TopLeft].w = 5;
  window_options[TopLeft].h = 5;
  window_options[TopCenter].w = scr_w*0.6;
  window_options[TopCenter].h = 2;
  window_options[TopRight].w = 6;
  window_options[TopRight].h = 5;
  window_options[Right].w = 3;
  window_options[Right].h = scr_h*0.6;
  window_options[BottomRight].w = 6;
  window_options[BottomRight].h = 6;
  window_options[BottomCenter].w = scr_w*0.6;
  window_options[BottomCenter].h = 2;
  window_options[BottomLeft].w = 5;
  window_options[BottomLeft].h = 5;
  window_options[Left].w = 2;
  window_options[Left].h = scr_h*0.6;
  
  window_options[TopLeft].x = 0;
  window_options[TopLeft].y = 0;
  window_options[TopCenter].x = (scr_w - window_options[TopCenter].w)/2;
  window_options[TopCenter].y = 0;
  window_options[TopRight].x = scr_w-5;
  window_options[TopRight].y = 0;
  window_options[Right].x = scr_w-2;
  window_options[Right].y = (scr_h - window_options[Right].h)/2;
  window_options[BottomRight].x = scr_w-5;
  window_options[BottomRight].y = scr_h-5;
  window_options[BottomCenter].x = (scr_w - window_options[BottomCenter].w)/2;
  window_options[BottomCenter].y = scr_h-2;
  window_options[BottomLeft].x = 0;
  window_options[BottomLeft].y = scr_h-5;
  window_options[Left].x = 0;
  window_options[Left].y = (scr_h - window_options[Left].h)/2;
}

int
str_eq (const char* s1, const char* s2)
{
  if (g_strcmp0(s1, s2) == 0) return 1;
  return 0;
}

void
config_add_entry (char* key, char* value)
{
  gchar** corner_and_button;
  int corner = -1;
  int button = -1;
  
  corner_and_button = g_strsplit(key, ".", 2); /* 0 is corner, 1 is button */
  
  if (str_eq(corner_and_button[0], "TopLeft"     )) corner = TopLeft     ; else
  if (str_eq(corner_and_button[0], "TopCenter"   )) corner = TopCenter   ; else
  if (str_eq(corner_and_button[0], "TopRight"    )) corner = TopRight    ; else
  if (str_eq(corner_and_button[0], "Right"       )) corner = Right       ; else
  if (str_eq(corner_and_button[0], "BottomRight" )) corner = BottomRight ; else
  if (str_eq(corner_and_button[0], "BottomCenter")) corner = BottomCenter; else
  if (str_eq(corner_and_button[0], "BottomLeft"  )) corner = BottomLeft  ; else
  if (str_eq(corner_and_button[0], "Left"        )) corner = Left;
  
  if (str_eq(corner_and_button[1], "LeftButton"    )) button = MouseLeft         ; else
  if (str_eq(corner_and_button[1], "MiddleButton"  )) button = MouseMiddle       ; else
  if (str_eq(corner_and_button[1], "RightButton"   )) button = MouseRight        ; else
  if (str_eq(corner_and_button[1], "WheelUp"       )) button = MouseWheelUp      ; else
  if (str_eq(corner_and_button[1], "WheelDown"     )) button = MouseWheelDown    ; else
  if (str_eq(corner_and_button[1], "WheelUpOnce"   )) button = MouseWheelUpOnce  ; else
  if (str_eq(corner_and_button[1], "WheelDownOnce" )) button = MouseWheelDownOnce;
  
  if (corner == -1 || button == -1) {
    printf("Error parsing key '%s' in the config", key);
  }
  
  window_options[corner].enabled = 1;
  strcpy(window_options[corner].commands[button], value);
  
  g_strfreev(corner_and_button);
}

int
config_parse_line (const char* line) 
{
  char *key, *value, *tmp_value;
  
  /* Skip useless lines */
  if (g_str_has_prefix(line, "#") || g_str_has_prefix(line, "\n")) return 0;
  if (!(tmp_value = strchr (line, '='))) return 0;
  
  gchar** key_and_value;
  key_and_value = g_strsplit(line, "=", 2); /* 0 is corner, 1 is button */
  
  key   = key_and_value[0];
  value = key_and_value[1];
  if (!key || !value) return 0;
  
  g_strstrip(key);
  g_strstrip(value);
  
  /* if the command has no & at the end - we add it :) */
  if (!g_str_has_suffix(value, "&")) {
    value = g_strdup_printf("%s &", value);
  }

  printf( " %s %s \n", key, value);
  config_add_entry(key, value);

  g_strfreev(key_and_value);
  
  return 1;
}

int 
config_read_file (const char *file_path)
{
  FILE *fp;
  char line[150];

  if ((fp = fopen(file_path, "r")) == NULL) return 0;
  
  printf("Reading & parsing config file... \n");
  while (fgets(line, sizeof(line), fp) != NULL)
    config_parse_line (line);
  
  printf("Done reading config file. \n");
  fclose (fp);
  return 1;
}

void 
fill_file(const char *file_path)
{
  FILE *fp;
  int  i;
   
  char* line1 = "#fittstoolrc example\n";
  char* line2 = "#volume control in the top right corner:\n";
  char* line3 = "TopRight.WheelUp     = amixer -q sset Master 2+\n";
  char* line4 = "TopRight.WheelDown   = amixer -q sset Master 2-\n";
  char* line5 = "TopRight.RightButton = amixer -q sset Master toggle\n";
  char* line6 = "TopRight.LeftButton  = xterm -C alsamixer\n";
  char* line7 = "#Available positions: Left, TopLeft, etc, TopCenter, BottomCenter, Right, TopRight, BottomRight, etc...\n";
  char* line8 = "#Available events: LeftButton, RightButton, MiddleButton, WheelUp, WheelDown, WheelUpOnce, WheelDownOnce \n";
  char* lines[8] = {line1, line2, line3, line4, line5, line6, line7, line8};

  fp = fopen(file_path, "wb");
  if (fp == NULL) return;

  for (i=0; i<8; i++) {
    fputs(lines[i], fp);
  }

  fclose (fp);
}

int 
config_read ()
{
  char *path1;
  gint i;

  /* check fittstoolrc in user directory */
  path1 = g_build_filename (g_get_user_config_dir(), "fittstool", "fittstoolrc", NULL);
  if (g_file_test (path1, G_FILE_TEST_EXISTS)) {
              i = config_read_file (path1);
              g_free(path1);
         return i;
  }

  char *dir = g_build_filename (g_get_user_config_dir(), "fittstool", NULL);
  if (!g_file_test (dir, G_FILE_TEST_IS_DIR)) g_mkdir(dir, 0777);
  g_free(dir);
  


  path1 = g_build_filename (g_get_user_config_dir(), "fittstool", "fittstoolrc", NULL);
  fill_file(path1);
  printf("Created a sample fittstoolrc for you in %s \n", path1);


  i = config_read_file (path1);
  g_free(path1);
  return i;
  
  return 0;
}

int
main(int argc, char* argv[])
{
   /* open connection to X server. */
  connection = xcb_connect (NULL, NULL);
  
  /* get screen*/
  screen = xcb_setup_roots_iterator (xcb_get_setup (connection)).data;
  scr_w = screen->width_in_pixels;
  scr_h = screen->height_in_pixels;
  
  init_options();
  config_read();
  
  /* create window */
  server_create_windows();
  
  /*event loop */
  server_event_loop();
  
  /*close connection to server */
  xcb_disconnect(connection);
  
  return 0;
}
