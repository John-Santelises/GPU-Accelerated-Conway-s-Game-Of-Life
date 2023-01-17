#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <thread>


#include <GL/glut.h>
#include <GL/GL.h>
#include <GL/GLU.h>
#include <amp.h>
#include <WinUser.h>

#include <iostream>
using namespace std;

int processDPIAware = SetProcessDPIAware();
HDC screen = GetDC(NULL);
int WINDOW_DIMEN[] = { GetDeviceCaps(screen, HORZRES) , GetDeviceCaps(screen, VERTRES) };

constexpr float CELL_SIZE = 1;
int WINDOW_ID;

int RESTART_FLAG = 0;
int QUIT_FLAG = 0;

double MAXIMUM_PLAY_AREA[]{ 0 , WINDOW_DIMEN[0] / (CELL_SIZE) , 0, WINDOW_DIMEN[1] / (CELL_SIZE) };

#define START_CAMARA() 	glMatrixMode(GL_PROJECTION); glLoadIdentity()
#define START_MODEL() 	glMatrixMode(GL_MODELVIEW); glLoadIdentity()
#define random() ((double) rand()) / RAND_MAX

class life {
private:
	int* life_matrix;
public:
	int width, height;
	const static int FAILURE = -1;

	life(int w, int h) {
		this->width = w;
		this->height = h;

		this->life_matrix = (int*)malloc(sizeof(int) * w * h); if (this->life_matrix == 0) exit(-1);

		double life_chance = (random() * 0.30) + 0.30;

		for (int i = 0; i < w * h; i++) {
			this->life_matrix[i] = (int)(random() < life_chance ? 1 : 0);
		}
	}
	~life() {
		free(this->life_matrix);
	}

	int get_cell(int x, int y) {
		if (x >= this->width || y >= this->height) {
			return life::FAILURE;
		}
		return this->life_matrix[(y * this->width) + x]; 
	}

	
	void new_board() {
		using namespace concurrency;
		int size = this->width * this->height;
		int size_bytes = size * sizeof(int);
		int* old_board = (int*)malloc(size_bytes); if (old_board == 0) exit(-1);
		memcpy(old_board, this->life_matrix, size_bytes);

		int board_dimen[] = { this->width, this->height };

		array_view<int, 1> old_board_amp(size, old_board);
		array_view<int, 1> board_dimen_amp(2, board_dimen);
		array_view<int, 1> result_board_amp(size, this->life_matrix);
		result_board_amp.discard_data();

		parallel_for_each(
			result_board_amp.extent,
			[=](index<1> idx) restrict(amp) {
				int width = board_dimen_amp[0];
				int height = board_dimen_amp[1];
				int index = idx[0];

				// Treating 1D Array like Matrix
				int x = index % width;
				int y = index / width;
				int act_index = (y * width) + x;

				int sum_of_alive = 0;
				
				// Iterate Neighbors
				for (int x_2 = -1; x_2 <= 1; x_2++) {
					for (int y_2 = -1; y_2 <= 1; y_2++) {
						int n_x = x + x_2, n_y = y + y_2;

						// Self check and out of bounds check
						if ((x_2 == 0 && y_2 == 0) || n_x < 0 || n_x >= width || n_y < 0 || n_y >= height) continue;
						
						// Neighbor Alive? Sum it
						if (old_board_amp[(n_y * width) + n_x]) sum_of_alive++;
					}
				}

				// Game rules, for determining if cell should live or die
				result_board_amp[act_index] = 0;
				if (sum_of_alive < 2 && old_board_amp[act_index]) {
					result_board_amp[act_index] = 0;
				}
				else if ((sum_of_alive == 2 || sum_of_alive == 3) && old_board_amp[act_index]) {
					result_board_amp[act_index] = 1;
				}
				else if (sum_of_alive > 3 && old_board_amp[act_index]) {
					result_board_amp[act_index] = 0;
				}
				else if (sum_of_alive == 3 && !old_board_amp[act_index]) {
					result_board_amp[act_index] = 1;
				}
			}
		);
		result_board_amp.synchronize();
		free(old_board);
	}

	void OPEN_GL_DRAW() {
		for (int x = 0; x < this->width; x++) {
			for (int y = 0; y < this->height; y++) {
				if (this->get_cell(x, y)) {
					START_MODEL();
					glTranslatef(x * CELL_SIZE, y * CELL_SIZE, 0.0f);
					glBegin(GL_POLYGON);
					glVertex2f(0.0, 0.0);
					glVertex2f(CELL_SIZE, 0.0);
					glVertex2f(CELL_SIZE, CELL_SIZE);
					glVertex2f(0.0, CELL_SIZE);
					glEnd();
				}
			}
		}
	}
};


void display() {
	using namespace std::this_thread;
	using namespace std::chrono;
	static life* life_instance = new life(MAXIMUM_PLAY_AREA[1], MAXIMUM_PLAY_AREA[3]);

	if (RESTART_FLAG) {
		delete life_instance;
		life_instance = new life(MAXIMUM_PLAY_AREA[1], MAXIMUM_PLAY_AREA[3]);
		RESTART_FLAG = 0;
	}

	if (QUIT_FLAG) {
		delete life_instance;
		exit(0);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

	life_instance->new_board();
	life_instance->OPEN_GL_DRAW();

	glFlush();

	glutSwapBuffers();
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 'r') RESTART_FLAG = 1;
	if (key == 'q' || key == 27) QUIT_FLAG = 1;
}

void reshape(int w, int h) {
	START_CAMARA();
	glViewport(0, 0, w, h);

	glOrtho(
		0,
		WINDOW_DIMEN[0],
		0,
		WINDOW_DIMEN[1],
		-1.0,
		1.0
	);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	WINDOW_ID = glutCreateWindow("Conway's Game of Life");
	glutFullScreen();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	srand(time(NULL));

	

	glutMainLoop();

	return 0;
}