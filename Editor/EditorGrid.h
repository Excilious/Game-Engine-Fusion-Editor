#ifndef __EDITOR_GRID_HEADER__
#define __EDITOR_GRID_HEADER__

#define DRAW_LINE IM_COL32(150, 255, 150, 255)
#define DRAW_CIRCLE_POINT IM_COL32(0,255,0,255)
#define CANVAS_GRID_LINE IM_COL32(200,200,200,40)

#include <vector>
#include "../../Resources/imgui/imgui.h"
#include "../../Resources/glm/glm.hpp"
#include "../../Resources/imgui/ImGuizmo.h"
#include "EditorToolbar.h"

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b)
{
    return ImVec2(a.x + b.x, a.y + b.y);
}

struct EditorVertex
{
    ImVec2 Position;
};

struct Edge
{
    int Start;
    int End;
};

enum ViewMode
{
    VIEW_TOP,    // XZ
    VIEW_FRONT,  // XY
    VIEW_SIDE    // ZY
};

struct Sector
{
    std::vector<EditorVertex> Vertices;
    std::vector<Edge> Edges;
    std::vector<unsigned int> Indices;
};

// -----------------------
// 3D Brush for Quake-style editing
// -----------------------
struct BrushVertex
{
    glm::vec3 Position;
};

struct Brush
{
    std::vector<BrushVertex> Vertices;
    std::vector<unsigned int> Indices;
    glm::vec3 Position = glm::vec3(0.0f); // XYZ editing
};

class EditorGrid
{
public:
    EditorGrid();

    void GenerateGrid(EditorToolbar* NewWorkspace);
    void GenerateSector(EditorToolbar* NewWorkspace, const ImVector<ImVec2>& Points, ViewMode currentView);

private:
    std::vector<float> PositionToFloats(std::vector<BrushVertex> OldVertex);

    std::vector<Sector> Sectors;
    std::vector<Brush> Brushes;
    int SelectedBrush = -1;
};

#endif