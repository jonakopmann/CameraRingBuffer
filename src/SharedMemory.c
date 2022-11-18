#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "SharedMemory.h"

sem_t* get_sem(gchar* name, gint initialValue)
{
    sem_t* sem = sem_open(name, O_CREAT, S_IRUSR|S_IWUSR, initialValue);
    if (sem == SEM_FAILED)
    {
        printf("couldnt open sem %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return sem;
}

static gint GetSharedBlock(gchar* fileName, gint size)
{
    key_t key;

    key = ftok(fileName, 0);

    if (key == IPC_RESULT_ERROR)
    {
        return IPC_RESULT_ERROR;
    }

    return shmget(key, size, 0644 | IPC_CREAT);
}

gpointer attach_memory_block(gchar* fileName, gint size)
{
    gint sharedBlockId = GetSharedBlock(fileName, size);
    gpointer result;

    if (sharedBlockId == IPC_RESULT_ERROR)
    {
        return NULL;
    }

    result = shmat(sharedBlockId, NULL, 0);
    if (result == (gpointer)IPC_RESULT_ERROR)
    {
        return NULL;
    }

    return result;
}

gboolean DetachMemoryBlock(gpointer block)
{
    return (shmdt(block) != IPC_RESULT_ERROR);
}

gboolean DestroyMemoryBlock(gchar* fileName)
{
    gint sharedBlockId = GetSharedBlock(fileName, 0);

    if (sharedBlockId == IPC_RESULT_ERROR)
    {
        return FALSE;
    }

    return (shmctl(sharedBlockId, IPC_RMID, NULL) != IPC_RESULT_ERROR);
}