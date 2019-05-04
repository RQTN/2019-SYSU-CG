#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"

// Note: include glad header file before include glfw header file

#include <iostream>

static void glfw_error_callback(GLint error, const GLchar* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 25.0f));

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLfloat lastX = SCR_WIDTH / 2.0;
GLfloat lastY = SCR_HEIGHT / 2.0;
GLboolean firstMouse = true;

float ortho[4] = { -5.5f, 2.5f, -3.0f, 4.0f };
float perspect[2] = { 45.0f, 1.3f };
float near_far[2] = { 0.1f, 100.0f };

int mouse_mode = 0;

enum draw_what
{
	STATIC, VIEW_CHANGING, BONUS
};

static void ShowHelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

int main()
{
	// set glfw error callback function before initialization
	glfwSetErrorCallback(glfw_error_callback);
	// glfw: initialize and configure
	// ------------------------------
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	const GLchar* glsl_version = "#version 330 core";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MyWindow", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);	// Enable vsync
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable keyboard Controls

	// Setup Dear ImGui style
	// ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Enable Depth Test
	// --------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader program
	// ------------------------------------
	Shader shader("shaders/shader.vs", "shaders/shader.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	GLfloat vertices[] = {
		// positions          // vertex color
		-2.0f, -2.0f, -2.0f,  1.0f, 0.0f, 0.0f, 
		 2.0f, -2.0f, -2.0f,  1.0f, 0.0f, 0.0f,
		 2.0f,  2.0f, -2.0f,  1.0f, 0.0f, 0.0f,
		 2.0f,  2.0f, -2.0f,  1.0f, 0.0f, 0.0f,
		-2.0f,  2.0f, -2.0f,  1.0f, 0.0f, 0.0f,
		-2.0f, -2.0f, -2.0f,  1.0f, 0.0f, 0.0f,

		-2.0f, -2.0f,  2.0f,  0.0f, 1.0f, 0.0f,
		 2.0f, -2.0f,  2.0f,  0.0f, 1.0f, 0.0f,
		 2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 0.0f,
		 2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 0.0f,
		-2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f,  2.0f,  0.0f, 1.0f, 0.0f,

		-2.0f,  2.0f,  2.0f,  0.0f, 0.0f, 1.0f,
		-2.0f,  2.0f, -2.0f,  0.0f, 0.0f, 1.0f,
		-2.0f, -2.0f, -2.0f,  0.0f, 0.0f, 1.0f,
		-2.0f, -2.0f, -2.0f,  0.0f, 0.0f, 1.0f,
		-2.0f, -2.0f,  2.0f,  0.0f, 0.0f, 1.0f,
		-2.0f,  2.0f,  2.0f,  0.0f, 0.0f, 1.0f,

		 2.0f,  2.0f,  2.0f,  1.0f, 1.0f, 0.0f,
		 2.0f,  2.0f, -2.0f,  1.0f, 1.0f, 0.0f,
		 2.0f, -2.0f, -2.0f,  1.0f, 1.0f, 0.0f,
		 2.0f, -2.0f, -2.0f,  1.0f, 1.0f, 0.0f,
		 2.0f, -2.0f,  2.0f,  1.0f, 1.0f, 0.0f,
		 2.0f,  2.0f,  2.0f,  1.0f, 1.0f, 0.0f,

		-2.0f, -2.0f, -2.0f,  1.0f, 0.0f, 1.0f,
		 2.0f, -2.0f, -2.0f,  1.0f, 0.0f, 1.0f,
		 2.0f, -2.0f,  2.0f,  1.0f, 0.0f, 1.0f,
		 2.0f, -2.0f,  2.0f,  1.0f, 0.0f, 1.0f,
		-2.0f, -2.0f,  2.0f,  1.0f, 0.0f, 1.0f,
		-2.0f, -2.0f, -2.0f,  1.0f, 0.0f, 1.0f,

		-2.0f,  2.0f, -2.0f,  0.0f, 1.0f, 1.0f,
		 2.0f,  2.0f, -2.0f,  0.0f, 1.0f, 1.0f,
		 2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 1.0f,
		 2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 1.0f,
		-2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 1.0f,
		-2.0f,  2.0f, -2.0f,  0.0f, 1.0f, 1.0f,
	};

	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	// texture coord attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	draw_what content = draw_what::STATIC;
	int projection_mode = 0;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// clear window
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Hw5 menu");
			// Display some text (you can use a format strings too)
			ImGui::Text("Choose one of the following content you want to draw!");

			// Buttons return true when clicked (most widgets return true when edited/activated)
			if (ImGui::Button("Static"))
				content = draw_what::STATIC;
			ImGui::SameLine();
			if (ImGui::Button("View Changing"))
				content = draw_what::VIEW_CHANGING;
			ImGui::SameLine();
			if (ImGui::Button("Bonus"))
				content = draw_what::BONUS;
			
			
				
			switch (content)
			{
			case draw_what::STATIC:
				ImGui::Text("Just a Cube");
				break;
			case draw_what::VIEW_CHANGING:
				ImGui::Text("Camera rotates around the Cube and looks at the Cube center");
				break;
			case draw_what::BONUS:
				ImGui::Text("My FPS Camera!");
				break;
			default:
				break;
			}

			ImGui::RadioButton("orthographic projection", &projection_mode, 0);
			ImGui::SameLine();
			ShowHelpMarker("Tick to switch to orthographic projection");
			ImGui::RadioButton("perspective projection", &projection_mode, 1);
			ImGui::SameLine();
			ShowHelpMarker("Tick to switch to perspective projection");
			switch (projection_mode)
			{
			case 0:	// orthographic projection
				ImGui::DragFloat4("left right bottom top", ortho, 0.1f);
				break;
			case 1:	// perspective projection
				ImGui::DragFloat2("fov aspect", perspect, 0.1f);
				break;
			default:
				break;
			}
			ImGui::DragFloat2("near far", near_far, 0.1f, 0.0f, 1000.0f);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		
		// render the triangle
		glm::mat4 model(1.0f);
		glm::mat4 view(1.0f);
		glm::mat4 projection(1.0f);
		
		float radius = 25.0f;
		float camX = sin(glfwGetTime()) * radius;
		float camZ = cos(glfwGetTime()) * radius;

		switch (projection_mode)
		{
		case 0: // Ortho
			projection = glm::ortho(ortho[0], ortho[1], ortho[2], ortho[3], near_far[0], near_far[1]);
			break;
		case 1:	// Perspect
			projection = glm::perspective(glm::radians(perspect[0]), perspect[1], near_far[0], near_far[1]);
			break;
		default:
			break;
		}

		switch (content)
		{
		case draw_what::STATIC:
			model = glm::translate(model, glm::vec3(-1.5f, 0.5f, -1.5f));
			view = glm::lookAt(glm::vec3(0.0f, 0.0f, 25.0f),
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
			break;
		case draw_what::VIEW_CHANGING:
			view = glm::lookAt(glm::vec3(camX, 0.0f, camZ),
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
			break;
		case draw_what::BONUS:
			if (mouse_mode == 0)
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			else
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			view = camera.GetViewMatrix();
			break;
		default:
			break;
		}

		shader.use();
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// render imgui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// imgui clean up
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// de-allocate all resources once they've outlived their purpose:
	// --------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow * window, GLint width, GLint height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow * window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS)
		if (mouse_mode == 0) 
			mouse_mode = 1;
		else
			mouse_mode = 0;

	// w a s d
	// --------------------------
	float cameraSpeed = 2.5f * deltaTime;	// adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

}

void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	camera.ProcessMouseMovement(xoffset, yoffset);

}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset, perspect[0]);
}