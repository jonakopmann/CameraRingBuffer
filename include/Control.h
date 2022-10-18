#ifndef CONTROL_H
#define CONTROL_H

#pragma once

#include <glib-2.0/glib-object.h>
#include <uca/uca-plugin-manager.h>
#include <uca/uca-camera.h>
#include "RingBuffer.h"

G_BEGIN_DECLS

#define PARAMS_TYPE_ (params_get_type())

G_DECLARE_FINAL_TYPE (Params, params, PARAMS, PARAMS, GObject)

struct _Params
{
    UcaCamera* camera;
    UcaCameraClass* cameraClass;
    RingBuffer* ringBuffer;
    GError* error;
};

Params* params_new (UcaCamera* camera, UcaCameraClass* cameraClass, RingBuffer* ringBuffer, GError* error);

G_END_DECLS

#endif
