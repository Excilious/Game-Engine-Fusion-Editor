#ifndef __EDITOR_MODEL_VIEWPORT_HEADER__
#define __EDITOR_MODEL_VIEWPORT_HEADER__

#include "../Rendering/Models/Model.h"

#include "../../Resources/imgui/imgui.h"
#include "../../Resources/imgui/imgui_impl_opengl3.h"
#include "../../Resources/imgui/imgui_impl_glfw.h"
#include "../../Resources/glad/glad.h"
#include "../../Resources/glm/glm.hpp"
#include "../../Resources/GLFW/glfw3.h"

class EditorModelViewport
{
	public:
		EditorModelViewport(void);

		void CreateModelViewport(Parser* EngineParser,int Width, int Height);
		void RenderViewport(GLFWwindow* EngineWindow, Camera* EngineCamera, Parser* EngineParser, float Near, float Far, float FieldOfView,bool IsOpen);
		void RenderSceneModel(GLFWwindow* EngineWindow, Camera* EngineCamera, Parser* EngineParser, float Near, float Far, float FieldOfView);
		void CreateFrameBuffer(void);
		void BindFrameBuffer(void);
		void UnbindFrameBuffer(void);
	private:
		Model	     NewModel;
		Shader	     NewShader;

		int		     Width;
		int		     Height;

		double		MousePositionX;
		double		MousePositionY;

		unsigned int FrameBufferIdentifer;
		unsigned int TextureIdentifer;
		unsigned int RenderIdentifer;
};

#endif