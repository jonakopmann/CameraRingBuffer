#ifndef PRODUCER_H
#define PRODUCER_H

#include <uca/uca-plugin-manager.h>
#include <uca/uca-camera.h>
#include "SharedMemory.h"

UcaCamera* uca_setup();
gint uca_camera_get_buffer_size(UcaCamera* camera);

#endif