precision mediump float;

#ifdef VERTEX

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

// NOTE: Add here your custom variables 

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}

#endif

#ifdef FRAGMENT

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
out vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables

const vec2  size    = vec2(360, 480);  // render size
const int   samples = 4;               // pixels per axis; higher = bigger glow, worse performance
const float quality = 0.75;            // lower = smaller glow, better quality

void main()
{
    vec4 sum = vec4(0);
    vec2 sizeFactor = vec2(1)/size*quality;

    // Texel color fetching from texture sampler
    vec4 source = texture2D(texture0, fragTexCoord);

    int range = samples / 2;

    for (int x = -range; x < range; x++)
    {
        for (int y = -range; y < range; y++)
        {
            float u = float(x) + 0.5;
            float v = float(y) + 0.5;
            // float weight = 1.0 / (u * u + v * v);
            sum += texture2D(texture0, fragTexCoord + vec2(u, v) * sizeFactor);
        }
    }

    // Calculate final fragment color
    fragColor = (0.75 * sum / float(samples * samples) + source) * colDiffuse;
}

#endif
