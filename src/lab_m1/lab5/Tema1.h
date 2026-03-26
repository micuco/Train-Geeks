#pragma once

#include "components/simple_scene.h"
#include "lab_m1/lab3/transform2D.h"
#include "lab_m1/lab3/object2D.h"

#include <vector>
#include <queue>



namespace m1
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        Tema1();
        ~Tema1();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;
        void UpdateLayout(const glm::ivec2& resolution);
        void RenderDigit(int digit, glm::vec2 bottomLeft, float size);


    private:
        enum BlockType
        {
            Empty,
            Solid,
            Gun,
            Engine,
            Bumper
        };

        enum GameState
        {
            Editor,
            Play
        };

        static const int GRID_W = 10;
        static const int GRID_H = 10;
        static const int MAX_BLOCKS = 10;

        struct ShipBlock
        {
            BlockType type;
            glm::vec2 localOffset;
        };

        struct Ball
        {
            glm::vec2 pos;
            glm::vec2 vel;
        };

        void InitMeshes();

        void RenderEditor(float dt);
        void RenderGrid();
        void RenderPalette();
        void RenderBlocksBar();
        void RenderStartButton();
        void RenderBlock(BlockType type, glm::vec2 origin, bool largeDecor = false);


        void RenderGame(float dt);
        void RenderScores();

        bool IsInsideRect(float x, float y,
            float rx, float ry, float rw, float rh) const;

        bool ScreenToGridCell(int mx, int my, int& gx, int& gy) const;
        bool ScreenToPalette(int mx, int my, BlockType& outType) const;

        bool IsShipValid() const;
        bool CheckConnectivity() const;

        void PreparePaddlesFromShip();
        void StartNewRound();

    private:
        GameState state;

        BlockType grid[GRID_H][GRID_W];
        int       blockCount;

        float     cellSize;
        glm::vec2 gridOrigin;

        glm::vec2 paletteOrigin;
        float     paletteCellH;

        glm::vec2 blocksBarOrigin;

        glm::vec2 startButtonOrigin;
        glm::vec2 startButtonSize;

        bool      isDragging;
        BlockType draggingType;

        int       mouseX, mouseY;

        std::vector<ShipBlock> shipBlocks;

        glm::vec3 baseCameraPos;
        float shakeTimer;
        float shakeDuration;
        float shakeAmplitude;

        void TriggerCameraShake();

        glm::vec2 leftPaddlePos;
        glm::vec2 rightPaddlePos;
        glm::vec2 paddleSize;

        float     leftAnimTime;
        float     rightAnimTime;

        float     ballRadius;
        std::vector<Ball> balls;

        int       leftScore;
        int       rightScore;
        int serveDir;
    };
}
