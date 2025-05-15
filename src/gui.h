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
    InitWindow(screenWidth, screenHeight, "Chess Board");
    squareSize = (screenHeight - 2 * padding) / boardSize;
    LoadPieceTextures();
    SetTargetFPS(60);
  }

  ~ChessGUI() {
    for (auto &[_, tex] : pieceTextures) {
      UnloadTexture(tex);
    }
    CloseWindow();
  }

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

  std::unordered_map<int, Texture2D> pieceTextures;

  void LoadPieceTextures() {
    std::string names[12] = {"wP", "wN", "wB", "wR", "wQ", "wK",
                             "bP", "bN", "bB", "bR", "bQ", "bK"};

    for (int i = 0; i < 12; ++i) {
      std::string path = "assets/" + names[i] + ".png";
      pieceTextures[i] = LoadTexture(path.c_str());
    }
  }

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
      uint64_t bb = board->bitboards[i];
      for (int sq = 0; sq < 64; ++sq) {
        if (bb & (1ULL << sq)) {
          int row = 7 - (sq / 8);
          int col = sq % 8;
          int x = padding + col * squareSize;
          int y = padding + row * squareSize;
          DrawTextureEx(pieceTextures[i], {(float)x, (float)y}, 0.0f,
                        (float)squareSize / pieceTextures[i].width, WHITE);
        }
      }
    }
  }

  void HandleMouseInput() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mouse = GetMousePosition();
      int file = (mouse.x - padding) / squareSize;
      int rank = 8 - (mouse.y - padding) / squareSize;

      if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
        int square = rank * 8 + file;

        std::string pieceStr = "Empty";
        for (int i = 0; i < 12; ++i) {
          if (board->bitboards[i] & (1ULL << square)) {
            std::string names[12] = {"wP", "wN", "wB", "wR", "wQ", "wK",
                                     "bP", "bN", "bB", "bR", "bQ", "bK"};
            pieceStr = names[i];
            break;
          }
        }

        std::cout << "Clicked square: " << (char)('a' + file)
                  <<  (1 + rank) << ", square=" << square
                  << ", piece=" << pieceStr << std::endl;
      }
    }
  }
};
