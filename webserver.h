#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "html.h"
#include "tracker.h"
#include "analog.h"
#include "ads1115.h"
#include "post.h"
#include "mpu9250.h"
#include "app.h"

void webserverSetup();
void webserverLoop();

#endif