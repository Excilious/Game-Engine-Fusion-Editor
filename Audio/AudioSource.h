#ifndef __AUDIO_SOURCE_HEADER__
#define __AUDIO_SOURCE_HEADER__

#include <vector>

#include "../../Resources/glm/glm.hpp"
#include "../../Resources/OpenAL/al.h"
#include "../Common/Console.h"

struct AudioSourceInfomation
{
    float               Pitch;
    float               Gain;
    bool                Loop;
    glm::vec3           Position;
    glm::vec3           Velocity;
    unsigned int        Buffer;
    unsigned int        Source;
};

// float                           Pitch = 1.0f;
// float                           Gain = 1.0f;
// bool                            LoopSound = false;
// glm::vec3                       Position = glm::vec3(0.0f,0.0f,0.0f);
// glm::vec3                       Velocity = glm::vec3(0.0f,0.0f,0.0f);
// unsigned int                    Buffer = 0;
// unsigned int                    Source = 0;

class AudioSource
{
    public:
                                        AudioSource(void);
        void                            Create(const AudioSourceInfomation& NewSourceInfomation);
        void                            Play(unsigned int BufferToPlay);
        void                            Destroy(void);
    private:
        AudioSourceInfomation           NewAudioSourceInfomation;
};

#endif