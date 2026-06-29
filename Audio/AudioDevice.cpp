#include "AudioDevice.h"

AudioDevice::AudioDevice(void): NewName(nullptr) {}

void AudioDevice::CreateAudio(void)
{
    this->ALCNewDevice  = alcOpenDevice(nullptr);
    this->ALCNewContext = alcCreateContext(this->ALCNewDevice,nullptr);

    if (!this->ALCNewDevice)                            {Console::SysPrint("Failed to get audio device.",2);}
    if (!this->ALCNewContext)                           {Console::SysPrint("Failed to load sound context.",2);}
    if (!alcMakeContextCurrent(this->ALCNewContext))    {Console::SysPrint("Failed to make the context current.",2);}

    if (alcIsExtensionPresent(this->ALCNewDevice,"ALC_ENUMERATE_ALL_EXT")) {this->NewName = alcGetString(this->ALCNewDevice,ALC_ALL_DEVICES_SPECIFIER);}
    if (!this->NewName || alcGetError(this->ALCNewDevice) != ALC_NO_ERROR) {this->NewName = alcGetString(this->ALCNewDevice,ALC_DEVICE_SPECIFIER);}
    Console::SysPrint("Using "+(std::string)this->NewName,1);
}

void AudioDevice::DestroyAudio(void)
{
    alcDestroyContext(this->ALCNewContext);
    if (!alcMakeContextCurrent(nullptr))        {Console::SysPrint("Failed to set context to null.",2);}
    if (this->ALCNewContext)                    {Console::SysPrint("Failed to unset during exit.",2);} 
    if (!alcCloseDevice(this->ALCNewDevice))    {Console::SysPrint("Failed to close sound device.",2);}
}