#include "fvrrenderer.h"

#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>

class FvrRenderer::FvrRendererPrivate
{
    friend class FvrRenderer;

private:
    unsigned short *m_cubemapBuffer;

    float m_outputWidth;
    float m_outputHeight;
};

FvrRenderer::FvrRenderer()
{
    d_ptr = new FvrRendererPrivate();
    d_ptr->m_cubemapBuffer = new uint16_t[256 * 6144 * 2];
}

FvrRenderer::~FvrRenderer()
{
    delete[] d_ptr->m_cubemapBuffer;
    delete d_ptr;
}

unsigned short *FvrRenderer::cubemapBuffer()
{
    return d_ptr->m_cubemapBuffer;
}

void FvrRenderer::setResolution(
    const float width,
    const float height)
{
    d_ptr->m_outputWidth = width;
    d_ptr->m_outputHeight = height;
}

void FvrRenderer::render(
    unsigned short *outputBuffer,
    float yaw,
    float pitch,
    float roll,
    float fov)
{
    // Calculate rotation matrix
    const float cosPitch = std::cos(-pitch + 1.57079633f);
    const float sinPitch = std::sin(-pitch + 1.57079633f);
    const float cosYaw = std::cos(-yaw - 1.57079633f);
    const float sinYaw = std::sin(-yaw - 1.57079633f);
    const float cosRoll = std::cos(roll);
    const float sinRoll = std::sin(roll);

    const float aspectRatio = d_ptr->m_outputWidth / d_ptr->m_outputHeight;
    const float tanHalfFov = std::tan(fov * 0.30f);
    const float cosRollTanHalfFov = cosRoll * tanHalfFov;
    const float sinRollTanHalfFov = sinRoll * tanHalfFov;

    const int outputWidth = static_cast<int>(d_ptr->m_outputWidth);
    const int outputHeight = static_cast<int>(d_ptr->m_outputHeight);

    // Precompute look-up tables
    std::vector<float> lutNormXSin(outputWidth);
    std::vector<float> lutNormXCos(outputWidth);
    for (int x = 0; x < outputWidth; ++x)
    {
        float tmp = -(2.0f * x / outputWidth - 1.0f) * aspectRatio;
        lutNormXSin[x] = tmp * sinRollTanHalfFov;
        lutNormXCos[x] = tmp * cosRollTanHalfFov;
    }

    // Render
    for (int y = 0; y < outputHeight; ++y)
    {
        const float tmp = (1.0f - 2.0f * y / outputHeight);
        const float normYSin = tmp * sinRollTanHalfFov;
        const float normYCos = tmp * cosRollTanHalfFov;

        for (int x = 0; x < outputWidth; ++x)
        {
            // Calculate ray direction
            const float rollX = lutNormXCos[x] + normYSin;
            const float rollY = lutNormXSin[x] - normYCos;

            const float pitchZ = sinPitch * rollY - cosPitch;

            const float coordX = -(cosYaw * rollX - sinYaw * pitchZ);
            const float coordY = -(cosPitch * rollY + sinPitch);
            const float coordZ = -(sinYaw * rollX + cosYaw * pitchZ);

            // Get color from cubemap
            const float absX = std::fabs(coordX);
            const float absY = std::fabs(coordY);
            const float absZ = std::fabs(coordZ);

            float u;
            float v;

            int faceOffset = 0;
            int subfaceOffset = 0;
            if (absX >= absY && absX >= absZ)
            {
                float tmpU = coordZ / coordX;
                float tmpV = coordY / coordX;

                if (coordX < 0.0)
                {
                    // Left
                    faceOffset = 1;
                    if (tmpU <= 0.0f)
                    {
                        u = std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 1;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 2;
                            v = std::fabs(tmpV);
                        }
                    }
                    else
                    {
                        u = 0.99999f - std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 0;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 3;
                            v = std::fabs(tmpV);
                        }
                    }
                }
                else
                {
                    // Right
                    faceOffset = 3;
                    if (tmpU <= 0.0f)
                    {
                        u = std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 2;
                            v = std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 1;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                    }
                    else
                    {
                        u = 0.99999f - std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 3;
                            v = std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 0;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                    }
                }
            }
            else if (absY >= absX && absY >= absZ)
            {
                float tmpU = coordX / coordY;
                float tmpV = coordZ / coordY;

                if (coordY < 0.0)
                {
                    // Down
                    faceOffset = 0;
                    if (tmpU <= 0.0f)
                    {
                        u = std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 1;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 2;
                            v = std::fabs(tmpV);
                        }
                    }
                    else
                    {
                        u = 0.99999f - std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 0;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 3;
                            v = std::fabs(tmpV);
                        }
                    }
                }
                else
                {
                    // Up
                    faceOffset = 2;
                    if (tmpU <= 0.0f)
                    {
                        u = 0.99999f - std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 0;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 3;
                            v = std::fabs(tmpV);
                        }
                    }
                    else
                    {
                        u = std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 1;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 2;
                            v = std::fabs(tmpV);
                        }
                    }
                }
            }
            else
            {
                float tmpU = coordX / coordZ;
                float tmpV = coordY / coordZ;

                if (coordZ < 0.0)
                {
                    // Back
                    faceOffset = 5;
                    if (tmpU <= 0.0f)
                    {
                        u = 0.99999f - std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 0;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 3;
                            v = std::fabs(tmpV);
                        }
                    }
                    else
                    {
                        u = std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 1;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 2;
                            v = std::fabs(tmpV);
                        }
                    }
                }
                else
                {
                    // Front
                    faceOffset = 4;
                    if (tmpU <= 0.0f)
                    {
                        u = 0.99999f - std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 3;
                            v = std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 0;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                    }
                    else
                    {
                        u = std::fabs(tmpU);
                        if (tmpV <= 0.0f)
                        {
                            subfaceOffset = 2;
                            v = std::fabs(tmpV);
                        }
                        else
                        {
                            subfaceOffset = 1;
                            v = 0.99999f - std::fabs(tmpV);
                        }
                    }
                }
            }

            int cubemapX = static_cast<int>(u * 256);
            int cubemapY = static_cast<int>(v * 256);

            outputBuffer[y * outputWidth + x] =
                d_ptr->m_cubemapBuffer[faceOffset * 256 * 256 * 4 +
                                       subfaceOffset * 256 * 256 +
                                       cubemapY * 256 + cubemapX];
        }
    }
}
