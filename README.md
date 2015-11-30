This is a clone of the project [fittstool from google code](https://code.google.com/p/fittstool/). I made it because the project has been archived and I found no other source of it.

#DEPENDENCIES
xcb, glib

#INSTALL

type "make"
as root type "make install"

execute "fittstool"


#Usage


fittstool uses a config file which is usually located at ~/.config/fittstool/fittstoolrc

Upon first launch it will create a sample file which demonstrates how to bind the upper right screen corner for volume control with the mouse wheel.

That's why I advise you to start it in a terminal and check if it gives you the message that it has created the file for you, then stop it with Ctrl-C.

Configuration is done via a keyfile. You have groups in the format of:

[GroupName]

which correspond to screen corners, and event bindings in the form of:

LeftButton=somecommand

which correspond to events on that screen corner and point to the command/script to be executed upon the event being fired.

Here's how the example config file looks:

#fittstoolrc example
#volume control in the top right corner:

[TopRight]
WheelUp=amixer -q sset Master 2+
WheelDown=amixer -q sset Master 2-
RightButton=amixer -q sset Master toggle
LeftButton=xterm -C alsamixer


#Available positions: Left, TopLeft, etc, TopCenter, BottomCenter, Right, TopRight, BottomRight, etc...
#Available events: LeftButton, RightButton, MiddleButton, WheelUp, WheelDown, WheelUpOnce, WheelDownOnce, Enter, Leave 

I think it's simple enough for you to get it. For example you want to bind the lower right corner to launch the htop process manager for you. All you have to do is add this at the bottom of your fittstoolrc:

[BottomRight]
LeftButton=xterm -C htop

You can add fittstool to your session, if you're using a session manager, or to your .xinitrc, or whichever way you autostart your applications with the start of the X server.
