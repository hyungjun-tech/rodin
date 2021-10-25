varying vec2 texcoords;

void main(void)
{
   gl_Position = gl_Vertex;
   texcoords = (gl_Vertex.xy + 1.0) * 0.5;
}