#include "AudioSource.h"

AudioSource::AudioSource(void) {}

void AudioSource::Create(const AudioSourceInfomation& NewSourceInfomation)
{
    this->NewAudioSourceInfomation = NewSourceInfomation;
    alGenSources(1,&this->NewAudioSourceInfomation.Source);
    alSourcef(this->NewAudioSourceInfomation.Source,AL_PITCH,this->NewAudioSourceInfomation.Pitch);
    alSourcef(this->NewAudioSourceInfomation.Source,AL_GAIN,this->NewAudioSourceInfomation.Gain);
    alSource3f(this->NewAudioSourceInfomation.Source,AL_POSITION,this->NewAudioSourceInfomation.Position.x,this->NewAudioSourceInfomation.Position.y,this->NewAudioSourceInfomation.Position.z);
    alSource3f(this->NewAudioSourceInfomation.Source,AL_VELOCITY,this->NewAudioSourceInfomation.Velocity.x,this->NewAudioSourceInfomation.Velocity.y,this->NewAudioSourceInfomation.Velocity.z);
    alSourcei(this->NewAudioSourceInfomation.Source,AL_LOOPING,this->NewAudioSourceInfomation.Loop);
    alSourcei(this->NewAudioSourceInfomation.Source,AL_BUFFER,this->NewAudioSourceInfomation.Buffer);
}

void AudioSource::Play(unsigned int BufferToPlay)
{
    alSourceStop(this->NewAudioSourceInfomation.Source);
    if (BufferToPlay != this->NewAudioSourceInfomation.Buffer) {this->NewAudioSourceInfomation.Buffer = BufferToPlay; alSourcei(this->NewAudioSourceInfomation.Source,AL_BUFFER,this->NewAudioSourceInfomation.Buffer);}
    alSourcePlay(this->NewAudioSourceInfomation.Source);
    Console::SysPrint("Playing Audio...",1);
}


void AudioSource::Destroy(void)
{
    alDeleteSources(1,&this->NewAudioSourceInfomation.Source);
}