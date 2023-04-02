#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>
#include <ctime>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>

#include "scene.h"
#include "raylib_extensions.h"

// Utility methods
constexpr double D65_X = 0.95047;
constexpr double D65_Y = 1.00000;
constexpr double D65_Z = 1.08883;

void oklchToRgb(float okl, float okc, float okh, uint8_t *red, uint8_t *green, uint8_t *blue) {
    // Convert OkLCH to Oklab
    float oklab_L = okl;
    float oklab_a = okc * cos(okh * PI / 180.0f);
    float oklab_b = okc * sin(okh * PI / 180.0f);

    // Convert OkLab to linear RGB
    float l_ = oklab_L + 0.3963377774f * oklab_a + 0.2158037573f * oklab_b;
    float m_ = oklab_L - 0.1055613458f * oklab_a - 0.0638541728f * oklab_b;
    float s_ = oklab_L - 0.0894841775f * oklab_a - 1.2914855480f * oklab_b;

    float l = l_*l_*l_;
    float m = m_*m_*m_;
    float s = s_*s_*s_;

    float r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
	float g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
	float b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

    // Apply gamma correction to the linear RGB values
    r = r > 0.0031308 ? 1.055 * pow(r, 1.0 / 2.4) - 0.055 : 12.92 * r;
    g = g > 0.0031308 ? 1.055 * pow(g, 1.0 / 2.4) - 0.055 : 12.92 * g;
    b = b > 0.0031308 ? 1.055 * pow(b, 1.0 / 2.4) - 0.055 : 12.92 * b;
    
    // Clamp and convert the final RGB values to 8-bit unsigned integers
    *red   = static_cast<uint8_t>(std::round(std::clamp(r, 0.0f, 1.0f) * 255.0f));
    *green = static_cast<uint8_t>(std::round(std::clamp(g, 0.0f, 1.0f) * 255.0f));
    *blue  = static_cast<uint8_t>(std::round(std::clamp(b, 0.0f, 1.0f) * 255.0f));
}


class ClockScene : public Scene
{
private:
    Shader litShader;
    Material litMaterial;

    Mesh cubeMesh;

    float nSeconds = 0;
    float nMinutes = 0;
    float nHours = 0;
    float yAntiBurnIn = 0;

public:
    ClockScene() {
        std::cout << "[INITIALIZING SCENE]: Clock" << std::endl;

        litShader = LoadShaderSingleFile("./Shaders/lit_instanced_2.glsl"); // Lit shader
        litShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(litShader, "matModel");
        litShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocationAttrib(litShader, "colDiffuse");
        litShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(litShader, "matView");
        litShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(litShader, "matProjection");

        Vector3 shadowColor = Vector3{0.0f, 0.0f, 0.0f};
        SetShaderValue(litShader, GetShaderLocation(litShader, "shadowColor"), &shadowColor, SHADER_UNIFORM_VEC3);
        Vector3 lightPos = Vector3{0.0f, 3.0f, 22.0f};
        SetShaderValue(litShader, GetShaderLocation(litShader, "lightPos"), &lightPos, SHADER_UNIFORM_VEC3);
        float planeZ = -2.0f;
        SetShaderValue(litShader, GetShaderLocation(litShader, "planeZ"), &planeZ, SHADER_UNIFORM_FLOAT);

        litMaterial = LoadMaterialDefault(); // Lit material
        litMaterial.shader = litShader;

        cubeMesh = GenMeshCube(1.5f, 1.5f, 1.5f);
        //cubeMesh = GenMeshPlaneY(1.5f, 1.5f, 1, 1);

    }
    ~ClockScene() {
        UnloadShader(litShader);
    }
    void Update() {
        float gameTime = GetTime();
        std::time_t now = time(nullptr);
        std::tm calender_time = *std::localtime( std::addressof(now) );
        nSeconds = calender_time.tm_sec;
        nMinutes = calender_time.tm_min  + nSeconds / 60.0;
        nHours   = calender_time.tm_hour + nMinutes / 60.0;
        yAntiBurnIn = 0.1 * sin(nHours / 24.0 * PI * 2.0);
    }
    virtual std::pair<int, int> GetTiles() override {
        return std::pair<int, int>(8, 6);
    }
    virtual std::pair<int, int> GetTileResolution() override {
        return std::pair<int, int>(360, 480);
    }
    virtual Color GetClearColor() override {
        return Color{0, 0, 0, 255};
    }
    virtual bool ShowFPS() override {
        return false;
    }
    virtual std::pair<float, float> GetAngleDistance() override {
        return std::pair<float, float>(20.0f, 20.0f);
    }
    void Draw() {
        Matrix transforms[1500];
        Vector4 colors[1500];
        int instanceIdx = 0;

        static constexpr float zStepOffset = -0.2;
        static constexpr float yStepOffset = -0.0;
        static constexpr float alphaMultiplier = 0.75;
        float alpha = 255;

        Color colorSeconds = (Color){0, 172, 206, 255};
        Color colorHoursMinutes = (Color){176, 180, 183, 255};

        for (int k = 0; k < 8; k++) {
            float zOffset = zStepOffset * k;
            float yOffset = yAntiBurnIn + yStepOffset * k;
            float zOffsetPointers = zOffset - 0.1;
            auto drawCube = [&] (Matrix m, Color c) {
                c.a = static_cast<int>(alpha);
                colors[instanceIdx] = ColorNormalize(c);
                transforms[instanceIdx++] = m;
            };

            float secondHandAngle = fmod(nSeconds, 60.0f)/60.0f * -360.0f;
            float minuteHandAngle = fmod(nMinutes, 60.0f)/60.0f * -360.0f;
            float hourHandAngle = fmod(nHours, 12.0f)/12.0f * -360.0f;

            if (k == 0) {
                //Seconds
                rlPushMatrix();
                    rlTranslatef(0.0f, yOffset, zOffsetPointers);
                    oklchToRgb(0.75, 0.15, minuteHandAngle - secondHandAngle, &colorSeconds.r, &colorSeconds.g, &colorSeconds.b);
                    rlRotatef(secondHandAngle, 0, 0, 1);
                    rlScalef(0.05f, 1.1f, 0.05f);
                    rlTranslatef(0, 0.6666f, 0);
                    drawCube(rlGetMatrixTransform(), colorSeconds);
                rlPopMatrix();
                //Minutes
                rlPushMatrix();
                    rlTranslatef(0.0f, yOffset, zOffsetPointers);
                    rlRotatef(minuteHandAngle, 0, 0, 1);
                    rlScalef(0.05f, 1.1f, 0.05f);
                    rlTranslatef(0, 0.6666f, 1.5f);
                    drawCube(rlGetMatrixTransform(), colorHoursMinutes);
                rlPopMatrix();
                //Hours
                rlPushMatrix();
                    rlTranslatef(0.0f, yOffset, zOffsetPointers);
                    rlRotatef(hourHandAngle, 0, 0, 1);
                    rlScalef(0.05f, 0.8f, 0.05f);
                    rlTranslatef(0, 0.6666f, -1.5f);
                    drawCube(rlGetMatrixTransform(), colorHoursMinutes);
                rlPopMatrix();
            }

            for (int i = 0; i < 60; i++) {
                if (k == 0 && i % 5) continue;
                rlPushMatrix();
                    float size = (i % 5) ? 0.2 : 0.75;
                    float markAngle = (i/60.0f) * 360.0f;
                    rlTranslatef(0.0f, yOffset, zOffset);
                    rlScalef(0.1f, 0.1f, 0.1f);
                    rlTranslatef(19.0f * cos(markAngle / 180.0 * PI), 19.0f * sin(markAngle / 180.0 * PI), 0);
                    rlRotatef(45, 0, 0, 1);
                    rlScalef(size, size, size);
                    drawCube(rlGetMatrixTransform(), DARKGRAY);
                rlPopMatrix();
            }
            alpha *= alphaMultiplier;
        }
        DrawMeshInstancedC(cubeMesh, litMaterial, transforms, colors, instanceIdx);
    }
};
