#version 310 es
precision mediump float;

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 worldPosition;

// NOTE: Add here your custom variables 

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    worldPosition = vertexPosition.xyz;
    
    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
