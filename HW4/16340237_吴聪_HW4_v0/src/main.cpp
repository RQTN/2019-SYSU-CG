#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	STATIC, TRANSLATION, ROTATION, SCALING, BONUS
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
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec3 white(1.0f, 1.0f, 1.0f);
	glm::vec3 red(1.0f, 0.0f, 0.0f);
	glm::vec3 blue(0.3f, 0.3f, 1.0f);
	glm::vec3 yellow(1.0f, 1.0f, 0.0f);
	draw_what content = draw_what::STATIC;
	bool isDepthTest = true;

	// build and compile our shader program
	// ------------------------------------
	Shader shader("shaders/shader.vs", "shaders/shader.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	GLfloat vertices[] = {
		// positions          // texture coords
		-2.0f, -2.0f, -2.0f,  0.0f, 0.0f,
		 2.0f, -2.0f, -2.0f,  1.0f, 0.0f,
		 2.0f,  2.0f, -2.0f,  1.0f, 1.0f,
		 2.0f,  2.0f, -2.0f,  1.0f, 1.0f,
		-2.0f,  2.0f, -2.0f,  0.0f, 1.0f,
		-2.0f, -2.0f, -2.0f,  0.0f, 0.0f,

		-2.0f, -2.0f,  2.0f,  0.0f, 0.0f,
		 2.0f, -2.0f,  2.0f,  1.0f, 0.0f,
		 2.0f,  2.0f,  2.0f,  1.0f, 1.0f,
		 2.0f,  2.0f,  2.0f,  1.0f, 1.0f,
		-2.0f,  2.0f,  2.0f,  0.0f, 1.0f,
		-2.0f, -2.0f,  2.0f,  0.0f, 0.0f,

		-2.0f,  2.0f,  2.0f,  1.0f, 0.0f,
		-2.0f,  2.0f, -2.0f,  1.0f, 1.0f,
		-2.0f, -2.0f, -2.0f,  0.0f, 1.0f,
		-2.0f, -2.0f, -2.0f,  0.0f, 1.0f,
		-2.0f, -2.0f,  2.0f,  0.0f, 0.0f,
		-2.0f,  2.0f,  2.0f,  1.0f, 0.0f,

		 2.0f,  2.0f,  2.0f,  1.0f, 0.0f,
		 2.0f,  2.0f, -2.0f,  1.0f, 1.0f,
		 2.0f, -2.0f, -2.0f,  0.0f, 1.0f,
		 2.0f, -2.0f, -2.0f,  0.0f, 1.0f,
		 2.0f, -2.0f,  2.0f,  0.0f, 0.0f,
		 2.0f,  2.0f,  2.0f,  1.0f, 0.0f,

		-2.0f, -2.0f, -2.0f,  0.0f, 1.0f,
		 2.0f, -2.0f, -2.0f,  1.0f, 1.0f,
		 2.0f, -2.0f,  2.0f,  1.0f, 0.0f,
		 2.0f, -2.0f,  2.0f,  1.0f, 0.0f,
		-2.0f, -2.0f,  2.0f,  0.0f, 0.0f,
		-2.0f, -2.0f, -2.0f,  0.0f, 1.0f,

		-2.0f,  2.0f, -2.0f,  0.0f, 1.0f,
		 2.0f,  2.0f, -2.0f,  1.0f, 1.0f,
		 2.0f,  2.0f,  2.0f,  1.0f, 0.0f,
		 2.0f,  2.0f,  2.0f,  1.0f, 0.0f,
		-2.0f,  2.0f,  2.0f,  0.0f, 0.0f,
		-2.0f,  2.0f, -2.0f,  0.0f, 1.0f
	};

	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);



	// load and create a texture
	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char *data = stbi_load("./textures/container.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// clear window
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		if (isDepthTest)
		{
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT);
		}
		

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Hw4 menu");
			// Display some text (you can use a format strings too)
			ImGui::Text("Choose one of the following transformation.");
			ImGui::Checkbox("Depth Test", &isDepthTest);
			ImGui::SameLine();
			ShowHelpMarker("You can check this to open or close depth test!");
			
			// Buttons return true when clicked (most widgets return true when edited/activated)
			if (ImGui::Button("No transformation"))
				content = draw_what::STATIC;
			ImGui::SameLine();
			if (ImGui::Button("Translation"))
				content = draw_what::TRANSLATION;
			ImGui::SameLine();
			if (ImGui::Button("Rotation"))
				content = draw_what::ROTATION;
			ImGui::SameLine();
			if (ImGui::Button("Scaling"))
				content = draw_what::SCALING;
			ImGui::SameLine();
			if (ImGui::Button("Bonus"))
				content = draw_what::BONUS;
			
			
				
			switch (content)
			{
			case draw_what::STATIC:
				ImGui::Text("Just a Cube");
				break;
			case draw_what::TRANSLATION:
				ImGui::Text("Cube translate");
				break;
			case draw_what::ROTATION:
				ImGui::Text("Cube rotate");
				break;
			case draw_what::SCALING:
				ImGui::Text("Cube scaling");
				break;
			case draw_what::BONUS:
				ImGui::Text("Galaxy!");
				break;
			default:
				break;
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		
		// render the triangle
		glm::mat4 model(1.0f);
		glm::mat4 view(1.0f);
		view = glm::lookAt(glm::vec3(0.0f, 5.0f, 25.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 projection(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (GLfloat)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

		shader.use();
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setVec3("myColor", white);
		switch (content)
		{
		case draw_what::STATIC:
			shader.setMat4("model", model);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			break;
		case draw_what::TRANSLATION:
			model = glm::translate(model, (GLfloat)sin(glfwGetTime()) * glm::vec3(5.0f, 0.0f, 0.0f));
			shader.setMat4("model", model);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			break;
		case draw_what::ROTATION:
			model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 1.0f));
			shader.setMat4("model", model);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			break;
		case draw_what::SCALING:
			model = glm::scale(model, (GLfloat)sin(glfwGetTime()) * glm::vec3(2.0f));
			shader.setMat4("model", model);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			break;
		case draw_what::BONUS:

			// Draw Sun
			// Sun self rotate
			model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-30.0f), glm::vec3(-1.0f, 1.0f, -1.0f));
			shader.setMat4("model", model);
			shader.setVec3("myColor", red);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			// Draw Earth
			model = glm::mat4(1.0f);
			// 4. Then totate, it will rotate around (0.0f, 0.0f, 0.0f)
			model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			// 3. Translate 
			model = glm::translate(model, glm::vec3(8.0f, 0.0f, 0.0f));
			// 2. Earth self rotate
			model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			// 1. Scale the box to earth
			model = glm::scale(model, glm::vec3(0.5f));
			shader.setMat4("model", model);
			shader.setVec3("myColor", blue);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);


			// Draw Moon
			model = glm::mat4(1.0f);
			// 5. This two transformation is from Earth 3. & 4.
			// now the moon will rotate around the Earth's pos
			model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(8.0f, 0.0f, 0.0f));
			// 4. Then totate, it will rotate around (0.0f, 0.0f, 0.0f)
			model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(150.0f), glm::vec3(1.0f, 1.0f, 0.0f));
			// 3. Translate
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 3.0f));
			// 2. Moon self rotate
			model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			// 1. Scale the box to Moon
			model = glm::scale(model, glm::vec3(0.3f));
			shader.setMat4("model", model);
			shader.setVec3("myColor", yellow);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

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
}