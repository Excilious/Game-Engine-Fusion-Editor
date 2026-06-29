 #include "Debug.h"

Debug::Debug(void) {}

void Debug::UploadOutline(Shader* DebugShader, std::vector<float> Vertices, std::vector<unsigned int> Indices,glm::vec3 Position,glm::quat Rotation,glm::vec3 Scale)
{
    DebugRenderBuffer NewRenderBuffer;

    this->DebugGlobalShader = DebugShader;

    NewRenderBuffer.Vertices = Vertices;
    NewRenderBuffer.Indices = Indices;

    NewRenderBuffer.Translation = glm::mat4(1.0f);
    NewRenderBuffer.Rotation = glm::mat4(1.0f);
    NewRenderBuffer.Scale = glm::mat4(1.0f);

    NewRenderBuffer.Translation = glm::translate(NewRenderBuffer.Translation, Position);
    NewRenderBuffer.Rotation = glm::mat4_cast(Rotation);
    NewRenderBuffer.Scale = glm::scale(NewRenderBuffer.Scale, Scale);

    glGenVertexArrays(1, &NewRenderBuffer.DebugVertexArray);
    glGenBuffers(1, &NewRenderBuffer.DebugVertexBuffer);
    glGenBuffers(1, &NewRenderBuffer.DebugElementBuffer);

    glBindVertexArray(NewRenderBuffer.DebugVertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, NewRenderBuffer.DebugVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, NewRenderBuffer.Vertices.size() * sizeof(float), NewRenderBuffer.Vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewRenderBuffer.DebugElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, NewRenderBuffer.Indices.size() * sizeof(unsigned int), NewRenderBuffer.Indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    this->SceneRenderGroup.push_back(NewRenderBuffer);
}

void Debug::UploadVerticesBoxValues(Shader* DebugShader, glm::vec3 MinBounds, glm::vec3 MaxBounds,glm::vec3 Position,glm::quat Rotation,glm::vec3 Scale)
{
    DebugRenderBuffer NewRenderBuffer;
    glm::vec3 Minimum;
    glm::vec3 Maximum;

    this->DebugGlobalShader = DebugShader;
    Minimum = MinBounds;
    Maximum = MaxBounds;

    std::vector<float> Vertices = {
        Minimum.x,Minimum.y,Minimum.z,
        Maximum.x,Minimum.y,Minimum.z,
        Maximum.x,Maximum.y,Minimum.z,
        Minimum.x,Maximum.y,Minimum.z,
        Minimum.x,Minimum.y,Maximum.z,
        Maximum.x,Minimum.y,Maximum.z,
        Maximum.x,Maximum.y,Maximum.z,
        Minimum.x,Maximum.y,Maximum.z
    };

    std::vector<unsigned int> Indices = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    NewRenderBuffer.Vertices = Vertices;
    NewRenderBuffer.Indices = Indices;

    NewRenderBuffer.Translation = glm::mat4(1.0f);
    NewRenderBuffer.Rotation = glm::mat4(1.0f);
    NewRenderBuffer.Scale = glm::mat4(1.0f);

    NewRenderBuffer.Translation = glm::translate(NewRenderBuffer.Translation, Position);
    NewRenderBuffer.Rotation = glm::mat4_cast(Rotation);
    NewRenderBuffer.Scale = glm::scale(NewRenderBuffer.Scale, Scale);

    glGenVertexArrays(1, &NewRenderBuffer.DebugVertexArray);
    glGenBuffers(1, &NewRenderBuffer.DebugVertexBuffer);
    glGenBuffers(1, &NewRenderBuffer.DebugElementBuffer);

    glBindVertexArray(NewRenderBuffer.DebugVertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, NewRenderBuffer.DebugVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, NewRenderBuffer.Vertices.size() * sizeof(float), NewRenderBuffer.Vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewRenderBuffer.DebugElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, NewRenderBuffer.Indices.size() * sizeof(unsigned int), NewRenderBuffer.Indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    this->DebugRenderGroup.push_back(NewRenderBuffer);
}

void Debug::UploadBoundingBoxValues(Shader* DebugShader,Model* EntityModel)
{
    DebugRenderBuffer NewRenderBuffer;
    glm::vec3 Minimum;
    glm::vec3 Maximum;

    this->DebugGlobalShader = DebugShader;
    Minimum                 = EntityModel->EntityBounds.MinBounds;
    Maximum                 = EntityModel->EntityBounds.MaxBounds;

    std::vector<float> Vertices = {
        Minimum.x,Minimum.y,Minimum.z,
        Maximum.x,Minimum.y,Minimum.z,
        Maximum.x,Maximum.y,Minimum.z,
        Minimum.x,Maximum.y,Minimum.z,
        Minimum.x,Minimum.y,Maximum.z,
        Maximum.x,Minimum.y,Maximum.z,
        Maximum.x,Maximum.y,Maximum.z,
        Minimum.x,Maximum.y,Maximum.z
    };

    std::vector<unsigned int> Indices = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    NewRenderBuffer.Vertices    = Vertices;
    NewRenderBuffer.Indices     = Indices;

    NewRenderBuffer.Translation = glm::mat4(1.0f);
    NewRenderBuffer.Rotation    = glm::mat4(1.0f);
    NewRenderBuffer.Scale       = glm::mat4(1.0f);

    NewRenderBuffer.Translation = glm::translate(NewRenderBuffer.Translation,EntityModel->GetModelPosition());
    NewRenderBuffer.Rotation    = glm::mat4_cast(EntityModel->GetModelRotation());
    NewRenderBuffer.Scale       = glm::scale(NewRenderBuffer.Scale,EntityModel->GetModelScale());

    glGenVertexArrays(1,&NewRenderBuffer.DebugVertexArray);
    glGenBuffers(1,&NewRenderBuffer.DebugVertexBuffer);
    glGenBuffers(1,&NewRenderBuffer.DebugElementBuffer);

    glBindVertexArray(NewRenderBuffer.DebugVertexArray);

    glBindBuffer(GL_ARRAY_BUFFER,NewRenderBuffer.DebugVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,NewRenderBuffer.Vertices.size()*sizeof(float),NewRenderBuffer.Vertices.data(),GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,NewRenderBuffer.DebugElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,NewRenderBuffer.Indices.size()*sizeof(unsigned int),NewRenderBuffer.Indices.data(),GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);

    glBindVertexArray(0);

    this->DebugRenderGroup.push_back(NewRenderBuffer);
}

void Debug::DestroyOneBounds(int Index)
{
    this->DebugRenderGroup.erase(this->DebugRenderGroup.begin() + Index);
}

void Debug::RenderOneBoundingBoxValues(int Index,const glm::mat4& Projection, const glm::mat4& View,glm::mat4 Translate,glm::mat4 Rotate,glm::mat4 Scale)
{
    glBindVertexArray(this->DebugRenderGroup[Index].DebugVertexArray);
    this->DebugGlobalShader->ActivateShader();
    this->DebugGlobalShader->SetMatrix4(Projection, "Projection");
    this->DebugGlobalShader->SetMatrix4(View, "View");
    this->DebugGlobalShader->SetMatrix4(Translate, "Translation");
    this->DebugGlobalShader->SetMatrix4(Rotate, "Rotation");
    this->DebugGlobalShader->SetMatrix4(Scale, "Scale");
    this->DebugGlobalShader->SetVector3(glm::vec3(0.0f, 1.0f, 0.0f), "BoundColour");

    glDrawElements(GL_LINES, this->DebugRenderGroup[Index].Indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Debug::RenderDebugValues(int Index, const glm::mat4& Projection, const glm::mat4& View, glm::mat4 Translate, glm::mat4 Rotate, glm::mat4 Scale)
{
    glBindVertexArray(this->DebugRenderGroup[Index].DebugVertexArray);
    this->DebugGlobalShader->ActivateShader();
    this->DebugGlobalShader->SetMatrix4(Projection, "Projection");
    this->DebugGlobalShader->SetMatrix4(View, "View");
    this->DebugGlobalShader->SetMatrix4(Translate, "Translation");
    this->DebugGlobalShader->SetMatrix4(Rotate, "Rotation");
    this->DebugGlobalShader->SetMatrix4(Scale, "Scale");
    this->DebugGlobalShader->SetVector3(glm::vec3(1.0, 0.5, 0.0), "BoundColour");

    glPointSize(8.0f);
    glDrawElements(GL_POINTS, this->DebugRenderGroup[Index].Indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Debug::RenderBoundingBoxValues(const glm::mat4& Projection,const glm::mat4& View)
{
    for (int Index = 0; Index < this->SceneRenderGroup.size(); Index++)
    {
        glBindVertexArray(this->SceneRenderGroup[Index].DebugVertexArray);
        this->DebugGlobalShader->ActivateShader();
        this->DebugGlobalShader->SetMatrix4(Projection,"Projection");
        this->DebugGlobalShader->SetMatrix4(View,"View");
        this->DebugGlobalShader->SetMatrix4(this->SceneRenderGroup[Index].Translation,"Translation");
        this->DebugGlobalShader->SetMatrix4(this->SceneRenderGroup[Index].Rotation,"Rotation");
        this->DebugGlobalShader->SetMatrix4(this->SceneRenderGroup[Index].Scale,"Scale");
        this->DebugGlobalShader->SetVector3(glm::vec3(1.0f,1.0f,1.0f),"BoundColour");

        glDrawElements(GL_LINES,this->SceneRenderGroup[Index].Indices.size(),GL_UNSIGNED_INT,0);
        glBindVertexArray(0);
    }
}

void Debug::DestroyDebugger(void)
{
    for (int Index = 0; Index < this->DebugRenderGroup.size(); Index++)
    {
        glDeleteVertexArrays(1,&this->DebugRenderGroup[Index].DebugVertexArray);
        glDeleteBuffers(1,&this->DebugRenderGroup[Index].DebugElementBuffer);
        glDeleteBuffers(1,&this->DebugRenderGroup[Index].DebugVertexBuffer);
    }
}