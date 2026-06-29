#include "Animation.h"

Animation::Animation(void) {};

template <typename T>
T Animation::InterpolateLinear(float T0,float T1,float T2,const T& Value0,const T& Value1)
{
    float Factor = (T0-T1)/(T2-T1);
    return glm::mix(Value0,Value1,Factor);
}

template <typename T>
T Animation::AnimationReadAccessorValue(const nlohmann::json& GLTFJson,const std::vector<uint8_t>& GLTFBuffer,int AccessorIndex,size_t Index)
{
    const auto& NewAccessor = GLTFJson["accessors"][AccessorIndex];
    const auto& BufferView  = GLTFJson["bufferViews"][NewAccessor["bufferView"].get<int>()];
    size_t Offset           = BufferView.value("byteOffset",0)+NewAccessor.value("byteOffset",0);
    size_t Stride           = sizeof(T);
    Offset                 += Index*Stride;
    return *reinterpret_cast<const T*>(&GLTFBuffer[Offset]);
}

glm::mat4 Animation::ComputeModelMatrix(const Node& NewNode)
{
    glm::mat4 Translation   = glm::translate(glm::mat4(1.0f),NewNode.Translation);
    glm::mat4 Rotation      = glm::mat4_cast(NewNode.Rotation);
    glm::mat4 Scale         = glm::scale(glm::mat4(1.0f),NewNode.Scale);
    return Translation*Rotation*Scale;
}

glm::mat4 Animation::ProduceMatrixModel(const Node& CurrentNewNode) {return this->ComputeModelMatrix(CurrentNewNode);}

AnimationNode Animation::LoadAnimation(const nlohmann::json& GLTFJson,const std::vector<uint8_t>& Buffer)
{
    AnimationNode NewAnimation;
    const auto& CurrentAnimation = GLTFJson["animations"][1];
  
    for (const auto& CurrentSampler: CurrentAnimation["samplers"])
    {
        Sampler NewSampler;

        int InputAccessor               = CurrentSampler["input"];
        int OutputAccessor              = CurrentSampler["output"];
        std::string Interpolate         = CurrentSampler.value("interpolation","LINEAR");

        const auto& NewInputAccessor    = GLTFJson["accessors"][InputAccessor];
        int Count                       = NewInputAccessor["count"];
        for (int Index = 0; Index < Count; Index++) {NewSampler.InputTimes.push_back(this->AnimationReadAccessorValue<float>(GLTFJson,Buffer,InputAccessor,Index));}

        const auto& NewOutputAccessor    = GLTFJson["accessors"][OutputAccessor];
        std::string NewType             = NewOutputAccessor["type"];
        int OutputCount                 = NewOutputAccessor["count"];

        if (NewType == "VEC3")
        {
            for (int Index = 0; Index < OutputCount; Index++)
            {NewSampler.OutputVector3.push_back(this->AnimationReadAccessorValue<glm::vec3>(GLTFJson,Buffer,OutputAccessor,Index));}
        }
        else if (NewType == "VEC4")
        {
            for (int Index = 0; Index < OutputCount; Index++)
            {NewSampler.OuputQuaternions.push_back(this->AnimationReadAccessorValue<glm::quat>(GLTFJson,Buffer,OutputAccessor,Index));}
        }
        else if (NewType == "SCALAR")
        {
            for (int Index = 0; Index < OutputCount; Index++)
            {NewSampler.OutputWeights.push_back(this->AnimationReadAccessorValue<float>(GLTFJson,Buffer,OutputAccessor,Index));}
        }
        NewSampler.Interpolation = Interpolate;
        NewAnimation.NewSampler.push_back(NewSampler);
    }

    for (const auto& CurrentChannel: CurrentAnimation["channels"])
    {
        Channel NewChannel;
        NewChannel.SamplerIndex = CurrentChannel["sampler"];
        NewChannel.Path         = CurrentChannel["target"]["path"];
        NewAnimation.NewChannel.push_back(NewChannel);
    }

    NewAnimation.Name = CurrentAnimation.value("name","UnnamedAnimation");
    return NewAnimation;
}

void Animation::ApplyAnimation(const AnimationNode& NewAnimationNode,float Time,Node& CurrentNode)
{
    for (size_t Index = 0; Index < NewAnimationNode.NewChannel.size(); Index++)
    {
        const Channel& NewChannel   = NewAnimationNode.NewChannel[Index];
        const Sampler& NewSampler   = NewAnimationNode.NewSampler[NewChannel.SamplerIndex];

        float MinimumTime   = NewSampler.InputTimes.front(); 
        float MaximumTime   = NewSampler.InputTimes.back(); 
        float Duration      = MaximumTime-MinimumTime;
        float Offset        = 0.05;

        float LocalTime     = fmod(Time,Duration)+(MinimumTime+Offset);
        size_t SampleFrame  = 0;

        while (SampleFrame+1 < NewSampler.InputTimes.size() && LocalTime >= NewSampler.InputTimes[SampleFrame+1]) {SampleFrame++;}
  
        float Time0 = NewSampler.InputTimes[SampleFrame];
        float Time1 = (SampleFrame+1 < NewSampler.InputTimes.size() ? NewSampler.InputTimes[SampleFrame+1] : Time0);

        if (NewChannel.Path == "translation")
        {
            glm::vec3 Vector0               = NewSampler.OutputVector3[SampleFrame];
            glm::vec3 Vector1               = NewSampler.OutputVector3[SampleFrame+1];
            CurrentNode.Translation         = (NewSampler.Interpolation == "LINEAR") ? this->InterpolateLinear(LocalTime,Time0,Time1,Vector0,Vector1) : Vector0;
        }
        else if (NewChannel.Path == "rotation")
        {
            glm::quat Quaternion0   = NewSampler.OuputQuaternions[SampleFrame];
            glm::quat Quaternion1   = NewSampler.OuputQuaternions[SampleFrame+1];
            CurrentNode.Rotation    = (NewSampler.Interpolation == "LINEAR") ? glm::slerp(Quaternion0,Quaternion1,(LocalTime-Time0)/(Time1-Time0)) : Quaternion0;
        }
        else if (NewChannel.Path == "scale")
        {
            glm::vec3 Scale0        = NewSampler.OutputVector3[SampleFrame];
            glm::vec3 Scale1        = NewSampler.OutputVector3[SampleFrame+1];
            CurrentNode.Scale       = (NewSampler.Interpolation == "LINEAR") ? this->InterpolateLinear(LocalTime,Time0,Time1,Scale0,Scale1) : Scale0;
        }
        else if (NewChannel.Path == "weights")
        {
            float Weight0               = NewSampler.OutputWeights[SampleFrame];
            float Weight1               = NewSampler.OutputWeights[SampleFrame+1];
            float Weight                = (NewSampler.Interpolation == "LINEAR") ? glm::mix(Weight0,Weight1,(LocalTime-Time0)/(Time1-Time0)) : Weight0;
            CurrentNode.Weights[0]      = Weight;
        }
    }
}
