varying vec3 position_eye, normal_eye, model_position;

//uniform mat4 modelview;
//uniform vec3 light_position_world;
uniform vec4 model_color;
uniform int layerCount;
uniform vec3 layerColors[2];
uniform float layerHeights[100];

// fixed point light properties

vec3 Ls = vec3 (0.2, 0.2, 0.2); // white specular colour
vec3 Ld = vec3 (0.9, 0.9, 0.9); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour
  
// surface reflectance
vec3 Ks = vec3 (1.0, 1.0, 1.0); // fully reflect specular light
//vec3 Kd = vec3 (1.0, 0.0, 0.0); // orange diffuse surface reflectance
vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
float specular_exponent = 10.0; // specular 'power'

//out vec4 fragment_colour; // final colour of surface

void main () {
	vec3 Kd = vec3 (model_color);
	float alpha = model_color[3];
	// ambient intensity
	vec3 Ia = La * Ka;
	// normalize in case interpolation has upset normals' lengths
	vec3 n_eye = normalize( normal_eye );

	// diffuse intensity
	// raise light position to eye space
	//vec3 light_position_eye = vec3 (modelview * vec4 (light_position_world, 1.0));
	vec3 light_position_eye = vec3 (0,0,0);
	vec3 distance_to_light_eye = light_position_eye - position_eye;
	vec3 direction_to_light_eye = normalize (distance_to_light_eye);
	float dot_prod = dot (direction_to_light_eye, n_eye);
	dot_prod = max (dot_prod, 0.0);
	
	vec3 color_;
	if (layerCount == 0) {
		color_ = Kd;
	} else {
        float before_height = 0.0;
		for (int i=0; i< layerCount + 1; i++)
		{
            int arrVal = i - 2 * (i/2);
			if (i == layerCount){
				color_ = layerColors[arrVal];
				break;
			}
			else if (before_height <= model_position[2] && model_position[2] < layerHeights[i]) {
				before_height = layerHeights[i];
				color_ = layerColors[arrVal];
				break;
			}
			else{
				color_ = layerColors[arrVal];
			}
		}
	}
	
	vec3 Id = Ld * color_ * dot_prod; // final diffuse intensity
	
	// specular intensity
	vec3 surface_to_viewer_eye = normalize (-position_eye);
	
	//vec3 reflection_eye = reflect (-direction_to_light_eye, n_eye);
	//float dot_prod_specular = dot (reflection_eye, surface_to_viewer_eye);
	//dot_prod_specular = max (dot_prod_specular, 0.0);
	//float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	// blinn
	vec3 half_way_eye = normalize (surface_to_viewer_eye + direction_to_light_eye);
	float dot_prod_specular = max (dot (half_way_eye, n_eye), 0.0);
	float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	vec3 Is = Ls * Ks * specular_factor; // final specular intensity
	
	// final colour
	gl_FragColor = vec4 (Is + Id + Ia, alpha);
}
