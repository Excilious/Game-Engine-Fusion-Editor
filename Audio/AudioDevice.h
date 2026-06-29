#ifndef __AUDIO_DEVICE_HEADER__
#define __AUDIO_DEVICE_HEADER__

#define NOMINMAX

#include <windows.h>
#include <fstream>

#include "../../Resources/glad/glad.h"
#include "../../Resources/GLFW/glfw3.h"
#include "../../Resources/glm/glm.hpp"
#include "../../Resources/OpenAL/alc.h"
#include "../Common/Console.h"

class AudioDevice
{
    public:
                            AudioDevice(void);
        void                CreateAudio(void);
        void                DestroyAudio(void);
    private:
        ALCdevice*          ALCNewDevice;
        ALCcontext*         ALCNewContext;
        const char*         NewName;
};

#endif