#pragma once

#include "board.h"
#include "raylib.h"
#include "search.h"
#include <iostream>
#include <ostream>
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
            HandleKeyboardInput();
            HandleAI();

            EndDrawing();
        }
    }

  private:
    int screenWidth;
    int screenHeight;
    int squareSize;
    int padding;
    std::unordered_map<int, Texture2D> pieceTextures;

    Board *board;

    int selectedSquare = -1; // -1 means no selection
    int clickedSquare = -1;
    std::vector<Move> legalMoves;
    std::vector<Move> moveHistory;

    // AI config
    bool aiEnabled = true;
    bool aiPlaysAsWhite = false;
    int aiDepth = 10;
    SearchEngine engine;

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
                DrawRectangle(padding + col * squareSize,
                              padding + row * squareSize, squareSize,
                              squareSize, color);
            }
        }

        for (const auto &move : legalMoves) {
            // Highlight possible target squares
            int sq = move.toSquare;
            int file = sq % 8;
            int rank = 7 - (sq / 8); // flip rank for display

            Color color = move.isCapture ? RED : YELLOW;
            DrawRectangle(padding + file * squareSize,
                          padding + rank * squareSize, (float)squareSize,
                          (float)squareSize, Fade(color, 0.4f));
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
                                  (float)squareSize / pieceTextures[i].width,
                                  WHITE);
                }
            }
        }
        if (selectedSquare != -1) {
            int selRow = 7 - (selectedSquare / 8);
            int selCol = selectedSquare % 8;
            DrawRectangle(padding + selCol * squareSize,
                          padding + selRow * squareSize, squareSize, squareSize,
                          Fade(PURPLE, 0.4f));
        }
    }

    void HandleAI() {
        // Check if it's AI's turn
        bool isAITurn = aiEnabled && (board->whiteToMove == aiPlaysAsWhite);
        if (isAITurn) {
            // Generate moves to check if game is over
            auto moves = board->GenerateMoves();
            std::cout << "=== AI Debug Info ===" << std::endl;
            std::cout << "Generated " << moves.size() << " moves" << std::endl;
            std::cout << "Current turn: "
                      << (board->whiteToMove ? "White" : "Black") << std::endl;
            std::cout << "AI plays as: " << (aiPlaysAsWhite ? "White" : "Black")
                      << std::endl;

            if (moves.empty()) {
                std::cout << "Game Over - No legal moves!" << std::endl;
                return;
            }

            std::cout << "AI thinking using SearchEngine..." << std::endl;

            // Call FindBestMove from the SearchEngine
            // This will perform the alpha-beta search with iterative deepening
            SearchEngine::SearchResult result =
                engine.FindBestMove(*board, aiDepth);

            Move selectedMove = result.bestMove;
            int bestScore = result.score;


            if (selectedMove.fromSquare == 0 && selectedMove.toSquare == 0 &&
                moves.size() > 0) {
                selectedMove = moves[0];
                std::cout << "SearchEngine returned a default move, falling "
                             "back to first legal move."
                          << std::endl;
            }

            std::cout << "AI selected: " << selectedMove.ToString()
                      << " with score: " << bestScore
                      << " (searched to depth: " << result.depth
                      << ", nodes: " << result.nodesSearched << ")"
                      << std::endl;

            moveHistory.push_back(selectedMove);

            board->MakeMove(selectedMove);

            // Clear any player selection
            selectedSquare = -1;
            clickedSquare = -1;
            legalMoves.clear();

            std::cout << "===================" << std::endl;
        }
    }

    void HandleMouseInput() {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

            Vector2 mouse = GetMousePosition();
            int file = (mouse.x - padding) / squareSize;
            int rank = 8 - ((mouse.y - padding) / squareSize); // flip back

            if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
                int square = rank * 8 + file;

                char file = 'a' + (square % 8);
                char rank = '1' + (square / 8);
                std::cout << "Is " << file << rank << " attacked??? "
                          << std::boolalpha
                          << board->IsSquareAttacked(square,
                                                     !board->whiteToMove)
                          << std::endl;

                if (selectedSquare == -1) {
                    // Selection stage

                    // Check if square has a piece of side to move
                    bool validSelection = false;

                    int start = board->whiteToMove == 1 ? 0 : 6;
                    int end = start + 6;

                    for (int i = start; i < end; i++) {
                        if (board->bitboards[i] & (1ULL << square)) {
                            validSelection = true;
                            break;
                        }
                    }

                    if (!validSelection)
                        return;

                    selectedSquare = square;
                    legalMoves.clear();

                    auto allMoves = board->GenerateMoves();
                    for (auto &move : allMoves) {
                        if (move.fromSquare == selectedSquare) {
                            legalMoves.push_back(move);
                        }
                    }

                } else {
                    // Move stage
                    clickedSquare = square;
                    bool moveMade = false;

                    for (auto &move : legalMoves) {
                        if (move.toSquare == clickedSquare) {
                            moveHistory.push_back(move); // Save move
                            board->MakeMove(move);
                            std::cout << move.ToString() << std::endl;
                            moveMade = true;
                            break;
                        }
                    }

                    selectedSquare = -1;
                    clickedSquare = -1;
                    legalMoves.clear();
                }
            }
        }
    }

    void HandleKeyboardInput() {
        if (IsKeyPressed(KEY_U)) {
            if (!moveHistory.empty()) {
                Move lastMove = moveHistory.back();
                board->UndoMove(lastMove);
                moveHistory.pop_back();
                std::cout << "Undid move: " << lastMove.ToString() << std::endl;

                // Clear any selection state
                selectedSquare = -1;
                clickedSquare = -1;
                legalMoves.clear();
            } else {
                std::cout << "No moves to undo!" << std::endl;
            }
        }
        if (IsKeyPressed(KEY_P)) {
            std::cout << "Attacked squares by "
                      << (board->whiteToMove ? "black" : "white") << ":\n";

            for (int square = 0; square < 64; ++square) {
                if (board->IsSquareAttacked(square, !board->whiteToMove)) {
                    char file = 'a' + (square % 8);
                    char rank = '1' + (square / 8);
                    std::cout << file << rank << " ";
                }
            }
            std::cout << std::endl;
        }
    }
};
