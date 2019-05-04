#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "Bresenham.h"

// Note: include glad header file before include glfw header file

#include <iostream>

static void glfw_error_callback(GLint error, const GLchar* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height);
void processInput(GLFWwindow* window);

// settings
const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

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

enum draw_what
{
	MY_TRIANGLE, MY_CIRCLE
};

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

	// Load Fonts: here we use default font

	// declare draw_what enum variable
	draw_what content = draw_what::MY_TRIANGLE;
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	// bonus part (judge if fill the triangle)
	bool isFilled = false;

	// build and compile our shader program
	// ------------------------------------
	Shader shader("shaders/shader.vs", "shaders/shader.fs");

	// Triangle vertex data
	// ------------------------------------------------------------------
	Point tri_vertex[] = {
		Point(-150, -150),
		Point(  20,  100),
		Point( 200,  -70)
	};
	// Circle's origin and radius
	Point origin(0, 0);
	// radius can be change
	GLint radius = 100;

	GLuint VAO[2], VBO[2];

	glGenVertexArrays(2, VAO);
	glGenBuffers(2, VBO);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// clear window
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Hw3 menu");
			// Display some text (you can use a format strings too)
			ImGui::Text("Choose the following content you want to draw!\n (Draw Triangle by default)");
			
			// Buttons return true when clicked (most widgets return true when edited/activated)
			if (ImGui::Button("Triangle"))
				content = MY_TRIANGLE;
			ImGui::SameLine();
			if (ImGui::Button("Circle"))
				content = MY_CIRCLE;

			switch (content)
			{
			case draw_what::MY_TRIANGLE:
				ImGui::Text("Draw triangle");
				ImGui::Checkbox("Is filled", &isFilled);
				break;
			case draw_what::MY_CIRCLE:
				ImGui::Text("Draw circle\n");
				ImGui::SliderInt("radius", &radius, 10, 200);
				ImGui::SameLine();
				ShowHelpMarker("You can change the circle's radius by adjusting the \nhorizontal slider or directly ctrl+click the slider \nthen input by keyboard");
				break;
			default:
				break;
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		std::vector<Point> triDataVec = Bresenham::genTriangleData(tri_vertex[0], tri_vertex[1], tri_vertex[2], isFilled);
		std::vector<float> triData = Utils::points2NDC(triDataVec, SCR_WIDTH, SCR_HEIGHT);

		std::vector<Point> circleDataVec = Bresenham::genCircleData(origin, radius);
		std::vector<float> circleData = Utils::points2NDC(circleDataVec, SCR_WIDTH, SCR_HEIGHT);
		// render the triangle
		switch (content)
		{
		case draw_what::MY_TRIANGLE:
			shader.use();
			// 1. bind Triangle VAO
			glBindVertexArray(VAO[MY_TRIANGLE]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO[MY_TRIANGLE]);

			glBufferData(GL_ARRAY_BUFFER, triData.size() * sizeof(GLfloat), triData.data(), GL_STATIC_DRAW);
			// position attribute
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
			glEnableVertexAttribArray(0);
			glDrawArrays(GL_POINTS, 0, triData.size() / 2);
			break;
		case draw_what::MY_CIRCLE:
			shader.use();
			// 2. bind Circle VAO
			glBindVertexArray(VAO[MY_CIRCLE]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO[MY_CIRCLE]);

			glBufferData(GL_ARRAY_BUFFER, circleData.size() * sizeof(GLfloat), circleData.data(), GL_STATIC_DRAW);
			// position attribute
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
			glEnableVertexAttribArray(0);
			glDrawArrays(GL_POINTS, 0, circleData.size() / 2);
			break;
		default:
			break;
		}

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
	glDeleteVertexArrays(2, VAO);
	glDeleteBuffers(2, VBO);

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
}