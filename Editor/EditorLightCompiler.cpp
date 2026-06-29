#include "EditorLightCompiler.h"
#include "../Common/Console.h"
#include <iostream>

const int Width = 512;
const int Height = 512;

FusionLightCompiler::FusionLightCompiler(void)
{ 

}

bool FusionLightCompiler::RayAABB(const RayLine& ray, const glm::vec3& minB, const glm::vec3& maxB)
{
	float tmin = (minB.x - ray.origin.x) / ray.dir.x;
	float tmax = (maxB.x - ray.origin.x) / ray.dir.x;
	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (minB.y - ray.origin.y) / ray.dir.y;
	float tymax = (maxB.y - ray.origin.y) / ray.dir.y;
	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;

	float tzmin = (minB.z - ray.origin.z) / ray.dir.z;
	float tzmax = (maxB.z - ray.origin.z) / ray.dir.z;
	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	return true;
}

bool FusionLightCompiler::IntersectBVH(const BVHCollisionNode* node, const RayLine& ray, float maxDist)
{
	if (!node) return false;

	// First: test node bounds (YOU ARE MISSING THIS TOO — see next section)
	if (!RayAABB(ray, node->BoundingBox.MinimumBounds, node->BoundingBox.MaximumBounds))
		return false;

	// Leaf node
	if (!node->LeftPointer && !node->RightPointer)
	{
		for (size_t i = 0; i < node->ClusterBounds.size(); i++)
		{
			if (RayAABB(ray,
				node->ClusterBounds[i]->LocalBounds.MinimumBounds,
				node->ClusterBounds[i]->LocalBounds.MaximumBounds))
			{
				return true;
			}
		}
		return false;
	}

	// Recurse properly
	if (IntersectBVH(node->LeftPointer, ray, maxDist)) return true;
	if (IntersectBVH(node->RightPointer, ray, maxDist)) return true;

	return false;
}

bool FusionLightCompiler::InShadow(glm::vec3 start, glm::vec3 lightPos, const BVHCollisionNode* bvh)
{
	glm::vec3 dir = lightPos - start;
	float dist = sqrt(glm::dot(dir, dir));

	RayLine ray;
	ray.origin = start;
	ray.dir = glm::normalize(dir);

	return IntersectBVH(bvh, ray, dist);
}

glm::vec3 FusionLightCompiler::ComputeLighting(glm::vec3 pos, glm::vec3 normal, const std::vector<FusionLightStructure>& lights, const BVHCollisionNode* bvh)
{
	glm::vec3 result = { 0.1f, 0.1f, 0.1f };

	for (const FusionLightStructure& light : lights)
	{
		glm::vec3 L = glm::normalize(light.LightPosition - pos);
		float NdotL = std::max(glm::dot(normal, L), 0.0f);

		if (NdotL > 0.0f)
		{
			if (!this->InShadow(pos + normal * 0.01f, light.LightPosition, bvh))
			{
				float dist = sqrt(glm::dot(light.LightPosition - pos, light.LightPosition - pos));
				float attenuation = 1.0f / (1.0f + dist * dist);

				result = result + light.LightColour * (NdotL * attenuation);
			}
		}
	}

	return result;
}

void FusionLightCompiler::CompileToImage(std::vector<LightBlock> PositionToLight)
{
	const int Width = 512;
	const int Height = 512;

	for (int Index = 0; Index < PositionToLight.size(); Index++)
	{
		std::vector<unsigned char> Image(Width * Height * 3);

		unsigned char r = (unsigned char)(glm::clamp(PositionToLight[Index].Colour.x, 0.0f, 1.0f) * 255.0f);
		unsigned char g = (unsigned char)(glm::clamp(PositionToLight[Index].Colour.g, 0.0f, 1.0f) * 255.0f);
		unsigned char b = (unsigned char)(glm::clamp(PositionToLight[Index].Colour.b, 0.0f, 1.0f) * 255.0f);
	}
}

void FusionLightCompiler::SubmitLightSurfaces(const std::vector<FusionMeshBlock>& Blocks,const std::vector<FusionLightStructure>& lights, const BVHCollisionNode* bvh)
{
	Console::SysPrint("Compiling Lights...", 2);
	std::vector<LightBlock> PositionToLight = {};
	if (lights.size() == 0) { Console::SysPrint("No Lights!", 2); return; }

	for (int Index = 0; Index < Blocks.size(); Index++)
	{
		PositionToLight.push_back(
			{
				Blocks[Index].Position,
				this->ComputeLighting(Blocks[Index].Position,glm::vec3(0.0f,1.0f,0.0f),lights,bvh)
			}
		);
	}

	for (int NewIndex = 0; NewIndex < PositionToLight.size(); NewIndex++)
	{
		std::cout << "Position = (" << PositionToLight[NewIndex].Position.x << "," << PositionToLight[NewIndex].Position.y << "," << PositionToLight[NewIndex].Position.z << ")\n";
		std::cout << "Colour = (" << PositionToLight[NewIndex].Colour.x << "," << PositionToLight[NewIndex].Colour.y << "," << PositionToLight[NewIndex].Colour.z << ")\n";
	}
}