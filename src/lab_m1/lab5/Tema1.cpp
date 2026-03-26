#include "lab_m1/lab5/Tema1.h"

#include <iostream>
#include <cstring>
#include <cmath>

using namespace m1;
using namespace std;


Tema1::Tema1()
{
}

Tema1::~Tema1()
{
}

void Tema1::Init()
{
    glm::ivec2 resolution = window->GetResolution();

    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    baseCameraPos = glm::vec3(0, 0, 50);
    shakeTimer = 0.0f;
    shakeDuration = 0.25f;
    shakeAmplitude = 12.0f;

    UpdateLayout(resolution);

    for (int y = 0; y < GRID_H; y++)
    {
        for (int x = 0; x < GRID_W; x++)
        {
            grid[y][x] = BlockType::Empty;
        }
    }

    blockCount = 0;
    isDragging = false;
    draggingType = BlockType::Empty;
    state = GameState::Editor;

    mouseX = mouseY = 0;

    leftPaddlePos = glm::vec2(80.0f, resolution.y * 0.5f);
    rightPaddlePos = glm::vec2(resolution.x - 80.0f, resolution.y * 0.5f);
    paddleSize = glm::vec2(20.0f, 140.0f);

    leftAnimTime = 0.0f;
    rightAnimTime = 0.0f;

    ballRadius = 10.0f;
    balls.clear();

    leftScore = 0;
    rightScore = 0;
    serveDir = 1;

    shipBlocks.clear();

    InitMeshes();
}

void Tema1::UpdateLayout(const glm::ivec2& resolution)
{
    float gridMaxH = resolution.y * 0.65f;
    float gridMaxW = resolution.x * 0.60f;

    float sizeY = gridMaxH / GRID_H;
    float sizeX = gridMaxW / GRID_W;

    cellSize = std::min(sizeX, sizeY);

    float gridW = GRID_W * cellSize;
    float gridH = GRID_H * cellSize;

    gridOrigin = glm::vec2(resolution.x * 0.5f - gridW * 0.5f, resolution.y * 0.5f - gridH * 0.5f);

    float previewGridSize = 3.0f * cellSize;
    float slotExtra = 0.4f * cellSize;
    float slotSize = previewGridSize + slotExtra;

    paletteCellH = slotSize + 20.0f;

    float totalPaletteH = 4 * paletteCellH;

    paletteOrigin = glm::vec2(gridOrigin.x - slotSize - 60.0f, gridOrigin.y + gridH * 0.5f - totalPaletteH * 0.5f);

    float barWidth = MAX_BLOCKS * cellSize + (MAX_BLOCKS - 1) * 6.0f;
    blocksBarOrigin = glm::vec2(gridOrigin.x + (gridW - barWidth) * 0.5f, gridOrigin.y + gridH + 40.0f);

    startButtonSize = glm::vec2(110.0f, 55.0f);
    startButtonOrigin = glm::vec2(resolution.x - startButtonSize.x - 40.0f, resolution.y - startButtonSize.y - 40.0f);
}

void Tema1::InitMeshes()
{
    glm::vec3 corner(0, 0, 0);

    Mesh* solid = object2D::CreateSquare("block_solid", corner, 1.0f, glm::vec3(0.5f, 0.7f, 1.0f), true);
    AddMeshToList(solid);

    Mesh* gun = object2D::CreateSquare("block_gun", corner, 1.0f, glm::vec3(0.7f, 0.7f, 0.7f), true);
    AddMeshToList(gun);

    Mesh* engine = object2D::CreateSquare("block_engine", corner, 1.0f, glm::vec3(1.0f, 0.6f, 0.0f), true);
    AddMeshToList(engine);

    Mesh* bumper = object2D::CreateSquare("block_bumper", corner, 1.0f, glm::vec3(1.0f, 1.0f, 0.7f), true);
    AddMeshToList(bumper);

    Mesh* cell = object2D::CreateSquare("grid_cell", corner, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f), false);
    AddMeshToList(cell);

    Mesh* ball = object2D::CreateCircle("ball", glm::vec3(0, 0, 0), 0.5f, glm::vec3(3.0f, 2.3f, 0.5f), true);
    AddMeshToList(ball);

    Mesh* digitSeg = object2D::CreateSquare("digit_segment", corner, 1.0f, glm::vec3(3.0f, 0.7f, 0.8f), true);
    AddMeshToList(digitSeg);

    Mesh* white = object2D::CreateSquare("white", corner, 1.0f, glm::vec3(1, 1, 1), true);
    AddMeshToList(white);

    Mesh* purple = object2D::CreateSquare("purple", corner, 1.0f, glm::vec3(0.6f, 0.0f, 0.8f), true);
    AddMeshToList(purple);

    Mesh* blue = object2D::CreateSquare("blue", corner, 1.0f, glm::vec3(0.0f, 0.0f, 0.8f), true);
    AddMeshToList(blue);

    Mesh* beige = object2D::CreateSquare("beige", corner, 1.0f, glm::vec3(0.95f, 0.93f, 0.75f), true);
    AddMeshToList(beige);

    Mesh* greyLight = object2D::CreateSquare("grey_light", corner, 1.0f, glm::vec3(0.8f, 0.8f, 0.8f), true);
    AddMeshToList(greyLight);

    Mesh* greyDark = object2D::CreateSquare("grey_dark", corner, 1.0f, glm::vec3(0.25f, 0.25f, 0.25f), true);
    AddMeshToList(greyDark);

    Mesh* orange = object2D::CreateSquare("orange", corner, 1.0f, glm::vec3(1.0f, 0.6f, 0.0f), true);
    AddMeshToList(orange);

    Mesh* fire = object2D::CreateSquare("fire", corner, 1.0f, glm::vec3(1.0f, 0.35f, 0.0f), true);
    AddMeshToList(fire);

    Mesh* metal = object2D::CreateSquare("metal", corner, 1.0f, glm::vec3(0.7f, 0.7f, 0.7f), true);
    AddMeshToList(metal);
}

void Tema1::FrameStart()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::Update(float deltaTimeSeconds)
{
    if (state == GameState::Editor) RenderEditor(deltaTimeSeconds);
    else RenderGame(deltaTimeSeconds);
}

void Tema1::FrameEnd()
{
}

void Tema1::RenderEditor(float dt)
{
    RenderPalette();
    RenderGrid();
    RenderBlocksBar();
    RenderStartButton();

    if (isDragging && draggingType != BlockType::Empty)
    {
        glm::vec2 pos((float)mouseX, (float)mouseY);
        glm::vec2 o = pos - glm::vec2(cellSize * 0.5f, cellSize * 0.5f);

        RenderBlock(draggingType, o, true);
    }
}

void Tema1::RenderGrid()
{
    for (int gy = 0; gy < GRID_H; gy++)
    {
        for (int gx = 0; gx < GRID_W; gx++)
        {
            glm::vec2 o = gridOrigin + glm::vec2(gx * cellSize, gy * cellSize);

            glm::mat3 model(1);
            model *= transform2D::Translate(o.x, o.y);
            model *= transform2D::Scale(cellSize, cellSize);
            RenderMesh2D(meshes["grid_cell"], shaders["VertexColor"], model);

            if (grid[gy][gx] != BlockType::Empty) RenderBlock(grid[gy][gx], o, true);
        }
    }
}

void Tema1::RenderPalette()
{
    BlockType types[4] = {
        BlockType::Bumper,
        BlockType::Gun,
        BlockType::Engine,
        BlockType::Solid
    };

    float previewGridSize = 3.0f * cellSize;
    float slotExtra = 0.4f * cellSize;
    float slotSize = previewGridSize + slotExtra;
    float innerMargin = 0.5f * (slotSize - previewGridSize);

    for (int i = 0; i < 4; i++)
    {
        glm::vec2 slotPos = paletteOrigin + glm::vec2(0, (float)(3 - i) * paletteCellH);

        glm::mat3 model(1);
        model *= transform2D::Translate(slotPos.x, slotPos.y);
        model *= transform2D::Scale(slotSize, slotSize);
        RenderMesh2D(meshes["grid_cell"], shaders["VertexColor"], model);

        glm::vec2 miniOrigin = slotPos + glm::vec2(innerMargin, innerMargin);

        glm::vec2 o;

        switch (types[i])
        {
        case BlockType::Bumper:
            o = glm::vec2(miniOrigin.x + 1.0f * cellSize, miniOrigin.y);
            RenderBlock(BlockType::Bumper, o, true);
            break;

        case BlockType::Gun:
            o = glm::vec2(miniOrigin.x + 1.0f * cellSize, miniOrigin.y);
            RenderBlock(BlockType::Gun, o, true);
            break;

        case BlockType::Engine:
            o = glm::vec2(miniOrigin.x + 1.0f * cellSize, miniOrigin.y + 0.9f * cellSize);
            RenderBlock(BlockType::Engine, o, true);
            break;

        case BlockType::Solid:
        default:
            o = glm::vec2(miniOrigin.x + 1.0f * cellSize, miniOrigin.y + 1.0f * cellSize);
            RenderBlock(BlockType::Solid, o, true);
            break;
        }
    }
}

void Tema1::RenderBlocksBar()
{
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        glm::vec2 o = blocksBarOrigin + glm::vec2(i * (cellSize + 6.0f), 0);

        glm::vec3 colorFree(0.0f, 0.8f, 0.0f);
        glm::vec3 colorUsed(0.2f, 0.2f, 0.2f);
        glm::vec3 c;
        if (i < blockCount) c = colorUsed;
        else c = colorFree;

        Mesh* m = object2D::CreateSquare("tmp_bar", glm::vec3(0, 0, 0), 1.0f, c, true);
        glm::mat3 model(1);
        model *= transform2D::Translate(o.x, o.y);
        model *= transform2D::Scale(cellSize, cellSize);
        RenderMesh2D(m, shaders["VertexColor"], model);
        delete m;
    }
}

void Tema1::RenderStartButton()
{
    bool ok = IsShipValid();

    glm::vec3 col;
    if (ok) col = glm::vec3(0.0f, 0.7f, 0.0f);
    else col = glm::vec3(0.8f, 0.0f, 0.0f);

    Mesh* m = object2D::CreateSquare("tmp_start", glm::vec3(0, 0, 0), 1.0f, col, true);

    glm::mat3 model(1);
    model *= transform2D::Translate(startButtonOrigin.x, startButtonOrigin.y);
    model *= transform2D::Scale(startButtonSize.x, startButtonSize.y);
    RenderMesh2D(m, shaders["VertexColor"], model);

    delete m;
}

void Tema1::RenderBlock(BlockType type, glm::vec2 o, bool largeDecor)
{
    float c = cellSize;

    auto DrawRect = [&](float x, float y, float w, float h, const glm::vec3& col)
        {
            Mesh* m = object2D::CreateSquare("tmp_block_piece", glm::vec3(0, 0, 0), 1.0f, col, true);
            glm::mat3 M(1);
            M *= transform2D::Translate(x, y);
            M *= transform2D::Scale(w, h);
            RenderMesh2D(m, shaders["VertexColor"], M);
            delete m;
        };

    const glm::vec3 colDarkMetal(0.15f, 0.15f, 0.15f);
    const glm::vec3 colLightGrey(0.85f, 0.85f, 0.85f);
    const glm::vec3 colBeige(0.95f, 0.93f, 0.75f);
    const glm::vec3 colOrange(1.00f, 0.60f, 0.00f);
    const glm::vec3 colFlame(1.00f, 0.40f, 0.00f);

    switch (type)
    {
    case BlockType::Solid:
    {
        if (!largeDecor)
        {
            float w = c * 0.7f;
            float x = o.x + c * 0.5f - w * 0.5f;
            float y = o.y + c * 0.5f - w * 0.5f;
            DrawRect(x, y, w, w, colLightGrey);
        }
        else
        {
            DrawRect(o.x, o.y, c, c, colLightGrey);
        }
        break;
    }
    case BlockType::Gun:
    {
        float c = cellSize;

        if (!largeDecor)
        {
            float baseW = 0.6f * c;
            float baseH = 0.25f * c;
            float baseX = o.x + c * 0.5f - baseW * 0.5f;
            float baseY = o.y + 0.15f * c;
            DrawRect(baseX, baseY, baseW, baseH, colLightGrey);

            float capW = baseW;
            float capH = 0.35f * c;
            float capX = baseX;
            float capY = baseY + baseH;

            const int segments = 24;
            for (int i = 0; i < segments; i++)
            {
                float t0 = (float)i / segments;
                float t1 = (float)(i + 1) / segments;

                float x0 = capX + capW * t0;
                float x1 = capX + capW * t1;

                float mid = (t0 + t1) * 0.5f;
                float tt = mid * 2.0f - 1.0f;

                float val = 1.0f - tt * tt;
                if (val <= 0.0f) continue;

                float h = capH * sqrt(val);
                if (h <= 0.0001f) continue;

                if (capY + h > o.y + c) h = (o.y + c) - capY;

                DrawRect(x0, capY, x1 - x0, h, colLightGrey);
            }

            float barrelW = 0.2f * c;
            float barrelH = 0.35f * c;
            float barrelX = o.x + c * 0.5f - barrelW * 0.5f;
            float barrelY = capY + 0.05f * c;
            if (barrelY + barrelH > o.y + c)
                barrelH = (o.y + c) - barrelY;

            DrawRect(barrelX, barrelY, barrelW, barrelH, colDarkMetal);
        }
        else
        {
            auto DrawRectGun = [&](float x, float y, float w, float h, const glm::vec3& col)
                {
                    Mesh* m = object2D::CreateSquare("tmp_gun_piece", glm::vec3(0, 0, 0), 1.0f, col, true);
                    glm::mat3 M(1);
                    M *= transform2D::Translate(x, y);
                    M *= transform2D::Scale(w, h);
                    RenderMesh2D(m, shaders["VertexColor"], M);
                    delete m;
                };

            float baseW = 0.7f * c;
            float baseH = 0.35f * c;
            float baseX = o.x + c * 0.5f - baseW * 0.5f;
            float baseY = o.y;
            DrawRectGun(baseX, baseY, baseW, baseH, colLightGrey);

            float capW = baseW;
            float capH = 0.6f * c;
            float capX = baseX;
            float capY = baseY + baseH;

            const int segments = 40;
            for (int i = 0; i < segments; i++)
            {
                float t0 = (float)i / segments;
                float t1 = (float)(i + 1) / segments;

                float x0 = capX + capW * t0;
                float x1 = capX + capW * t1;

                float mid = (t0 + t1) * 0.5f;
                float tt = mid * 2.0f - 1.0f;

                float val = 1.0f - tt * tt;
                if (val <= 0.0f) continue;

                float h = capH * sqrt(val);
                if (h <= 0.0001f) continue;

                DrawRectGun(x0, capY, x1 - x0, h, colLightGrey);
            }
            float barrelW = 0.3f * c;
            float barrelH = 2.0f * c;
            float barrelX = o.x + c * 0.5f - barrelW * 0.5f;
            float barrelY = o.y + c;

            DrawRectGun(barrelX, barrelY, barrelW, barrelH, colDarkMetal);
        }

        break;
    }

    case BlockType::Engine:
    {
        if (!largeDecor)
        {
            float bodyW = c * 0.6f;
            float bodyH = c * 0.55f;
            float bodyX = o.x + c * 0.5f - bodyW * 0.5f;
            float bodyY = o.y + c * 0.45f;
            DrawRect(bodyX, bodyY, bodyW, bodyH, colOrange);

            float flameH = c * 0.30f;
            float flameY = bodyY - flameH;
            float stripW = bodyW / 3.0f;

            DrawRect(bodyX, flameY, stripW * 0.9f, flameH * 0.8f, colFlame);
            DrawRect(bodyX + stripW, flameY, stripW * 0.9f, flameH, colFlame);
            DrawRect(bodyX + 2 * stripW, flameY, stripW * 0.9f, flameH * 0.8f, colFlame);
        }
        else
        {
            DrawRect(o.x, o.y, c, c, colOrange);

            float flameMaxH = 0.9f * c;
            float baseY = o.y;
            float baseX = o.x;
            float baseW = c;

            const int NUM_TIPS = 4;
            float centers[NUM_TIPS] = { 0.12f, 0.35f, 0.63f, 0.88f };
            float amps[NUM_TIPS] = { 0.7f, 1.0f, 0.85f, 0.6f };
            float widths[NUM_TIPS] = { 0.16f, 0.22f, 0.20f, 0.18f };

            const int segments = 60;
            for (int i = 0; i < segments; i++)
            {
                float t0 = (float)i / segments;
                float t1 = (float)(i + 1) / segments;

                float x0 = baseX + baseW * t0;
                float x1 = baseX + baseW * t1;
                float uMid = (t0 + t1) * 0.5f;

                float hNorm = 0.0f;
                for (int k = 0; k < NUM_TIPS; k++)
                {
                    float d = fabs(uMid - centers[k]);
                    float w = widths[k];
                    if (d >= w) continue;

                    float contrib = 1.0f - d / w;
                    float val = amps[k] * contrib;
                    if (val > hNorm) hNorm = val;
                }

                if (hNorm <= 0.0f) continue;

                float h = flameMaxH * hNorm;
                if (h <= 0.0001f) continue;

                DrawRect(x0, baseY - h, x1 - x0, h, colFlame);
            }
        }
        break;
    }


    case BlockType::Bumper:
    {
        float c = cellSize;

        if (!largeDecor)
        {
            float padSide = 0.18f * c;
            float padTop = 0.12f * c;
            float padBottom = 0.18f * c;

            float bodyW = c - 2.0f * padSide;
            float bodyH = 0.40f * c;
            float bodyX = o.x + padSide;
            float bodyY = o.y + padBottom;
            DrawRect(bodyX, bodyY, bodyW, bodyH, colLightGrey);

            float capY = bodyY + bodyH;
            float capHMax = (o.y + c - padTop) - capY;
            if (capHMax < 0.0f) capHMax = 0.0f;

            float capW = bodyW;
            float capX = bodyX;

            const int segments = 32;
            for (int i = 0; i < segments; i++)
            {
                float t0 = (float)i / segments;
                float t1 = (float)(i + 1) / segments;

                float x0 = capX + capW * t0;
                float x1 = capX + capW * t1;
                float mid = (t0 + t1) * 0.5f;
                float tt = mid * 2.0f - 1.0f;

                float val = 1.0f - tt * tt;
                if (val <= 0.0f) continue;

                float h = capHMax * sqrt(val);
                if (h <= 0.0001f) continue;

                DrawRect(x0, capY, x1 - x0, h, colBeige);
            }
        }
        else
        {
            auto DrawRectB = [&](float x, float y, float w, float h, const glm::vec3& col)
                {
                    Mesh* m = object2D::CreateSquare("tmp_block_piece", glm::vec3(0, 0, 0), 1.0f, col, true);
                    glm::mat3 M(1);
                    M *= transform2D::Translate(x, y);
                    M *= transform2D::Scale(w, h);
                    RenderMesh2D(m, shaders["VertexColor"], M);
                    delete m;
                };

            DrawRectB(o.x, o.y, c, c, colLightGrey);

            float capW = 3.0f * c;
            float capH = 1.0f * c;
            float capX = o.x - c;
            float capY = o.y + c;

            const int segments = 40;
            for (int i = 0; i < segments; i++)
            {
                float t0 = (float)i / segments;
                float t1 = (float)(i + 1) / segments;

                float x0 = capX + capW * t0;
                float x1 = capX + capW * t1;
                float mid = (t0 + t1) * 0.5f;
                float tt = mid * 2.0f - 1.0f;

                float yNorm = 0.0f;
                float val = 1.0f - tt * tt;
                if (val > 0.0f) yNorm = sqrt(val);

                float h = capH * yNorm;
                if (h <= 0.0001f) continue;

                DrawRectB(x0, capY, x1 - x0, h, colBeige);
            }
        }
        break;
    }
    default:
        break;
    }
}

void Tema1::RenderGame(float dt)
{
    glm::ivec2 res = window->GetResolution();
    auto camera = GetSceneCamera();

    if (shakeTimer > 0.0f)
    {
        shakeTimer -= dt;
        if (shakeTimer < 0.0f) shakeTimer = 0.0f;

        float t = shakeTimer / shakeDuration;
        float currentAmp = shakeAmplitude * t;

        float rx = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        float ry = (float)rand() / RAND_MAX * 2.0f - 1.0f;

        glm::vec3 offset(rx * currentAmp, ry * currentAmp, 0.0f);
        camera->SetPosition(baseCameraPos + offset);
        camera->Update();
    }
    else
    {
        camera->SetPosition(baseCameraPos);
        camera->Update();
    }

    const float animDuration = 0.3f;
    if (leftAnimTime > 0.0f) leftAnimTime -= dt;
    if (rightAnimTime > 0.0f) rightAnimTime -= dt;
    if (leftAnimTime < 0.0f) leftAnimTime = 0.0f;
    if (rightAnimTime < 0.0f) rightAnimTime = 0.0f;

    const float leftFieldX = 40.0f;
    const float rightFieldX = (float)res.x - 40.0f;
    const float bottomY = 40.0f;
    const float topY = (float)res.y - 40.0f;

    const float MAX_BALL_SPEED = 700.0f;
    const float BUMPER_SPEED_FACTOR = 1.2f;

    for (int i = 0; i < (int)balls.size(); i++)
    {
        Ball& b = balls[i];
        b.pos += b.vel * dt;

        if (b.pos.y - ballRadius < bottomY)
        {
            b.pos.y = bottomY + ballRadius;
            b.vel.y = -b.vel.y;
        }
        if (b.pos.y + ballRadius > topY)
        {
            b.pos.y = topY - ballRadius;
            b.vel.y = -b.vel.y;
        }

        glm::vec2 centers[2] = { leftPaddlePos, rightPaddlePos };
        for (int ci = 0; ci < 2; ci++)
        {
            glm::vec2 center = centers[ci];
            glm::vec2 half = paddleSize * 0.5f;

            float L = center.x - half.x;
            float R = center.x + half.x;
            float B = center.y - half.y;
            float T = center.y + half.y;

            if (b.pos.x + ballRadius < L || b.pos.x - ballRadius > R) continue;
            if (b.pos.y + ballRadius < B || b.pos.y - ballRadius > T) continue;

            float rel = (b.pos.y - center.y) / half.y;
            if (rel < -1.0f) rel = -1.0f;
            if (rel > 1.0f) rel = 1.0f;

            float angle = rel * 1.0f;
            float speed = glm::length(b.vel);
            if (speed < 1.0f) speed = 1.0f;

            float dir;
            if (ci == 0) dir = 1.0f;
            else dir = -1.0f;

            b.vel.x = dir * speed * cos(angle);
            b.vel.y = speed * sin(angle);

            if (ci == 0) leftAnimTime = animDuration;
            else rightAnimTime = animDuration;
        }

        for (size_t sb = 0; sb < shipBlocks.size(); sb++)
        {
            if (shipBlocks[sb].type != BlockType::Bumper) continue;

            for (int ci = 0; ci < 2; ci++)
            {
                glm::vec2 centerPaddle;
                if (ci == 0) centerPaddle = leftPaddlePos;
                else centerPaddle = rightPaddlePos;

                glm::vec2 local = shipBlocks[sb].localOffset;

                if (ci == 1) local.x = -local.x;

                glm::vec2 centerRect = centerPaddle + local;
                glm::vec2 halfRect(cellSize * 0.5f, cellSize * 0.5f);

                float L = centerRect.x - halfRect.x;
                float R = centerRect.x + halfRect.x;
                float B = centerRect.y - halfRect.y;
                float T = centerRect.y + halfRect.y;

                if (b.pos.x + ballRadius < L || b.pos.x - ballRadius > R) continue;
                if (b.pos.y + ballRadius < B || b.pos.y - ballRadius > T) continue;

                float speed = glm::length(b.vel);
                if (speed < 1.0f) speed = 1.0f;

                speed *= BUMPER_SPEED_FACTOR;
                if (speed > MAX_BALL_SPEED) speed = MAX_BALL_SPEED;

                glm::vec2 dir = glm::normalize(b.vel);
                b.vel = dir * speed;
            }
        }

        bool remove = false;
        if (b.pos.x < leftFieldX)
        {
            rightScore++;
            serveDir = 1;
            remove = true;
            TriggerCameraShake();
        }
        else if (b.pos.x > rightFieldX)
        {
            leftScore++;
            serveDir = -1;
            remove = true;
            TriggerCameraShake();
        }

        if (remove)
        {
            balls.erase(balls.begin() + i);
            i--;
        }
    }

    if (balls.empty()) StartNewRound();

    glm::mat3 model(1);
    float border = 4.0f;

    model = glm::mat3(1);
    model *= transform2D::Translate(leftFieldX, bottomY);
    model *= transform2D::Scale(border, topY - bottomY);
    RenderMesh2D(meshes["white"], shaders["VertexColor"], model);

    model = glm::mat3(1);
    model *= transform2D::Translate(rightFieldX - border, bottomY);
    model *= transform2D::Scale(border, topY - bottomY);
    RenderMesh2D(meshes["white"], shaders["VertexColor"], model);

    model = glm::mat3(1);
    model *= transform2D::Translate(leftFieldX, topY - border);
    model *= transform2D::Scale(rightFieldX - leftFieldX, border);
    RenderMesh2D(meshes["white"], shaders["VertexColor"], model);

    model = glm::mat3(1);
    model *= transform2D::Translate(leftFieldX, bottomY);
    model *= transform2D::Scale(rightFieldX - leftFieldX, border);
    RenderMesh2D(meshes["white"], shaders["VertexColor"], model);

    float lineWidth = ballRadius * 0.4f;
    float lineHeight = ballRadius * 1.4f;
    float lineGap = ballRadius * 1.0f;

    float centerX = (leftFieldX + rightFieldX) * 0.5f - lineWidth * 0.5f;

    for (float y = bottomY; y + lineHeight <= topY; y += lineHeight + lineGap)
    {
        glm::mat3 model(1);
        model *= transform2D::Translate(centerX, y);
        model *= transform2D::Scale(lineWidth, lineHeight);
        RenderMesh2D(meshes["white"], shaders["VertexColor"], model);
    }

    float leftScaleY = 1.0f;
    float rightScaleY = 1.0f;
    if (leftAnimTime > 0.0f)
    {
        float t = 1.0f - leftAnimTime / animDuration;
        leftScaleY = 1.0f + 0.25f * sin(t * 3.1415926f);
    }
    if (rightAnimTime > 0.0f)
    {
        float t = 1.0f - rightAnimTime / animDuration;
        rightScaleY = 1.0f + 0.25f * sin(t * 3.1415926f);
    }

    glm::vec2 halfBase = paddleSize * 0.5f;
    float drawHeightL = paddleSize.y * leftScaleY;
    glm::vec2 halfDrawL(halfBase.x, drawHeightL * 0.5f);

    model = glm::mat3(1);
    model *= transform2D::Translate(leftPaddlePos.x - halfDrawL.x, leftPaddlePos.y - halfDrawL.y);
    model *= transform2D::Scale(paddleSize.x, drawHeightL);
    RenderMesh2D(meshes["purple"], shaders["VertexColor"], model);

    float drawHeightR = paddleSize.y * rightScaleY;
    glm::vec2 halfDrawR(halfBase.x, drawHeightR * 0.5f);

    model = glm::mat3(1);
    model *= transform2D::Translate(rightPaddlePos.x - halfDrawR.x, rightPaddlePos.y - halfDrawR.y);
    model *= transform2D::Scale(paddleSize.x, drawHeightR);
    RenderMesh2D(meshes["blue"], shaders["VertexColor"], model);

    for (size_t i = 0; i < balls.size(); i++)
    {
        model = glm::mat3(1);

        model *= transform2D::Translate(balls[i].pos.x, balls[i].pos.y);
        model *= transform2D::Scale(ballRadius, ballRadius);

        RenderMesh2D(meshes["ball"], shaders["VertexColor"], model);
    }
    RenderScores();
}

void Tema1::TriggerCameraShake()
{
    shakeTimer = shakeDuration;
}

void Tema1::RenderDigit(int digit, glm::vec2 bottomLeft, float size)
{
    if (digit < 0 || digit > 9) return;

    static const bool seg[10][7] = {
        { 1, 1, 1, 1, 1, 1, 0 },
        { 0, 1, 1, 0, 0, 0, 0 },
        { 1, 1, 0, 1, 1, 0, 1 },
        { 1, 1, 1, 1, 0, 0, 1 },
        { 0, 1, 1, 0, 0, 1, 1 },
        { 1, 0, 1, 1, 0, 1, 1 },
        { 1, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 0, 0, 0, 0 },
        { 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 0, 1, 1 }
    };

    float w = size * 0.6f;
    float h = size;
    float t = size * 0.12f;
    float h2 = (h - 3 * t) * 0.5f;

    float x = bottomLeft.x;
    float y = bottomLeft.y;

    glm::mat3 m(1);

    if (seg[digit][0])
    {
        m = glm::mat3(1);
        m *= transform2D::Translate(x, y + h - t);
        m *= transform2D::Scale(w, t);
        RenderMesh2D(meshes["digit_segment"], shaders["VertexColor"], m);
    }

    if (seg[digit][1])
    {
        m = glm::mat3(1);
        m *= transform2D::Translate(x + w - t, y + h2 + 2 * t);
        m *= transform2D::Scale(t, h2);
        RenderMesh2D(meshes["digit_segment"], shaders["VertexColor"], m);
    }

    if (seg[digit][2])
    {
        m = glm::mat3(1);
        m *= transform2D::Translate(x + w - t, y + t);
        m *= transform2D::Scale(t, h2);
        RenderMesh2D(meshes["digit_segment"], shaders["VertexColor"], m);
    }

    if (seg[digit][3])
    {
        m = glm::mat3(1);
        m *= transform2D::Translate(x, y);
        m *= transform2D::Scale(w, t);
        RenderMesh2D(meshes["digit_segment"], shaders["VertexColor"], m);
    }

    if (seg[digit][4])
    {
        m = glm::mat3(1);
        m *= transform2D::Translate(x, y + t);
        m *= transform2D::Scale(t, h2);
        RenderMesh2D(meshes["digit_segment"], shaders["VertexColor"], m);
    }

    if (seg[digit][5])
    {
        m = glm::mat3(1);
        m *= transform2D::Translate(x, y + h2 + 2 * t);
        m *= transform2D::Scale(t, h2);
        RenderMesh2D(meshes["digit_segment"], shaders["VertexColor"], m);
    }

    if (seg[digit][6])
    {
        m = glm::mat3(1);
        m *= transform2D::Translate(x, y + h2 + t);
        m *= transform2D::Scale(w, t);
        RenderMesh2D(meshes["digit_segment"], shaders["VertexColor"], m);
    }
}


void Tema1::RenderScores()
{
    glm::ivec2 res = window->GetResolution();

    float leftFieldX = 40.0f;
    float rightFieldX = (float)res.x - 40.0f;
    float bottomY = 40.0f;
    float topY = (float)res.y - 40.0f;

    float digitSize = 40.0f;
    float digitWidth = digitSize * 0.6f;

    float paddingFromBorder = 12.0f;

    float baseY = topY - paddingFromBorder - digitSize;

    int sL = leftScore;
    if (sL < 0)   sL = 0;
    if (sL > 99)  sL = 99;

    int lTens = sL / 10;
    int lOnes = sL % 10;

    float baseXL = leftFieldX + paddingFromBorder;

    if (sL < 10)
    {
        RenderDigit(lOnes, glm::vec2(baseXL, baseY), digitSize);
    }
    else
    {
        RenderDigit(lTens, glm::vec2(baseXL, baseY), digitSize);
        RenderDigit(lOnes, glm::vec2(baseXL + digitWidth + 8.0f, baseY), digitSize);
    }

    int sR = rightScore;
    if (sR < 0)   sR = 0;
    if (sR > 99)  sR = 99;

    int rTens = sR / 10;
    int rOnes = sR % 10;

    float baseXR;

    if (sR < 10)
    {
        baseXR = rightFieldX - paddingFromBorder - digitWidth;
        RenderDigit(rOnes, glm::vec2(baseXR, baseY), digitSize);
    }
    else 
    {
        baseXR = rightFieldX - paddingFromBorder - (2 * digitWidth + 8.0f);
        RenderDigit(rTens, glm::vec2(baseXR, baseY), digitSize);
        RenderDigit(rOnes, glm::vec2(baseXR + digitWidth + 8.0f, baseY), digitSize);
    }
}

void Tema1::OnInputUpdate(float dt, int mods)
{
    if (state == GameState::Play)
    {
        glm::ivec2 res = window->GetResolution();
        float speed = 350.0f;

        if (window->KeyHold(GLFW_KEY_W))
        {
            leftPaddlePos.y += speed * dt;
        }
        if (window->KeyHold(GLFW_KEY_S))
        {
            leftPaddlePos.y -= speed * dt;
        }
        if (window->KeyHold(GLFW_KEY_UP))
        {
            rightPaddlePos.y += speed * dt;
        }
        if (window->KeyHold(GLFW_KEY_DOWN))
        {
            rightPaddlePos.y -= speed * dt;
        }

        float halfH = paddleSize.y * 0.5f;

        if (leftPaddlePos.y < 40.0f + halfH)
        {
            leftPaddlePos.y = 40.0f + halfH;
        }
        if (leftPaddlePos.y > res.y - 40.0f - halfH)
        {
            leftPaddlePos.y = res.y - 40.0f - halfH;
        }

        if (rightPaddlePos.y < 40.0f + halfH)
        {
            rightPaddlePos.y = 40.0f + halfH;
        }
        if (rightPaddlePos.y > res.y - 40.0f - halfH)
        {
            rightPaddlePos.y = res.y - 40.0f - halfH;
        }
    }
}

void Tema1::OnKeyPress(int, int) {}
void Tema1::OnKeyRelease(int, int) {}

void Tema1::OnMouseMove(int mx, int my, int, int)
{
    glm::ivec2 res = window->GetResolution();
    mouseX = mx;
    mouseY = res.y - my;
}

void Tema1::OnMouseBtnPress(int mx, int my, int button, int mods)
{
    glm::ivec2 res = window->GetResolution();
    int wx = mx;
    int wy = res.y - my;

    mouseX = wx;
    mouseY = wy;

    const int BTN_PRIMARY = 1;
    const int BTN_SECONDARY = 2;

    if (state == GameState::Editor)
    {
        BlockType t;

        if (button == BTN_PRIMARY)
        {
            int gx, gy;
            if (ScreenToGridCell(wx, wy, gx, gy))
            {
                if (grid[gy][gx] != BlockType::Empty)
                {
                    grid[gy][gx] = BlockType::Empty;
                    if (blockCount > 0) blockCount--;
                    return;
                }
            }
        }

        if (ScreenToPalette(wx, wy, t))
        {
            isDragging = true;
            draggingType = t;
            return;
        }

        if (button == BTN_PRIMARY && IsInsideRect((float)wx, (float)wy, startButtonOrigin.x, startButtonOrigin.y,
            startButtonSize.x, startButtonSize.y))
        {
            if (IsShipValid())
            {
                PreparePaddlesFromShip();
                StartNewRound();
                state = GameState::Play;
            }
            return;
        }

        if (button == BTN_SECONDARY)
        {
            int gx, gy;
            if (ScreenToGridCell(wx, wy, gx, gy))
            {
                if (grid[gy][gx] != BlockType::Empty)
                {
                    grid[gy][gx] = BlockType::Empty;
                    if (blockCount > 0) blockCount--;
                }
            }
        }
    }
}


void Tema1::OnMouseBtnRelease(int mx, int my, int button, int mods)
{
    glm::ivec2 res = window->GetResolution();
    int wx = mx;
    int wy = res.y - my;

    mouseX = wx;
    mouseY = wy;

    if (state == GameState::Editor && isDragging)
    {
        int gx, gy;
        if (ScreenToGridCell(wx, wy, gx, gy))
        {
            if (grid[gy][gx] == BlockType::Empty && blockCount < MAX_BLOCKS)
            {
                grid[gy][gx] = draggingType;
                blockCount++;
            }
        }
        isDragging = false;
        draggingType = BlockType::Empty;
    }
}

void Tema1::OnMouseScroll(int, int, int, int) {}
void Tema1::OnWindowResize(int width, int height)
{
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0.0f, (float)width, 0.0f, (float)height, 0.01f, 400.0f);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();

    baseCameraPos = glm::vec3(0, 0, 50);

    glm::ivec2 res(width, height);
    UpdateLayout(res);

    if (state == GameState::Play)
    {
        PreparePaddlesFromShip();
        StartNewRound();
    }
}

bool Tema1::IsInsideRect(float x, float y, float rx, float ry, float rw, float rh) const
{
    return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

bool Tema1::ScreenToGridCell(int mx, int my, int& gx, int& gy) const
{
    float lx = (float)mx - gridOrigin.x;
    float ly = (float)my - gridOrigin.y;
    if (lx < 0 || ly < 0) return false;

    gx = (int)(lx / cellSize);
    gy = (int)(ly / cellSize);

    if (gx < 0 || gx >= GRID_W || gy < 0 || gy >= GRID_H) return false;

    return true;
}

bool Tema1::ScreenToPalette(int mx, int my, BlockType& outType) const
{
    BlockType types[4] = {
        BlockType::Bumper,
        BlockType::Gun,
        BlockType::Engine,
        BlockType::Solid
    };

    float previewGridSize = 3.0f * cellSize;
    float slotExtra = 0.4f * cellSize;
    float slotSize = previewGridSize + slotExtra;

    for (int i = 0; i < 4; i++)
    {
        glm::vec2 slotPos = paletteOrigin + glm::vec2(0, (float)(3 - i) * paletteCellH);

        if (IsInsideRect((float)mx, (float)my, slotPos.x, slotPos.y, slotSize, slotSize))
        {
            outType = types[i];
            return true;
        }
    }
    return false;
}



bool Tema1::IsShipValid() const
{
    if (blockCount == 0 || blockCount > MAX_BLOCKS) return false;

    if (!CheckConnectivity()) return false;

    int x, y, z;

    for (y = 0; y < GRID_H; y++)
    {
        for (x = 0; x < GRID_W; x++)
        {
            if (grid[y][x] == BlockType::Engine)
            {
                for (z = 0; z < y; z++)
                {
                    if (grid[z][x] != BlockType::Empty) return false;
                }
            }
        }
    }

    for (y = 0; y < GRID_H; y++)
    {
        for (x = 0; x < GRID_W; x++)
        {
            if (grid[y][x] == BlockType::Gun)
            {
                for (z = y + 1; z < GRID_H; z++)
                {
                    if (grid[z][x] != BlockType::Empty) return false;
                }
            }
        }
    }

    for (y = 0; y < GRID_H; y++)
    {
        for (x = 0; x < GRID_W; x++)
        {
            if (grid[y][x] == BlockType::Bumper)
            {
                for (int cx = x - 1; cx <= x + 1; cx++)
                {
                    if (cx < 0 || cx >= GRID_W) continue;
                    for (z = y + 1; z < GRID_H; z++)
                    {
                        if (grid[z][cx] != BlockType::Empty) return false;
                    }
                }
            }
        }
    }

    for (y = 0; y < GRID_H; y++)
    {
        for (x = 0; x < GRID_W; x++)
        {
            if (grid[y][x] == BlockType::Bumper)
            {
                if (x - 1 >= 0 && grid[y][x - 1] == BlockType::Gun) return false;
                if (x + 1 < GRID_W && grid[y][x + 1] == BlockType::Gun) return false;
            }
        }
    }

    for (y = 0; y < GRID_H; y++)
    {
        for (x = 0; x < GRID_W; x++)
        {
            if (grid[y][x] == BlockType::Bumper)
            {
                const int dx[4] = { 1, -1, 0, 0 };
                const int dy[4] = { 0, 0, 1, -1 };
                for (z = 0; z < 4; z++)
                {
                    int nx = x + dx[z];
                    int ny = y + dy[z];
                    if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) continue;
                    if (grid[ny][nx] == BlockType::Bumper) return false;
                }
            }
        }
    }
    return true;
}

bool Tema1::CheckConnectivity() const
{
    int sx = -1, sy = -1;
    for (int y = 0; y < GRID_H && sx == -1; y++)
    {
        for (int x = 0; x < GRID_W; x++)
        {
            if (grid[y][x] != BlockType::Empty)
            {
                sx = x;
                sy = y;
                break;
            }
        }
    }

    if (sx == -1) return false;

    bool vis[GRID_H][GRID_W];
    memset(vis, 0, sizeof(vis));

    std::queue< std::pair<int, int> > q;
    q.push(std::pair<int, int>(sx, sy));
    vis[sy][sx] = true;
    int cnt = 0;

    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    while (!q.empty())
    {
        std::pair<int, int> p = q.front();
        q.pop();

        int cx = p.first;
        int cy = p.second;
        cnt++;

        for (int k = 0; k < 4; k++)
        {
            int nx = cx + dx[k];
            int ny = cy + dy[k];

            if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) continue;
            if (vis[ny][nx]) continue;
            if (grid[ny][nx] == BlockType::Empty) continue;

            vis[ny][nx] = true;
            q.push(std::pair<int, int>(nx, ny));
        }
    }

    return cnt == blockCount;
}

void Tema1::PreparePaddlesFromShip()
{
    int minx = GRID_W, maxx = -1;
    int miny = GRID_H, maxy = -1;

    for (int y = 0; y < GRID_H; y++)
    {
        for (int x = 0; x < GRID_W; x++)
        {
            if (grid[y][x] != BlockType::Empty)
            {
                if (x < minx) minx = x;
                if (x > maxx) maxx = x;
                if (y < miny) miny = y;
                if (y > maxy) maxy = y;
            }
        }
    }

    if (maxx < minx || maxy < miny) return;

    paddleSize.x = (maxx - minx + 1) * cellSize;
    paddleSize.y = (maxy - miny + 1) * cellSize;

    ballRadius = cellSize * 0.55f;

    float cx = (minx + maxx + 1) * 0.5f;
    float cy = (miny + maxy + 1) * 0.5f;

    shipBlocks.clear();
    for (int y = 0; y < GRID_H; y++)
    {
        for (int x = 0; x < GRID_W; x++)
        {
            if (grid[y][x] != BlockType::Empty)
            {
                ShipBlock sb;
                sb.type = grid[y][x];

                float ox = ((float)x + 0.5f - cx) * cellSize;
                float oy = ((float)y + 0.5f - cy) * cellSize;
                sb.localOffset = glm::vec2(ox, oy);

                shipBlocks.push_back(sb);
            }
        }
    }

    glm::ivec2 res = window->GetResolution();
    leftPaddlePos = glm::vec2(40.0f + paddleSize.x * 0.5f + 10.0f, res.y * 0.5f);
    rightPaddlePos = glm::vec2(res.x - 40.0f - paddleSize.x * 0.5f - 10.0f, res.y * 0.5f);
}


void Tema1::StartNewRound()
{
    balls.clear();

    glm::ivec2 res = window->GetResolution();
    const float BASE_SPEED = 280.0f;

    bool hasGun = false;
    for (size_t i = 0; i < shipBlocks.size(); i++)
    {
        if (shipBlocks[i].type == BlockType::Gun)
        {
            hasGun = true;
            break;
        }
    }

    if (hasGun)
    {
        for (size_t i = 0; i < shipBlocks.size(); i++)
        {
            if (shipBlocks[i].type != BlockType::Gun) continue;

            glm::vec2 offsetL = shipBlocks[i].localOffset;
            Ball bl;
            bl.pos = leftPaddlePos + offsetL;
            bl.vel = glm::vec2(BASE_SPEED, 0.0f);
            balls.push_back(bl);

            glm::vec2 offsetR = shipBlocks[i].localOffset;
            offsetR.x = -offsetR.x;

            Ball br;
            br.pos = rightPaddlePos + offsetR;
            br.vel = glm::vec2(-BASE_SPEED, 0.0f);
            balls.push_back(br);
        }
    }
    else
    {
        Ball b;
        b.pos = glm::vec2(res.x * 0.5f, res.y * 0.5f);

        if (serveDir != 1 && serveDir != -1) serveDir = 1;

        float dir = (float)serveDir;

        b.vel = glm::vec2(BASE_SPEED * dir, BASE_SPEED * 0.6f);
        balls.push_back(b);
    }

    leftAnimTime = 0.0f;
    rightAnimTime = 0.0f;
}
