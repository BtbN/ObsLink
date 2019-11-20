#include <iostream>

#include <obs.h>

void log_handler(int lvl, const char* msg, va_list args, void* opaque)
{
    vprintf(msg, args);
    printf("\n");
}

int main(int argc, char* argv[])
{
    base_set_log_handler(&log_handler, NULL);

    if (!obs_startup("en-US", NULL, NULL))
    {
        std::cout << "OBS startup failed." << std::endl;
        return -1;
    }

    obs_video_info vinfo = { 0 };
    vinfo.output_format = VIDEO_FORMAT_RGBA; //TODO: figure out best format
    vinfo.base_width = vinfo.output_width = 1920;
    vinfo.base_height = vinfo.output_height = 1080;
    vinfo.fps_num = 60;
    vinfo.fps_den = 1;
    vinfo.graphics_module = "libobs-d3d11";

    if (obs_reset_video(&vinfo) != OBS_VIDEO_SUCCESS)
    {
        std::cout << "OBS video reset failed." << std::endl;
        return -1;
    }

    obs_audio_info ainfo = { 0 };
    ainfo.samples_per_sec = 48000;
    ainfo.speakers = SPEAKERS_STEREO;

    if (!obs_reset_audio(&ainfo))
    {
        std::cout << "OBS audio reset failed." << std::endl;
        return -1;
    }

    return 0;
}
