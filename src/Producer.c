#include <stdio.h>
#include <sys/ipc.h>

#include "Producer.h"

UcaCamera* uca_setup()
{
    UcaPluginManager* manager;
    UcaCamera* camera;
    UcaCameraClass* class;
    
    GError* error = NULL;
    
#if !(GLIB_CHECK_VERSION (2, 36, 0))
    g_type_init();
#endif

    manager = uca_plugin_manager_new();
    camera = uca_plugin_manager_get_camera(manager, "mock", &error, NULL);
    if (camera == NULL)
    {
        g_error ("Initialization: %s", error->message);
        return NULL;
    }
    class = UCA_CAMERA_GET_CLASS(camera);

    return camera;
}

gint uca_camera_get_buffer_size(UcaCamera* camera)
{
    // get buffer size
    guint width, height, bitdepth;
    guint pixelSize;

    g_object_get (camera,
        "roi-width", &width,
        "roi-height", &height,
        "sensor-bitdepth", &bitdepth,
        NULL);

    pixelSize = bitdepth <= 8 ? 1 : 2;

    return width * height * pixelSize;
}

gint main()
{
    // setup sem
    //sem_unlink(SEM_PRODUCER_FNAME);
    //sem_unlink(SEM_CONSUMER_FNAME);

    sem_t* semProd = get_sem(SEM_PRODUCER_FNAME, 0);
    sem_t* semCons = get_sem(SEM_CONSUMER_FNAME, 1);

    UcaCamera* camera = uca_setup();
    UcaCameraClass* class = UCA_CAMERA_GET_CLASS(camera);
    GError* error = NULL;

    gint size = BUFFER_CAPACITY * BUFFER_ITEM_SIZE;
    
    gpointer block = attach_memory_block(FILENAME, size + 2 * sizeof(glong));
    if (!block)
    {
        printf("couldnt get block");
        return -1;
    }

    glong* head = (glong*)block;
    block += sizeof(glong);

    glong* tail = (glong*)block;
    block += sizeof(glong);

    uca_camera_start_recording(camera, NULL);

    // copy data to block
    *head = 0;
    *tail = 0;
    for (gint i = 0; i < 13; i++)
    {
        *head += BUFFER_ITEM_SIZE;
        if (*head % size == 0)
        {
            sem_wait(semCons);
            *tail += BUFFER_ITEM_SIZE;
            sem_post(semCons);
        }
        
        if (!(*class->grab) (camera, block + (*head - BUFFER_ITEM_SIZE) % size, &error))
        {
            break;
        }
        
        sem_post(semProd);
        printf("wrote frame to buffer\n");
    }

    uca_camera_stop_recording(camera, NULL);

    sem_close(semProd);
    sem_close(semCons);
    g_object_unref(camera);
    return 0;
}