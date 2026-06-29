#include "EditorGrid.h"
#include "../../Resources/IconsFontAwesome5.h"
#include <cmath>

EditorGrid::EditorGrid() {}

// -----------------------
// Map 2D grid to 3D depending on current view
// -----------------------
glm::vec3 GridToWorld(const ImVec2& p, ViewMode mode)
{
    switch (mode)
    {
    case VIEW_TOP:    return glm::vec3(p.x, 0.0f, p.y); // XZ
    case VIEW_FRONT:  return glm::vec3(p.x, p.y, 0.0f); // XY
    case VIEW_SIDE:   return glm::vec3(0.0f, p.y, p.x); // ZY
    }
    return glm::vec3(0.0f);
}

ImVec2 WorldToGrid(const glm::vec3& v, ViewMode mode)
{
    switch (mode)
    {
    case VIEW_TOP:    return ImVec2(v.x, v.z);
    case VIEW_FRONT:  return ImVec2(v.x, v.y);
    case VIEW_SIDE:   return ImVec2(v.z, v.y);
    }
    return ImVec2(0, 0);
}

std::vector<float> EditorGrid::PositionToFloats(std::vector<BrushVertex> OldVertex)
{
    std::vector<float> Vertices;
    Vertices.reserve(OldVertex.size() * 8);

    float uvScale = 0.05f; // IMPORTANT

    for (int i = 0; i < OldVertex.size(); i++)
    {
        glm::vec3 pos = OldVertex[i].Position;

        glm::vec3 normal = glm::vec3(0.0f,1.0f,0.0f); // FIX: use real normal!
        glm::vec2 uv;

        // position
        Vertices.push_back(pos.x);
        Vertices.push_back(pos.y);
        Vertices.push_back(pos.z);

        glm::vec3 n = glm::abs(normal);

        if (n.x > n.y && n.x > n.z)
        {
            uv = glm::vec2(pos.y, pos.z);
        }
        else if (n.y > n.z)
        {
            uv = glm::vec2(pos.x, pos.z);
        }
        else
        {
            uv = glm::vec2(pos.x, pos.y);
        }

        uv *= uvScale; // 🔥 KEY FIX

        // normal
        Vertices.push_back(normal.x);
        Vertices.push_back(normal.y);
        Vertices.push_back(normal.z);

        // uv
        Vertices.push_back(uv.x);
        Vertices.push_back(uv.y);
    }

    return Vertices;
}

// -----------------------
// Generate 3D brush from 2D points
// -----------------------
void EditorGrid::GenerateSector(EditorToolbar* NewWorkspace,
    const ImVector<ImVec2>& Points,
    ViewMode currentView)
{
    if (Points.size() < 3) return;

    Sector NewSector;
    int count = (int)Points.size();

    for (auto& p : Points)
        NewSector.Vertices.push_back({ p });

    for (int i = 0; i < count; i++)
        NewSector.Edges.push_back({ i, (i + 1) % count });

    for (int i = 1; i < count - 1; i++)
    {
        NewSector.Indices.push_back(0);
        NewSector.Indices.push_back(i);
        NewSector.Indices.push_back(i + 1);
    }

    Sectors.push_back(NewSector);

    // -----------------------
    // 3D BRUSH
    // -----------------------
    Brush brush;

    float floor = 0.0f;
    float ceil = 128.0f;

    // bottom vertices
    for (auto& p : Points)
        brush.Vertices.push_back({ GridToWorld(p, currentView) + glm::vec3(0, floor, 0) });

    // top vertices
    for (auto& p : Points)
        brush.Vertices.push_back({ GridToWorld(p, currentView) + glm::vec3(0, ceil, 0) });

    int countV = (int)Points.size();

    // -----------------------
    // FLOOR (CCW)
    // -----------------------
    for (int i = 1; i < countV - 1; i++)
    {
        brush.Indices.push_back(0);
        brush.Indices.push_back(i);
        brush.Indices.push_back(i + 1);
    }

    // -----------------------
    // CEILING (FIXED WINDING)
    // -----------------------
    int offset = countV;

    for (int i = 1; i < countV - 1; i++)
    {
        brush.Indices.push_back(offset);
        brush.Indices.push_back(offset + i);
        brush.Indices.push_back(offset + i + 1);
    }

    // -----------------------
    // WALLS (FIXED CONSISTENT WINDING)
    // -----------------------
    for (int i = 0; i < countV; i++)
    {
        int next = (i + 1) % countV;

        unsigned int bottom0 = i;
        unsigned int bottom1 = next;
        unsigned int top0 = i + countV;
        unsigned int top1 = next + countV;

        // First triangle
        brush.Indices.push_back(bottom0);
        brush.Indices.push_back(top0);
        brush.Indices.push_back(top1);

        // Second triangle
        brush.Indices.push_back(bottom0);
        brush.Indices.push_back(top1);
        brush.Indices.push_back(bottom1);
    }

    // -----------------------
    // Spawn object
    // -----------------------
    NewWorkspace->SpawnObject(
        SHAPE_CUBE,
        "LineGeneratedModel",
        ICON_FA_CUBE,
        DATATYPE_BLOCK,
        glm::vec3(0.0f),
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f),
        this->PositionToFloats(brush.Vertices),
        brush.Indices
    );

    Brushes.push_back(brush);
}

// -----------------------
// Draw 2D grid editor
// -----------------------
void EditorGrid::GenerateGrid(EditorToolbar* NewWorkspace)
{
    ImGui::Begin("Grid System");

    // View selection
    static ViewMode currentView = VIEW_TOP;
    if (ImGui::Button("Top"))   currentView = VIEW_TOP;
    ImGui::SameLine();
    if (ImGui::Button("Front")) currentView = VIEW_FRONT;
    ImGui::SameLine();
    if (ImGui::Button("Side"))  currentView = VIEW_SIDE;

    // Grid & points
    static ImVector<glm::vec3> Points3D; // Store points in 3D
    static ImVec2 Scrolling(0.0f, 0.0f);

    const float GRID_STEP = 32.0f;
    const float CLOSE_DISTANCE = GRID_STEP * 0.5f;

    ImVec2 CanvasPos = ImGui::GetCursorScreenPos();
    ImVec2 CanvasSize = ImGui::GetContentRegionAvail();
    CanvasSize.x = std::max(CanvasSize.x, 50.0f);
    CanvasSize.y = std::max(CanvasSize.y, 50.0f);
    ImVec2 CanvasEnd = CanvasPos + CanvasSize;

    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    // Background
    draw->AddRectFilled(CanvasPos, CanvasEnd, IM_COL32(0, 0, 0, 255));
    draw->AddRect(CanvasPos, CanvasEnd, IM_COL32(255, 255, 255, 255));

    ImGui::InvisibleButton("Canvas", CanvasSize,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();

    ImVec2 Origin = CanvasPos + Scrolling;
    ImVec2 MouseRaw(io.MousePos.x - Origin.x, io.MousePos.y - Origin.y);

    // Snap mouse to grid
    auto Snap = [&](ImVec2 p) {
        return ImVec2(
            floorf(p.x / GRID_STEP + 0.5f) * GRID_STEP,
            floorf(p.y / GRID_STEP + 0.5f) * GRID_STEP
        );
        };
    ImVec2 Mouse2D = Snap(MouseRaw);

    // Convert 2D mouse to 3D depending on view
    glm::vec3 Mouse3D;
    switch (currentView)
    {
    case VIEW_TOP:   Mouse3D = glm::vec3(Mouse2D.x, 0.0f, Mouse2D.y); break;
    case VIEW_FRONT: Mouse3D = glm::vec3(Mouse2D.x, Mouse2D.y, 0.0f); break;
    case VIEW_SIDE:  Mouse3D = glm::vec3(0.0f, Mouse2D.y, Mouse2D.x); break;
    }

    // Left click: add points / close polygon
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (Points3D.empty())
        {
            Points3D.push_back(Mouse3D);
        }
        else
        {
            glm::vec3 first = Points3D[0];
            float dx = Mouse3D.x - first.x;
            float dy = Mouse3D.y - first.y;
            float dz = Mouse3D.z - first.z;
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);

            if (dist < CLOSE_DISTANCE)
            {
                // Generate sector and brush
                ImVector<ImVec2> Points2D;
                for (auto& p : Points3D)
                {
                    switch (currentView)
                    {
                    case VIEW_TOP:   Points2D.push_back(ImVec2(p.x, p.z)); break;
                    case VIEW_FRONT: Points2D.push_back(ImVec2(p.x, p.y)); break;
                    case VIEW_SIDE:  Points2D.push_back(ImVec2(p.z, p.y)); break;
                    }
                }
                this->GenerateSector(NewWorkspace, Points2D, currentView);
                Points3D.clear();
            }
            else
            {
                Points3D.push_back(Mouse3D);
            }
        }
    }

    // Right click: drag view
    if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        Scrolling.x += io.MouseDelta.x;
        Scrolling.y += io.MouseDelta.y;
    }

    // Draw grid lines
    for (float x = fmodf(Scrolling.x, GRID_STEP); x < CanvasSize.x; x += GRID_STEP)
        draw->AddLine(ImVec2(CanvasPos.x + x, CanvasPos.y), ImVec2(CanvasPos.x + x, CanvasEnd.y), CANVAS_GRID_LINE);
    for (float y = fmodf(Scrolling.y, GRID_STEP); y < CanvasSize.y; y += GRID_STEP)
        draw->AddLine(ImVec2(CanvasPos.x, CanvasPos.y + y), ImVec2(CanvasEnd.x, CanvasPos.y + y), CANVAS_GRID_LINE);

    draw->PushClipRect(CanvasPos, CanvasEnd, true);

    // Draw in-progress polygon
    for (int i = 0; i < Points3D.Size - 1; i++)
    {
        ImVec2 p0, p1;
        switch (currentView)
        {
        case VIEW_TOP:   p0 = ImVec2(Points3D[i].x, Points3D[i].z); p1 = ImVec2(Points3D[i + 1].x, Points3D[i + 1].z); break;
        case VIEW_FRONT: p0 = ImVec2(Points3D[i].x, Points3D[i].y); p1 = ImVec2(Points3D[i + 1].x, Points3D[i + 1].y); break;
        case VIEW_SIDE:  p0 = ImVec2(Points3D[i].z, Points3D[i].y); p1 = ImVec2(Points3D[i + 1].z, Points3D[i + 1].y); break;
        }
        draw->AddLine(Origin + p0, Origin + p1, DRAW_LINE, 2.0f);
    }

    // Draw line from last point to mouse
    if (!Points3D.empty())
    {
        ImVec2 p0;
        switch (currentView)
        {
        case VIEW_TOP:   p0 = ImVec2(Points3D.back().x, Points3D.back().z); break;
        case VIEW_FRONT: p0 = ImVec2(Points3D.back().x, Points3D.back().y); break;
        case VIEW_SIDE:  p0 = ImVec2(Points3D.back().z, Points3D.back().y); break;
        }
        draw->AddLine(Origin + p0, Origin + Mouse2D, IM_COL32(255, 255, 0, 255), 2.0f);
    }

    // Draw points
    for (auto& p : Points3D)
    {
        ImVec2 pt;
        switch (currentView)
        {
        case VIEW_TOP:   pt = ImVec2(p.x, p.z); break;
        case VIEW_FRONT: pt = ImVec2(p.x, p.y); break;
        case VIEW_SIDE:  pt = ImVec2(p.z, p.y); break;
        }
        draw->AddCircleFilled(Origin + pt, 5.0f, DRAW_CIRCLE_POINT);
    }

    // Highlight first point if near
    if (!Points3D.empty())
    {
        glm::vec3 first = Points3D[0];
        float dx = Mouse3D.x - first.x;
        float dy = Mouse3D.y - first.y;
        float dz = Mouse3D.z - first.z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        if (dist < CLOSE_DISTANCE)
        {
            ImVec2 pt;
            switch (currentView)
            {
            case VIEW_TOP:   pt = ImVec2(first.x, first.z); break;
            case VIEW_FRONT: pt = ImVec2(first.x, first.y); break;
            case VIEW_SIDE:  pt = ImVec2(first.z, first.y); break;
            }
            draw->AddCircleFilled(Origin + pt, 8.0f, IM_COL32(0, 255, 0, 255));
        }
    }

    // Draw all existing sectors projected to current view
    for (const Brush& brush : Brushes)
    {
        auto Project = [&](const glm::vec3& v) -> ImVec2
            {
                switch (currentView)
                {
                case VIEW_TOP:   return ImVec2(v.x, v.z);
                case VIEW_FRONT: return ImVec2(v.x, v.y);
                case VIEW_SIDE:  return ImVec2(v.z, v.y);
                }
                return ImVec2(0, 0);
            };

        // Draw edges
        for (size_t i = 0; i < brush.Indices.size(); i += 3)
        {
            ImVec2 p0 = Project(brush.Vertices[brush.Indices[i]].Position);
            ImVec2 p1 = Project(brush.Vertices[brush.Indices[i + 1]].Position);
            ImVec2 p2 = Project(brush.Vertices[brush.Indices[i + 2]].Position);
            draw->AddTriangleFilled(Origin + p0, Origin + p1, Origin + p2, IM_COL32(60, 60, 60, 255));
        }

        // Draw wire edges
        for (size_t i = 0; i < brush.Vertices.size(); i++)
        {
            for (size_t j = i + 1; j < brush.Vertices.size(); j++)
            {
                ImVec2 p0 = Project(brush.Vertices[i].Position);
                ImVec2 p1 = Project(brush.Vertices[j].Position);
                draw->AddLine(Origin + p0, Origin + p1, IM_COL32(255, 255, 255, 255), 2.0f);
            }
        }
    }

    draw->PopClipRect();
    ImGui::End();
}