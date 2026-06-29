#include "EditorModelViewport.h"

EditorModelViewport::EditorModelViewport(void) {}

void EditorModelViewport::CreateModelViewport(Parser* EngineParser,int Width, int Height)
{
    this->Width = Width;
    this->Height = Height;   

    this->NewShader.CreateNewShader(EngineParser->GetDatapackData("Shaders/Default.vert").c_str(), EngineParser->GetDatapackData("Shaders/Default.frag").c_str());
    this->NewModel.LoadFromGLTF(EngineParser, "Models/Ranger.gltf", glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(0.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
}

void EditorModelViewport::CreateFrameBuffer(void)
{
    glGenFramebuffers(1, &this->FrameBufferIdentifer);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FrameBufferIdentifer);

    glGenTextures(1, &this->TextureIdentifer);
    glBindTexture(GL_TEXTURE_2D, this->TextureIdentifer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->Width,this->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->TextureIdentifer, 0);

    glGenRenderbuffers(1, &this->RenderIdentifer);
    glBindRenderbuffer(GL_RENDERBUFFER, this->RenderIdentifer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->Width,this->Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->RenderIdentifer);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void EditorModelViewport::BindFrameBuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->FrameBufferIdentifer);
}

void EditorModelViewport::UnbindFrameBuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void EditorModelViewport::RenderViewport(GLFWwindow* EngineWindow,Camera* EngineCamera,Parser* EngineParser,float Near,float Far,float FieldOfView,bool IsOpen)
{
    if (!IsOpen) { return; }
    this->BindFrameBuffer();
    this->RenderSceneModel(EngineWindow,EngineCamera,EngineParser,Near,Far,FieldOfView);
    this->UnbindFrameBuffer();

	ImGui::Begin("Model Viewport",&IsOpen);

	const float ViewportWidth = ImGui::GetContentRegionAvail().x;
	const float ViewportHeight = ImGui::GetContentRegionAvail().y;

	glViewport(0, 0, ViewportWidth, ViewportHeight);
	ImVec2 Position = ImGui::GetCursorScreenPos();

	ImGui::Image((void*)this->TextureIdentifer, ImVec2(ViewportWidth, ViewportHeight), ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();
}

void EditorModelViewport::RenderSceneModel(GLFWwindow* EngineWindow,Camera* EngineCamera,Parser* EngineParser,float Near,float Far,float FieldOfView)
{
    //TODO: Use a new camera? could be interfering with the actual camera.
    EngineCamera->MoveCamera();
    EngineCamera->UpdateCameraView(EngineWindow, this->MousePositionX, this->MousePositionY);
    EngineCamera->UpdateCameraMatrix(FieldOfView, 0.001f, 1000.0f);
    EngineCamera->MoveFunctionCamera(EngineWindow);

    this->NewModel.RenderModel(&this->NewShader,EngineCamera,Near,Far,FieldOfView);

    glfwGetCursorPos(EngineWindow, &this->MousePositionX, &this->MousePositionY);
}