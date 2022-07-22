#include "control.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

gsize BUFFER_CAPACITY = 0x1000;
gboolean IS_RECORDING = FALSE;
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

gpointer grab_func(gpointer data)
{
    Params* params = (Params*)data;
    while (IS_RECORDING)
    {
        gboolean result = (*(params->cameraClass)->grab) (params->camera, (params->ringBuffer)->head, &(params->error));
        ring_buffer_advance(params->ringBuffer, 1);
        FRAME_COUNT++;
    }
}

int main()
{
    struct timeval begin, end;
    UcaPluginManager* manager;
    UcaCamera* camera;
    UcaCameraClass* class;
    RingBuffer* rb;
    
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
  
    rb = ring_buffer_new(BUFFER_CAPACITY, width * height * pixel_size);
    
    uca_camera_start_recording(camera, NULL);
    IS_RECORDING = TRUE;

    GThread* thread = g_thread_new("grab_frames", grab_func, params_new(camera, class, rb, error));

    sleep(10);

    IS_RECORDING = FALSE;
    g_thread_join(thread);
    uca_camera_stop_recording(camera, NULL);

    printf("%d\n", FRAME_COUNT / 10);

    // dispose stuff
    g_object_unref(camera);
    ring_buffer_dispose(rb);
    
    return 0;
}

