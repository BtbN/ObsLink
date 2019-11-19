#include <iostream>

#include <obs.h>

int main(int argc, char* argv[])
{
    if (!obs_startup("en-US", NULL, NULL))
    {
        std::cout << "OBS startup failed." << std::endl;
        return -1;
    }

    return 0;
}
