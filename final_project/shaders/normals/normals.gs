#version 330
layout(triangles) in;

// Three lines will be generated: 6 vertices
layout(line_strip, max_vertices=6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

out vec3 color;

const float MAGNITUDE = 0.5;

void GenerateLine(int index)
{
  gl_Position = gl_in[index].gl_Position;
  color=vec3(1.0,1.0,1.0);
  EmitVertex();
  gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE;
  color=vec3(1.0,0.0,0.0);
  EmitVertex();
  EndPrimitive();
}

void main()
{
  GenerateLine(0); // first vertex normal
  GenerateLine(1); // second vertex normal
  GenerateLine(2); // third vertex normal
}