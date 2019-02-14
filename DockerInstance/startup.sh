#!/bin/bash
export DISPLAY=:1
Xvfb :1 -screen 0 800x600x24 &
sleep 5
openbox-session&
x11vnc -display :1 -nopw -listen localhost -localhost -xkb -noncache -forever &
cd /root/noVNC && ln -s vnc_auto.html index.html && ./utils/launch.sh --vnc localhost:5900
