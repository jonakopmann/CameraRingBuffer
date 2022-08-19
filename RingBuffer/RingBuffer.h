#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#pragma once

#include <glib-2.0/glib-object.h>
#include <stdio.h>

G_BEGIN_DECLS

#define RING_TYPE_BUFFER (ring_buffer_get_type())

G_DECLARE_FINAL_TYPE (RingBuffer, ring_buffer, RING, BUFFER, GObject)

struct _RingBuffer
{
  gpointer buffer;
  gpointer head;
  gpointer tail;
  gpointer end;
  gsize capacity;
  gsize itemSize;
  gsize index;
};

RingBuffer* ring_buffer_new     (gsize capacity, gsize itemSize);

gboolean ring_buffer_add        (RingBuffer* self, const gpointer item);

void ring_buffer_advance        (RingBuffer* self, gsize count);

gpointer ring_buffer_get_write  (RingBuffer* self);

void ring_buffer_dispose        (RingBuffer* self);

G_END_DECLS

#endif
