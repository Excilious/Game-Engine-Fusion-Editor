#ifndef __BOUNDS_HEADER__
#define __BOUNDS_HEADER__

#include <vector>
#include "../../../Resources/glm/glm.hpp"

struct BoundingBox
{
	glm::vec3 MinBounds = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 MaxBounds = glm::vec3(0.0f, 0.0f, 0.0f);
};

class BoundSystem
{
	public:
		BoundingBox GetBoundingBoxes(const std::vector<glm::vec3>& Vertices, const glm::vec3& StartModelPosition);
		BoundingBox UpdateBounds(const glm::vec3& NewModelPosition, BoundingBox CurrentBoundingBox);
		glm::vec3	EntityStartPosition;
};

#endif