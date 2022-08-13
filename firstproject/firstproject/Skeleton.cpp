//=============================================================================================
// Computer Graphics Homework
//
// Name: dhouioui oussama
// Neptun code: XFCBM7
// 
// I hereby declare that the homework has been made by me, including the problem interpretation,
// algorithm selection, and coding. Should I use materials and programs not from the course webpage, 
// the sources are clearly indentified as a comment in the code. 
//=============================================================================================
#include "framework.h"
 
using namespace std;
 
 
// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char* const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers
 
	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0
 
	void main() {
	    vec4 vpos = vec4(vp.x, vp.y, 0, 1) * MVP;
      float z = sqrt(vpos.x * vpos.x + vpos.y * vpos.y + 1); 
     gl_Position = vec4(vpos.x / (z + 1), vpos.y / (z + 1), 0, 1);
	 
 
	}
)";
 
// fragment shader in GLSL
const char* const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel
 
	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";
 
int x = 1;
 
 
GPUProgram gpuProgram; // vertex and fragment shaders
   // virtual world on the GPU
const int nv = 50;
vec2 v(0, 0);
vec2 p(0, 0);
vec2 vertices[nv];
vec2 view(0, 0);
 
 
class particule {
public:
	unsigned int vao;
	float mass;
	float charge;
	vec2 position;
	vec2 velocity;
	vec3 color;
	float scale;
	vector<particule> particules;
 
	particule(float a, float b, vec2 f, vec2 d) {
		color = vec3(0, 0, 0);
		mass = a;
		charge = b;
		position = f;
		velocity = d;
		b > 0 ? color.x = charge * 2 : color.y = -charge * 2;
		scale = (float)mass/ 2;
	}
 
	particule() {
	}
 
	void add(particule s) {
		particules.push_back(s);
	}
	void create() {
		
		glGenVertexArrays(1, &vao);	// get 1 vao id
		glBindVertexArray(vao);		// make it active
 
		unsigned int vbo;		// vertex buffer object
		glGenBuffers(1, &vbo);	// Generate 1 buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
 
		for (int i = 0; i < nv; i++) {
			float phi = i * 2 * M_PI / nv;
			vertices[i] = (vec2(cos(phi), sin(phi)) * 0.2f);
		}
 
 
 
		glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
			sizeof(vertices),  // # bytes
			vertices,	      	// address
			GL_STATIC_DRAW);	// we do not change later
 
		glEnableVertexAttribArray(0);  // AttribArray 0
		glVertexAttribPointer(0,       // vbo -> AttribArray 0
			2, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
			0, NULL);
 
	}
 
	void animate(float a, vec3 c, int i) {
 
 
		mat4 animation = ScaleMatrix(vec3(a, a, 0)) * TranslateMatrix(vec3(particules[i].position.x, particules[i].position.y, 0) + vec3(view.x, view.y, 0) );
		gpuProgram.setUniform(animation, "MVP");
		gpuProgram.setUniform(c, "color");
	}
 
	void coulomblaw(int i , float dt) {
 
 
		for (int z = 0; z < particules.size(); z++)
		{
			if (i != z) {
				float Fcoulomb = 0.899* particules[i].charge * particules[z].charge / pow(length(particules[i].position - particules[z].position), 2);
				vec2 force = normalize(particules[i].position - particules[z].position) * Fcoulomb;
				particules[i].velocity = particules[i].velocity + force / particules[i].mass * dt;
 
			}
 
			
 
		}
		particules[i].velocity = particules[i].velocity + 0.01 * -particules[i].velocity / particules[i].mass * dt;
 
 
		particules[i].position = particules[i].position + particules[i].velocity  * dt;
 
	}
 
	void simulation(float t) {
		int i = 0;
		while (i < particules.size()) {
 
 
			coulomblaw(i,t);
 
			
			
			i++;
		}
 
	}
 
	void draw() {
		glClear(GL_COLOR_BUFFER_BIT);
 
		glBindVertexArray(vao);
 
		int i = 0;
		while (i < particules.size()) {
		
			animate(particules[i].scale, particules[i].color, i);
			glDrawArrays(GL_TRIANGLE_FAN, 0, nv);
 
			i++;
		
		}
	}
 
 
};
particule mainone;
particule part1((float)(rand() % 1000) / 100.0f - 5.0f,
	((float)rand() / RAND_MAX + 1) - ((float)rand() / RAND_MAX + 1), p, v);
particule part2((float)(rand() % 1000) / 100.0f - 5.0f,
	((float)rand() / RAND_MAX + 1) - ((float)rand() / RAND_MAX + 1), (0.5, 0.5), v);
 
 
 
 
// Initialization, create an OpenGL context
void onInitialization() {
	
	glViewport(0, 0, 600, 600);
	//glViewport(0, 0, windowWidth, windowHeight);
	mainone.create();
	mainone.add(part1);
	mainone.add(part2);
	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
 
}
 
// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);     // background color
 
	
 
	mainone.draw();
 
 
	glutSwapBuffers(); // exchange buffers for double buffering
}
 
// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == ' ') {
		
		vec2 a;
		a.x = ((float)rand() / RAND_MAX + 1) - ((float)rand() / RAND_MAX + 1);
		a.y = ((float)rand() / RAND_MAX + 1) - ((float)rand() / RAND_MAX + 1);
		particule part((float)(rand() % 1000) / 100.0f - 5.0f,
			((float)rand() / RAND_MAX + 1) - ((float)rand() / RAND_MAX + 1),
			a,
			vec2(0, 0));
		mainone.add(part);
	}
	else
		if (key == 'd') {
			view.x = view.x + 0.1;
		
		}
		else
			if (key == 's') {
				view.x = view.x - 0.1;
			}
			else
				if (key == 'x') {
					view.y = view.y - 0.1;
				}
				else if (key == 'e') {
					view.y = view.y + 0.1;
				}
	glutPostRedisplay();
}
 
// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}
 
// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
 
}
 
// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
 
}
 
// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	static long time = 0;
	long current = glutGet(GLUT_ELAPSED_TIME);
	long elapsed = current - time;
	time = current;
	while (elapsed > 0) {
		float dt = min((float)elapsed / 1000.0f, 0.01f);
		elapsed -= 10;
		mainone.simulation(dt);
	}
	glutPostRedisplay();
}
 