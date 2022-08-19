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

gpointer ring_buffer_get_write(RingBuffer* self)
{
    self->head += self->itemSize;
    if (self->head == self->end)
    {
        self->head = self->buffer;
        if (self->tail == self->head)
        {
            self->tail += self->itemSize;
        }
    }
    return self->head - self->itemSize;
}

void ring_buffer_advance(RingBuffer* self, gsize count)
{
    for (gsize i = 0; i < count; i++)
    {
        self->head += self->itemSize;
        if (self->head == self->end)
        {
            // write pointer reached end, write from start again
            self->head = self->buffer;

            if (self->tail == self->head)
            {
                // writing was faster than reading, so force advance read pointer
                self->tail += self->itemSize;
            }
        }
    }
}

