#include <iostream>
#include <windows.h>
#include <obs.hpp>

static inline OBSData CreateOBSData()
{
    obs_data_t* settings = obs_data_create();
    OBSData res(settings);
    obs_data_release(settings);
    return res;
}

static void log_handler(int lvl, const char* msg, va_list args, void* opaque)
{
    vprintf(msg, args);
    printf("\n");
}

const int OUTPUT_WIDTH = 1920;
const int OUTPUT_HEIGHT = 1080;
const int OUTPUT_FPS = 60;

const int MONITOR_NUM = 0;
const int DEV_NUM = 0;
const int MODE_NUM = 0;

static bool init_obs()
{
    if (!obs_startup("en-US", NULL, NULL))
    {
        std::cout << "OBS startup failed." << std::endl;
        return false;
    }

    obs_video_info vinfo = { 0 };
    vinfo.output_format = VIDEO_FORMAT_RGBA; //TODO: figure out best format
    vinfo.base_width = vinfo.output_width = OUTPUT_WIDTH;
    vinfo.base_height = vinfo.output_height = OUTPUT_HEIGHT;
    vinfo.fps_num = OUTPUT_FPS;
    vinfo.fps_den = 1;
    vinfo.graphics_module = "libobs-d3d11";

    if (obs_reset_video(&vinfo) != OBS_VIDEO_SUCCESS)
    {
        std::cout << "OBS video reset failed." << std::endl;
        return false;
    }

    obs_audio_info ainfo = { 0 };
    ainfo.samples_per_sec = 48000;
    ainfo.speakers = SPEAKERS_STEREO;

    if (!obs_reset_audio(&ainfo))
    {
        std::cout << "OBS audio reset failed." << std::endl;
        return false;
    }

    obs_load_all_modules();

    return true;
}

static bool init_scene()
{
    OBSData monCaptureSettings = CreateOBSData();
    obs_data_set_int(monCaptureSettings, "monitor", MONITOR_NUM);
    obs_data_set_bool(monCaptureSettings, "capture_cursor", true);
    obs_data_set_bool(monCaptureSettings, "compatibility", false);

    obs_source_t *monCapture = obs_source_create("monitor_capture", "monitor_capture", monCaptureSettings, NULL);
    if (!monCapture)
    {
        std::cout << "Could not create capture source." << std::endl;
        return false;
    }

    obs_scene_t* scene = obs_scene_create("main_scene");
    if (!scene)
    {
        std::cout << "Could not create main scene." << std::endl;
        return false;
    }

    obs_sceneitem_t *item = obs_scene_add(scene, monCapture);
    if (!item)
    {
        std::cout << "Could not add source to scene." << std::endl;
        return false;
    }

    obs_sceneitem_set_bounds_type(item, OBS_BOUNDS_SCALE_INNER);
    obs_sceneitem_set_bounds_alignment(item, OBS_ALIGN_CENTER);

    vec2 bounds;
    vec2_set(&bounds, OUTPUT_WIDTH, OUTPUT_HEIGHT);
    obs_sceneitem_set_bounds(item, &bounds);

    obs_set_output_source(0, obs_scene_get_source(scene));

    return true;
}

static bool init_output()
{
    obs_properties_t *props = obs_get_output_properties("decklink_output");
    if (!props)
    {
        std::cout << "No decklink properties found." << std::endl;
        return false;
    }

    obs_property_t *deviceList = obs_properties_get(props, "device_hash");
    if (!deviceList)
    {
        obs_properties_destroy(props);
        std::cout << "No decklink device list found." << std::endl;
        return false;
    }

    if (obs_property_list_item_count(deviceList) <= DEV_NUM)
    {
        obs_properties_destroy(props);
        std::cout << "No decklink devices found." << std::endl;
        return false;
    }

    OBSData settings = CreateOBSData();

    std::cout << "Using decklink device " << obs_property_list_item_name(deviceList, DEV_NUM) << std::endl;
    const char *decklinkHash = obs_property_list_item_string(deviceList, DEV_NUM);
    obs_data_set_string(settings, "device_hash", decklinkHash);
    obs_property_modified(deviceList, settings);

    obs_property_t *modeList = obs_properties_get(props, "mode_id");
    if (!modeList)
    {
        obs_properties_destroy(props);
        std::cout << "No modelist found." << std::endl;
        return false;
    }

    if (obs_property_list_item_count(modeList) <= MODE_NUM)
    {
        obs_properties_destroy(props);
        std::cout << "No decklink modes found." << std::endl;
        return false;
    }

    std::cout << "Using decklink mode " << obs_property_list_item_name(modeList, MODE_NUM) << std::endl;
    long long modeId = obs_property_list_item_int(modeList, MODE_NUM);
    obs_data_set_int(settings, "mode_id", modeId);

    // 0 = Disabled, 1 = External, 2 = Internal
    obs_data_set_int(settings, "keyer", 0);

    obs_properties_destroy(props);

    obs_output_t *output = obs_output_create("decklink_output", "decklink_output", settings, NULL);
    if (!output)
    {
        std::cout << "Failed creating decklink output." << std::endl;
        return false;
    }

    return obs_output_start(output);
}

int main(int argc, char* argv[])
{
    base_set_log_handler(&log_handler, NULL);

    if (!init_obs())
        return -1;

    if (!init_scene())
        return -1;

    if (!init_output())
        return -1;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    obs_shutdown();

    return 0;
}
