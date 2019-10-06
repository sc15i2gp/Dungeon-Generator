#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include "graphics.h"
#include "rng.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640

typedef graphical_data_buffer tile_graphical_data;

//IN PROGRESS:
//TODO: Generate 128*128 dungeon using procedural gen method
//	- Change it such that every partition has a room in it (any partition split down further should produce two partitions big enough to contain at least the minimum sized rooms)
//	- Fill in hallways between rooms

//PENDING:
//TODO: Make this better
//	- The same dungeon should be able to be generated from the same abstract model multiple times, currently everything is randomly decided when the tile map is generated
//	- Toggle partition lines
//	- Only generate new command buffers when necessary, instead of every frame
//	- Parameterise room gen better (more formally)
//	- Transfer generated dungeon to gpu as texture
//ISSUE: The cells which are set to be floor tiles were transposed in the generate_rooms() method to display correctly, find out why.
//TODO: Separate vulkan code from platform code as much as is possible
//TODO: Implement my own trig functions
//TODO: Automate build to include c files in compilation command
//TODO: Remove uint64_t, make own macros
//TODO: Proper error handling in graphics code

int max(int n, int m)
{
	return (n >= m) ? n : m;
}

int min(int n, int m)
{
	return (n <= m) ? n : m;
}

struct bsp_node
{
	vec2d top_right;
	vec2d bottom_left;
	vec2d room_top_right;
	vec2d room_bottom_left;
	int partition_direction;
	int partition_position;
	bsp_node* left_child;
	bsp_node* right_child;
};

#define MIN_PARTITION 16
#define MIN_ROOM 4

#define HORIZONTAL 0
#define VERTICAL 1

//bsp tree:
//	- At least 2 levels deep

bsp_node* generate_bsp_tree(vec2d bottom_left, vec2d top_right, int level = 0)
{
	bsp_node* tree = (bsp_node*)malloc(sizeof(bsp_node));
	tree->bottom_left = bottom_left;
	tree->top_right = top_right;
	tree->left_child = NULL;
	tree->right_child = NULL;
	tree->partition_direction = -1;

	vec2d dimensions = top_right - bottom_left;

	if(dimensions[HORIZONTAL] > MIN_PARTITION || dimensions[VERTICAL] > MIN_PARTITION)
	{
		int should_partition = rng() % 5;
		if(should_partition || level < 2)
		{
			//Choose direction of partition
			int direction = rng() % 2;
			if(dimensions[direction] < MIN_PARTITION) direction = (direction+1)%2;

			//Choose position of partition along direction
			int min = bottom_left[direction] + MIN_ROOM + 2;
			int max = top_right[direction] - MIN_ROOM - 2;
			int partition_position = rng_range(min, max);
			
			//Find bottom_left and top_right for left and right child nodes
			//If direction is x (partition line is drawn parallel to y axis)
			//	Left child bottom_left is same as current_bottom_left
			//	Right child bottom_left is {partition_position, bottom_left.y}
			//	Left child top_right is {partition_position - 1, top_right.y}
			//	Right child top_right is same as current top_right
			//If direction is y (partition line is drawn parallel to x axis)
			//	Left child bottom left is {bottom_left.x, partition_position}
			//	Right child bottom_left is same as current_bottom_left
			//	Left child top_right is same as current top_right
			//	Right child top_right is {top_right.x, partition_position - 1}
			vec2d l_child_bottom_left = (direction == HORIZONTAL) ? bottom_left : vec2d{bottom_left.x, partition_position};
			vec2d l_child_top_right = (direction == HORIZONTAL) ? vec2d{partition_position - 1, top_right.y} : top_right;
			vec2d r_child_bottom_left = (direction == HORIZONTAL) ? vec2d{partition_position, bottom_left.y} : bottom_left;
			vec2d r_child_top_right = (direction == HORIZONTAL) ? top_right : vec2d{top_right.x, partition_position - 1};

			tree->partition_position = partition_position;

			//Create child nodes
			tree->partition_direction = direction;
			tree->left_child = generate_bsp_tree(l_child_bottom_left, l_child_top_right, level+1);
			tree->right_child = generate_bsp_tree(r_child_bottom_left, r_child_top_right, level+1);
		}
	}
	return tree;
}

void destroy_bsp_tree(bsp_node* tree)
{
	if(tree->left_child) destroy_bsp_tree(tree->left_child);
	if(tree->right_child) destroy_bsp_tree(tree->right_child);
	free(tree);
}

bool running = false;
bool resizing = false;
bool resized = false;

LRESULT CALLBACK WindowEventHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch(message)
	{
		case WM_SYSCOMMAND:
			resized = wParam == SC_MAXIMIZE || wParam == SC_RESTORE;
			DefWindowProcA(window, message, wParam, lParam);
			break;
		case WM_SIZING:
			//This case only applies when click+drag
			resizing = true;
			break;
		case WM_EXITSIZEMOVE:
			if(resizing)
			{
				resizing = false;
				resized = true;
			}
			break;
		case WM_DESTROY:
			printf("DESTROY WINDOW MESSAGE CALLED\n");
			break;
		case WM_CLOSE:
			running = false;
			break;
		default:
			result = DefWindowProc(window, message, wParam, lParam);
			break;
	}
	return result;
}

tile_graphical_data buffer_rect(vulkan_state* vulkan, vec3d colour)
{
	vertex vertices[] =
	{
		{{1.0f, 1.0f}, colour},
		{{0.0f, 1.0f}, colour},
		{{0.0f, 0.0f}, colour},
		{{1.0f, 0.0f}, colour}
	};

	uint16_t indices[] = {0, 1, 2, 2, 3, 0};

	graphical_data_buffer tile = buffer_graphical_data(vulkan, vertices, 4, indices, 6);
	return tile;
}

graphical_data_buffer buffer_triangle(vulkan_state* vulkan, vec3d colour)
{
	vertex vertices[] =
	{
		{{64.0f, 128.0f}, colour},
		{{0.0f, 0.0f}, colour},
		{{128.0f, 0.0f}, colour}
	};

	uint16_t indices[] = {0, 1, 2};

	graphical_data_buffer triangle = buffer_graphical_data(vulkan, vertices, 3, indices, 3);
	return triangle;
}

graphical_data_buffer buffer_line(vulkan_state* vulkan, vec2d p_0, vec2d p_1, vec3d colour)
{
	vertex vertices[] = 
	{
		{p_0, colour},
		{p_1, colour}
	};
	graphical_data_buffer line = buffer_graphical_data(vulkan, vertices, 2);
	return line;
}

struct timer
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER start;
	LARGE_INTEGER end;
};

void start_timer(timer* t)
{
	QueryPerformanceFrequency(&t->frequency);
	QueryPerformanceCounter(&t->start);
}

void end_timer(timer* t)
{
	QueryPerformanceCounter(&t->end);
}

long int time_elapsed_millisec(timer* t)
{
	LARGE_INTEGER elapsed;
	elapsed.QuadPart = t->end.QuadPart - t->start.QuadPart;
	elapsed.QuadPart *= 1000;
	elapsed.QuadPart /= t->frequency.QuadPart;
}

long int current_time()
{
	timer t;
	start_timer(&t);
	return t.start.QuadPart;
}

#define WALL 0
#define FLOOR 1
#define PARTITION 2

char tile_map[128][128] = {};
tile_graphical_data tgd_table[8] = {};

//Recursively generates hallways connecting the given node's child nodes
void generate_hallways(bsp_node* node)
{
	//Each hallway is 1 wide and n long
	//Need to connect from one of the first child's outer floor tile to one of the second's outer floor tile
	//Take bounding boxes containing all floor tiles of each child, rectangle (minx,miny) (maxx, maxy)
	//Hallways generated should be single straight lines of floor tiles
	//The partitions always have matching bounds and are adjacent, but within the bounds of each the rooms may not be aligned along the partition direction
	
	//If bounds of both children can be connected by a single line across the partition direction
	//Generate hallway at random position between overlapping bounds
	//Else
	//Either generate hallway connecting random position along bound of first partition in direction perpendicular to the partition direction, to random position along
	//bound of second partition in the partition direction
	//or generate hallway along fist partition bound in partition direction, connecting to random position along second bound in direction perpendicular to partition direction

	//If node's children are not leaf nodes
	//Generate hallways between child nodes
	if(node->left_child && node->left_child->left_child) generate_hallways(node->left_child);
	if(node->right_child && node->right_child->right_child) generate_hallways(node->right_child);

	//Get bounds of both children's floor tiles
	vec2d left_child_room_bounds[2] = {node->left_child->room_bottom_left, node->left_child->room_top_right};
	vec2d right_child_room_bounds[2] = {node->right_child->room_bottom_left, node->right_child->room_top_right};

	int overlap;
	int bound_direction = 1 - node->partition_direction;
	int bound_max = min(left_child_room_bounds[1][bound_direction], right_child_room_bounds[1][bound_direction]);
	int bound_min = max(left_child_room_bounds[0][bound_direction], right_child_room_bounds[0][bound_direction]);
	int hallway_position = rng_range(bound_min, bound_max);
	overlap = max(0, bound_max - bound_min);

	vec2d hallway_center = {}; //Not literal center, just at hallway position along partition line
	hallway_center[bound_direction] = (float)hallway_position;
	hallway_center[1-bound_direction] = node->partition_position;
	if(overlap > 0)
	{

		if(node->partition_direction == HORIZONTAL)
		{
			for(int x = (int)hallway_center.x; tile_map[(int)hallway_center.y][x] == WALL; ++x) tile_map[(int)hallway_center.y][x] = FLOOR;
			for(int x = (int)hallway_center.x - 1; tile_map[(int)hallway_center.y][x] == WALL; --x) tile_map[(int)hallway_center.y][x] = FLOOR;
		}
		else
		{
			for(int y = (int)hallway_center.y; tile_map[y][(int)hallway_center.x] == WALL; ++y) tile_map[y][(int)hallway_center.x] = FLOOR;
			for(int y = (int)hallway_center.y - 1; tile_map[y][(int)hallway_center.x] == WALL; --y) tile_map[y][(int)hallway_center.x] = FLOOR;
		}
	}
	else
	{
		int hallway_position_1_max = (int)right_child_room_bounds[1][node->partition_direction]; //Oriented to different axis to position_0
		int hallway_position_1_min = (int)right_child_room_bounds[0][node->partition_direction];

		int hallway_position_1 = rng_range(hallway_position_1_min, hallway_position_1_max);
		
		vec2d hallway_center_1 = {};
		hallway_center_1[1-bound_direction] = hallway_position_1;
		hallway_center_1[bound_direction] = hallway_center[bound_direction];
		if(node->partition_direction == HORIZONTAL)
		{
			for(int x = (int)hallway_center.x-1; tile_map[(int)hallway_center.y][x] == WALL; --x) tile_map[(int)hallway_center.y][x] = FLOOR;
			for(int x = (int)hallway_center.x; x <= hallway_center_1.x; ++x) tile_map[(int)hallway_center.y][x] = FLOOR;
			for(int y = (int)hallway_center_1.y; tile_map[y][(int)hallway_center_1.x] == WALL; --y) tile_map[y][(int)hallway_center_1.x] = FLOOR;
		}
		else
		{
			for(int y = (int)hallway_center.y-1; tile_map[y][(int)hallway_center.x] == WALL; --y) tile_map[y][(int)hallway_center.x] = FLOOR;
			for(int y = (int)hallway_center.y; y <= hallway_center_1.y; ++y) tile_map[y][(int)hallway_center.x] = FLOOR;
			for(int x = (int)hallway_center_1.x; tile_map[(int)hallway_center_1.y][x] == WALL; --x) tile_map[(int)hallway_center_1.y][x] = FLOOR;
		}
	}
	node->room_bottom_left = {min(node->left_child->room_bottom_left.x, node->right_child->room_bottom_left.x), min(node->left_child->room_bottom_left.y, node->right_child->room_bottom_left.y)};
	node->room_top_right = {max(node->left_child->room_top_right.x, node->right_child->room_top_right.x), max(node->left_child->room_top_right.y, node->right_child->room_top_right.y)};
}

void generate_rooms(bsp_node* node)
{
	//If node is a leaf
	if(!node->left_child && !node->right_child)
	{
		vec2d dimensions = node->top_right - node->bottom_left + vec2d{1.0f, 1.0f};

		int left_side = rng_range(node->bottom_left.x+1, node->top_right.x-MIN_ROOM+1);
		int right_side = rng_range(left_side+MIN_ROOM, node->top_right.x+1);
		int bottom_side = rng_range(node->bottom_left.y+1, node->top_right.y-MIN_ROOM+1);
		int top_side = rng_range(bottom_side+MIN_ROOM, node->top_right.y+1);
		node->room_bottom_left = vec2d{left_side, bottom_side};
		node->room_top_right = vec2d{right_side, top_side};
		for(int i = bottom_side; i < top_side; i++) for(int j = left_side; j < right_side; j++) tile_map[i][j] = FLOOR;
	}
	else
	{
		generate_rooms(node->left_child);
		generate_rooms(node->right_child);
	}
}

bsp_node* generate_dungeon()
{
	for(int i = 0; i < 128; i++) for(int j = 0; j < 128; j++) tile_map[i][j] = WALL;
	bsp_node* tree = generate_bsp_tree(vec2d{0.0f, 0.0f}, vec2d{127.0f, 127.0f});
	generate_rooms(tree);
	generate_hallways(tree);
	return tree;
}

int buffer_partition_lines(vulkan_state* vulkan, bsp_node* node, graphical_data_buffer** line_buffer)
{
	int partition_count = 0;
	if(node->left_child && node->right_child)
	{
		++partition_count;
		vec2d p_0 = (node->partition_direction == 0) ? node->right_child->bottom_left : node->left_child->bottom_left;
		vec2d p_1 = (node->partition_direction == 0) ? node->left_child->top_right + vec2d{1.0f, 1.0f} : node->right_child->top_right + vec2d{1.0f, 1.0f};
		p_0.x = (p_0.x / 64.0f) - 1.0f;
		p_0.y = (p_0.y / 64.0f) - 1.0f;
		p_1.x = (p_1.x / 64.0f) - 1.0f;
		p_1.y = (p_1.y / 64.0f) - 1.0f;
		**line_buffer = buffer_line(vulkan, p_0, p_1, vec3d{1.0f, 0.0f, 0.0f});
		*line_buffer += 1;
		partition_count += buffer_partition_lines(vulkan, node->left_child, line_buffer);
		partition_count += buffer_partition_lines(vulkan, node->right_child, line_buffer);
	}
	return partition_count;
}

int APIENTRY WinMain(HINSTANCE hinstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Set window class attributes
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = WindowEventHandler;
	window_class.lpszClassName = "DungeonGeneratorClass";

	timer t;

	seed_rng(current_time());
	if(RegisterClass(&window_class))
	{
		//Set window attributes
		HWND window = CreateWindowEx(0, window_class.lpszClassName, "Dungeon Generator",
				WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
				WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, prevInstance, 0);
		if(window)
		{
			vulkan_state vulkan;
			uint8_t vulkan_startup_result = startup_vulkan(&vulkan, window, hinstance);
			if(vulkan_startup_result != 0)
			{
				printf("Vulkan startup failed\n");
				return -1;
			}

			//Load tile graphical data
			tgd_table[PARTITION] = buffer_rect(&vulkan, vec3d{0.5f, 0.5f, 0.5f});
			tgd_table[WALL] = buffer_rect(&vulkan, vec3d{0.0f, 0.0f, 0.0f});
			tgd_table[FLOOR] = buffer_rect(&vulkan, vec3d{1.0f, 1.0f, 1.0f});

			mat4 ortho = orthographic_projection(0.0f, 128.0f, 0.0f, 128.0f, -1.0f, 1.0f);
			update_world_matrix(&vulkan, identity(), ortho);
			for(int i = 0; i < 128; i++)
			{
				for(int j = 0; j < 128; j++)
				{
					tile_map[i][j] = FLOOR;
				}
			}

			//Set partition lines in grid
			bsp_node* tree = generate_dungeon();
			graphical_data_buffer partition_lines[2048] = {};
			graphical_data_buffer* partition_lines_buffer = &partition_lines[0];
			int partition_count = 0;

			partition_count = buffer_partition_lines(&vulkan, tree, &partition_lines_buffer);

			running = true;

			//Window event handle loop
			while(running)
			{
				MSG message;
				while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if(message.message == WM_QUIT) running = false;
					else if(message.message == WM_SIZING) printf("DOING Resized\n");

					TranslateMessage(&message);
					DispatchMessage(&message);
				}

				start_timer(&t);
				if(resized)
				{
					complete_graphical_tasks(&vulkan);
					resize_window(&vulkan);
					resized = false;
				}

				begin_frame(&vulkan);
				
				for(int i = 0; i < 128; i++)
				{
					for(int j = 0; j < 128; j++)
					{
						vec3d position = {(float)j, (float)i, 0.0f};
						mat4 translation = translate(position);
						push_model_matrix(&vulkan, translation);
						draw(&vulkan, &tgd_table[tile_map[i][j]]);
					}
				}
				push_model_matrix(&vulkan, identity());
				for(int i = 0; i < partition_count; i++)
				{
					draw_line(&vulkan, &partition_lines[i]);
				}

				end_timer(&t);
				long int elapsed = time_elapsed_millisec(&t);
				//printf("Time elapsed = %ld\n", elapsed);
				render_frame(&vulkan);

			}
			complete_graphical_tasks(&vulkan);

			destroy_bsp_tree(tree);
			for(int i = 0; i < partition_count; i++) destroy_graphical_data(&vulkan, &partition_lines[i]);

			destroy_graphical_data(&vulkan, &tgd_table[PARTITION]);
			destroy_graphical_data(&vulkan, &tgd_table[FLOOR]);
			destroy_graphical_data(&vulkan, &tgd_table[WALL]);

			shutdown_vulkan(&vulkan);
			PostQuitMessage(0);
		}
		else
		{
			//TODO: Add error handler here
		}

	}
	else
	{
		//TODO: Add error handler here
	}
	return 0;
}
