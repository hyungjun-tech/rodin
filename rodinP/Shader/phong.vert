attribute vec3 vertex_position;
attribute vec3 vertex_normal;

uniform mat4 proj, view, model;
//uniform vec4 clipPlane;

varying vec3 position_eye, normal_eye, model_position;


void main () {
	position_eye = vec3 (view * model * vec4 (vertex_position, 1.0));
	normal_eye = vec3 (view * model * vec4 (vertex_normal, 0.0));
	gl_Position = proj * vec4 (position_eye, 1.0);
	model_position = vec3 (model * vec4 (vertex_position, 1.0));
	//gl_ClipDistance[0] = dot(model * vec4 (vertex_position, 1.0), clipPlane);
}