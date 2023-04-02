precision mediump float;

#ifdef VERTEX

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

in vec4 colDiffuse;
in mat4 matModel;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matView;
uniform mat4 matProjection;

uniform vec3 shadowColor;
uniform vec3 lightPos;
uniform float planeZ;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;

    vec4 modelPos = matModel*vec4(vertexPosition, 1.0);
    vec3 lightDir = normalize(vec3(-3.0, 5.0, 8.0) - modelPos.xyz);
    float diff = max(dot(vertexNormal, lightDir), 0.0) + 0.2;

    fragColor = vec4((diff * colDiffuse).xyz, colDiffuse.w);
    gl_Position = (matProjection*matView*matModel)*vec4(vertexPosition, 1.0);   
}

#endif
#ifdef FRAGMENT

precision mediump float;

// IN OUT
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

void main (void) {
    finalColor = fragColor;
}

#endif
