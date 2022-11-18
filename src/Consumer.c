#include <math.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>

#include "Consumer.h"

gint main()
{
    // setup sem
    sem_unlink(SEM_PRODUCER_FNAME);
    sem_unlink(SEM_CONSUMER_FNAME);

    sem_t* semProd = get_sem(SEM_PRODUCER_FNAME, 0);
    sem_t* semCons = get_sem(SEM_CONSUMER_FNAME, 1);

    g_mkdir_with_parents("../Images", 0);
    
    gint size = BUFFER_CAPACITY * BUFFER_ITEM_SIZE;

    gpointer block = attach_memory_block(FILENAME, size + 2 * sizeof(glong));
    if (!block)
    {
        printf("couldnt get block\n");
        return -1;
    }
    
    // head
    block += sizeof(glong);

    // tail
    glong* tail = (glong*)block;
    block += sizeof(glong);


    for (gint i = 0; i < 13; i++)
    {
        gint size = i == 0 ? 1 : (int)log10(i) + 1;
        gchar* path = g_malloc(sizeof(gchar) * size);
        sprintf(path, "../Images/%d.raw", i);

        FILE* file;
        file = fopen(path, "w");
        
        // wait until data is in the buffer
        sem_wait(semProd);

        sem_wait(semCons);

        gpointer data = block + (*tail % size);
        
        gsize retCode = fwrite(data, BUFFER_ITEM_SIZE, 1, file);
        if (retCode == 1)
        {
            printf("wrote to file\n");
        }

        *tail += BUFFER_ITEM_SIZE;

        sem_post(semCons);
    }
    
    sem_close(semProd);
    sem_close(semCons);
}