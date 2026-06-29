#ifndef __ANIMATION_HEADER__
#define __ANIMATION_HEADER__

#include <vector>
#include <string>

#include "../../Resources/glm/glm.hpp"
#include "../../Resources/glm/gtc/quaternion.hpp"
#include "../../Resources/glm/gtc/matrix_transform.hpp"
#include "../../Resources/jsonimport/json.h"
#include "../Common/Console.h"

struct Sampler
{
    std::vector<float>          InputTimes;
    std::string                 Interpolation;
    std::vector<glm::vec3>      OutputVector3;
    std::vector<glm::quat>      OuputQuaternions;
    std::vector<float>          OutputWeights;
};

struct Channel
{
    int             SamplerIndex;
    std::string     Path;
};

struct AnimationNode
{
    std::string             Name;
    std::vector<Sampler>    NewSampler;
    std::vector<Channel>    NewChannel;
};

struct Node
{
    glm::vec3           Translation;
    glm::quat           Rotation;
    glm::vec3           Scale;
    std::vector<float>  Weights;
};

class Animation
{
    public:
                                Animation(void);
        void                    ApplyAnimation(const AnimationNode& NewAnimationNode,float Time,Node& CurrentNode);

        AnimationNode           LoadAnimation(const nlohmann::json& GLTFJson,const std::vector<uint8_t>& Buffer);
        glm::mat4               ComputeModelMatrix(const Node& NewNode);
        glm::mat4               ProduceMatrixModel(const Node& CurrentNewNode);
    private:
        template <typename T> T InterpolateLinear(float T0,float T1,float T2,const T& Value0,const T& Value1);
        template <typename T> T AnimationReadAccessorValue(const nlohmann::json& GLTFJson,const std::vector<uint8_t>& GLTFBuffer,int AccessorIndex,size_t Index);
        Node                    NewNode;
};

#endif