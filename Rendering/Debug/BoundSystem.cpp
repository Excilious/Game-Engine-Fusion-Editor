#include "BoundSystem.h"

BoundingBox BoundSystem::GetBoundingBoxes(const std::vector<glm::vec3>& Vertices, const glm::vec3& StartModelPosition)
{
	BoundingBox NewBounds;
	NewBounds.MinBounds = Vertices[0];
	NewBounds.MaxBounds = Vertices[0];
	this->EntityStartPosition = StartModelPosition;

	for (int Index = 0; Index < Vertices.size(); Index++)
	{
		if (Vertices[Index].x < NewBounds.MinBounds.x) { NewBounds.MinBounds.x = Vertices[Index].x; }
		if (Vertices[Index].x > NewBounds.MaxBounds.x) { NewBounds.MaxBounds.x = Vertices[Index].x; }
		if (Vertices[Index].y < NewBounds.MinBounds.y) { NewBounds.MinBounds.y = Vertices[Index].y; }
		if (Vertices[Index].y > NewBounds.MaxBounds.y) { NewBounds.MaxBounds.y = Vertices[Index].y; }
		if (Vertices[Index].z < NewBounds.MinBounds.z) { NewBounds.MinBounds.z = Vertices[Index].z; }
		if (Vertices[Index].z > NewBounds.MaxBounds.z) { NewBounds.MaxBounds.z = Vertices[Index].z; }
	}
	return NewBounds;
}

BoundingBox BoundSystem::UpdateBounds(const glm::vec3& NewModelPosition, BoundingBox CurrentBoundingBox)
{
	CurrentBoundingBox.MaxBounds += this->EntityStartPosition - NewModelPosition;
	CurrentBoundingBox.MinBounds += this->EntityStartPosition - NewModelPosition;
	this->EntityStartPosition = NewModelPosition;
	return CurrentBoundingBox;
}
