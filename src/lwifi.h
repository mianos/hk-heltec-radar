#pragma once
#include "display.h"

extern void wifi_connect(Display *display);
extern void load_settings();


extern char mqtt_server[40];
extern char mqtt_port[6];
extern char sensor_name[40];
extern char radar_module[30];
