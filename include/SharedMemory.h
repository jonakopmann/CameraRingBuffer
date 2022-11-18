/*

Layout of shared memory ringbuffer:

| head | tail |          data          |

*/

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <semaphore.h>
#include <glib-2.0/glib-object.h>

sem_t* get_sem(gchar* name, gint initialValue);
gpointer attach_memory_block(gchar* fileName, gint size);
gboolean DetachMemoryBlock(gpointer block);
gboolean DestroyMemoryBlock(gchar* fileName);

#define FILENAME "../include/SharedMemory.h"
#define IPC_RESULT_ERROR (-1)
#define BUFFER_CAPACITY (10)
#define BUFFER_ITEM_SIZE (512 * 512 * 1)

#define SEM_PRODUCER_FNAME "/producer"
#define SEM_CONSUMER_FNAME "/consumer"

#endif