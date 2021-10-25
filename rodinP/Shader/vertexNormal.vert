attribute vec3 vertex_position;
attribute vec3 vertex_normal;
uniform mat4 model, view, proj, normal;

varying vec3 colour;

void main() {
	vec4 world_normal = normal * vec4(vertex_normal, 0.0);
	colour = (vec3(world_normal) + vec3(1.0, 1.0, 1.0));
	colour[0] = 0.5*colour[0];
	colour[1] = 0.5*colour[1];
	colour[2] = 0.5*colour[2];
	gl_Position = proj * view * model * vec4 (vertex_position, 1.0);
}
