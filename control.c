#include "control.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

gsize BUFFER_CAPACITY = 0x1000;
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
    while (IsRecording)
    {
        if (!(*(params->cameraClass)->grab) (params->camera, ring_buffer_get_write(params->ringBuffer), &(params->error)))
        {
            IsRecording = FALSE;
            break;
        }
        FRAME_COUNT++;
    }
}

gpointer read_func(gpointer data)
{
    RingBuffer* ringBuffer = (RingBuffer*)data;

    gpointer test = ring_buffer_get_read(ringBuffer);
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
                  NULL);

    pixel_size = bitdepth <= 8 ? 1 : 2;
  
    RingBuffer* rb = ring_buffer_new(BUFFER_CAPACITY, width * height * pixel_size);
    
    uca_camera_start_recording(camera, NULL);
    IsRecording = TRUE;

    GThread* thread = g_thread_new("grab_frames", write_func, params_new(camera, class, rb, error));

    sleep(5);

    IsRecording = FALSE;
    g_thread_join(thread);
    uca_camera_stop_recording(camera, NULL);

    printf("%d\n", FRAME_COUNT / 5);

    // free camera and ringbuffer
    g_object_unref(camera);
    ring_buffer_free(rb);
    
    return 0;
}

