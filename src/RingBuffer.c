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
    self->canWrite = TRUE;
    self->canRead = FALSE;

    g_mutex_init(&self->readLock);
    g_mutex_init(&self->writeLock);

    sem_init(&self->items, 0, 0);

    return self;
}

void ring_buffer_free(RingBuffer* self)
{
    g_free(self->start);
    
    // depracted idk need to look into what i need to use instead
    printf("lock\n");
    g_mutex_free(&self->readLock);
    g_mutex_free(&self->writeLock);

    sem_destroy(&self->items);
}

gpointer ring_buffer_get_write(RingBuffer* self)
{
    g_mutex_lock(&self->writeLock);
    
    ring_buffer_advance(self, TRUE);
    printf("write\n");
    
    int* itemCount = g_malloc0(sizeof(int));
    sem_getvalue(&self->items, itemCount);
    if ((self->head == self->tail) && *itemCount)
    {
        // writing was faster than reading, so force advance read pointer
        printf("writing was faster\n");
        g_mutex_lock(&self->readLock);

        sem_wait(&self->items);

        ring_buffer_advance(self, FALSE);

        g_mutex_unlock(&self->readLock);
    }

    g_free(itemCount);
    
    g_mutex_lock(&self->writeLock);

    return self->head - self->itemSize;
}

gpointer ring_buffer_get_read(RingBuffer* self)
{
    g_mutex_lock(&self->readLock);

    sem_wait(&self->items);

    ring_buffer_advance(self, FALSE);

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
