#pragma once
#include <stdint.h>

void s3xy_begin(const char* deviceName = "ENH_BTN");
void s3xy_loop();
bool s3xy_ready();

void s3xy_send_single();
void s3xy_send_long();
void s3xy_send_double();

void s3xy_set_id(const uint8_t id[10]);

typedef void (*s3xy_cb_t)();
void s3xy_on_connect(s3xy_cb_t cb);
void s3xy_on_disconnect(s3xy_cb_t cb);
