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
    printf("size: %d\n", capacity * itemSize);
    self->start = g_malloc(capacity * itemSize);
    self->end = self->start + capacity * itemSize;
    self->head = self->start;
    self->tail = self->start;

    self->capacity = capacity;
    self->itemSize = itemSize;

    g_mutex_init(&self->readLock);
    g_mutex_init(&self->writeLock);

    sem_init(&self->items, 0, 0);

    return self;
}

void ring_buffer_free(RingBuffer* self)
{
    g_free(self->start);
    
    // depracted idk need to look into what i need to use instead
    g_mutex_clear(&self->readLock);
    g_mutex_clear(&self->writeLock);

    sem_destroy(&self->items);
}

gpointer ring_buffer_get_write(RingBuffer* self)
{
    gpointer retVal = self->head;
    
    ring_buffer_advance(self, TRUE);
    
    int* itemCount = g_malloc0(sizeof(int));
    sem_getvalue(&self->items, itemCount);
    if ((self->head == self->tail) && *itemCount)
    {
        // writing was faster than reading, so force advance read pointer
        g_mutex_lock(&self->readLock);

        sem_wait(&self->items);

        ring_buffer_advance(self, FALSE);

        g_mutex_unlock(&self->readLock);
    }

    g_free(itemCount);

    return retVal;
}

gpointer ring_buffer_get_read(RingBuffer* self)
{
    gpointer retVal = self->head;
    
    sem_wait(&self->items);

    ring_buffer_advance(self, FALSE);

    return retVal;
}

void ring_buffer_advance(RingBuffer* self, gboolean isWrite)
{
    if (isWrite)
    {
        self->head += self->itemSize;

        if (self->head == self->end)
        {
            // ptr reached the end, go back to start
            self->head = self->start;
        }
    }
    else
    {
        self->tail += self->itemSize;

        if (self->tail == self->end)
        {
            // ptr reached the end, go back to start
            self->tail = self->start;
        }
    }
}
