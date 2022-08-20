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
    
    self->start = g_malloc0(capacity * itemSize);
    self->end = self->start + capacity * itemSize;
    self->head = self->start;
    self->tail = self->start;

    self->capacity = capacity;
    self->itemSize = itemSize;
    self->index = 0;
    self->flags = 0;

    g_mutex_init(&self->readLock);
    g_mutex_init(&self->writeLock);

    g_cond_init(&self->isNotEmpty);

    return self;
}

void ring_buffer_free(RingBuffer* self)
{
    g_free(self->start);

    // depracted idk need to look into what i need to use instead
    //g_mutex_free(&self->readLock);
    //g_mutex_free(&self->writeLock);

    //g_cond_free(&self->isNotEmpty);
}

gpointer ring_buffer_get_write(RingBuffer* self)
{
    g_mutex_lock(&self->writeLock);

    if (self->flags & 1)
    {
        // writing was faster than reading, so force advance read pointer
        g_mutex_lock(&self->readLock);

        ring_buffer_advance(self, FALSE);

        g_mutex_unlock(&self->readLock);

        // reset flag
        self->flags &= ~1;
    }

    ring_buffer_advance(self, TRUE);

    if (self->head == self->tail)
    {
        self->flags |= 1 << 0;
    }
    else
    {
        // clear flags
        self->flags &= 3;
    }

    g_cond_signal(&self->isNotEmpty);

    g_mutex_unlock(&self->writeLock);

    return self->head - self->itemSize;
}

gpointer ring_buffer_get_read(RingBuffer* self)
{
    g_mutex_lock(&(self->readLock));

    if (self->flags & 2)
    {
        // reading was faster than writing, we need to wait
        g_cond_wait(&self->isNotEmpty, &self->readLock);

        // reset flag
        self->flags &= ~2;
    }

    ring_buffer_advance(self, FALSE);

    if (self->tail == self->head)
    {
        self->flags |= 1 << 1;
    }
    else
    {
        // clear flags
        self->flags &= 3;
    }

    g_mutex_unlock(&self->readLock);

    return self->tail - self->itemSize;
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
