#include "Control.h"
#include <stdio.h>
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

ReadParams* readParams_new (RingBuffer* ringBuffer, gint index)
{
    ReadParams* retVal = g_malloc(sizeof(ReadParams));

    retVal->ringBuffer = ringBuffer;
    retVal->index = index;
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
    
    for (gint i = 0; i < 13; i++)
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
    ReadParams* params = (ReadParams*)data;
    
    g_mkdir_with_parents("../Images", 0);

    for (gint i = 0; i < 4; i++)
    {
        gint size = i > 99 ? 18 : i > 9 ? 17 : 16;
        gchar* path = g_malloc(sizeof(gchar) * size);
        sprintf(path, "../Images/%d.raw", i);
        
        FILE* file;
        file = fopen(path, "w");

        gpointer test = NULL;
        while (!test)
        {
            test = ring_buffer_get_read(params->ringBuffer, params->index);
        }
        
        fwrite(test, params->ringBuffer->itemSize, 1, file);
        printf("wrote file\n");

        fclose(file);
    }
    
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

    printf("width: %d\n", width);
    printf("height: %d\n", height);
    printf("bitdepth: %d\n", bitdepth);

    g_object_set (camera,
        "frames-per-second", fps * 3,
        NULL);

    pixel_size = bitdepth <= 8 ? 1 : 2;
    RingBuffer* rb = ring_buffer_new(BUFFER_CAPACITY, width * height * pixel_size, 1);
    
    uca_camera_start_recording(camera, NULL);
    IsRecording = TRUE;

    GThread* writeThread = g_thread_new("write_frames", write_func, params_new(camera, class, rb, error));
    GThread* readThread = g_thread_new("read_frames", read_func, readParams_new(rb, 0));

    IsRecording = FALSE;
    g_thread_join(writeThread);
    g_thread_join(readThread);
    uca_camera_stop_recording(camera, NULL);

    // free camera and ringbuffer
    ring_buffer_free(rb);
    g_object_unref(camera);
    
    return 0;
}

