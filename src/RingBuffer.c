#include "RingBuffer.h"

G_DEFINE_TYPE(RingBuffer, ring_buffer, G_TYPE_OBJECT)

static void ring_buffer_class_init(RingBufferClass* klass)
{
}

static void ring_buffer_init(RingBuffer* klass)
{
}

RingBuffer* ring_buffer_new(gsize capacity, gsize itemSize, gsize readerCount)
{
    RingBuffer* self = g_malloc(sizeof(RingBuffer));

    self->items = g_malloc(sizeof(Item) * capacity);
    self->data = g_malloc(capacity * itemSize);

    for (gint i = 0; i < capacity; i++)
    {
        self->items[i].data = self->data + (i * itemSize);
    }

    self->writeIndex = 0;
    self->readIndices = g_malloc0(readerCount * sizeof(gulong));

    self->capacity = capacity;
    self->itemSize = itemSize;

    return self;
}

void ring_buffer_free(RingBuffer* self)
{
    g_free(self->data);
    g_free(self->items);
    g_free(self->readIndices);
}

Item* ring_buffer_get_write(RingBuffer* self)
{
    glong idx = self->writeIndex++;

    idx %= self->capacity;
    self->items[idx].isSetting = TRUE;
    return &self->items[idx];
}

gpointer ring_buffer_get_read(RingBuffer* self, gsize index)
{
    glong idx = self->readIndices[index];
    idx %= self->capacity;

    if (self->readIndices[index] >= self->writeIndex || self->items[idx].isSetting)
    {
        return NULL;
    }
    glong diff = self->writeIndex - self->readIndices[index];
    if (diff> self->capacity)
    {
        self->readIndices[index] += diff - self->capacity;
        idx += diff - self->capacity;
        idx %= self->capacity;
    }

    self->readIndices[index]++;

    return self->items[idx].data;
}
