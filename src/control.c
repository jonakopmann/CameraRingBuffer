#include "control.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

gsize BUFFER_CAPACITY = 10;
gboolean IsRecording = FALSE;
gsize FRAME_COUNT = 0;

Params* params_new(UcaCamera* camera, UcaCameraClass* cameraClass, RingBuffer* ringBuffer, GError* error)
{
    Params* retVal = g_malloc(sizeof(Params));
    
    retVal->camera = camera;
    retVal->cameraClass = cameraClass;
    retVal->ringBuffer = ringBuffer;
    retVal->error = error;

    return retVal;
}

gpointer write_func(gpointer data)
{
    Params* params = (Params*)data;
    /*int* ptr = g_malloc(sizeof(int));
    *ptr = 13;
    for (int i = 0; i < 13; i++)
    {
        (*ptr)++;
        g_mutex_lock(&params->ringBuffer->writeLock);
        gpointer writePtr = ring_buffer_get_write(params->ringBuffer);
        g_memmove(writePtr, ptr, sizeof(int));

        sem_post(&params->ringBuffer->items);
        sleep(1);
        g_mutex_unlock(&params->ringBuffer->writeLock);
    }

    g_free(ptr);*/

    for (int i = 0; i < 5; i++)
    {
        printf("write frame %d\n", i);
        if (!(*(params->cameraClass)->grab) (params->camera, ring_buffer_get_write(params->ringBuffer), &(params->error)))
        {
            IsRecording = FALSE;
            break;
        }
        sem_post(&params->ringBuffer->items);
        FRAME_COUNT++;
    }
    g_free(data);
}

gpointer read_func(gpointer data)
{
    RingBuffer* ringBuffer = (RingBuffer*)data;
    //sleep(1);
    printf("read thread started\n");
    /*for (int i = 0; i < 3; i++)
    {
        gpointer test = ring_buffer_get_read(ringBuffer);
        printf("result: %lu\n", *(u_int64_t*)test);
    }*/
    gpointer test = ring_buffer_get_read(ringBuffer);
    FILE* file;

    file = fopen("test", "w");
    
    fwrite(test, ringBuffer->itemSize, 1, file);
}

int main()
{
    struct timeval begin, end;
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
        return 1;
    }
    class = UCA_CAMERA_GET_CLASS(camera);
    
    // get buffer size
    guint width, height, bitdepth;
    guint pixel_size;
    gdouble exposure_time;
    gdouble fps;

    g_object_get (camera,
        "roi-width", &width,
        "roi-height", &height,
        "sensor-bitdepth", &bitdepth,
        "frames-per-second", &fps, // default is 20
        NULL);

    g_object_set (camera,
        "frames-per-second", fps * 3,
        "fill-data", TRUE,
        NULL);

    pixel_size = bitdepth <= 8 ? 1 : 2;
  
    RingBuffer* rb = ring_buffer_new(BUFFER_CAPACITY, width * height * pixel_size);
    
    uca_camera_start_recording(camera, NULL);
    IsRecording = TRUE;

    GThread* writeThread = g_thread_new("write_frames", write_func, params_new(camera, class, rb, error));
    GThread* readThread = g_thread_new("read_frames", read_func, rb);

    //sleep(5);

    IsRecording = FALSE;
    g_thread_join(writeThread);
    g_thread_join(readThread);
    uca_camera_stop_recording(camera, NULL);

    //printf("%d\n", FRAME_COUNT / 5);

    // free camera and ringbuffer
    g_object_unref(camera);
    ring_buffer_free(rb);
    
    return 0;
}

