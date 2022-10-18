#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#pragma once

#include <glib-2.0/glib-object.h>
#include <stdio.h>
#include <stdatomic.h>

typedef struct _Item
{
  gpointer data;
  atomic_bool isSetting;
} Item;

G_BEGIN_DECLS

#define RING_TYPE_BUFFER (ring_buffer_get_type())

G_DECLARE_FINAL_TYPE (RingBuffer, ring_buffer, RING, BUFFER, GObject)

struct _RingBuffer
{
  Item* items;
  gpointer data;

  gulong capacity;
  gulong itemSize;

  atomic_long writeIndex;
  atomic_long maxReadIndex;
  gulong* readIndices;
};

RingBuffer* ring_buffer_new     (gsize capacity, gsize itemSize, gsize readerCount);

Item* ring_buffer_get_write     (RingBuffer* self);

gpointer ring_buffer_get_read   (RingBuffer* self, gsize index);

void ring_buffer_free           (RingBuffer* self);

G_END_DECLS

#endif
