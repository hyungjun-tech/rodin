attribute vec3 vertex_position;
attribute vec4 vertex_color;
uniform mat4 model, view, proj;
varying vec4 f_color;


void main() {
	f_color = vertex_color;
	gl_Position = proj * view * model * vec4 (vertex_position, 1.0);
}
