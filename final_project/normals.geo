
varying vec3 View;
varying vec3 Light;
varying vec3 Normal;


void main(void)
{
    int i;

    for (i = 0; i < gl_in.length(); i++)
    {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
    Normal=Normal;
    Light=Light;
    View=View;
}