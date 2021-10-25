uniform sampler2D mask;
uniform vec4 lineColor;
uniform float width;
uniform float height;
uniform float lineThickness;

float IsEdge(in vec2 coords)
{
	float dxtex = lineThickness / width;
	float dytex = lineThickness / height;
  
	float depth0 = texture2D(mask, coords).r;
	float depth1 = texture2D(mask, coords + vec2(dxtex, 0.0)).r;
	float depth2 = texture2D(mask, coords + vec2(0.0, -dytex)).r;
	float depth3 = texture2D(mask, coords + vec2(-dxtex, 0.0)).r;
	float depth4 = texture2D(mask, coords + vec2(0.0, dytex)).r;
  
	float ddx = abs((depth1 - depth0) - (depth0 - depth3));
	float ddy = abs((depth2 - depth0) - (depth0 - depth4));
	return clamp((ddx + ddy - 0.01)*100.0, 0.0, 1.0);
}

varying vec2 texcoords;

void main(void)
{
	gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);

	float delta = IsEdge(texcoords);
	if(delta > 0.0)
	{
		gl_FragColor = vec4(lineColor.xyz, 1.0);
	}
}