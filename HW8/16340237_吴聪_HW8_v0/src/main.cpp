#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "shader.h"

using namespace::std;

// callback function
static void glfw_error_callback(GLint error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mode);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// all control points
vector<glm::vec2> control_points;
// all bezier curve_points
vector<glm::vec2> bezier_curve_points;
// util_iterator record the mouse left button selected control point
vector<glm::vec2>::iterator util_iterator = control_points.end();

// utils
// normalize screen index to ndc index
glm::vec2 ndc_norm(glm::vec2 scr_pos);
// get the neaest point of the given pos
vector<glm::vec2>::iterator get_nearby_point(glm::vec2 scr_pos, const float sensitivity);
// global falg
bool isMouseLeftButtonPressed = false;
bool isBanClick = false;
bool isDynamicDraw = false;

// bezier curve relevant
int factorial(int n);
float bernstein(int i, int n, float t);
void get_bezier_curve(int ctrl_points_num);
glm::vec2 get_bezier_curve_point(int n, float t);
void draw_intermediate_points(Shader &shader, const vector<glm::vec2> &points, float t);

// render functions
void render_points_and_lines(Shader &shader, const vector<glm::vec2> &points);
void render_bezier_curve(Shader &shader, const vector<glm::vec2> &points, ImVec4 color);


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
	const char* glsl_version = "#version 330 core";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CG-HOMEWORK 8", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);	// Enable vsync
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);

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

	// build and compile shaders
	// -------------------------
	Shader shader("./shaders/shader.vs", "./shaders/shader.fs");

	// line color
	ImVec4 line_color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
	float dynamic_t = 0.0f;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{

		// std::cout << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z  << "|" << camera.Yaw << ", " << camera.Pitch << std::endl;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{

			// Create a window called "Hello, world!" and append into it.
			ImGui::Begin("Bezier Curve Draw Menu");
			// Display some text (you can use a format strings too)
			ImGui::Text("1. Click left mouse button to create a new control point");
			ImGui::Text("2. Click right mouse button to eliminate the existing control point");
			ImGui::Text("3. Move mouse to the control point, hold down left mouse button \n   to move the control point");
			ImGui::ColorEdit3("line color", (float*)&line_color);

			ImGui::Checkbox("Ban Click", &isBanClick);
			ImGui::SameLine();
			ImGui::Checkbox("Dynamic", &isDynamicDraw);

			ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// render control points and its lines
		render_points_and_lines(shader, control_points);
		// generate bezier_curve
		get_bezier_curve(control_points.size());
		// render points on the bezier curve
		render_bezier_curve(shader, bezier_curve_points, line_color);

		if (isDynamicDraw)
		{
			if (dynamic_t <= 1.0f)
			{
				dynamic_t += 0.002f;
				draw_intermediate_points(shader, control_points, dynamic_t);
			}
		}
		else
		{
			dynamic_t = 0.0f;
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

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


// render points and the line between (P0,P1), (P1,P2), ..., (Pn-1,Pn)
GLuint pointVAO, pointVBO;
void render_points_and_lines(Shader &shader, const vector<glm::vec2> &points)
{
	if (pointVAO == 0)
	{
		glGenVertexArrays(1, &pointVAO);
		glGenBuffers(1, &pointVBO);
	}

	// draw point
	glBindVertexArray(pointVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * points.size() * sizeof(GLfloat), points.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader.use();
	shader.setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));
	glBindVertexArray(pointVAO);
	glPointSize(5.0f);
	glDrawArrays(GL_POINTS, 0, points.size());

	// draw line
	glDrawArrays(GL_LINE_STRIP, 0, points.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// render bezier curve (all points on the curve)
GLuint curveVAO, curveVBO;
void render_bezier_curve(Shader &shader, const vector<glm::vec2> &points, ImVec4 color)
{
	if (curveVAO == 0)
	{
		glGenVertexArrays(1, &curveVAO);
		glGenBuffers(1, &curveVBO);
	}

	// draw point
	glBindVertexArray(curveVAO);
	glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * points.size() * sizeof(GLfloat), points.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader.use();
	shader.setVec3("color", glm::vec3(color.x, color.y, color.z));
	glBindVertexArray(curveVAO);
	glPointSize(3.0f);
	glDrawArrays(GL_POINTS, 0, points.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	glm::vec2 ndc_pos = ndc_norm(glm::vec2(xpos, ypos));

	if (isMouseLeftButtonPressed)
	{
		util_iterator = get_nearby_point(ndc_pos, 1);
		if (util_iterator != control_points.end())
		{
			util_iterator->x = ndc_pos.x;
			util_iterator->y = ndc_pos.y;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		isBanClick = !isBanClick;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		isDynamicDraw = !isDynamicDraw;
	}
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mode)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	glm::vec2 ndc_pos = ndc_norm(glm::vec2(xpos, ypos));

	// cout << control_points.size() << endl;
	// cout << xpos << ", " << ypos << endl;

	if (button == GLFW_MOUSE_BUTTON_LEFT && !isBanClick && !isDynamicDraw)
	{
		// Add point to control_points
		if (action == GLFW_PRESS)
		{
			// if no nearby point
			isMouseLeftButtonPressed = true;
			util_iterator = get_nearby_point(ndc_pos, 5e-3);
			if (util_iterator == control_points.end())
			{
				control_points.push_back(ndc_pos);
				util_iterator = control_points.end();
			}
		}

		if (action == GLFW_RELEASE)
		{
			isMouseLeftButtonPressed = false;
		}
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && !isBanClick && !isDynamicDraw)
	{
		util_iterator = get_nearby_point(ndc_pos, 5e-3);
		if (util_iterator != control_points.end())
		{
			control_points.erase(util_iterator);
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

glm::vec2 ndc_norm(glm::vec2 scr_pos)
{
	// (scr_x - scr_width/2) / scr_width  -> [-0.5. 0.5]
	// ndc_x = 2 * ((scr_x - scr_width/2) / scr_width)) = 2 * (scr_x / scr_width) - 1
	// (scr_y - scr_height/2) / scr_height -> [-0.5, 0.5]
	// ndc_y = -2 * ((scr_y - scr_height/2) / scr_height) = -2 * (scr_y / scr_height) + 1
	glm::vec2 ret;
	ret.x = 2 * (scr_pos.x / SCR_WIDTH) - 1;
	ret.y = 1 - 2 * (scr_pos.y / SCR_HEIGHT);
	return ret;
}

vector<glm::vec2>::iterator get_nearby_point(glm::vec2 scr_pos, const float sensitivity)
{

	vector<glm::vec2>::iterator ret = control_points.end();
	float minDist = 5;

	glm::vec2 ndc_pos = ndc_norm(scr_pos);

	// lambda function getDist: calculate the euclidean distance of two 2-d point
	auto getDist = [scr_pos](const vector<glm::vec2>::iterator iter) -> float
	{
		return pow((scr_pos.x - iter->x), 2) + pow((scr_pos.y - iter->y), 2);
	};

	// find nearby point: the nearest one!
	for (auto iter = control_points.begin(); iter != control_points.end(); ++iter)
	{
		auto dist = getDist(iter);
		// in sensitivity range
		if (dist < sensitivity)
		{
			if (dist < minDist)
			{
				ret = iter;
				minDist = dist;
			}
		}
	}

	return ret;
}


int factorial(int n)
{
	if (n == 0 || n == 1) return 1;
	int ret = 1;
	for (int i = 1; i <= n; ++i)
	{
		ret *= i;
	}
	return ret;
}

float bernstein(int i, int n, float t)
{
	float ret = float(factorial(n)) / float(factorial(i) * factorial(n - i)) * pow(t, i) * pow(1 - t, n - i);
	return ret;
}

void get_bezier_curve(int ctrl_points_num)
{
	bezier_curve_points.clear();
	// draw bezier curve when more then 2 control points: 2 control points means 1 degree of bezier curve
	if (ctrl_points_num >= 2)
	{
		float step = 0.001;
		int points_num = (1 / 0.001);
		bezier_curve_points.resize(points_num);
		for (int i = 0; i < points_num; ++i)
		{
			float t = i * step;
			int n = ctrl_points_num - 1;
			bezier_curve_points[i] = get_bezier_curve_point(n, t);
		}
		
	}
}

glm::vec2 get_bezier_curve_point(int n, float t)
{
	
	glm::vec2 ret(0.0f);
	for (int i = 0; i <= n; ++i)
	{
		glm::vec2 P_i = control_points[i];
		float B_in = bernstein(i, n, t);
		ret += B_in * P_i;
	}

	return ret;
}

// draw intermediate points iteratively
void draw_intermediate_points(Shader &shader, const vector<glm::vec2> &points, float t)
{
	if (points.size() == 1)
	{
		return;
	}
	vector<glm::vec2> next_points;
	next_points.resize(points.size() - 1);
	for (int i = 0; i < points.size() - 1; ++i)
	{
		float x = (1 - t) * points[i].x + t * points[i + 1].x;
		float y = (1 - t) * points[i].y + t * points[i + 1].y;
		next_points[i] = glm::vec2(x, y);
	}
	render_points_and_lines(shader, next_points);
	draw_intermediate_points(shader, next_points, t);
}