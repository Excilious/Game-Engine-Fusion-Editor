#include "EditorCompiler.h"
#include "../Common/Console.h"

EditorCompiler::EditorCompiler(void) {}

bool AABBContains(const ObjectBounds& p, const ObjectBounds& c)
{
    return
        c.MinimumBounds.x >= p.MinimumBounds.x &&
        c.MinimumBounds.y >= p.MinimumBounds.y &&
        c.MinimumBounds.z >= p.MinimumBounds.z &&
        c.MaximumBounds.x <= p.MaximumBounds.x &&
        c.MaximumBounds.y <= p.MaximumBounds.y &&
        c.MaximumBounds.z <= p.MaximumBounds.z;
}

glm::vec3 Center(const ObjectBounds& Bounds)
{
    return glm::vec3(
        (Bounds.MinimumBounds.x + Bounds.MaximumBounds.x) * 0.5f,
        (Bounds.MinimumBounds.y + Bounds.MaximumBounds.y) * 0.5f,
        (Bounds.MinimumBounds.z + Bounds.MaximumBounds.z) * 0.5f
    );
}

std::vector<glm::vec3> EditorCompiler::VerticesToPosition(const std::vector<float>& Vertices, const glm::mat4& ModelMatrix, size_t Stride) 
{ 
    std::vector<glm::vec3> PositionVertices; 
    
    for (int Index = 0; Index < Vertices.size(); Index += Stride) 
    { 
        PositionVertices.push_back(
            glm::vec3(
                Vertices[Index] * ModelMatrix[0][0] + Vertices[Index + 1] * ModelMatrix[0][1] + Vertices[Index + 2] * ModelMatrix[0][2] + ModelMatrix[0][3], 
                Vertices[Index] * ModelMatrix[1][0] + Vertices[Index + 1] * ModelMatrix[1][1] + Vertices[Index + 2] * ModelMatrix[1][2] + ModelMatrix[1][3], 
                Vertices[Index] * ModelMatrix[2][0] + Vertices[Index + 1] * ModelMatrix[2][1] + Vertices[Index + 2] * ModelMatrix[2][2] + ModelMatrix[2][3]
            )
        ); 
    } 
    return PositionVertices; 
}


ObjectBounds EditorCompiler::GenerateNewBounds(const std::vector<FusionVertex>& PositionVertices, const glm::vec3& Position)
{
    ObjectBounds NewBounds;
    NewBounds.MaximumBounds = PositionVertices[0].Position;
    NewBounds.MinimumBounds = PositionVertices[0].Position;
    NewBounds.BoundCenter = Position;

    for (int Index = 0; Index < PositionVertices.size(); Index++)
    {
        NewBounds.MinimumBounds.x = std::min(NewBounds.MinimumBounds.x, PositionVertices[Index].Position.x);
        NewBounds.MinimumBounds.y = std::min(NewBounds.MinimumBounds.y, PositionVertices[Index].Position.y);
        NewBounds.MinimumBounds.z = std::min(NewBounds.MinimumBounds.z, PositionVertices[Index].Position.z);

        NewBounds.MaximumBounds.x = std::max(NewBounds.MaximumBounds.x, PositionVertices[Index].Position.x);
        NewBounds.MaximumBounds.y = std::max(NewBounds.MaximumBounds.y, PositionVertices[Index].Position.y);
        NewBounds.MaximumBounds.z = std::max(NewBounds.MaximumBounds.z, PositionVertices[Index].Position.z);
    }

    NewBounds.MinimumBounds -= glm::vec3(BOUNDS_EPSILON);
    NewBounds.MaximumBounds += glm::vec3(BOUNDS_EPSILON);

    return NewBounds;
}

ObjectBounds EditorCompiler::BuildUnion(ObjectBounds BoundsA, ObjectBounds BoundsB)
{
    return {
        Center(BoundsA),
        glm::vec3(std::min(BoundsA.MinimumBounds.x,BoundsB.MinimumBounds.x), std::min(BoundsA.MinimumBounds.y,BoundsB.MinimumBounds.y), std::min(BoundsA.MinimumBounds.z,BoundsB.MinimumBounds.z)),
        glm::vec3(std::max(BoundsA.MaximumBounds.x,BoundsB.MaximumBounds.x), std::max(BoundsA.MaximumBounds.y,BoundsB.MaximumBounds.y), std::max(BoundsA.MaximumBounds.z,BoundsB.MaximumBounds.z)),
    };
}

void PrintBVH(BVHCollisionNode* Node, int Depth = 0)
{
    for (int Index = 0; Index < Depth; Index++) Console::SysPrint("  ", 1);

    if (!Node->LeftPointer && !Node->RightPointer)
    {
        Console::SysPrint("Leaf Count: " + std::to_string(Node->CountIndex), 6);
    }
    else
    {
        if (Node->LeftPointer)
        {
            Console::SysPrint("Leaf is found on the left pointer", 6);
        }
        else if (Node->RightPointer)
        {
            Console::SysPrint("Leaf is found on the right pointer", 6);
        }
        else
        {
            Console::SysPrint("Leaf is unknown.", 7);
        }
    }
   
    if (Node->LeftPointer)  PrintBVH(Node->LeftPointer, Depth + 1);
    if (Node->RightPointer) PrintBVH(Node->RightPointer, Depth + 1);
}

void PrintSerialized(const std::vector<BVHNodeSerialized>& Nodes, int Index, int Depth = 0)
{
    if (Index < 0 || Index >= (int)Nodes.size())
    {
        Console::SysPrint("Invalid Index " + std::to_string(Index), 8);
        return;
    }

    for (int Index = 0; Index < Depth; Index++) Console::SysPrint("  ", 6);
    const BVHNodeSerialized& CurrentNode = Nodes[Index];

    if (CurrentNode.LeftIndex == -1)
    {
        Console::SysPrint("Leaf Objects: " + std::to_string(CurrentNode.ObjectCount), 6);
        return;
    }

    Console::SysPrint("Node Index: " + std::to_string(Index), 6);
    Console::SysPrint("Left Index: " + std::to_string(Index), 6);
    Console::SysPrint("Right Index: " + std::to_string(Index), 6);

    if (CurrentNode.LeftIndex == Index || CurrentNode.RightIndex == Index)
    {
        Console::SysPrint("Recursive self detected!", 8);
        return;
    }

    PrintSerialized(Nodes, CurrentNode.LeftIndex, Depth + 1);
    PrintSerialized(Nodes, CurrentNode.RightIndex, Depth + 1);
}


BlockData EditorCompiler::CompileIntoBVHTree(DataContainer NewFormat)
{
    //Messy refactoring
    Console::SysPrint("Compiling Bounding Volume Hirarchy Tree...", 6);
    BlockData NewData;
    std::vector<BlockStructureFormat*> BlockStructure = {};

    //Reserve block size to place bounds of mesh block
    //BlockStructure.resize(NewFormat.NumberOfMeshBlocks);

    for (int Index = 0; Index < NewFormat.NumberOfMeshBlocks; Index++)
    {
        //Create block using physics system.
        BlockStructureFormat CurrentBlockStructure;

        //Create bounds for blocks to compare
        CurrentBlockStructure.LocalBounds = this->GenerateNewBounds(
            NewFormat.WorldMeshBlock[Index].Vertices, NewFormat.WorldMeshBlock[Index].Position
        ); //Vertex is in world space. position is used just in case.

        //Assign correct index referring to block (using block index)
        CurrentBlockStructure.BlockIndex = NewFormat.WorldMeshBlock[Index].BlockIndex;

        BlockStructure.push_back(&CurrentBlockStructure); //Push to be used.
    }

    //Create collision nodes based on block structure (block -> tree)
    this->RootNode = this->CreateCollisionNode(BlockStructure, 0, BlockStructure.size());
    PrintBVH(this->RootNode); //Print for debug (goes into console)
    
    //Sterilise tree/flatten tree (Tree -> flattened array)
    this->SteriliseTree(RootNode, NewData.NewSterilised, NewData.IndexArray);
    PrintSerialized(NewData.NewSterilised,0); //Print for debug

    //Return new block data.
    return NewData;
}

BVHCollisionNode* EditorCompiler::CreateCollisionNode(std::vector<BlockStructureFormat*>& Blocks, int Start, int End, int Depth)
{
    std::cout << Start << "\n";
    std::cout << End << "\n";
    std::cout << Depth << "\n";

    //Create tree nodes from here
    BVHCollisionNode* NewNode = new BVHCollisionNode();
    int Count = End - Start;
    int Middle = (Start + End) / 2;
    int Axis = 0;
    //Tree refactoring from array

    NewNode->LeftPointer = nullptr;
    NewNode->RightPointer = nullptr;

    //Create bounding box.
    NewNode->BoundingBox = Blocks[Start]->LocalBounds;
    for (int Index = Start + 1; Index < End; Index++)
        NewNode->BoundingBox = BuildUnion(NewNode->BoundingBox, Blocks[Index]->LocalBounds);

    //Do not exceed maximum depth. If so, just make that as a parent leaf (no child leaf)
    if (Count <= 2 || Depth >= 16)
    {
        NewNode->StartIndex = Start;
        NewNode->CountIndex = Count;
        return NewNode;
    }

    //Absolute size.
    glm::vec3 Size = NewNode->BoundingBox.MaximumBounds - NewNode->BoundingBox.MinimumBounds;

    if (Size.y > Size.x)        Axis = 1;
    if (Size.z > Size[Axis])    Axis = 2;

    //Resort based on size (easier)
    std::sort(Blocks.begin() + Start,
        Blocks.begin() + End,
        [Axis](const BlockStructureFormat* a, const BlockStructureFormat* b)
        {
            return Center(a->LocalBounds)[Axis] < Center(b->LocalBounds)[Axis];
        }
    );

    if (Middle == Start || Middle == End)
    {
        NewNode->StartIndex = Start;
        NewNode->CountIndex = Count;
        return NewNode;
    }

    //Assign to each pointer node, then traverse via recursion.
    NewNode->LeftPointer  = CreateCollisionNode(Blocks, Start, Middle, Depth + 1);
    NewNode->RightPointer = CreateCollisionNode(Blocks, Middle, End, Depth + 1);

    return NewNode;
}


int EditorCompiler::SteriliseTree(BVHCollisionNode* Node, std::vector<BVHNodeSerialized>& ReturnNodes, std::vector<int>& ObjectIndices)
{
    static std::unordered_set<BVHCollisionNode*> Visited;
    int NewIndex = (int)ReturnNodes.size();
    BVHNodeSerialized SterilisedNode = {};

    std::cout << ObjectIndices.size() << "\n";

    //Parent -> Parent node check.
    if (Visited.count(Node))
    {
        Console::SysPrint("Tree pointer recursive detected!", 8);
        return -1;
    }
    //We have visited the node, place in visited list.
    Visited.insert(Node);

    //Default values.
    SterilisedNode.LeftIndex    = -1;
    SterilisedNode.RightIndex   = -1;
    SterilisedNode.ObjectStart  = -1;
    SterilisedNode.ObjectCount  = 0;
    SterilisedNode.Bounds       = Node->BoundingBox;

    ReturnNodes.push_back(SterilisedNode);

    //A base node pointer, no more nodes on either side.
    if (!Node->LeftPointer && !Node->RightPointer)
    {
        ReturnNodes[NewIndex].ObjectStart   = (int)ObjectIndices.size();
        ReturnNodes[NewIndex].ObjectCount   = Node->CountIndex;

        std::cout << ReturnNodes[NewIndex].ObjectStart << "\n";
        std::cout << ReturnNodes[NewIndex].ObjectCount << "\n";

        for (int Index = 0; Index < Node->CountIndex; Index++)
            ObjectIndices.push_back(Node->StartIndex + Index);
    }
    else
    {
        //Create a sterilised array based on the next index, (Essentially child tree node gets pushed into next array index)
        int Left  = SteriliseTree(Node->LeftPointer, ReturnNodes, ObjectIndices);
        int Right = SteriliseTree(Node->RightPointer, ReturnNodes, ObjectIndices);

        //Is the child the upper parent of the current node (Above the parent node)
        //This should raise a assertion error (Essentially a crash, dont want anything to be done)
        assert(Left > NewIndex);
        assert(Left > NewIndex);

        //Assign to array nodes.
        ReturnNodes[NewIndex].LeftIndex     = Left;
        ReturnNodes[NewIndex].RightIndex    = Right;

        std::cout << "Left " << Left << "\n";
        std::cout << "Right" << Right << "\n";
    }

    return NewIndex;
}

void EditorCompiler::CompilePrograms(const std::string& FileName, DataContainer& NewFormat)
{
    Console::SysPrint("Compiling scene...", 6);
    std::ofstream NewFile(FileName, std::ios::binary);
    if (!NewFile.is_open())
    {
        Console::SysPrint("Failed to write to " + FileName, 8);
        return;
    }

    //Assign the new collision block data into the NewFormat system to be written.
    NewFormat.CollisionBlockData = this->CompileIntoBVHTree(NewFormat);

    NewFormat.NumberOfTreeIndex = NewFormat.CollisionBlockData.IndexArray.size();
    NewFormat.NumberOfTreeNodes = NewFormat.CollisionBlockData.NewSterilised.size();

    int NewFileVersion = FILE_VERSION;
    int ImageVersion = 0x120;

    //Pack as structs !!!IF THE STRUCTS ARE DIFFERENT FROM THE MAIN ENGINE, THERE CAN BE CORRUPTION!!!
    //Vector3 Math and such packs should be similar in terms of bytes to the glm library.
    NewFile.write(reinterpret_cast<const char*>(&NewFileVersion), sizeof(int));

    NewFile.write(reinterpret_cast<const char*>(&NewFormat.NumberOfTreeIndex), sizeof(size_t));
    NewFile.write(reinterpret_cast<const char*>(&NewFormat.NumberOfTreeNodes), sizeof(size_t));
    NewFile.write(reinterpret_cast<const char*>(&NewFormat.NumberOfPlayerSpawns), sizeof(size_t));
    NewFile.write(reinterpret_cast<const char*>(&NewFormat.NumberOfImages), sizeof(size_t));
    NewFile.write(reinterpret_cast<const char*>(&NewFormat.NumberOfShaders), sizeof(size_t));
    NewFile.write(reinterpret_cast<const char*>(&NewFormat.NumberOfSkybox), sizeof(size_t));
    NewFile.write(reinterpret_cast<const char*>(&NewFormat.NumberOfLights), sizeof(size_t));
    NewFile.write(reinterpret_cast<const char*>(&NewFormat.NumberOfMeshBlocks), sizeof(size_t));
    NewFile.write(reinterpret_cast<const char*>(&NewFormat.NumberOfEntityBlocks), sizeof(size_t));

    this->LightCompiler.SubmitLightSurfaces(NewFormat.WorldMeshBlock,NewFormat.LightGroup,this->RootNode);
    
    uint32_t indexCount = NewFormat.CollisionBlockData.IndexArray.size();
    NewFile.write((char*)&indexCount, sizeof(uint32_t));

    if (indexCount > 0)
    {
        NewFile.write((char*)NewFormat.CollisionBlockData.IndexArray.data(), indexCount * sizeof(int));
    }

    uint32_t nodeCount = NewFormat.CollisionBlockData.NewSterilised.size();
    NewFile.write((char*)&nodeCount, sizeof(uint32_t));

    for (uint32_t i = 0; i < nodeCount; i++)
    {
        auto& node = NewFormat.CollisionBlockData.NewSterilised[i];

        NewFile.write((char*)glm::value_ptr(node.Bounds.BoundCenter), sizeof(float) * 3);
        NewFile.write((char*)glm::value_ptr(node.Bounds.MaximumBounds), sizeof(float) * 3);
        NewFile.write((char*)glm::value_ptr(node.Bounds.MinimumBounds), sizeof(float) * 3);

        NewFile.write((char*)&node.LeftIndex, sizeof(int));
        NewFile.write((char*)&node.RightIndex, sizeof(int));
        NewFile.write((char*)&node.ObjectCount, sizeof(int));
        NewFile.write((char*)&node.ObjectStart, sizeof(int));
    }

    for (int Index = 0; Index < NewFormat.NumberOfPlayerSpawns; Index++)
    {
        NewFile.write(reinterpret_cast<const char*>(glm::value_ptr(NewFormat.PlayerSpawns[Index].PlayerDimensions)), sizeof(float) * 3);
        NewFile.write(reinterpret_cast<const char*>(glm::value_ptr(NewFormat.PlayerSpawns[Index].PlayerPosition)), sizeof(float) * 3);
    }

    for (int Index = 0; Index < NewFormat.NumberOfImages; Index++)
    {
        FusionGraphicalImage& NewImage = NewFormat.ImageGroup[Index];

        NewFile.write(reinterpret_cast<const char*>(&NewImage.Header), sizeof(int));

        NewFile.write(reinterpret_cast<const char*>(&NewImage.ImageAmount), sizeof(int));
        NewFile.write(reinterpret_cast<const char*>(&NewImage.ImageChannel), sizeof(int));
        NewFile.write(reinterpret_cast<const char*>(&NewImage.FileNameLength), sizeof(int));
        NewFile.write(reinterpret_cast<const char*>(&NewImage.ImageContentLength), sizeof(int));
        NewFile.write(reinterpret_cast<const char*>(glm::value_ptr(NewImage.ImageDimensions)), sizeof(float) * 2);
        NewFile.write(reinterpret_cast<const char*>(&NewImage.ImageType), sizeof(FusionGraphicalImageType));

        NewFile.write(NewImage.FileName.data(), NewImage.FileNameLength);
        NewFile.write(reinterpret_cast<char*>(NewImage.ImageContents.data()), NewImage.ImageContentLength);
    }

    for (int Index = 0; Index < NewFormat.NumberOfShaders; Index++)
    {
        FusionShaderFile& NewShader = NewFormat.ShaderGroup[Index];

        NewFile.write(reinterpret_cast<char*>(NewShader.Header), sizeof(int));

        NewFile.write(reinterpret_cast<char*>(&NewShader.ShaderNameL), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewShader.ShaderContentsL), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewShader.CurrentType), sizeof(FusionShaderType));

        NewFile.write(NewShader.ShaderName.data(), NewShader.ShaderNameL);
        NewFile.write(NewShader.ShaderContents.data(), NewShader.ShaderContentsL);
    }

    for (int Index = 0; Index < NewFormat.NumberOfSkybox; Index++)
    {
        std::vector<int> SkyboxImagesDimensions = {
            NewFormat.SkyboxGroup[Index].SkyboxFront,
            NewFormat.SkyboxGroup[Index].SkyboxBack,
            NewFormat.SkyboxGroup[Index].SkyboxLeft,
            NewFormat.SkyboxGroup[Index].SkyboxRight,
            NewFormat.SkyboxGroup[Index].SkyboxTop,
            NewFormat.SkyboxGroup[Index].SkyboxBottom
        };

        NewFile.write(reinterpret_cast<const char*>(SkyboxImagesDimensions.data()), 6 * sizeof(int));

    }

    for (int Index = 0; Index < NewFormat.NumberOfLights; Index++)
    {
        FusionLightStructure& NewLight = NewFormat.LightGroup[Index];

        NewFile.write(reinterpret_cast<char*>(&NewLight.LightGroupSystem), sizeof(FusionLightType));
        NewFile.write(reinterpret_cast<char*>(glm::value_ptr(NewLight.LightPosition)), sizeof(float) * 3);
        NewFile.write(reinterpret_cast<char*>(glm::value_ptr(NewLight.LightColour)), sizeof(float) * 3);
        NewFile.write(reinterpret_cast<char*>(glm::value_ptr(NewLight.LightDirection)), sizeof(float) * 3);
    }

    for (int Index = 0; Index < NewFormat.NumberOfMeshBlocks; Index++)
    {
        int Dude = 0x12;
        FusionMeshBlock& NewMesh = NewFormat.WorldMeshBlock[Index];

        NewFile.write(reinterpret_cast<char*>(&Dude), sizeof(int));

        NewFile.write(reinterpret_cast<char*>(&NewMesh.BlockIndex), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewMesh.ImageFileSize), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewMesh.IndicesSize), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewMesh.VerticesSize), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewMesh.LightmapDataSize), sizeof(int));

        NewFile.write(reinterpret_cast<char*>(&NewMesh.BlockType), sizeof(FusionWorldType));
        NewFile.write(reinterpret_cast<char*>(&NewMesh.MaterialIndex), sizeof(FusionMaterials));

        NewFile.write(NewMesh.ImageFile.data(), NewMesh.ImageFileSize);
        NewFile.write(reinterpret_cast<char*>(NewMesh.Indices.data()), NewMesh.IndicesSize * sizeof(unsigned int));
        NewFile.write(reinterpret_cast<char*>(NewMesh.Vertices.data()), NewMesh.VerticesSize * sizeof(FusionVertex));
        NewFile.write(reinterpret_cast<char*>(NewMesh.LightmapData.data()), NewMesh.LightmapDataSize);

        NewFile.write(reinterpret_cast<char*>(&NewMesh.Position.x), sizeof(float) * 3);
        NewFile.write(reinterpret_cast<char*>(&NewMesh.Rotation.x), sizeof(float) * 4);
        NewFile.write(reinterpret_cast<char*>(&NewMesh.Scale.x), sizeof(float) * 3);
    }

    for (int Index = 0; Index < NewFormat.NumberOfEntityBlocks; Index++)
    {
        ModelFile& NewModel = NewFormat.EntityBlock[Index];

        NewFile.write(reinterpret_cast<char*>(&NewModel.Header), sizeof(int));

        NewFile.write(reinterpret_cast<char*>(&NewModel.BlockIndex), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewModel.AnimationLength), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewModel.IndicesSize), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewModel.VerticesSize), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewModel.ImageNameSize), sizeof(int));
        NewFile.write(reinterpret_cast<char*>(&NewModel.FileNameSize), sizeof(int));

        NewFile.write(reinterpret_cast<char*>(&NewModel.ModelMatrix[0]), sizeof(float) * 16);
        NewFile.write(reinterpret_cast<char*>(glm::value_ptr(NewModel.Position)), sizeof(float) * 3);
        NewFile.write(reinterpret_cast<char*>(glm::value_ptr(NewModel.Rotation)), sizeof(float) * 4);
        NewFile.write(reinterpret_cast<char*>(glm::value_ptr(NewModel.Scale)), sizeof(float) * 3);

        NewFile.write(reinterpret_cast<char*>(&NewModel.CurrentMaterials), sizeof(FusionMaterials));
        NewFile.write(reinterpret_cast<char*>(&NewModel.PhysController), sizeof(PhysicsController));

        NewFile.write(reinterpret_cast<char*>(NewModel.AnimationKeyframe.data()), NewModel.AnimationLength * sizeof(int));
        NewFile.write(reinterpret_cast<char*>(NewModel.Indices.data()), NewModel.IndicesSize * sizeof(unsigned int));
        NewFile.write(reinterpret_cast<char*>(NewModel.Vertices.data()), NewModel.VerticesSize * sizeof(float));
        NewFile.write(NewModel.ImageName.data(), NewModel.ImageNameSize);
        NewFile.write(NewModel.FileName.data(), NewModel.FileNameSize);
    }

    //We are finished (Finally)
    NewFile.close();
    Console::SysPrint("Saved to file " + FileName + "!", 6);
}



DataContainer EditorCompiler::ReadFile(const char* File)
{
    return {};
}

#if 0
FileStructureFormat EditorCompiler::ReadFile(const char* File)
{
    std::ifstream FilePointer(File, std::ios::binary);

    FileStructureFormat NewFormat;
    BlockStructureFormat NewBlockStructure = {};
    LightStructure NewLightStructure = {};
    ImageStructure NewImageStructure = {};
    BlockData NewData;

    size_t VertexBlockCount;
    size_t IndicesBlockCount;
    size_t BlockSize;
    size_t LightSize;
    size_t TextureSixe;
    size_t PlayerCount = 0;

    size_t NodeCount;
    size_t IndexArrayCount;

    int FileVersion;
    int FileBegin, FileEnd;

    FilePointer.read(reinterpret_cast<char*>(&FileVersion), sizeof(int));
    if (FileVersion != 0x147) { Console::SysPrint("Invalid file version. Got:" + std::to_string(FileVersion), 8); return NewFormat; }

    FilePointer.read(reinterpret_cast<char*>(&BlockSize), sizeof(size_t));
    FilePointer.read(reinterpret_cast<char*>(&LightSize), sizeof(size_t));
    FilePointer.read(reinterpret_cast<char*>(&TextureSixe), sizeof(size_t));

    FilePointer.read(reinterpret_cast<char*>(&PlayerCount), sizeof(size_t));

    FilePointer.read(reinterpret_cast<char*>(&NodeCount), sizeof(size_t));
    FilePointer.read(reinterpret_cast<char*>(&IndexArrayCount), sizeof(size_t));

    FilePointer.read(reinterpret_cast<char*>(NewData.NewSterilised.data()), NewData.NewSterilised.size() * sizeof(BVHNodeSerialized));
    FilePointer.read(reinterpret_cast<char*>(NewData.IndexArray.data()), NewData.IndexArray.size() * sizeof(int));

    if (PlayerCount == 0)
    {
        FilePointer.read(reinterpret_cast<char*>(&NewFormat.CurrentPlayerStructure[0].Position.x), sizeof(float) * 3);
    }
    else
    {
        NewFormat.CurrentPlayerStructure.resize(PlayerCount);
        for (int Index = 0; Index < PlayerCount; Index++)
        {
            FilePointer.read(reinterpret_cast<char*>(&NewFormat.CurrentPlayerStructure[Index].Position.x), sizeof(float) * 3);
        }
    }

    for (int Index = 0; Index < BlockSize; Index++)
    {
        FilePointer.read(reinterpret_cast<char*>(&VertexBlockCount), sizeof(size_t));
        NewBlockStructure.Vertices.resize(VertexBlockCount);
        if (VertexBlockCount > 0)
        {
            FilePointer.read(reinterpret_cast<char*>(NewBlockStructure.Vertices.data()), VertexBlockCount * sizeof(float));
        }

        FilePointer.read(reinterpret_cast<char*>(&IndicesBlockCount), sizeof(size_t));
        NewBlockStructure.Indices.resize(IndicesBlockCount);
        if (IndicesBlockCount > 0)
        {
            FilePointer.read(reinterpret_cast<char*>(NewBlockStructure.Indices.data()), IndicesBlockCount * sizeof(unsigned int));
        }

        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.PhysicsLocked), sizeof(int));

        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.TranslationMatrix[0]), sizeof(float) * 16);
        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.RotationMatrix[0]), sizeof(float) * 16);
        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.ScaleMatrix[0]), sizeof(float) * 16);

        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.Position.x), sizeof(float) * 3);
        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.Rotation.x), sizeof(float) * 4);
        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.Scale.x), sizeof(float) * 3);

        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.BlockIndex), sizeof(int));
        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.BlockColour.x), sizeof(float) * 3);
        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.UseBlockColour), sizeof(int));

        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.MaterialIndex), sizeof(unsigned int));
        FilePointer.read(reinterpret_cast<char*>(&NewBlockStructure.TextureIndex), sizeof(unsigned int));
        NewFormat.CurrentBlockStructure.push_back(NewBlockStructure);
    }

    for (int Index = 0; Index < LightSize; Index++)
    {
        FilePointer.read(reinterpret_cast<char*>(&NewLightStructure.Position.x), sizeof(float) * 3);
        FilePointer.read(reinterpret_cast<char*>(&NewLightStructure.LightColour.x), sizeof(float) * 3);
        FilePointer.read(reinterpret_cast<char*>(&NewLightStructure.Direction.x), sizeof(float) * 3);

        NewFormat.CurrentLightStructure.push_back(NewLightStructure);
    }

    size_t TextureSize;
    for (int Index = 0; Index < TextureSixe; Index++)
    {
        TextureSize = 0;

        FilePointer.read(reinterpret_cast<char*>(&FileBegin), sizeof(int));
        if (FileBegin != 0xA) { Console::SysPrint("Start file corrupted (0xA).", 7);}

        FilePointer.read(reinterpret_cast<char*>(&NewImageStructure.TextureID), sizeof(unsigned int));
        FilePointer.read(reinterpret_cast<char*>(&NewImageStructure.TextureWidth), sizeof(unsigned int));
        FilePointer.read(reinterpret_cast<char*>(&NewImageStructure.TextureHeight), sizeof(unsigned int));
        FilePointer.read(reinterpret_cast<char*>(&NewImageStructure.TextureChannel), sizeof(unsigned int)); 

        FilePointer.read(reinterpret_cast<char*>(&TextureSize), sizeof(size_t));
        NewImageStructure.TextureData.resize(TextureSize);
        FilePointer.read(reinterpret_cast<char*>(&NewImageStructure.TextureData[0]), sizeof(unsigned char) * TextureSize);

        FilePointer.read(reinterpret_cast<char*>(&FileEnd), sizeof(int));
        if (FileEnd != 0xF) { Console::SysPrint("Start file corrupted (0xF).", 7); }
        NewFormat.CurrentImageStructure.push_back(NewImageStructure);
    }

    return NewFormat;
}
#endif