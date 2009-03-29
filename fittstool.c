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
#define TopLeft      0
#define TopCenter    1
#define TopRight     2
#define Right        3
#define BottomRight  4
#define BottomCenter 5
#define BottomLeft   6
#define Left         7

/* INCLUDES */

#include <xcb/xcb.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>		/* getenv(), etc. */

/* STRUCTS */
struct str_window_options {
  char enabled;
  int x;
  int y;
  int h;
  int w;
  char lb_command[100];
  char mb_command[100];
  char rb_command[100];
  char wu_command[100];
  char wd_command[100];
};

/*GLOBALS*/

int done = 0; /* for event loop */

xcb_connection_t *connection;	/* pointer to XCB connection*/
xcb_screen_t *screen;	/* number of screen to place the window on.  */
int scr_w;
int scr_h;
xcb_window_t windows[8];	/* pointer to the newly created window.      */
struct str_window_options window_options[8];

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
        if (strlen(window_options[cur_win].lb_command)>0) 
          system(window_options[cur_win].lb_command);
        break;
      case 2: /* middle button */
        if (strlen(window_options[cur_win].mb_command)>0) 
          system(window_options[cur_win].mb_command);
        break;
      case 3: /* right button */
        if (strlen(window_options[cur_win].rb_command)>0) 
          system(window_options[cur_win].rb_command);
        break;
      case 4: /*mouse wheel up*/
        if (strlen(window_options[cur_win].wu_command)>0) 
          system(window_options[cur_win].wu_command);
        break;
      case 5: /*mouse wheel down*/
        if (strlen(window_options[cur_win].wd_command)>0) 
          system(window_options[cur_win].wd_command);
        break;
      }
    }
    free (event);
    if (done) break;
  }
}

void
init_options ()
{
  int i;
  for (i=0; i<8; i++) {
    window_options[i].enabled = 0;
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

void
config_add_entry (char* key, char* value)
{
  /* top left */
  if (strcmp (key, "TopLeft.LeftButton") == 0) {
    window_options[TopLeft].enabled=1;
    strcpy(window_options[TopLeft].lb_command, value);
  } else 
  if (strcmp (key, "TopLeft.MiddleButton") == 0) {
    window_options[TopLeft].enabled=1;
    strcpy(window_options[TopLeft].mb_command, value);
  } else 
  if (strcmp (key, "TopLeft.RightButton") == 0) {
    window_options[TopLeft].enabled=1;
    strcpy(window_options[TopLeft].rb_command, value);
  } else 
  if (strcmp (key, "TopLeft.WheelUp") == 0) {
    window_options[TopLeft].enabled=1;
    strcpy(window_options[TopLeft].wu_command, value);
  } else 
  if (strcmp (key, "TopLeft.WheelDown") == 0) {
    window_options[TopLeft].enabled=1;
    strcpy(window_options[TopLeft].wd_command, value);
  } else 
  /* top center */
  if (strcmp (key, "TopCenter.LeftButton") == 0) {
    window_options[TopCenter].enabled=1;
    strcpy(window_options[TopCenter].lb_command, value);
  } else 
  if (strcmp (key, "TopCenter.MiddleButton") == 0) {
    window_options[TopCenter].enabled=1;
    strcpy(window_options[TopCenter].mb_command, value);
  } else 
  if (strcmp (key, "TopCenter.RightButton") == 0) {
    window_options[TopCenter].enabled=1;
    strcpy(window_options[TopCenter].rb_command, value);
  } else 
  if (strcmp (key, "TopCenter.WheelUp") == 0) {
    window_options[TopCenter].enabled=1;
    strcpy(window_options[TopCenter].wu_command, value);
  } else 
  if (strcmp (key, "TopCenter.WheelDown") == 0) {
    window_options[TopCenter].enabled=1;
    strcpy(window_options[TopCenter].wd_command, value);
  } else 
  /* top right */
  if (strcmp (key, "TopRight.LeftButton") == 0) {
    window_options[TopRight].enabled=1;
    strcpy(window_options[TopRight].lb_command, value);
  } else 
  if (strcmp (key, "TopRight.MiddleButton") == 0) {
    window_options[TopRight].enabled=1;
    strcpy(window_options[TopRight].mb_command, value);
  } else 
  if (strcmp (key, "TopRight.RightButton") == 0) {
    window_options[TopRight].enabled=1;
    strcpy(window_options[TopRight].rb_command, value);
  } else 
  if (strcmp (key, "TopRight.WheelUp") == 0) {
    window_options[TopRight].enabled=1;
    strcpy(window_options[TopRight].wu_command, value);
  } else 
  if (strcmp (key, "TopRight.WheelDown") == 0) {
    window_options[TopRight].enabled=1;
    strcpy(window_options[TopRight].wd_command, value);
  } else 
  /* right */
  if (strcmp (key, "Right.LeftButton") == 0) {
    window_options[Right].enabled=1;
    strcpy(window_options[Right].lb_command, value);
  } else 
  if (strcmp (key, "Right.MiddleButton") == 0) {
    window_options[Right].enabled=1;
    strcpy(window_options[Right].mb_command, value);
  } else 
  if (strcmp (key, "Right.RightButton") == 0) {
    window_options[Right].enabled=1;
    strcpy(window_options[Right].rb_command, value);
  } else 
  if (strcmp (key, "Right.WheelUp") == 0) {
    window_options[Right].enabled=1;
    strcpy(window_options[Right].wu_command, value);
  } else 
  if (strcmp (key, "Right.WheelDown") == 0) {
    window_options[Right].enabled=1;
    strcpy(window_options[Right].wd_command, value);
  } else 
  /* bottom right */
  if (strcmp (key, "BottomRight.LeftButton") == 0) {
    window_options[BottomRight].enabled=1;
    strcpy(window_options[BottomRight].lb_command, value);
  } else 
  if (strcmp (key, "BottomRight.MiddleButton") == 0) {
    window_options[BottomRight].enabled=1;
    strcpy(window_options[BottomRight].mb_command, value);
  } else 
  if (strcmp (key, "BottomRight.RightButton") == 0) {
    window_options[BottomRight].enabled=1;
    strcpy(window_options[BottomRight].rb_command, value);
  } else 
  if (strcmp (key, "BottomRight.WheelUp") == 0) {
    window_options[BottomRight].enabled=1;
    strcpy(window_options[BottomRight].wu_command, value);
  } else 
  if (strcmp (key, "BottomRight.WheelDown") == 0) {
    window_options[BottomRight].enabled=1;
    strcpy(window_options[BottomRight].wd_command, value);
  } else 
  /* bottom center */
  if (strcmp (key, "BottomCenter.LeftButton") == 0) {
    window_options[BottomCenter].enabled=1;
    strcpy(window_options[BottomCenter].lb_command, value);
  } else 
  if (strcmp (key, "BottomCenter.MiddleButton") == 0) {
    window_options[BottomCenter].enabled=1;
    strcpy(window_options[BottomCenter].mb_command, value);
  } else 
  if (strcmp (key, "BottomCenter.RightButton") == 0) {
    window_options[BottomCenter].enabled=1;
    strcpy(window_options[BottomCenter].rb_command, value);
  } else 
  if (strcmp (key, "BottomCenter.WheelUp") == 0) {
    window_options[BottomCenter].enabled=1;
    strcpy(window_options[BottomCenter].wu_command, value);
  } else 
  if (strcmp (key, "BottomCenter.WheelDown") == 0) {
    window_options[BottomCenter].enabled=1;
    strcpy(window_options[BottomCenter].wd_command, value);
  } else 
  /* bottom left */
  if (strcmp (key, "BottomLeft.LeftButton") == 0) {
    window_options[BottomLeft].enabled=1;
    strcpy(window_options[BottomLeft].lb_command, value);
  } else 
  if (strcmp (key, "BottomLeft.MiddleButton") == 0) {
    window_options[BottomLeft].enabled=1;
    strcpy(window_options[BottomLeft].mb_command, value);
  } else 
  if (strcmp (key, "BottomLeft.RightButton") == 0) {
    window_options[BottomLeft].enabled=1;
    strcpy(window_options[BottomLeft].rb_command, value);
  } else 
  if (strcmp (key, "BottomLeft.WheelUp") == 0) {
    window_options[BottomLeft].enabled=1;
    strcpy(window_options[BottomLeft].wu_command, value);
  } else 
  if (strcmp (key, "BottomLeft.WheelDown") == 0) {
    window_options[BottomLeft].enabled=1;
    strcpy(window_options[BottomLeft].wd_command, value);
  } else 
  /* left */
  if (strcmp (key, "Left.LeftButton") == 0) {
    window_options[Left].enabled=1;
    strcpy(window_options[Left].lb_command, value);
  } else 
  if (strcmp (key, "Left.MiddleButton") == 0) {
    window_options[Left].enabled=1;
    strcpy(window_options[Left].mb_command, value);
  } else 
  if (strcmp (key, "Left.RightButton") == 0) {
    window_options[Left].enabled=1;
    strcpy(window_options[Left].rb_command, value);
  } else 
  if (strcmp (key, "Left.WheelUp") == 0) {
    window_options[Left].enabled=1;
    strcpy(window_options[Left].wu_command, value);
  } else 
  if (strcmp (key, "Left.WheelDown") == 0) {
    window_options[Left].enabled=1;
    strcpy(window_options[Left].wd_command, value);
  }
}

int
config_parse_line (const char* line) 
{
  char *a, *b, *key, *value, *tmp_value;
  char have_fo_free_value;

  /* Skip useless lines */
  if ((line[0] == '#') || (line[0] == '\n')) return 0;
  if (!(a = strchr (line, '='))) return 0;

  /* overwrite '=' with '\0' */
  a[0] = '\0';
  key = strdup (line);
  a++;

  /* overwrite '\n' with '\0' if '\n' present */
  if ((b = strchr (a, '\n'))) b[0] = '\0';

  value = strdup (a);

  g_strstrip(key);
  g_strstrip(value);

  /* if the command has no & at the end - we add it :) */
  have_fo_free_value = 1;
  
  if (!g_str_has_suffix(value, "&")) {
    tmp_value = g_strdup_printf("%s &", value);
    g_free (value);
    have_fo_free_value = 0;
    value = tmp_value;
  }
  
  
  config_add_entry(key, value);

  g_free (key);
  if (have_fo_free_value) g_free (value);
  if (!have_fo_free_value) g_free (tmp_value);
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
  char* line8 = "#Available events: LeftButton, RightButton, MiddleButton, WheelUp, WheelDown \n";
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
  //config_read_file();
  config_read();
  
  /* create window */
  server_create_windows();
  
  /*event loop */
  server_event_loop();
  
  /*close connection to server */
  xcb_disconnect(connection);
  
  return 0;
}
