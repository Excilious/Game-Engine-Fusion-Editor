#ifndef __AUDIO_BUFFER_HEADER__
#define __AUDIO_BUFFER_HEADER__

#include <string>
#include <vector>
#include <inttypes.h>

#include "../../Resources/OpenAL/al.h"
#include "../../Resources/OpenAL/alext.h"
#include "../../Resources/OpenAL/sndfile.h"
#include "../../Resources/OpenAL/alext.h"
#include "../Common/Console.h"

class AudioBuffer
{
    public:
                                AudioBuffer(void);
        static AudioBuffer*     GetBuffer(void);
        unsigned int            AddSoundEffect(const std::string &FileName);
        bool                    RemoveSoundEffect(const ALuint& NewBuffer);
        void                    Destroy(void);
    private:
        std::vector<ALuint>     SoundEffectBuffer;
};

#endif