#ifndef PI_GRAPHICS_TRIANGLE_C
#define PI_GRAPHICS_TRIANGLE_C

#include "line.c"
#include "utils.c"
#include "../vectors.c"
#include "../obj.h"
#include "../texture.c"

// TODO: Check buffer boundaries
void DrawTriangleScan(vec2i v0, vec2i v1, vec2i v2, graphics_buffer *buffer, int color)
{
    if (v1.y < v0.y) { SWAP(v0, v1, temp, vec2i) }
    if (v2.y < v0.y) { SWAP(v0, v2, temp, vec2i) }
    if (v2.y < v1.y) { SWAP(v1, v2, temp, vec2i) }

    int *pixel = (int *)buffer->data;

    // We draw the top half of the borders
    double t;
    int v0v1 = (v1.y - v0.y);
    double v0v1Diff = (double)(v0v1 + 1);
    int v0v2 = (v2.y - v0.y);
    double v0v2Diff = (double)(v0v2 + 1);

    // First half of the triangle
    for (int y = v0.y; y <= v1.y; ++y)
    {
        if ((y < 0) || (y >= buffer->height)) { continue; }

        int x0 = v0.x;
        if (v0v1 > 0)
        {
            t = (double)(y - v0.y)/v0v1Diff;                // v0.y == v1.y ???
            x0 = v0.x + (int)(t * (v1.x - v0.x));
            if ((x0 >= 0) && (x0 < buffer->width))
            {
                pixel[buffer->width * y + x0] = color;
            }
        }

        int x1 = v1.x;
        if (v0v2 > 0)
        {
            t = (double)(y - v0.y)/v0v2Diff;
            x1 = v0.x + (int)(t * (v2.x - v0.x));
            if ((x1 >= 0) && (x1 < buffer->width))
            {
                pixel[buffer->width * y + x1] = color;
            }
        }

        // We fill the half
        if (x0 < 0) { x0 = 0; }
        if (x0 >= buffer->width) { x0 = buffer->width; }

        if (x1 < 0) { x1 = 0; }
        if (x1 >= buffer->width) { x1 = buffer->width; }

        if (x1 < x0) { SWAP(x0, x1, t, int) }
        for (int x = x0 + 1; x < x1; ++x)
        {
            pixel[buffer->width * y + x] = color;
        }
    }

    // Second half of the triangle
    double v1v2Diff = (double)(v2.y - v1.y);
    for (int y = v1.y + 1; y <= v2.y; ++y)
    {
        t = (double)(y - v1.y)/v1v2Diff;
        int x1 = v1.x + (int)(t * (v2.x - v1.x));
        pixel[buffer->width * y + x1] = color;

        t = (double)(y - v0.y)/v0v2Diff;
        int x0 = v0.x + (int)(t * (v2.x - v0.x));
        pixel[buffer->width * y + x0] = color;

        // We fill the half
        if (x1 < x0) { SWAP(x0, x1, t, int) }
        for (int x = x0 + 1; x < x1; ++x)
        {
            pixel[buffer->width * y + x] = color;
        }
    }
}

void DrawTriangle(vec2i vertices[3], graphics_buffer *buffer, int color)
{
    int *pixel = (int *)buffer->data;
    box2i boundingBox = GetTriangleBoundingBox2i(vertices, buffer);
    for (int y = boundingBox.min.y; y <= boundingBox.max.y; ++y)
    {
        for (int x = boundingBox.min.x; x <= boundingBox.max.x; ++x)
        {
            vec3i xVec = { vertices[1].x - vertices[0].x,
                           vertices[2].x - vertices[0].x,
                                       x - vertices[0].x };

            vec3i yVec = { vertices[1].y - vertices[0].y,
                           vertices[2].y - vertices[0].y,
                                       y - vertices[0].y };

            vec3i cross = CrossProducti(xVec, yVec);

            vec3d baricentric = { -1.0,
                                  (double)cross.x / -(double)cross.z,
                                  (double)cross.y / -(double)cross.z };
            baricentric.x = 1.0 - (baricentric.y + baricentric.z);
            if ((baricentric.x < 0) || (baricentric.x > 1) ||
                (baricentric.y < 0) || (baricentric.y > 1) ||
                (baricentric.z < 0) || (baricentric.z > 1))
            {
                continue;
            }


            pixel[buffer->width * y + x] = color;
        }
    }
}

void DrawVertices(vertex3d *v0, vertex3d *v1, graphics_buffer *buffer)
{
    int x0 = (int)((v0->position.x + 1.0) * (double)buffer->width / (double)2);
    int y0 = (int)((v0->position.y + 1.0) * (double)buffer->height / (double)2);

    int x1 = (int)((v1->position.x + 1.0) * (double)buffer->width / (double)2);
    int y1 = (int)((v1->position.y + 1.0) * (double)buffer->height / (double)2);

    DrawLine(x0, y0, x1, y1, buffer, 0xFFFFFFFF);
}

vec2i GetVec2iFromVertex3d(vertex3d *v, graphics_buffer *buffer)
{
    vec2i result = { (int)((v->position.x + 1.0) * (double)buffer->width / (double)2),
                     (int)((v->position.y + 1.0) * (double)buffer->height / (double)2) };
    return result;
}


void DrawTriangleFromFace(face f, graphics_buffer *buffer, int color)
{
    vec2i vertices[3] = { GetVec2iFromVertex3d(f.v1, buffer),
                          GetVec2iFromVertex3d(f.v2, buffer),
                          GetVec2iFromVertex3d(f.v3, buffer) };

    float *zBuffer = (float *)buffer->zBuffer;
    int *pixel = (int *)buffer->data;
    box2i boundingBox = GetTriangleBoundingBox2i(vertices, buffer);
    for (int y = boundingBox.min.y; y <= boundingBox.max.y; ++y)
    {
        for (int x = boundingBox.min.x; x <= boundingBox.max.x; ++x)
        {
            vec3i xVec = { vertices[1].x - vertices[0].x,
                           vertices[2].x - vertices[0].x,
                                       x - vertices[0].x };

            vec3i yVec = { vertices[1].y - vertices[0].y,
                           vertices[2].y - vertices[0].y,
                                       y - vertices[0].y };

            vec3i cross = CrossProducti(xVec, yVec);

            vec3d baricentric = { -1.0,
                                  (double)cross.x / -(double)cross.z,
                                  (double)cross.y / -(double)cross.z };
            baricentric.x = 1.0 - (baricentric.y + baricentric.z);
            if ((baricentric.x < 0) || (baricentric.x > 1) ||
                (baricentric.y < 0) || (baricentric.y > 1) ||
                (baricentric.z < 0) || (baricentric.z > 1))
            {
                continue;
            }

            // We check the z buffer
            float z = (float)((baricentric.x * f.v1->position.z) +
                              (baricentric.y * f.v2->position.z) +
                              (baricentric.z * f.v3->position.z));

            if (zBuffer[buffer->width * y + x] >= z) 
            { 
                continue; 
            }
            zBuffer[buffer->width * y + x] = z;
            pixel[buffer->width * y + x] = color;
        }
    }
}

void DrawTriangleFromFaceWithTexture(face f, graphics_buffer *buffer, texture *tex, float intensity)
{
    vec2i vertices[3] = { GetVec2iFromVertex3d(f.v1, buffer),
                          GetVec2iFromVertex3d(f.v2, buffer),
                          GetVec2iFromVertex3d(f.v3, buffer) };

    int *pixel = (int *)buffer->data;
    box2i boundingBox = GetTriangleBoundingBox2i(vertices, buffer);
    for (int y = boundingBox.min.y; y <= boundingBox.max.y; ++y)
    {
        for (int x = boundingBox.min.x; x <= boundingBox.max.x; ++x)
        {
            vec3d baricentric = ObtainBaricentricCoordenate(x, y, vertices);
            if ((baricentric.x < 0) || (baricentric.x > 1) ||
                (baricentric.y < 0) || (baricentric.y > 1) ||
                (baricentric.z < 0) || (baricentric.z > 1))
            {
                continue;
            }

            // We check the z buffer
            float z = InterpolateBaricentric(baricentric, f.v1->position.z,
                                                          f.v2->position.z,
                                                          f.v3->position.z);
            if (!UpdateZBuffer(x, y, z, buffer)) { continue; }

            // We obtain the color
            float texCoordX = InterpolateBaricentric(baricentric, f.v1->textureCoord.x,
                                                                  f.v2->textureCoord.x,
                                                                  f.v3->textureCoord.x);
            texCoordX = Clampd(texCoordX, 0.0, 1.0);
            float texCoordY = InterpolateBaricentric(baricentric, f.v1->textureCoord.y,
                                                                  f.v2->textureCoord.y,
                                                                  f.v3->textureCoord.y);
            texCoordY = 1.0 - texCoordY;
            texCoordY = Clampd(texCoordY, 0.0, 1.0);

            int texX = (int)(texCoordX * (tex->width - 1));
            int texY = (int)(texCoordY * (tex->height - 1));
            int *texPixel = (int *)tex->data;
            // Format is ABGR
            int color = ObtainARGBFromABGR(texPixel[tex->width * texY + texX]);
            color = 0xFF000000 |
                    ((int)(((color >> 16) & 0x000000FF) * intensity) << 16) |
                    ((int)(((color >>  8) & 0x000000FF) * intensity) <<  8) |
                    ((int)(((color >>  0) & 0x000000FF) * intensity) <<  0);

            pixel[buffer->width * y + x] = color;
        }
    }
}


#endif
