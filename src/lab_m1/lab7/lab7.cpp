#include "lab_m1/lab7/lab7.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <cstdio>

using namespace std;
using namespace m1;

static glm::vec3 StartPoint(const glm::vec3& start, const glm::vec3& end, bool reversed)
{
    return reversed ? end : start;
}

static glm::vec3 EndPoint(const glm::vec3& start, const glm::vec3& end, bool reversed)
{
    return reversed ? start : end;
}

static float WrapAnglePi(float a)
{
    while (a > (float)M_PI) a -= 2.f * (float)M_PI;
    while (a < -(float)M_PI) a += 2.f * (float)M_PI;
    return a;
}

static glm::vec3 RailDirFromSE(const glm::vec3& start, const glm::vec3& end, bool reversed)
{
    glm::vec3 a = StartPoint(start, end, reversed);
    glm::vec3 b = EndPoint(start, end, reversed);
    glm::vec3 d = b - a;
    d.y = 0.f;

    float len = glm::length(d);
    if (len < 1e-6f) return glm::vec3(1, 0, 0);
    return d / len;
}

static float kModelYawOffset()
{
    return 0.f;
}

static glm::vec3 NormalizeXZ(const glm::vec3& v)
{
    glm::vec3 d(v.x, 0.f, v.z);
    float l = glm::length(d);
    if (l < 1e-6f) return glm::vec3(1, 0, 0);
    return d / l;
}

static float YawWorldFromDir(const glm::vec3& dir)
{
    glm::vec3 d = NormalizeXZ(dir);
    return atan2(d.z, d.x);
}

static bool SameXZ(const glm::vec3& a, const glm::vec3& b, float eps = 1e-3f)
{
    return (fabs(a.x - b.x) < eps) && (fabs(a.z - b.z) < eps);
}

static bool ComputeReversedByConnection(const glm::vec3& nextStart,
    const glm::vec3& nextEnd,
    const glm::vec3& enterPoint)
{
    if (SameXZ(nextStart, enterPoint)) return false;
    if (SameXZ(nextEnd, enterPoint)) return true;

    glm::vec3 es(enterPoint.x, 0, enterPoint.z);
    glm::vec3 s(nextStart.x, 0, nextStart.z);
    glm::vec3 e(nextEnd.x, 0, nextEnd.z);

    float ds = glm::distance(s, es);
    float de = glm::distance(e, es);
    return (de < ds);
}

static glm::vec3 gTrainForwardDir(1, 0, 0);
static glm::vec3 gDrezMoveDir(1, 0, 0);

Lab7::Lab7() {}
Lab7::~Lab7() {}

void Lab7::Init()
{
    srand((unsigned)time(nullptr));

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("plane");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        std::vector<VertexFormat> vertices;
        std::vector<unsigned int> indices;

        const int slices = 40;
        const float r = 0.5f;
        const float x0 = -0.5f, x1 = 0.5f;

        for (int i = 0; i <= slices; i++)
        {
            float t = (float)i / (float)slices;
            float ang = t * 2.f * (float)M_PI;

            float y = r * cosf(ang);
            float z = r * sinf(ang);

            glm::vec3 normal(0.f, cosf(ang), sinf(ang));

            vertices.emplace_back(glm::vec3(x0, y, z), glm::vec3(1, 1, 1), normal, glm::vec2(t, 0));
            vertices.emplace_back(glm::vec3(x1, y, z), glm::vec3(1, 1, 1), normal, glm::vec2(t, 1));
        }

        for (int i = 0; i < slices; i++)
        {
            int a = 2 * i;
            int b = 2 * i + 1;
            int c = 2 * (i + 1);
            int d = 2 * (i + 1) + 1;

            indices.insert(indices.end(), { (unsigned)a, (unsigned)b, (unsigned)c,
                                            (unsigned)b, (unsigned)d, (unsigned)c });
        }

        int baseIndex = (int)vertices.size();
        vertices.emplace_back(glm::vec3(x0, 0, 0), glm::vec3(1, 1, 1), glm::vec3(-1, 0, 0), glm::vec2(0, 0));
        vertices.emplace_back(glm::vec3(x1, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1, 0, 0), glm::vec2(0, 0));
        int centerL = baseIndex;
        int centerR = baseIndex + 1;

        int ringStart = (int)vertices.size();
        for (int i = 0; i <= slices; i++)
        {
            float t = (float)i / (float)slices;
            float ang = t * 2.f * (float)M_PI;

            float y = r * cosf(ang);
            float z = r * sinf(ang);

            vertices.emplace_back(glm::vec3(x0, y, z), glm::vec3(1, 1, 1), glm::vec3(-1, 0, 0), glm::vec2(t, 0));
            vertices.emplace_back(glm::vec3(x1, y, z), glm::vec3(1, 1, 1), glm::vec3(1, 0, 0), glm::vec2(t, 1));
        }

        for (int i = 0; i < slices; i++)
        {
            int l0 = ringStart + 2 * i;
            int l1 = ringStart + 2 * (i + 1);
            indices.insert(indices.end(), { (unsigned)centerL, (unsigned)l0, (unsigned)l1 });

            int r0 = ringStart + 2 * i + 1;
            int r1 = ringStart + 2 * (i + 1) + 1;
            indices.insert(indices.end(), { (unsigned)centerR, (unsigned)r1, (unsigned)r0 });
        }

        Mesh* cyl = new Mesh("cylinder");
        cyl->InitFromData(vertices, indices);
        meshes[cyl->GetMeshID()] = cyl;
    }

    {
        std::vector<VertexFormat> vertices;
        std::vector<unsigned int> indices;

        vertices.emplace_back(glm::vec3(-0.5f, 0.f, -0.5f), glm::vec3(1), glm::vec3(0, -1, 0));
        vertices.emplace_back(glm::vec3(0.5f, 0.f, -0.5f), glm::vec3(1), glm::vec3(0, -1, 0));
        vertices.emplace_back(glm::vec3(0.5f, 0.f, 0.5f), glm::vec3(1), glm::vec3(0, -1, 0));
        vertices.emplace_back(glm::vec3(-0.5f, 0.f, 0.5f), glm::vec3(1), glm::vec3(0, -1, 0));

        vertices.emplace_back(glm::vec3(0.f, 0.8f, 0.f), glm::vec3(1), glm::vec3(0, 1, 0));

        indices.insert(indices.end(), { 0, 1, 2, 0, 2, 3 });

        indices.insert(indices.end(), { 0, 1, 4 });
        indices.insert(indices.end(), { 1, 2, 4 });
        indices.insert(indices.end(), { 2, 3, 4 });
        indices.insert(indices.end(), { 3, 0, 4 });

        Mesh* pyramid = new Mesh("pyramid");
        pyramid->InitFromData(vertices, indices);
        meshes[pyramid->GetMeshID()] = pyramid;
    }

    {
        Shader* shader = new Shader("LabShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "lab7", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "lab7", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        lightPosition = glm::vec3(0, 3, 3);
        materialShininess = 30;
        materialKd = 0.6f;
        materialKs = 0.4f;
    }

    GetSceneCamera()->SetPosition(glm::vec3(0, 4, 6));
    GetSceneCamera()->SetRotation(glm::vec3(-0.4f, 0, 0));
    GetSceneCamera()->Update();

    BuildTrackAndStations();

    train.railIndex = 0;
    train.progress = 0.f;
    train.speed = 0.25f;
    train.reversed = false;

    {
        const Rail& r = rails[train.railIndex];
        glm::vec3 a = StartPoint(r.start, r.end, train.reversed);
        glm::vec3 b = EndPoint(r.start, r.end, train.reversed);
        train.position = glm::mix(a, b, train.progress);
        gTrainForwardDir = RailDirFromSE(r.start, r.end, train.reversed);
    }

    drezina.railIndex = 0;
    drezina.progress = 0.10f;
    drezina.speed = 0.45f;
    drezina.choice = 0;
    drezina.reversed = false;

    {
        const Rail& r = rails[drezina.railIndex];
        glm::vec3 a = StartPoint(r.start, r.end, drezina.reversed);
        glm::vec3 b = EndPoint(r.start, r.end, drezina.reversed);
        drezina.position = glm::mix(a, b, drezina.progress);

        glm::vec3 dir = RailDirFromSE(r.start, r.end, drezina.reversed);
        gDrezMoveDir = dir;

        drezina.yaw = YawWorldFromDir(dir) + kModelYawOffset();
    }

    timeAcc = 0.f;
    damageTick = 0.f;
    gameOver = false;

    camInited = false;
    UpdateCameraTPS(0.f);

    {
        glm::ivec2 res = window->GetResolution();
        text = new gfxc::TextRenderer(window->props.selfDir, (GLuint)res.x, (GLuint)res.y);

        std::string fontPath = PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf");
        text->Load(fontPath, 24);

        textReady = true;
    }

    gameTime = 0.f;
    damagedSegments = 0;
}

void Lab7::BuildTrackAndStations()
{
    rails.clear();
    stations.clear();

    Rail r0{ glm::vec3(0,0,0), glm::vec3(6,0,0), 1, 3, false, -1, -1 };
    Rail r1{ glm::vec3(6,0,0), glm::vec3(6,0,6), 2, 0, true, -1, -1 };
    Rail r2{ glm::vec3(6,0,6), glm::vec3(0,0,6), 3, 1, false, -1, -1 };
    Rail r3{ glm::vec3(0,0,6), glm::vec3(0,0,0), 0, 2, false, -1, -1 };

    rails.push_back(r0);
    rails.push_back(r1);
    rails.push_back(r2);
    rails.push_back(r3);

    stations.push_back({ glm::vec3(-0.8f, 0, -0.8f), glm::vec3(1, 0.7f, 0.2f) });
    stations.push_back({ glm::vec3(6.8f, 0, -0.8f), glm::vec3(0.2f, 1, 0.7f) });
    stations.push_back({ glm::vec3(6.8f, 0,  6.8f), glm::vec3(0.7f, 0.2f, 1) });

    pieceDamaged.assign(rails.size() * kPiecesPerRail, false);
    trainBlocked = false;
    trainBlockedTime = 0.f;

    signalGreen.assign(rails.size(), true);
    signalTick = 0.f;

    trainWaitingSignal = false;
    trainSignalWait = 0.f;
    waitingAtRailIdx = -1;
}

void Lab7::FrameStart()
{
    if (gameOver) glClearColor(0.f, 0.f, 0.f, 1.f);
    else glClearColor(0.6f, 0.8f, 1.f, 1.f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Lab7::Update(float dt)
{
    if (gameOver) {
        DrawGameOverScreen();
        return;
    }

    gameTime += dt;
    timeAcc += dt;

    signalTick += dt;
    if (signalTick >= 4.0f) {
        signalTick = 0.f;

        for (int i = 0; i < (int)rails.size(); i++) {
            if (!rails[i].isJunction) continue;
            signalGreen[i] = (rand() % 2) == 0;
        }
    }

    damageTick += dt;
    if (damageTick >= 15.f) {
        damageTick = 0.f;

        int totalPieces = (int)pieceDamaged.size();
        glm::vec3 trainPos = train.position;
        const float avoidRadius = 2.8f;

        auto pieceCenterByIndex = [&](int p) -> glm::vec3 {
            int ri = p / kPiecesPerRail;
            int pi = p % kPiecesPerRail;
            return PieceCenterWorld(ri, pi);
            };

        int chosen = -1;

        for (int tries = 0; tries < 400; tries++) {
            int p = rand() % totalPieces;

            int pi = p % kPiecesPerRail;
            if (pi == 0) continue;
            if (pieceDamaged[p]) continue;

            glm::vec3 c = pieceCenterByIndex(p);
            if (glm::distance(c, trainPos) < avoidRadius) continue;

            chosen = p;
            break;
        }

        if (chosen == -1) {
            for (int p = 0; p < totalPieces; p++) {
                if (pieceDamaged[p]) continue;

                glm::vec3 c = pieceCenterByIndex(p);
                if (glm::distance(c, trainPos) < avoidRadius) continue;

                chosen = p;
                break;
            }
        }

        if (chosen != -1) {
            pieceDamaged[chosen] = true;
            damagedSegments++;
        }

        int damagedCount = 0;
        for (bool d : pieceDamaged) damagedCount += (d ? 1 : 0);
        if (damagedCount > totalPieces / 2) gameOver = true;
    }

    UpdateTrain(dt);
    UpdateCameraTPS(dt);

    DrawTerrain();
    DrawRails();
    DrawStations();
    DrawTrain();
    DrawDrezina();

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, lightPosition);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.12f));
        RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], modelMatrix, glm::vec3(1, 1, 1));
    }

    DrawHUD();
}

void Lab7::UpdateTrain(float dt)
{
    if (gameOver) return;

    if (trainWaitingSignal) {
        trainSignalWait += dt;

        const Rail& r = rails[train.railIndex];
        glm::vec3 a = StartPoint(r.start, r.end, train.reversed);
        glm::vec3 b = EndPoint(r.start, r.end, train.reversed);
        train.position = glm::mix(a, b, train.progress);
        gTrainForwardDir = NormalizeXZ(b - a);

        if (trainSignalWait >= kSignalBlockSeconds) {
            trainWaitingSignal = false;
            trainSignalWait = 0.f;

            if (waitingAtRailIdx >= 0 && waitingAtRailIdx < (int)signalGreen.size())
                signalGreen[waitingAtRailIdx] = true;

            waitingAtRailIdx = -1;
        }

        return;
    }

    if (trainBlocked && blockedRailIdx >= 0 && blockedPieceIdx >= 0)
    {
        int idx = blockedRailIdx * kPiecesPerRail + blockedPieceIdx;
        if (idx >= 0 && idx < (int)pieceDamaged.size() && pieceDamaged[idx]) {

            trainBlockedTime += dt;
            if (trainBlockedTime >= 30.f) gameOver = true;

            const Rail& r = rails[train.railIndex];
            glm::vec3 a = StartPoint(r.start, r.end, train.reversed);
            glm::vec3 b = EndPoint(r.start, r.end, train.reversed);
            train.position = glm::mix(a, b, train.progress);
            gTrainForwardDir = NormalizeXZ(b - a);
            return;
        }
        else {
            trainBlocked = false;
            blockedRailIdx = -1;
            blockedPieceIdx = -1;
        }
    }

    float move = dt * train.speed;
    const float frontOffsetWorld = 2.2f;
    const float safetyWorld = 0.4f;

    while (move > 1e-6f)
    {
        const Rail& cur = rails[train.railIndex];

        float railLen = glm::distance(cur.start, cur.end);
        if (railLen < 1e-6f) return;

        float frontT = frontOffsetWorld / railLen;
        float safetyT = safetyWorld / railLen;

        float remain = 1.f - train.progress;
        float step = (move < remain) ? move : remain;

        float p0 = train.progress;
        float p1 = train.progress + step;

        float frontP0 = p0 + frontT;
        float frontP1 = p1 + frontT;

        float check0 = glm::clamp(frontP0, 0.0f, 1.0f);
        float check1 = glm::clamp(frontP1, 0.0f, 1.0f);

        if (check1 < check0) std::swap(check0, check1);

        if (check1 > check0 + 1e-6f)
        {
            int piece0 = (int)floorf(glm::min(check0, 0.999999f) * kPiecesPerRail);
            int piece1 = (int)floorf(glm::min(glm::max(check1 - 1e-6f, 0.0f), 0.999999f) * kPiecesPerRail);

            for (int pi = piece0; pi <= piece1; pi++)
            {
                int idx = train.railIndex * kPiecesPerRail + pi;
                if (!pieceDamaged[idx]) continue;

                float tStart = (float)pi / (float)kPiecesPerRail;
                float tEnd = (float)(pi + 1) / (float)kPiecesPerRail;

                bool hit = (check1 >= tStart) && (check0 <= tEnd);

                if (hit)
                {
                    float stopT = tStart - frontT - safetyT;
                    train.progress = glm::clamp(stopT, 0.0f, 1.0f);

                    trainBlocked = true;
                    blockedRailIdx = train.railIndex;
                    blockedPieceIdx = pi;

                    move = 0.f;
                    break;
                }
            }
        }

        if (trainBlocked) break;

        train.progress = glm::min(p1, 1.0f);
        move -= step;

        if (train.progress >= 1.f - 1e-6f)
        {
            int nextIdx = cur.nextDefault;
            if (nextIdx < 0 || nextIdx >= (int)rails.size()) nextIdx = 0;

            if (rails[nextIdx].isJunction && !signalGreen[nextIdx]) {
                train.progress = 1.f - 1e-6f;
                trainWaitingSignal = true;
                trainSignalWait = 0.f;
                waitingAtRailIdx = nextIdx;

                move = 0.f;
                break;
            }

            train.progress = 0.f;

            glm::vec3 exitP = EndPoint(cur.start, cur.end, train.reversed);
            const Rail& nxt = rails[nextIdx];

            train.reversed = ComputeReversedByConnection(nxt.start, nxt.end, exitP);
            train.railIndex = nextIdx;
        }
    }

    if (trainBlocked) {
        trainBlockedTime += dt;
        if (trainBlockedTime >= 30.f) gameOver = true;
    }
    else {
        trainBlockedTime = 0.f;
    }

    const Rail& r = rails[train.railIndex];
    glm::vec3 a = StartPoint(r.start, r.end, train.reversed);
    glm::vec3 b = EndPoint(r.start, r.end, train.reversed);

    train.position = glm::mix(a, b, train.progress);
    gTrainForwardDir = NormalizeXZ(b - a);
}

void Lab7::UpdateCameraTPS(float dt)
{
    glm::vec3 target = drezina.position + glm::vec3(0.f, 0.55f, 0.f);

    glm::vec3 forwardRaw = NormalizeXZ(gDrezMoveDir);

    static bool fInit = false;
    static glm::vec3 forwardSmooth(1, 0, 0);

    if (!fInit) {
        fInit = true;
        forwardSmooth = forwardRaw;
    }
    else {
        float d = forwardSmooth.x * forwardRaw.x + forwardSmooth.z * forwardRaw.z;
        if (d < -0.2f) {
            forwardSmooth = forwardRaw;
        }
        else {
            float k = 1.f - expf(-12.f * dt);
            forwardSmooth = NormalizeXZ(glm::mix(forwardSmooth, forwardRaw, k));
        }
    }

    float distBack = 4.2f;
    float heightUp = 2.2f;

    glm::vec3 camPosDesired = target - forwardSmooth * distBack + glm::vec3(0.f, heightUp, 0.f);

    if (!camInited || dt <= 0.f) {
        camInited = true;
        camPosSmooth = camPosDesired;
        camTargetSmooth = target;
    }
    else {
        float aPos = 1.f - expf(-10.f * dt);
        camPosSmooth = glm::mix(camPosSmooth, camPosDesired, aPos);
        camTargetSmooth = glm::mix(camTargetSmooth, target, aPos);
    }

    glm::vec3 look = camTargetSmooth - camPosSmooth;
    float len = glm::length(look);
    if (len < 1e-6f) look = glm::vec3(0, 0, -1);
    else look /= len;

    glm::vec3 lookXZ = NormalizeXZ(look);

    float yaw = atan2(lookXZ.x, -lookXZ.z);

    bool onX = (fabs(forwardSmooth.x) > fabs(forwardSmooth.z));
    if (onX) {
        yaw += (float)M_PI;
    }
    yaw = WrapAnglePi(yaw);

    float pitch = asinf(glm::clamp(look.y, -0.95f, 0.95f));

    GetSceneCamera()->SetPosition(camPosSmooth);
    GetSceneCamera()->SetRotation(glm::vec3(pitch, yaw, 0.f));
    GetSceneCamera()->Update();
}

void Lab7::DrawTerrain()
{
    {
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(3.f, 0.0f, 3.f));
        model = glm::scale(model, glm::vec3(0.17f, 1.f, 0.17f));
        RenderSimpleMesh(meshes["plane"], shaders["LabShader"], model, glm::vec3(0.75f, 0.95f, 0.65f));
    }

    auto drawRectOnGround = [&](glm::vec3 center, float sizeX, float sizeZ, float y, glm::vec3 color)
        {
            glm::mat4 m = glm::mat4(1);
            m = glm::translate(m, glm::vec3(center.x, y, center.z));
            m = glm::scale(m, glm::vec3(sizeX / 50.f, 1.f, sizeZ / 50.f));
            RenderSimpleMesh(meshes["plane"], shaders["LabShader"], m, color);
        };

    const float riverW = 0.8f;

    drawRectOnGround(glm::vec3(2.0f, 0, 1.5f), riverW, 3.2f, 0.001f, glm::vec3(0.15f, 0.75f, 0.85f));
    drawRectOnGround(glm::vec3(3.6f, 0, 3.0f), 3.4f, riverW, 0.001f, glm::vec3(0.15f, 0.75f, 0.85f));
    drawRectOnGround(glm::vec3(5.3f, 0, 4.1f), riverW, 2.2f, 0.001f, glm::vec3(0.15f, 0.75f, 0.85f));
    drawRectOnGround(glm::vec3(6.2f, 0, 5.2f), 2.0f, riverW, 0.001f, glm::vec3(0.15f, 0.75f, 0.85f));
    drawRectOnGround(glm::vec3(2.4f, 0, 5.4f), 4.6f, 1.8f, 0.002f, glm::vec3(0.80f, 0.72f, 0.62f));
}

void Lab7::DrawRails()
{
    const float yOffset = 0.05f;
    const float thicknessBase = 0.12f;
    const float widthZ = 0.30f;

    const glm::vec3 cA = glm::vec3(0.05f);
    const glm::vec3 cB = glm::vec3(1.0f);

    auto applyDamaged = [&](bool damagedPiece, glm::vec3 baseColor, float& outThickness) -> glm::vec3
        {
            outThickness = thicknessBase;
            if (!damagedPiece) return baseColor;

            float flick = 0.5f + 0.5f * sin(12.f * timeAcc);
            outThickness = 0.06f;
            return glm::mix(glm::vec3(1, 0, 0), glm::vec3(1, 1, 0), flick);
        };

    auto drawBoxSegment = [&](bool damagedPiece,
        float yaw,
        const glm::vec3& center,
        float segLen,
        float segWidth,
        float segThick,
        const glm::vec3& color)
        {
            glm::mat4 m = glm::mat4(1);
            m = glm::translate(m, center + glm::vec3(0, yOffset, 0));
            m = glm::rotate(m, yaw, glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(segLen, segThick, segWidth));

            if (damagedPiece) {
                m = glm::rotate(m, RADIANS(10.f), glm::vec3(0, 0, 1));
            }

            RenderSimpleMesh(meshes["box"], shaders["LabShader"], m, color);
        };

    auto drawNormal = [&](bool damagedPiece, const glm::vec3& subStart, const glm::vec3& subEnd)
        {
            glm::vec3 d = subEnd - subStart;
            d.y = 0.f;
            float L = glm::length(d);
            if (L < 1e-6f) return;

            glm::vec3 dir = d / L;
            float yaw = atan2(dir.z, dir.x);
            glm::vec3 mid = (subStart + subEnd) * 0.5f;

            float t;
            glm::vec3 col = applyDamaged(damagedPiece, glm::vec3(0.05f), t);
            drawBoxSegment(damagedPiece, yaw, mid, L, widthZ, t, col);
        };

    auto drawBridge = [&](bool damagedPiece, const glm::vec3& subStart, const glm::vec3& subEnd)
        {
            glm::vec3 d = subEnd - subStart;
            d.y = 0.f;
            float L = glm::length(d);
            if (L < 1e-6f) return;

            glm::vec3 dir = d / L;
            float yaw = atan2(dir.z, dir.x);
            glm::vec3 mid = (subStart + subEnd) * 0.5f;

            float pieceLen = L / 4.f;
            for (int i = 0; i < 4; i++)
            {
                float t;
                glm::vec3 base = (i % 2 == 0) ? cB : cA;
                glm::vec3 col = applyDamaged(damagedPiece, base, t);

                float localX = (-L * 0.5f) + (i + 0.5f) * pieceLen;
                glm::vec3 center = mid + dir * localX;

                drawBoxSegment(damagedPiece, yaw, center, pieceLen, widthZ, t, col);
            }
        };

    auto drawTunnel = [&](bool damagedPiece, const glm::vec3& subStart, const glm::vec3& subEnd)
        {
            glm::vec3 d = subEnd - subStart;
            d.y = 0.f;
            float L = glm::length(d);
            if (L < 1e-6f) return;

            glm::vec3 dir = d / L;
            float yaw = atan2(dir.z, dir.x);
            glm::vec3 mid = (subStart + subEnd) * 0.5f;

            float sliceW = widthZ / 4.f;
            glm::vec3 right = glm::vec3(-dir.z, 0.f, dir.x);

            for (int i = 0; i < 4; i++)
            {
                float t;
                glm::vec3 base = (i % 2 == 0) ? cB : cA;
                glm::vec3 col = applyDamaged(damagedPiece, base, t);

                float localZ = (-widthZ * 0.5f) + (i + 0.5f) * sliceW;
                glm::vec3 center = mid + right * localZ;

                drawBoxSegment(damagedPiece, yaw, center, L, sliceW, t, col);
            }
        };

    auto styleAt = [&](int railIdx, float tMid) -> int {
        if (railIdx == 1) return (tMid < 0.5f) ? 0 : 1;
        if (railIdx == 2) return (tMid < 0.5f) ? 0 : 2;
        return 0;
        };

    for (int ri = 0; ri < (int)rails.size(); ri++)
    {
        const Rail& r = rails[ri];

        for (int pi = 0; pi < kPiecesPerRail; pi++)
        {
            int idx = ri * kPiecesPerRail + pi;
            bool damagedPiece = pieceDamaged[idx];

            float t0 = (float)pi / (float)kPiecesPerRail;
            float t1 = (float)(pi + 1) / (float)kPiecesPerRail;
            float tMid = 0.5f * (t0 + t1);

            glm::vec3 subA = glm::mix(r.start, r.end, t0);
            glm::vec3 subB = glm::mix(r.start, r.end, t1);

            int st = styleAt(ri, tMid);

            if (damagedPiece) {
                glm::vec3 mid = (subA + subB) * 0.5f;
                subA = glm::mix(mid, subA, 0.60f);
                subB = glm::mix(mid, subB, 0.60f);
            }

            if (st == 0) drawNormal(damagedPiece, subA, subB);
            else if (st == 1) drawBridge(damagedPiece, subA, subB);
            else drawTunnel(damagedPiece, subA, subB);
        }
    }

    for (int ri = 0; ri < (int)rails.size(); ri++) {
        if (!rails[ri].isJunction) continue;

        const Rail& r = rails[ri];

        glm::vec3 dir = r.end - r.start;
        dir.y = 0.f;
        float L = glm::length(dir);
        if (L < 1e-6f) continue;
        dir /= L;

        glm::vec3 right = glm::vec3(-dir.z, 0.f, dir.x);

        float tNearStart = 0.10f;
        glm::vec3 pOnRail = glm::mix(r.start, r.end, tNearStart);

        float lateral = 0.70f;
        glm::vec3 signalPos = pOnRail + right * lateral;
        signalPos.y = 0.f;

        {
            glm::mat4 m = glm::mat4(1);
            m = glm::translate(m, signalPos + glm::vec3(0.0f, 0.35f, 0.0f));
            m = glm::scale(m, glm::vec3(0.10f, 0.70f, 0.10f));
            RenderSimpleMesh(meshes["box"], shaders["LabShader"], m, glm::vec3(0.2f));
        }

        {
            glm::vec3 col = signalGreen[ri] ? glm::vec3(0.f, 1.f, 0.f) : glm::vec3(1.f, 0.f, 0.f);

            glm::mat4 m = glm::mat4(1);
            m = glm::translate(m, signalPos + glm::vec3(0.0f, 0.85f, 0.0f));
            m = glm::scale(m, glm::vec3(0.18f));
            RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], m, col);
        }
    }
}

void Lab7::DrawStations()
{
    for (int i = 0; i < (int)stations.size(); i++)
    {
        const auto& s = stations[i];

        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, s.pos + glm::vec3(0, 0.4f, 0));
        model = glm::scale(model, glm::vec3(0.8f));

        if (i == 0)
        {
            model = glm::scale(model, glm::vec3(1.2f, 0.7f, 1.2f));
            RenderSimpleMesh(meshes["box"], shaders["LabShader"], model, s.color);
        }
        else if (i == 1)
        {
            RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], model, s.color);
        }
        else if (i == 2)
        {
            glm::mat4 m = glm::mat4(1);
            m = glm::translate(m, s.pos + glm::vec3(0, 0.05f, 0));
            m = glm::scale(m, glm::vec3(0.8f));
            RenderSimpleMesh(meshes["pyramid"], shaders["LabShader"], m, s.color);
        }
    }
}

void Lab7::DrawTrain()
{
    float yaw = atan2(gTrainForwardDir.z, gTrainForwardDir.x) + kModelYawOffset();

    bool onZ = (fabs(gTrainForwardDir.z) > fabs(gTrainForwardDir.x));
    if (onZ) yaw += (float)M_PI;

    glm::mat4 base = glm::translate(glm::mat4(1), train.position + glm::vec3(0, 0.25f, 0));
    base = glm::rotate(base, yaw, glm::vec3(0, 1, 0));
    base = glm::scale(base, glm::vec3(1.0f));

    {
        glm::mat4 cab = base;
        cab = glm::translate(cab, glm::vec3(0.10f, 0.75f, 0.0f));
        cab = glm::scale(cab, glm::vec3(0.95f, 1.05f, 0.95f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], cab, glm::vec3(0.1f, 0.9f, 0.1f));
    }

    {
        glm::mat4 mot = base;
        mot = glm::translate(mot, glm::vec3(1.10f, 0.55f, 0.0f));
        mot = glm::scale(mot, glm::vec3(1.55f, 0.5f, 0.85f));
        RenderSimpleMesh(meshes["cylinder"], shaders["LabShader"], mot, glm::vec3(0.1f, 0.2f, 0.9f));
    }

    {
        glm::mat4 dot = base;
        dot = glm::translate(dot, glm::vec3(1.10f + 1.55f * 0.5f, 0.50f, 0.0f));
        dot = glm::scale(dot, glm::vec3(0.10f));
        RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], dot, glm::vec3(1.0f, 0.0f, 1.0f));
    }

    {
        glm::mat4 body = base;
        body = glm::translate(body, glm::vec3(0.0f, 0.12f, 0.0f));
        body = glm::scale(body, glm::vec3(4.05f, 0.17f, 0.90f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], body, glm::vec3(0.95f, 0.85f, 0.1f));
    }

    {
        glm::mat4 wag = base;
        wag = glm::translate(wag, glm::vec3(-1.2f, 0.60f, 0));
        wag = glm::scale(wag, glm::vec3(1.67f, 0.75f, 0.9f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], wag, glm::vec3(0.1f, 0.9f, 0.1f));
    }

    auto drawWheel = [&](float x, float z) {
        glm::mat4 w = base;
        w = glm::translate(w, glm::vec3(x, -0.18f, z));
        w = glm::rotate(w, RADIANS(90.f), glm::vec3(1, 0, 0));
        w = glm::scale(w, glm::vec3(0.24f, 0.24f, 0.16f));
        RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], w, glm::vec3(0.8f, 0.1f, 0.1f));
        };

    float zWheel = 0.32f;
    float xs[] = { 0.10f, 0.35f, 0.60f, 0.85f, 1.10f, 1.35f, 1.60f, 1.85f };
    for (float xw : xs) {
        drawWheel(xw, zWheel);
        drawWheel(xw, -zWheel);
    }
    drawWheel(-1.5f, zWheel);
    drawWheel(-1.5f, -zWheel);
    drawWheel(-0.55f, zWheel);
    drawWheel(-0.55f, -zWheel);
}

void Lab7::DrawDrezina()
{
    glm::mat4 base = glm::translate(glm::mat4(1), drezina.position + glm::vec3(0, 0.20f, 0));
    base = glm::rotate(base, drezina.yaw, glm::vec3(0, 1, 0));

    const glm::vec3 orange = glm::vec3(1.0f, 0.55f, 0.10f);
    const glm::vec3 dark = glm::vec3(0.20f);
    const glm::vec3 green = glm::vec3(0.10f, 0.90f, 0.10f);

    {
        glm::mat4 body = base;
        body = glm::scale(body, glm::vec3(0.8f, 0.25f, 0.6f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], body, orange);
    }

    {
        glm::mat4 topBox = base;
        topBox = glm::translate(topBox, glm::vec3(0.0f, 0.28f, 0.0f));
        topBox = glm::scale(topBox, glm::vec3(0.22f, 0.28f, 0.22f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], topBox, orange);
    }

    {
        glm::mat4 post = base;
        post = glm::translate(post, glm::vec3(0.0f, 0.58f, 0.0f));
        post = glm::rotate(post, RADIANS(90.f), glm::vec3(0, 0, 1));
        post = glm::scale(post, glm::vec3(0.55f, 0.10f, 0.10f));
        RenderSimpleMesh(meshes["cylinder"], shaders["LabShader"], post, dark);
    }

    {
        float a = 0.35f * sin(4.f * timeAcc);

        glm::mat4 handle = base;
        handle = glm::translate(handle, glm::vec3(0.0f, 0.90f, 0.0f));
        handle = glm::rotate(handle, a, glm::vec3(0, 0, 1));

        {
            glm::mat4 bar = handle;
            bar = glm::scale(bar, glm::vec3(1.25f, 0.06f, 0.06f));
            RenderSimpleMesh(meshes["cylinder"], shaders["LabShader"], bar, dark);
        }

        float xOff = 0.70f;
        float gLen = 0.36f;
        float gRad = 0.07f;

        {
            glm::mat4 capL = handle;
            capL = glm::translate(capL, glm::vec3(-xOff, 0.0f, 0.0f));
            capL = glm::rotate(capL, RADIANS(90.f), glm::vec3(0, 1, 0));
            capL = glm::scale(capL, glm::vec3(gLen, gRad, gRad));
            RenderSimpleMesh(meshes["cylinder"], shaders["LabShader"], capL, green);
        }

        {
            glm::mat4 capR = handle;
            capR = glm::translate(capR, glm::vec3(+xOff, 0.0f, 0.0f));
            capR = glm::rotate(capR, RADIANS(90.f), glm::vec3(0, 1, 0));
            capR = glm::scale(capR, glm::vec3(gLen, gRad, gRad));
            RenderSimpleMesh(meshes["cylinder"], shaders["LabShader"], capR, green);
        }
    }

    auto wheel = [&](float x, float z)
        {
            glm::mat4 w = base;
            w = glm::translate(w, glm::vec3(x, 0.02f, z));
            w = glm::rotate(w, RADIANS(90.f), glm::vec3(0, 1, 0));
            w = glm::scale(w, glm::vec3(0.12f, 0.18f, 0.18f));
            RenderSimpleMesh(meshes["cylinder"], shaders["LabShader"], w, glm::vec3(0.20f));
        };

    float zWheel = 0.32f;
    float xFront = +0.30f;
    float xBack = -0.30f;

    wheel(xFront, +zWheel);
    wheel(xFront, -zWheel);
    wheel(xBack, +zWheel);
    wheel(xBack, -zWheel);
}

void Lab7::FrameEnd()
{
    DrawCoordinateSystem();
}

void Lab7::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID()) return;
    if (!mesh->GetBuffers() || mesh->indices.empty() || mesh->GetBuffers()->m_VAO == 0) return;

    glUseProgram(shader->program);

    int loc_light_pos = glGetUniformLocation(shader->program, "light_position");
    glUniform3fv(loc_light_pos, 1, glm::value_ptr(lightPosition));

    glm::vec3 eyePosition = GetSceneCamera()->m_transform->GetWorldPosition();
    int loc_eye_pos = glGetUniformLocation(shader->program, "eye_position");
    glUniform3fv(loc_eye_pos, 1, glm::value_ptr(eyePosition));

    int loc_shininess = glGetUniformLocation(shader->program, "material_shininess");
    glUniform1i(loc_shininess, materialShininess);

    int loc_kd = glGetUniformLocation(shader->program, "material_kd");
    glUniform1f(loc_kd, materialKd);

    int loc_ks = glGetUniformLocation(shader->program, "material_ks");
    glUniform1f(loc_ks, materialKs);

    int loc_object_color = glGetUniformLocation(shader->program, "object_color");
    glUniform3fv(loc_object_color, 1, glm::value_ptr(color));

    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void Lab7::OnInputUpdate(float dt, int mods)
{
    if (gameOver) return;

    if (window->KeyHold(GLFW_KEY_A)) drezina.choice = -1;
    else if (window->KeyHold(GLFW_KEY_D)) drezina.choice = +1;

    int moveDir = 0;
    if (window->KeyHold(GLFW_KEY_W)) moveDir++;
    if (window->KeyHold(GLFW_KEY_S)) moveDir--;
    if (moveDir == 0) return;
    glm::vec3 prevPos = drezina.position;

    drezina.progress += dt * drezina.speed * (float)moveDir;

    while (drezina.progress < 0.f)
    {
        const Rail& cur = rails[drezina.railIndex];
        int prevIdx = cur.prevDefault;

        if (prevIdx < 0 || prevIdx >= (int)rails.size()) {
            drezina.progress = 0.f;
            break;
        }

        const Rail& prv = rails[prevIdx];

        glm::vec3 joint = StartPoint(cur.start, cur.end, drezina.reversed);

        bool prevRev = ComputeReversedByConnection(prv.start, prv.end, joint);
        prevRev = !prevRev;

        drezina.reversed = prevRev;
        drezina.railIndex = prevIdx;
        drezina.progress += 1.f;
    }

    while (drezina.progress >= 1.f)
    {
        drezina.progress -= 1.f;

        const Rail& cur = rails[drezina.railIndex];

        int nextIdx = cur.nextDefault;
        if (cur.isJunction) {
            if (drezina.choice < 0 && cur.nextLeft != -1) nextIdx = cur.nextLeft;
            if (drezina.choice > 0 && cur.nextRight != -1) nextIdx = cur.nextRight;
        }

        if (nextIdx < 0 || nextIdx >= (int)rails.size())
            nextIdx = 0;

        const Rail& nxt = rails[nextIdx];

        glm::vec3 exitP = EndPoint(cur.start, cur.end, drezina.reversed);

        drezina.reversed = ComputeReversedByConnection(nxt.start, nxt.end, exitP);

        drezina.railIndex = nextIdx;
        drezina.choice = 0;
    }

    const Rail& rr = rails[drezina.railIndex];
    glm::vec3 a = StartPoint(rr.start, rr.end, drezina.reversed);
    glm::vec3 b = EndPoint(rr.start, rr.end, drezina.reversed);

    drezina.position = glm::mix(a, b, drezina.progress);

    glm::vec3 segmentDir = b - a;
    segmentDir.y = 0.f;
    if (glm::length(segmentDir) > 0.0001f) {
        segmentDir = glm::normalize(segmentDir);
    }

    if (moveDir < 0) {
        segmentDir = -segmentDir;
    }

    gDrezMoveDir = NormalizeXZ(segmentDir);

    float yawDesired = atan2(gDrezMoveDir.z, gDrezMoveDir.x) + kModelYawOffset();

    bool onX = (fabs(gDrezMoveDir.x) > fabs(gDrezMoveDir.z));
    if (onX) yawDesired += (float)M_PI;

    float angleDiff = WrapAnglePi(yawDesired - drezina.yaw);
    drezina.yaw += angleDiff * (1.f - expf(-20.f * dt));

    glm::vec3 v = drezina.position - prevPos;
    v.y = 0.f;

    if (glm::length(v) > 1e-5f) {
        gDrezMoveDir = glm::normalize(v);
    }
    else {
        gDrezMoveDir = NormalizeXZ(gDrezMoveDir);
    }
}

void Lab7::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_F) {
        for (int ri = 0; ri < (int)rails.size(); ri++) {
            for (int pi = 0; pi < kPiecesPerRail; pi++) {
                int idx = ri * kPiecesPerRail + pi;
                if (!pieceDamaged[idx]) continue;

                glm::vec3 c = PieceCenterWorld(ri, pi);

                if (glm::distance(drezina.position, c) <= 1.5f) {
                    pieceDamaged[idx] = false;
                    damagedSegments = std::max(0, damagedSegments - 1);
                }
            }
        }
    }

    if (key == GLFW_KEY_C) {
        drezina.choice = 0;
    }
}

glm::vec3 Lab7::PieceCenterWorld(int railIdx, int pieceIdx) const
{
    const Rail& r = rails[railIdx];
    float tMid = ((float)pieceIdx + 0.5f) / (float)kPiecesPerRail;
    return glm::mix(r.start, r.end, tMid);
}

void Lab7::DrawHUD()
{
    if (!textReady || !text) return;

    char line1[128];
    char line2[128];
    char line3[128];

    snprintf(line1, sizeof(line1), "Time: %.1f s", gameTime);
    snprintf(line2, sizeof(line2), "Damaged: %d", damagedSegments);

    float remaining = kMaxTrainWait;
    if (trainBlocked) remaining = kMaxTrainWait - trainBlockedTime;
    if (remaining < 0.f) remaining = 0.f;

    snprintf(line3, sizeof(line3), "Train wait left: %.0f s", remaining);

    float x = 20.f;
    float y = 30.f;

    text->RenderText(line1, x, y, 1.0f, glm::vec3(1, 1, 1));
    text->RenderText(line2, x, y + 28.f, 1.0f, glm::vec3(1, 1, 1));
    text->RenderText(line3, x, y + 56.f, 1.0f, glm::vec3(1, 1, 1));
}

void Lab7::DrawGameOverScreen()
{
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!textReady || !text) return;

    glm::ivec2 res = window->GetResolution();
    float x = res.x * 0.5f - 140.f;
    float y = res.y * 0.5f;

    text->RenderText("GAME OVER", x, y, 2.0f, glm::vec3(1, 1, 1));
}
