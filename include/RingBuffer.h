#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#pragma once

#include <glib-2.0/glib-object.h>
#include <stdio.h>
#include <semaphore.h>

G_BEGIN_DECLS

#define RING_TYPE_BUFFER (ring_buffer_get_type())

G_DECLARE_FINAL_TYPE (RingBuffer, ring_buffer, RING, BUFFER, GObject)

struct _RingBuffer
{
  gpointer start;
  gpointer head;

  gpointer tail;
  gpointer end;
  
  gulong capacity;
  gulong itemSize;

  GMutex readLock;
  GMutex writeLock;
  
  sem_t items;
};

RingBuffer* ring_buffer_new     (gsize capacity, gsize itemSize);

gboolean ring_buffer_add        (RingBuffer* self, const gpointer item);

void ring_buffer_advance        (RingBuffer* self, gboolean iswrite);

gpointer ring_buffer_get_write  (RingBuffer* self);

gpointer ring_buffer_get_read   (RingBuffer* self);

void ring_buffer_free        (RingBuffer* self);

G_END_DECLS

#endif
