#pragma once

#include "board.h"
#include "raylib.h"
#include <iostream>
#include <string>

class ChessGUI {
public:
  static constexpr int boardSize = 8; // fixed board size

  ChessGUI(int screenWidth, int screenHeight, Board *board, int padding = 40)
      : screenWidth(screenWidth), screenHeight(screenHeight), board(board),
        padding(padding) {
    InitWindow(screenWidth, screenHeight, "Chess Board with Padding");
    squareSize = (screenHeight - 2 * padding) / boardSize;
    SetTargetFPS(60);
  }

  ~ChessGUI() { CloseWindow(); }

  void Run() {
    while (!WindowShouldClose()) {
      BeginDrawing();
      ClearBackground(RAYWHITE);

      DrawBoard();
      DrawCoordinates();
      DrawPieces();
      HandleMouseInput();

      EndDrawing();
    }
  }

private:
  int screenWidth;
  int screenHeight;
  int squareSize;
  int padding;
  Board *board;

  const char *pieceSymbols[12] = {"P", "N", "B", "R", "Q", "K",
                                  "p", "n", "b", "r", "q", "k"};

  void DrawBoard() {
    Color lightColor = {240, 217, 181, 255};
    Color darkColor = {181, 136, 99, 255};

    for (int row = 0; row < boardSize; ++row) {
      for (int col = 0; col < boardSize; ++col) {
        Color color = ((row + col) % 2 == 0) ? lightColor : darkColor;
        DrawRectangle(padding + col * squareSize, padding + row * squareSize,
                      squareSize, squareSize, color);
      }
    }
  }

  void DrawCoordinates() {
    for (int i = 0; i < boardSize; ++i) {
      // Files (a-h)
      char fileChar = 'a' + i;
      int fileX = padding + i * squareSize + squareSize / 2 - 5;
      int fileY = padding + boardSize * squareSize + 5;
      DrawText(TextFormat("%c", fileChar), fileX, fileY, 20, DARKGRAY);

      // Ranks (8-1)
      char rankChar = '8' - i;
      int rankX = padding - 25;
      int rankY = padding + i * squareSize + squareSize / 2 - 10;
      DrawText(TextFormat("%c", rankChar), rankX, rankY, 20, DARKGRAY);
    }
  }

  void DrawPieces() {
    for (int i = 0; i < 12; ++i) {
      uint64_t bitboard = board->bitboards[i];
      for (int square = 0; square < 64; ++square) {
        if (bitboard & (1ULL << square)) {
          int row = 7 - (square / 8); // top-to-bottom
          int col = square % 8;

          int x = padding + col * squareSize + squareSize / 4;
          int y = padding + row * squareSize + squareSize / 4;

          DrawText(pieceSymbols[i], x, y, squareSize / 2, BLACK);
        }
      }
    }
  }

  void HandleMouseInput() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      int mouseX = GetMouseX();
      int mouseY = GetMouseY();

      // Convert mouse position to board indices
      int col = (mouseX - padding) / squareSize;
      int row = (mouseY - padding) / squareSize;

      if (col >= 0 && col < boardSize && row >= 0 && row < boardSize) {
        char file = 'a' + col;
        char rank = '8' - row;

        std::cout << "Clicked on square: " << file << rank << std::endl;
      }
    }
  }
};
