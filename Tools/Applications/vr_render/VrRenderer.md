# VrRenderer Documentation

This document describes the `ofnx::VrRenderer` used by the `vr_render` app. It is intended to be complete enough to implement a new compatible renderer without reading the current source.

## Purpose

`VrRenderer` renders a static panoramic `.vr` image into a 2D RGB565 framebuffer. The `.vr` panorama is decoded by `FileVr` as a 256 x 6144 RGB565 image. `VrRenderer` treats that image as 24 stacked square texture tiles, each 256 x 256 pixels, mapped onto a subdivided cube around the camera.

The renderer is a CPU software rasterizer. It does not call SDL, OpenGL, Direct3D, or any platform graphics API. The `vr_render` app supplies SDL only for the window, the streaming texture, input, and final presentation.

## Public API

Header: `ofnx/include/ofnx/vrrenderer.h`

```cpp
namespace ofnx {

class VrRenderer {
public:
    VrRenderer();
    ~VrRenderer();

    uint16_t* cubemapBuffer();

    void init(
        float offsetX,
        float offsetY,
        float width,
        float height,
        int stride);

    void render(
        uint16_t* outputBuffer,
        float yaw,
        float pitch,
        float roll,
        float fov);
};

}
```

### Construction

Constructing `VrRenderer` allocates the internal cubemap texture buffer:

```text
24 tiles * 256 pixels * 256 pixels * sizeof(uint16_t)
```

The buffer holds 1,572,864 RGB565 pixels.

### `cubemapBuffer()`

Returns a writable pointer to the internal cubemap buffer.

Expected layout:

```text
tile 0:  pixels [0, 65535]
tile 1:  pixels [65536, 131071]
...
tile 23: pixels [1507328, 1572863]
```

Each tile is row-major, 256 pixels wide, 256 pixels high, RGB565, one `uint16_t` per pixel.

The `vr_render` app fills this buffer by decoding the `.vr` file with `FileVr::getDataRgb565()` and copying the full 256 x 6144 output directly into `cubemapBuffer()`.

### `init(offsetX, offsetY, width, height, stride)`

Initializes the output viewport and internal rasterization workspace.

Parameters:

- `offsetX`: left edge of the viewport in output-buffer pixel coordinates.
- `offsetY`: top edge of the viewport in output-buffer pixel coordinates.
- `width`: viewport width in pixels.
- `height`: viewport height in pixels.
- `stride`: number of `uint16_t` pixels between consecutive output rows.

The app passes:

```cpp
vr.init(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, texturePitch / sizeof(uint16_t));
```

For the default app window this is:

```text
offsetX = 0
offsetY = 0
width   = 640
height  = 480
stride  = SDL streaming texture pitch / 2
```

Derived screen values:

```text
endX       = offsetX + width
endY       = offsetY + height
centerX    = (offsetX + endX) * 0.5
centerY    = (offsetY + endY) * 0.5
halfWidth  = width * 0.5
halfHeight = height * 0.5
```

The renderer allocates an edge table of `(height + 1) * 8` floats. This table is reused for polygon rasterization.

Important: `render()` assumes `init()` has been called first.

### `render(outputBuffer, yaw, pitch, roll, fov)`

Renders one view into `outputBuffer`.

Parameters:

- `outputBuffer`: writable RGB565 destination buffer. It must contain at least `stride * height` pixels for the initialized viewport.
- `yaw`: horizontal camera angle in radians.
- `pitch`: vertical camera angle in radians.
- `roll`: camera roll angle in radians.
- `fov`: field of view in radians.

The renderer draws only visible cubemap subfaces. It does not clear the output buffer before rendering. The caller should clear or overwrite as needed.

The current app converts degrees to radians like this:

```cpp
vr.render(
    static_cast<uint16_t*>(texturePixels),
    yawDeg * 0.0174532925f - 1.570795f,
    pitchDeg * 0.0174532925f,
    rollDeg * 0.0174532925f,
    fov);
```

The app subtracts approximately 90 degrees from yaw to align the decoded panorama with the expected initial camera direction.

## Input Data

The renderer expects the cubemap data created from a panoramic `.vr` file:

```text
FileVr type:   VR_STATIC_VR
Decoded width: 256
Decoded height: 6144
Pixel format:  RGB565
Tile count:    24
Tile size:     256 x 256
```

The renderer does not validate the data format or size. A compatible caller must ensure that `cubemapBuffer()` contains exactly the decoded panorama in the original stacked-tile order.

## Coordinate Systems

### World Cubemap Coordinates

The cubemap is modeled as a cube centered on the camera. Its outer cube corners use coordinates `-255` and `255`.

The cube is subdivided into 24 quadrilateral subfaces. Extra vertices exist at face midpoints and face centers, producing 26 total vertices.

### View Space and Clipping

After applying camera rotation and projection scale, each cubemap vertex is classified against these clip tests:

```text
near:   point.z >= 1
left:   -point.z <= point.x
right:   point.x <= point.z
top:     point.y <= point.z
bottom: -point.z <= point.y
```

The renderer uses positive `z` as visible forward depth after transformation. The near plane is `z = 1`.

Clip flag values:

```text
CLIP_NEAR   = 0x01
CLIP_LEFT   = 0x02
CLIP_RIGHT  = 0x04
CLIP_TOP    = 0x08
CLIP_BOTTOM = 0x10
CLIP_ALL    = 0x1f
```

Despite the flag names, a set flag means the vertex is inside that clipping boundary.

### Screen Space

Projected screen coordinates are:

```text
invZ = 1 / clipSpace.z
screen.x = clipSpace.x * invZ * halfWidth + centerX
screen.y = centerY - clipSpace.y * invZ * halfHeight
```

`screen.y` is inverted so positive view-space Y maps upward while output-buffer Y increases downward.

## Cubemap Geometry

### Constants

```text
CUBE_VERTEX_COUNT         = 26
SUBFACE_COUNT             = 24
SUBFACE_VERTEX_COUNT      = 4
MAX_CLIPPED_VERTEX_COUNT  = 9
SUBFACE_SIZE              = 256
CUBEMAP_PIXEL_COUNT       = 24 * 256 * 256
NEAR_CLIP_Z               = 1.0
```

### Vertices

The 26 cubemap vertices are:

```text
0:  (-255, -255, -255)
1:  ( 255, -255, -255)
2:  ( 255,  255, -255)
3:  (-255,  255, -255)
4:  (-255, -255,  255)
5:  ( 255, -255,  255)
6:  ( 255,  255,  255)
7:  (-255,  255,  255)
8:  (   0, -255, -255)
9:  ( 255,    0, -255)
10: (   0,  255, -255)
11: (-255,    0, -255)
12: (   0,    0, -255)
13: ( 255, -255,    0)
14: ( 255,    0,  255)
15: ( 255,  255,    0)
16: ( 255,    0,    0)
17: (   0, -255,  255)
18: (   0,  255,  255)
19: (-255,    0,  255)
20: (   0,    0,  255)
21: (-255, -255,    0)
22: (-255,  255,    0)
23: (-255,    0,    0)
24: (   0, -255,    0)
25: (   0,  255,    0)
```

### Subfaces

Each subface is a quad. The subface index is also the texture tile index in `cubemapBuffer()`.

```text
0:  3, 10, 12, 11
1:  10, 2, 9, 12
2:  12, 9, 1, 8
3:  11, 12, 8, 0
4:  4, 19, 23, 21
5:  19, 7, 22, 23
6:  23, 22, 3, 11
7:  21, 23, 11, 0
8:  4, 17, 20, 19
9:  17, 5, 14, 20
10: 20, 14, 6, 18
11: 19, 20, 18, 7
12: 6, 14, 16, 15
13: 14, 5, 13, 16
14: 16, 13, 1, 9
15: 15, 16, 9, 2
16: 7, 18, 25, 22
17: 18, 6, 15, 25
18: 25, 15, 2, 10
19: 22, 25, 10, 3
20: 5, 17, 24, 13
21: 17, 4, 21, 24
22: 24, 21, 0, 8
23: 13, 24, 8, 1
```

### Per-Subface Texture Coordinates

Every subface uses the same four source texture coordinates, assigned in vertex order:

```text
vertex 0: (  128,   128)
vertex 1: (65152,   128)
vertex 2: (65152, 65152)
vertex 3: (  128, 65152)
```

These are 16.8-style fixed-point texel coordinates stored as floats before rasterization. The effective sampled texel index is:

```text
u = (fixedU >> 8) & 0xff
v = (fixedV >> 8) & 0xff
pixel = tile[v * 256 + u]
```

The 128 minimum and 65152 maximum bias samples toward texel centers and avoids touching the exact outside edge of the 256 x 256 tile.

## Camera and Projection

`render()` builds a combined view/projection transform from `yaw`, `pitch`, `roll`, and `fov`.

Trigonometric inputs:

```text
cosYaw   = cos(yaw)
sinYaw   = sin(yaw)
cosPitch = cos(pitch)
sinPitch = sin(pitch)
cosRoll  = cos(roll)
sinRoll  = sin(roll)
```

The initial rotation matrix is:

```text
r0.x = -(cosRoll * sinYaw) - sinRoll * cosPitch * cosYaw
r0.y =  cosPitch * cosYaw * cosRoll - sinRoll * sinYaw
r0.z =  sinPitch * cosYaw

r1.x =  cosPitch * sinYaw * sinRoll - cosYaw * cosRoll
r1.y = -(sinRoll * cosYaw) - cosPitch * sinYaw * cosRoll
r1.z = -sinPitch * sinYaw

r2.x = -sinPitch * sinRoll
r2.y =  sinPitch * cosRoll
r2.z = -cosPitch
```

Projection scale:

```text
tanHalfFov = tan(fov * 0.5)
scaleX = 1 / tanHalfFov
scaleY = screenWidth / (screenHeight * tanHalfFov)
```

The current implementation folds this projection scale into the rotation matrix before transforming vertices. A new implementation may use an equivalent matrix multiplication or apply the scales after rotation, provided the final clip-space coordinates match.

## Render Pipeline

Each call to `render()` performs these stages:

1. Build the combined view/projection transform.
2. Transform all 26 cubemap vertices.
3. Classify every transformed vertex against the five clip planes.
4. Iterate all 24 subfaces.
5. Reject subfaces that are fully outside the clip volume.
6. Build a working polygon for each visible subface.
7. Clip the polygon against the near plane if needed.
8. Project remaining vertices to screen space.
9. Clip the polygon against right, left, bottom, and top viewport edges.
10. Rasterize every surviving polygon into `outputBuffer`.

Pseudocode:

```text
render(output, yaw, pitch, roll, fov):
    matrix = build_view_projection(yaw, pitch, roll, fov)

    for each cube vertex:
        viewPoint = transform(matrix, vertex)
        flags = classify(viewPoint)
        store viewPoint and flags

    visibleQuads.clear()

    for subfaceIndex in 0..23:
        indices = SUBFACE_INDICES[subfaceIndex]
        allFlags = CLIP_ALL
        anyFlags = 0

        for each index in indices:
            allFlags &= vertexFlags[index]
            anyFlags |= vertexFlags[index]

        if anyFlags != CLIP_ALL:
            continue

        quad = four subface vertices with texcoords

        if near flag is not set for every vertex:
            clip quad against z = 1
            if quad has fewer than 3 vertices:
                continue

        project every quad vertex to screen

        for edge in right, left, bottom, top:
            if allFlags does not say the original quad was fully inside that edge:
                clip quad against that screen edge
                if quad has fewer than 3 vertices:
                    discard

        quad.textureData = cubemapBuffer + subfaceIndex * 65536
        visibleQuads.push(quad)

    for each visible quad:
        rasterize quad
```

Note about `anyFlags != CLIP_ALL`: because flags represent inside tests, this rejects a quad only when no vertex is inside at least one of the five clip boundaries.

## Clipping

### Near-Plane Clipping

Near clipping uses Sutherland-Hodgman polygon clipping against `z = 1`.

For an edge crossing the near plane:

```text
t = (1 - outside.z) / (inside.z - outside.z)

intersection.x = outside.x + (inside.x - outside.x) * t
intersection.y = outside.y + (inside.y - outside.y) * t
intersection.z = 1
intersection.u = outside.u + (inside.u - outside.u) * t
intersection.v = outside.v + (inside.v - outside.v) * t
```

The generated vertex is reclassified, with the near flag forced to inside.

Maximum polygon vertex count is 9.

### Screen Clipping

After projection, polygons are clipped against viewport edges in this order:

```text
right
left
bottom
top
```

Bounds:

```text
left   = offsetX
right  = offsetX + width
top    = offsetY
bottom = offsetY + height
```

For a screen-space crossing:

```text
t = (clipBound - outsidePosition) / (insidePosition - outsidePosition)

intersection.screen.x = outside.x + (inside.x - outside.x) * t
intersection.screen.y = outside.y + (inside.y - outside.y) * t
intersection.invZ     = outside.invZ + (inside.invZ - outside.invZ) * t
intersection.u        = outside.u + (inside.u - outside.u) * t
intersection.v        = outside.v + (inside.v - outside.v) * t
```

The coordinate on the clipped axis is forced exactly to the clip bound.

## Rasterization

The rasterizer fills a convex polygon using scanline edge walking.

### Edge Table

Each scanline stores eight float values:

```text
0: left x
1: right x
2: left texture u
3: right texture u
4: left texture v
5: right texture v
6: left invZ
7: right invZ
```

The polygon's minimum and maximum Y values are computed by truncating each vertex `screen.y` to `int`.

If `maxY - minY <= 0`, the polygon is skipped.

Two edge chains are walked from the topmost vertex to the bottommost vertex:

```text
forward index direction  = right edge
backward index direction = left edge
```

For each edge with positive `dy`, x is stepped as 16.16 fixed-point, while texture coordinates and inverse depth are stepped as floats.

### Span Filling

For each scanline:

```text
xLeft  = int(edge.leftX)
xRight = int(edge.rightX)
width  = xRight - xLeft
```

If `xRight <= xLeft`, nothing is drawn for that scanline.

The renderer linearly interpolates `u / z`, `v / z`, and `1 / z` across the span:

```text
uStep    = (uRight - uLeft) / width
vStep    = (vRight - vLeft) / width
invZStep = (invZRight - invZLeft) / width
```

Perspective-correct texture coordinates are recovered as:

```text
fixedU = int((uOverZ / invZ) + 0.5)
fixedV = int((vOverZ / invZ) + 0.5)
```

### 32-Pixel Perspective Chunks

For speed, the current renderer does not divide per pixel across the entire span. Instead, it computes true perspective-correct coordinates every 32 pixels, then linearly steps fixed-point texture coordinates within that 32-pixel chunk.

Constants:

```text
SPAN_PERSPECTIVE_CHUNK = 32
TEXEL_ROUND_BIAS       = 0.5
FIXED_16_SCALE         = 65536
```

For each 32-pixel chunk:

```text
nextUOverZ = uOverZ + uStep * 32
nextVOverZ = vOverZ + vStep * 32
nextInvZ   = invZ   + invZStep * 32

u0 = int(uOverZ / invZ + 0.5)
v0 = int(vOverZ / invZ + 0.5)
u1 = int(nextUOverZ / nextInvZ + 0.5)
v1 = int(nextVOverZ / nextInvZ + 0.5)

uFixedStep = (u1 - u0) >> 5
vFixedStep = (v1 - v0) >> 5
```

Then each pixel samples:

```text
texelU = (uFixed >> 8) & 0xff
texelV = (vFixed >> 8) & 0xff
color = tile[texelV * 256 + texelU]
```

The current implementation writes two RGB565 pixels at a time through a `uint32_t*` when possible. A replacement renderer does not need to preserve this optimization, only the visual result.

## `vr_render` App Integration

Source: `apps/vr_render/main.cpp`

The app is a simple viewer for panoramic `.vr` files.

### Startup Flow

1. Read the `.vr` file path from the first command-line argument.
2. Derive a `.tst` path by removing the `.vr` extension and adding `.tst`.
3. Load the `.vr` file with `ofnx::FileVr`.
4. Require `FileVr::Type::VR_STATIC_VR`.
5. Decode the panorama to RGB565 with `getDataRgb565()`.
6. Try to load the optional `.tst` file with `ofnx::FileTst`.
7. Initialize SDL video.
8. Create a 640 x 480 SDL window and renderer.
9. Create a 640 x 480 streaming RGB565 texture.
10. Lock the texture once to get its pitch.
11. Initialize `VrRenderer`.
12. Copy the decoded panorama into `VrRenderer::cubemapBuffer()`.
13. Enable relative mouse mode.
14. Enter the render loop.

### Default App Constants

```text
WINDOW_WIDTH       = 640
WINDOW_HEIGHT      = 480
WINDOW_FOV         = 1.0 radians
MOUSE_SENSITIVITY  = 0.1 degrees per mouse unit
initial yawDeg     = 270
initial pitchDeg   = 90
initial rollDeg    = 0
```

### Controls

```text
Mouse move X: adjust yaw
Mouse move Y: adjust pitch
Mouse wheel:  adjust fov
Q:            roll left
E:            roll right
Escape:       quit
Window close: quit
```

Yaw handling:

```text
yawDeg += mouseXRelative * 0.1
if yawDeg < 0: yawDeg = 360
if yawDeg > 360: yawDeg = 0
```

Pitch handling:

```text
pitchDeg -= mouseYRelative * 0.1
if pitchDeg < 0: pitchDeg = 0
if pitchDeg > 360: pitchDeg = 0
pitchDeg = clamp(pitchDeg, 0, 180)
```

FOV handling:

```text
fov -= wheelY * 0.1
fov = clamp(fov, 0.5, 2.0)
```

### Per-Frame Rendering

Each frame:

1. Process SDL events and update yaw, pitch, roll, and fov.
2. Lock the streaming texture.
3. Call `VrRenderer::render()` into the texture pixels.
4. Unlock the texture.
5. Log average CPU render time once per second.
6. Clear the SDL renderer to black.
7. Copy the streaming texture to the SDL renderer.
8. Draw a crosshair at the screen center.
9. Present the SDL renderer.

The crosshair is white normally and red when `FileTst::checkZoneVr(yawDeg, pitchDeg)` returns a zone index other than `-1`.

## TST Hotspot Overlay

The renderer itself is unaware of hotspots. The app optionally loads a sibling `.tst` file and checks whether the current camera yaw and pitch fall inside a hotspot zone.

`checkZoneVr()` expects degrees and converts them to radians internally. TST zone coordinates are stored in radians. Zones may wrap around `2*pi`; if a zone spans more than `pi`, the checker treats it as wrapping.

Crosshair color:

```text
red   = current yaw/pitch is inside any VR zone
white = no matching zone
```

## Output Format

The destination buffer is RGB565:

```text
bits 15..11: red
bits 10..5:  green
bits 4..0:   blue
```

The current renderer copies source texels directly to output. There is no filtering, blending, lighting, gamma correction, or color conversion.

## Behavioral Requirements for a Compatible Rewrite

A compatible renderer should:

- Accept a 24-tile, 256 x 256, RGB565 cubemap in the same tile order.
- Expose a writable cubemap buffer or an equivalent upload API.
- Support an initialized output viewport with arbitrary stride.
- Accept yaw, pitch, roll, and fov in radians.
- Render into a caller-owned RGB565 output buffer.
- Use the 26-vertex, 24-subface cubemap topology documented above.
- Map subface index directly to tile index.
- Clip against the near plane and viewport bounds.
- Preserve the orientation implied by the documented vertex order, texture coordinates, and app yaw offset.
- Produce nearest-neighbor RGB565 output.
- Leave buffer clearing and presentation to the caller.

Performance-compatible rewrites should additionally:

- Reuse allocations across frames.
- Avoid copying the rendered frame after rasterization.
- Rasterize only visible subfaces.
- Use perspective-correct texture mapping, either per pixel or with a visually equivalent chunked approximation.

## Known Implementation Details and Caveats

- The current implementation stores texture coordinates as floats, but the values represent fixed-point texel coordinates.
- The current span drawer approximates perspective correction in 32-pixel chunks. This is an optimization, not a file-format requirement.
- The renderer does not guard against null output buffers.
- The renderer does not clear pixels that are not covered by any subface.
- `fov` must be greater than zero and should remain in a practical range. The app clamps it to `0.5..2.0`.
- The screen clip flags are named after boundaries but represent "inside" tests.
- The SDL app uses a `WIN32` executable target, so on Windows it is built as a GUI subsystem executable.

## Minimal Usage Example

```cpp
ofnx::FileVr vrFile;
if (!vrFile.load(vrPath)) {
    return false;
}
if (vrFile.getType() != ofnx::FileVr::Type::VR_STATIC_VR) {
    return false;
}

std::vector<uint16_t> panorama;
if (!vrFile.getDataRgb565(panorama)) {
    return false;
}

ofnx::VrRenderer renderer;
renderer.init(0, 0, width, height, stridePixels);
std::memcpy(
    renderer.cubemapBuffer(),
    panorama.data(),
    panorama.size() * sizeof(uint16_t));

renderer.render(outputRgb565, yawRadians, pitchRadians, rollRadians, fovRadians);
```
