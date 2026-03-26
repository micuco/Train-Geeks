#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"
#include "components/text_renderer.h"

#include <vector>

namespace m1
{
    class Lab7 : public gfxc::SimpleScene
    {
    public:
        Lab7();
        ~Lab7();

        void Init() override;
        static const int kPiecesPerRail = 4;

        std::vector<bool> pieceDamaged;

        bool trainBlocked = false;
        float trainBlockedTime = 0.f;
        int blockedRailIdx = -1;
        int blockedPieceIdx = -1;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix,
            const glm::vec3& color = glm::vec3(1));

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;

        struct Rail {
            glm::vec3 start;
            glm::vec3 end;

            int nextDefault;
            int prevDefault;

            bool isJunction;
            int nextLeft;
            int nextRight;
        };

        struct Station {
            glm::vec3 pos;
            glm::vec3 color;
        };

        struct Train {
            int railIndex;
            float progress;
            float speed;
            glm::vec3 position;
            bool reversed;
        };

        struct Drezina {
            int railIndex;
            float progress;
            float speed;

            int choice;

            glm::vec3 position;
            float yaw;
            bool reversed;
        };

        std::vector<Rail> rails;
        std::vector<Station> stations;

        Train train;
        Drezina drezina;

        float timeAcc;
        float damageTick;
        bool gameOver;

        glm::vec3 lightPosition;
        unsigned int materialShininess;
        float materialKd;
        float materialKs;
        gfxc::TextRenderer* text = nullptr;
        bool textReady = false;

        float gameTime = 0.f;
        int damagedSegments = 0;

        const float kMaxTrainWait = 30.f;

        void DrawHUD();
        void DrawGameOverScreen();

        static constexpr float kSignalBlockSeconds = 5.0f;

        std::vector<bool> signalGreen;
        float signalTick = 0.f;

        bool trainWaitingSignal = false;
        float trainSignalWait = 0.f;
        int waitingAtRailIdx = -1;

        glm::vec3 camPosSmooth;
        glm::vec3 camTargetSmooth;
        bool camInited;

        void BuildTrackAndStations();

        void UpdateTrain(float dt);
        void UpdateCameraTPS(float dt);

        void DrawTerrain();
        void DrawRails();
        void DrawStations();
        void DrawTrain();
        void DrawDrezina();

        glm::vec3 PieceCenterWorld(int railIdx, int pieceIdx) const;
    };
}
