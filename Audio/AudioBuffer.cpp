#include "AudioBuffer.h"

AudioBuffer::AudioBuffer(void)              {this->SoundEffectBuffer.clear();}
AudioBuffer* AudioBuffer::GetBuffer(void)   {static AudioBuffer* NewBuffer = new AudioBuffer(); return NewBuffer;}

unsigned int AudioBuffer::AddSoundEffect(const std::string &FileName)
{
    int             Error,Format;
    unsigned int    Buffer;
    SNDFILE*        SoundFile;
    SF_INFO         SoundInfo;
    sf_count_t      NumberFrames;
    int             NumberBytes;
    short*          MemoryBuffer;

    SoundFile = sf_open(FileName.c_str(),SFM_READ,&SoundInfo);
    if (!SoundFile)                                                                                         {Console::SysPrint("Couldnt open file: "+FileName,3); return -1;}
    if (SoundInfo.frames < 1 || SoundInfo.frames > (sf_count_t)(INT_MAX/sizeof(short))/SoundInfo.channels)  {Console::SysPrint("Bad sample count.",3); sf_close(SoundFile); return 0;}

    Format = AL_NONE;
    if (SoundInfo.channels == 1)        {Format = AL_FORMAT_MONO16;}
    else if (SoundInfo.channels == 2)   {Format = AL_FORMAT_STEREO16;}
    else if (SoundInfo.channels == 3)   {if (sf_command(SoundFile,SFC_WAVEX_GET_AMBISONIC,NULL,0) == SF_AMBISONIC_B_FORMAT) {Format = AL_FORMAT_BFORMAT2D_16;}}
    else if (SoundInfo.channels == 4)   {if (sf_command(SoundFile,SFC_WAVEX_GET_AMBISONIC,NULL,0) == SF_AMBISONIC_B_FORMAT) {Format = AL_FORMAT_BFORMAT3D_16;}}
    if (!Format)                        {Console::SysPrint("Unsupported channel count! channels: "+std::to_string(SoundInfo.channels),2); sf_close(SoundFile); return 0;}

    MemoryBuffer    = static_cast<short*>(malloc((size_t)(SoundInfo.frames*SoundInfo.channels)*sizeof(short)));
    NumberFrames    = sf_readf_short(SoundFile,MemoryBuffer,SoundInfo.frames);

    if (NumberFrames < 1)
    {
        free(MemoryBuffer);
        sf_close(SoundFile);
        Console::SysPrint("Failed to read samples in "+FileName+" of "+std::to_string(NumberFrames),3);
        return 0;
    }

    NumberBytes     = (ALsizei)(NumberFrames*SoundInfo.channels)*(ALsizei)sizeof(short);
    Buffer          = 0;

    alGenBuffers(1,&Buffer);
    if (!alIsBuffer(Buffer)) {Console::SysPrint("Invalid buffer generated: "+std::to_string(Buffer), 3); return 0;}

    alBufferData(Buffer,Format,MemoryBuffer,NumberBytes,SoundInfo.samplerate);
    free(MemoryBuffer);
    sf_close(SoundFile);

    Error = alGetError();
    if (Error != AL_NO_ERROR)
    {
        Console::SysPrint("OpenAL code: "+std::to_string(Error),3);
        Console::SysPrint("OpenAL error: "+(std::string)alGetString(Error),3);
        if (Buffer && alIsBuffer(Buffer)) {alDeleteBuffers(1,&Buffer); return 0;}
    }
    this->SoundEffectBuffer.push_back(Buffer);
    return Buffer;
}

bool AudioBuffer::RemoveSoundEffect(const ALuint& NewBuffer)
{
    auto BeginBuffer = this->SoundEffectBuffer.begin();
    while (BeginBuffer != this->SoundEffectBuffer.end())
    {
        if (*BeginBuffer == NewBuffer)
        {
            alDeleteBuffers(1,&*BeginBuffer);
            BeginBuffer = this->SoundEffectBuffer.erase(BeginBuffer);
            return true;
        }
        else {BeginBuffer++;}
    }
    return false;
}

void AudioBuffer::Destroy(void)
{
    alDeleteBuffers(this->SoundEffectBuffer.size(),this->SoundEffectBuffer.data());
    this->SoundEffectBuffer.clear();
}