#include "RingBuffer.h"

G_DEFINE_TYPE(RingBuffer, ring_buffer, G_TYPE_OBJECT)

static void ring_buffer_class_init(RingBufferClass* klass)
{
}

static void ring_buffer_init(RingBuffer* klass)
{
}

RingBuffer* ring_buffer_new(gsize capacity, gsize itemSize)
{
    RingBuffer* self = g_malloc(sizeof(RingBuffer));
    
    self->buffer = g_malloc0(capacity * itemSize);
    self->end = self->buffer + capacity * itemSize;
    self->capacity = capacity;
    self->itemSize = itemSize;
    self->index = 0;
    self->head = self->buffer;
  
    return self;
}

void ring_buffer_dispose(RingBuffer* self)
{
    g_free(self->buffer);
    self->capacity = 0;
    self->index = 0;
}

gboolean ring_buffer_add(RingBuffer* self, const gpointer item)
{
    if (self == NULL)
    {
        return FALSE;
    }
    if (self->head == self->end)
    {
        self->head = self->buffer;
    }
    
    self->head = memcpy(self->head, item, self->itemSize);
    
    self->head += self->itemSize;
    self->index++;
    
    return TRUE;
}

gpointer ring_buffer_get(RingBuffer* self)
{
    return self->buffer + self->index * self->itemSize;
}

void ring_buffer_advance(RingBuffer* self, gsize count)
{
    for (gsize i = 0; i < count; i++)
    {
        self->head += self->itemSize;
        if (self->head == self->end)
            self->head = self->buffer;
    }
    self->index += count;
}

