#include "Control.h"
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

void writeToBuffer(gpointer buffer)
{
    gpointer ptr = g_malloc0(sizeof(u_int64_t));
    memcpy(buffer, ptr, sizeof(u_int64_t));

    g_free(ptr);
}

gpointer write_func(gpointer data)
{
    Params* params = (Params*)data;

    /*for (int i = 0; i < 10; i++)
    {
        g_mutex_lock(&params->ringBuffer->writeLock);
        printf("head%d: %lx\n", i, params->ringBuffer->head);
        gpointer writePtr = ring_buffer_get_write(params->ringBuffer);
        printf("ptr%d: %lx\n", i, writePtr);
        writeToBuffer(writePtr);
        sem_post(&params->ringBuffer->items);
        g_mutex_unlock(&params->ringBuffer->writeLock);
    }
    printf("start: %lx\n", params->ringBuffer->start);
    printf("end: %lx\n", params->ringBuffer->end);*/
    gint i;
    for (i = 0; i < 13; i++)
    {
        printf("write frame %d\n", i);
        Item* item = ring_buffer_get_write(params->ringBuffer);
        if (!(*(params->cameraClass)->grab) (params->camera, item->data, &(params->error)))
        {
            IsRecording = FALSE;
            item->isSetting = FALSE;
            break;
        }
        item->isSetting = FALSE;
        FRAME_COUNT++;
    }
    g_free(data);
}

gpointer read_func(gpointer data)
{
    RingBuffer* ringBuffer = (RingBuffer*)data;

    FILE* file;
    file = fopen("test.raw", "w");

    gpointer test = NULL;
    while (!test)
    {
        test = ring_buffer_get_read(ringBuffer, 0);
    }
    
    fwrite(test, ringBuffer->itemSize, 1, file);
    printf("wrote file\n");

    fclose(file);
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
    gboolean fillData;

    g_object_get (camera,
        "roi-width", &width,
        "roi-height", &height,
        "sensor-bitdepth", &bitdepth,
        "frames-per-second", &fps, // default is 20
        NULL);

    g_object_set (camera,
        "frames-per-second", fps * 3,
        NULL);

    pixel_size = bitdepth <= 8 ? 1 : 2;
    RingBuffer* rb = ring_buffer_new(BUFFER_CAPACITY, width * height * pixel_size, 1);
    
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
    ring_buffer_free(rb);
    g_object_unref(camera);
    
    return 0;
}

