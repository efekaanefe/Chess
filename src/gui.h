#include "raylib.h"
#include <string>

class ChessGUI {
public:
    static constexpr int boardSize = 8;  // fixed board size


    ChessGUI(int screenWidth, int screenHeight, int padding = 40)
        : screenWidth(screenWidth), screenHeight(screenHeight), padding(padding)
    {
        InitWindow(screenWidth, screenHeight, "Chess");
        squareSize = (screenHeight - 2 * padding) / boardSize;  // scale squares to fit inside padding
        SetTargetFPS(60);
    }

    ~ChessGUI() {
        CloseWindow();
    }

    void Run() {
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawBoard();
            DrawCoordinates();
            EndDrawing();
        }
    }

private:
    int screenWidth;
    int screenHeight;
    int squareSize;
    int padding;

    void DrawBoard() {
        Color lightColor = { 240, 217, 181, 255 };
        Color darkColor = { 181, 136, 99, 255 };

        for (int row = 0; row < boardSize; ++row) {
            for (int col = 0; col < boardSize; ++col) {
                Color color = ((row + col) % 2 == 0) ? lightColor : darkColor;
                DrawRectangle(padding + col * squareSize,
                              padding + row * squareSize,
                              squareSize,
                              squareSize,
                              color);
            }
        }
    }

    void DrawCoordinates() {
        for (int i = 0; i < boardSize; ++i) {
            // Files (a-h) at bottom, centered under each square, offset below the board
            char fileChar = 'a' + i;
            int fileX = padding + i * squareSize + squareSize / 2 - 5;
            int fileY = padding + boardSize * squareSize + 5;  // a bit below squares
            DrawText(TextFormat("%c", fileChar), fileX, fileY, 20, DARKGRAY);

            // Ranks (8-1) on the left side, centered next to each square, offset left of the board
            char rankChar = '8' - i;
            int rankX = padding - 25;  // 25 pixels left of board
            int rankY = padding + i * squareSize + squareSize / 2 - 10;
            DrawText(TextFormat("%c", rankChar), rankX, rankY, 20, DARKGRAY);
        }
    }
};

