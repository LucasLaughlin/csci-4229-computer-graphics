#version 330
layout(points) in;

// Three lines will be generated: 6 vertices
layout(line_strip, max_vertices=6) out;

//  Transformation matrices
uniform mat4 ModelViewMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat3 NormalMatrix;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float MAGNITUDE = 10;

void GenerateLine(int index)
{
  gl_Position = gl_in[index].gl_Position;
  EmitVertex();
  gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE;
  EmitVertex();
  EndPrimitive();
}

void main()
{
  GenerateLine(0); // first vertex normal
  GenerateLine(1); // second vertex normal
  GenerateLine(2); // third vertex normal
}