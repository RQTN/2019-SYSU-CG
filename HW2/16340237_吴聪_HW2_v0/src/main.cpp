
#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"

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

enum draw_what
{
	MY_TRIANGLE, MY_POINT, MY_LINE, MY_MULTIPLE_TRIANGLES
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
	ImVec4 up_vertex_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 left_bottom_vertex_color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 right_bottom_vertex_color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	GLfloat pointSize = 3.0f;
	bool isWireframeMode = false;

	// build and compile our shader program
	// ------------------------------------
	Shader basic_shader("shaders/basic.vs", "shaders/basic.fs");
	Shader bonus_shader("shaders/bonus.vs", "shaders/bonus.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	GLfloat v1[] = {
		// positions         
		 0.5f, -0.5f, 0.0f,		// right bottom
		-0.5f, -0.5f, 0.0f,		// left bottom
		 0.0f,  0.5f, 0.0f		// up
	};

	GLfloat v2[] = {
		// positions
		 0.5f,  0.5f, 0.0f,		// right up
		 0.5f, -0.5f, 0.0f,		// right bottom
		-0.5f, -0.5f, 0.0f,		// left bottom
		-0.5f,  0.5f, 0.0f		// left up
	};

	GLuint i1[] = {
		0, 2,					// right up to left bottom
		1, 3					// right bottom to left up
	};

	GLuint i2[] = {
		0, 1, 3,				// first triangle
		1, 2, 3					// second triangle
	};

	GLuint VAO[4], VBO[2], EBO[2];

	glGenVertexArrays(4, VAO);
	glGenBuffers(2, VBO);
	glGenBuffers(2, EBO);
	
	// 1. bind Triangle VAO
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v1), v1, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v2), v2, GL_STATIC_DRAW);
	// 2. bind Point VAO
	glBindVertexArray(VAO[1]);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// 3. bind Line VAO
	glBindVertexArray(VAO[2]);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i1), i1, GL_STATIC_DRAW);
	glBindVertexArray(0);

	// 4. bind Multiple triangles VAO
	glBindVertexArray(VAO[3]);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i2), i2, GL_STATIC_DRAW);
	glBindVertexArray(0);

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
			ImGui::Begin("Hw2 menu");							
			// Display some text (you can use a format strings too)
			ImGui::Text("Choose the following content you want to draw!\n (Draw Triangle by default)");					
			ImGui::Checkbox("Wireframe Mode", &isWireframeMode);
			// Buttons return true when clicked (most widgets return true when edited/activated)
			if (ImGui::Button("Triangle"))
				content = MY_TRIANGLE;
			ImGui::SameLine();
			if (ImGui::Button("Point"))
				content = MY_POINT;
			ImGui::SameLine();
			if (ImGui::Button("Line"))
				content = MY_LINE;
			ImGui::SameLine();
			if (ImGui::Button("Multiple triangles"))
				content = MY_MULTIPLE_TRIANGLES;

			switch (content)
			{
			case draw_what::MY_TRIANGLE:
				ImGui::Text("You can change the color of the triangle by\n setting the following 3 parameters");
				// Edit 3 floats representing a color
				ImGui::ColorEdit3("up", (float*)&up_vertex_color);
				ImGui::ColorEdit3("left bottom", (float*)&left_bottom_vertex_color);
				ImGui::ColorEdit3("right bottom", (float*)&right_bottom_vertex_color);
				break;
			case draw_what::MY_POINT:
				ImGui::Text("Draw 4 points\n");
				ImGui::Text("You can change the point's size by adjusting\n the horizontal slider");
				ImGui::SliderFloat("point's size", &pointSize, 1.0f, 10.0f);
				break;
			case draw_what::MY_LINE:
				ImGui::Text("Draw 2 lines");
				break;
			case draw_what::MY_MULTIPLE_TRIANGLES:
				ImGui::Text("Draw 2 triangles");
				ImGui::Text("You can choose Wireframe Mode to show the\n rectangle is indeed made up of 2 triangles");
				break;
			default:
				break;
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// render triangle
		// ------
		if (isWireframeMode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// render the triangle
		switch (content)
		{
		case draw_what::MY_TRIANGLE:
			basic_shader.use();
			basic_shader.setVec4("up_vertex_color", up_vertex_color.x, up_vertex_color.y, up_vertex_color.z, 1.0f);
			basic_shader.setVec4("left_bottom_vertex_color", left_bottom_vertex_color.x, left_bottom_vertex_color.y, left_bottom_vertex_color.z, 1.0f);
			basic_shader.setVec4("right_bottom_vertex_color", right_bottom_vertex_color.x, right_bottom_vertex_color.y, right_bottom_vertex_color.z, 1.0f);
			glBindVertexArray(VAO[0]);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			break;
		case draw_what::MY_POINT:
			bonus_shader.use();
			glPointSize(pointSize);
			glBindVertexArray(VAO[1]);
			glDrawArrays(GL_POINTS, 0, 4);
			break;
		case draw_what::MY_LINE:
			bonus_shader.use();
			glBindVertexArray(VAO[2]);
			glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, 0);
			break;
		case draw_what::MY_MULTIPLE_TRIANGLES:
			bonus_shader.use();
			glBindVertexArray(VAO[3]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
	glDeleteVertexArrays(4, VAO);
	glDeleteBuffers(2, VBO);
	glDeleteBuffers(2, EBO);

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