#include <iostream>
#include "Vec3.h"
#include <string>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>

struct Object
{
	Vec3 centre_position;
	std::vector<Vec3> vertices;
	std::vector<int> edges;
	float size;
};

void set_square_font()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_FONT_INFOEX fontInfo = { sizeof(fontInfo) };
	fontInfo.dwFontSize.X = 8; // Width of each character in pixels
	fontInfo.dwFontSize.Y = 8; // Height of each character in pixels
	SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
}

#endif

std::vector<std::pair<float, float>> project_vertices(const Object& object, int width, int height, float fov, float camera_distance)
{
	std::vector<std::pair<float, float>> projected_vertices;
	
	float aspect_ratio = static_cast<float>(width) / height;
	float fov_scale_factor = 1.0f / tan(fov * 0.5f * 3.14 / 180);

	for (const auto& vertex : object.vertices)
	{
		float distance = vertex.z + camera_distance;
		float division_factor = fov_scale_factor / distance * object.size;

		float x = vertex.x / division_factor; //perspective projection
		float y = vertex.y / division_factor;
		projected_vertices.emplace_back(x, y);
	}
	return projected_vertices;
}

std::pair<int, int> convert_position(const std::pair<float, float> point, float scaling, int width, int height)
{
	int x = static_cast<int>(((point.first * scaling) + 1.0f) * 0.5f * static_cast<float>(width - 1));
	int y = static_cast<int>((1.0f - ((point.second * scaling) + 1.0f) * 0.5f) * static_cast<float>(height - 1));
	return { x, y };
}

void rotate_x(Vec3& point, float angle)
{
	float angle_radians = angle * 3.14 / 180;
	float y = point.y * cos(angle) - point.z * sin(angle);
	float z = point.y * sin(angle) + point.z * cos(angle);
	point.y = y;
	point.z = z;
}

void rotate_y(Vec3& point, float angle)
{
	float angle_radians = angle * 3.14 / 180;
	float x = point.x * cos(angle) + point.z * sin(angle);
	float z = -point.x * sin(angle) + point.z * cos(angle);
	point.x = x;
	point.z = z;
}

void draw_line(std::pair<int, int> start_vertex, std::pair<int, int> end_vertex, std::vector<char>& buffer, int width)
{
	int dx = end_vertex.first - start_vertex.first;
	int dy = end_vertex.second - start_vertex.second;

	int steps = std::max(std::abs(dx), std::abs(dy));

	if (steps == 0) return;

	float x_increment = static_cast<float>(dx) / steps;
	float y_increment = static_cast<float>(dy) / steps;
	float x = static_cast<float>(start_vertex.first);
	float y = static_cast<float>(start_vertex.second);

	for (int j = 0; j <= steps; ++j)
	{
		int screen_x = static_cast<int>(std::round(x + j * x_increment));
		int screen_y = static_cast<int>(std::round(y + j * y_increment));

		size_t screen_index = screen_y * width + screen_x;
		buffer[screen_index] = '.';

	}
}

std::vector<char> create_buffer(const Object& object, std::vector<char>& buffer, int width, int height)
{
	float fov = 90.0f;
	float camera_distance = 3.0f;

	std::vector<std::pair<float, float>> projected_vertices = project_vertices(object, width, height, fov, camera_distance);
	std::vector<std::pair<int, int>> screen_vertices;

	for (const auto& vertex : projected_vertices)
	{
		std::pair<int, int> position = convert_position(vertex, 1.0f, width, height);
		buffer[position.second * width + position.first] = '.';
		screen_vertices.push_back(position);
	}

	for (size_t i = 0; i < object.edges.size(); i += 2)
	{
		int start_index = object.edges[i];
		int end_index = object.edges[i + 1];

		std::pair<int, int> start_vertex = screen_vertices[start_index];
		std::pair<int, int> end_vertex = screen_vertices[end_index];

		draw_line(start_vertex, end_vertex, buffer, width);
	}

	return buffer;
}


int main()
{
#ifdef _WIN32
	set_square_font();
	std::ios_base::sync_with_stdio(false);
#endif

	std::cout << "\033[?25l"; // Hide cursor

	const std::string gradient = "|/-\|";

	int width = 80;
	int height = 40;

	Object cube;
	cube.size = 12.0f;
	cube.centre_position = Vec3(0, 0, 0);
	cube.vertices = {
		Vec3(-1, -1, -1),
		Vec3(1, -1, -1),
		Vec3(1,  1, -1),
		Vec3(-1,  1, -1),
		Vec3(-1, -1,  1),
		Vec3(1, -1,  1),
		Vec3(1,  1,  1),
		Vec3(-1,  1,  1)
	};

	cube.edges = {
		// Bottom square
		0, 1,  1, 2,  2, 3,  3, 0,
		// Top square
		4, 5,  5, 6,  6, 7,  7, 4,
		// Vertical edges
		0, 4,  1, 5,  2, 6,  3, 7
	};

	std::vector<char> buffer(width * height, ' ');
	std::string screen;
	screen.reserve((width*1) * height + 10);

	float x_rotation = 0.0f;
	float y_rotation = 0.0f;

	std::vector<Vec3> original_vertices = cube.vertices;

	while (true)
	{ 
		screen.clear();
		screen += "\033[H";

		buffer.assign(width * height, ' ');

		std::vector<Vec3> vertices_copy = original_vertices;

		for (auto& v : vertices_copy)
		{
			rotate_x(v, x_rotation);
			rotate_y(v, y_rotation);
		}

		cube.vertices = vertices_copy;

		buffer = create_buffer(cube, buffer, width, height);

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				screen += buffer[y * width + x];
			}
			screen += '\n';
		}

		std::cout << screen << std::flush;

		x_rotation += 0.1f;
		y_rotation += 0.2f;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	

	// Restore cursor visibility before exit
	std::cout << "\033[?25h";
}