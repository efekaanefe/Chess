#pragma once

#include "board.h"
#include <cstdint>

class Evaluator {
  public:
    // Piece values in centipawns
    static constexpr int PIECE_VALUES[6] = {
        100,  // Pawn
        320,  // Knight
        330,  // Bishop
        500,  // Rook
        900,  // Queen
        20000 // King
    };

    // Position bonus tables (from white's perspective)
    static constexpr int PAWN_TABLE[64] = {
        0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
        10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
        0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
        5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0};

    static constexpr int KNIGHT_TABLE[64] = {
        -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
        0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
        15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
        -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
        5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50};

    static constexpr int BISHOP_TABLE[64] = {
        -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
        0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
        5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
        -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
        0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20};

    static constexpr int ROOK_TABLE[64] = {
        0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,
        -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
        -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
        -5, 0, 0, 0, 0, 0, 0, -5, 0,  0,  0,  5,  5,  0,  0,  0};

    static constexpr int QUEEN_TABLE[64] = {
        -20, -10, -10, -5,  -5,  -10, -10, -20, -10, 0,   0,   0,  0,
        0,   0,   -10, -10, 0,   5,   5,   5,   5,   0,   -10, -5, 0,
        5,   5,   5,   5,   0,   -5,  0,   0,   5,   5,   5,   5,  0,
        -5,  -10, 5,   5,   5,   5,   5,   0,   -10, -10, 0,   5,  0,
        0,   0,   0,   -10, -20, -10, -10, -5,  -5,  -10, -10, -20};

    static constexpr int KING_MIDDLE_GAME[64] = {
        -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
        -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
        -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
        -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
        0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};

    static constexpr int KING_END_GAME[64] = {
        -50, -40, -30, -20, -20, -30, -40, -50, -30, -20, -10, 0,   0,
        -10, -20, -30, -30, -10, 20,  30,  30,  20,  -10, -30, -30, -10,
        30,  40,  40,  30,  -10, -30, -30, -10, 30,  40,  40,  30,  -10,
        -30, -30, -10, 20,  30,  30,  20,  -10, -30, -30, -30, 0,   0,
        0,   0,   -30, -30, -50, -30, -30, -30, -30, -30, -30, -50};

    // Flip square index for black pieces
    static int FlipSquare(int square) {
        return square ^ 56; // Flips rank
    }

    // Check if position is endgame (few pieces remaining)
    static bool IsEndgame(const Board &board) {
        int pieceCount = 0;
        for (int i = 0; i < 12; i++) {
            pieceCount += __builtin_popcountll(board.bitboards[i]);
        }
        return pieceCount <= 10; // Arbitrary threshold
    }

    // Evaluate material balance
    static int EvaluateMaterial(const Board &board) {
        int score = 0;
        for (int pieceType = 0; pieceType < 6; pieceType++) {
            int whitePieces = __builtin_popcountll(board.bitboards[pieceType]);
            int blackPieces =
                __builtin_popcountll(board.bitboards[pieceType + 6]);
            score += (whitePieces - blackPieces) * PIECE_VALUES[pieceType];
        }
        return score;
    }

    // Evaluate piece positions
    static int EvaluatePosition(const Board &board) {
        int score = 0;
        bool endgame = IsEndgame(board);

        // White pieces
        for (int pieceType = 0; pieceType < 6; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1; // Clear LSB

                switch (pieceType) {
                case 0:
                    score += PAWN_TABLE[square];
                    break;
                case 1:
                    score += KNIGHT_TABLE[square];
                    break;
                case 2:
                    score += BISHOP_TABLE[square];
                    break;
                case 3:
                    score += ROOK_TABLE[square];
                    break;
                case 4:
                    score += QUEEN_TABLE[square];
                    break;
                case 5:
                    score += endgame ? KING_END_GAME[square]
                                     : KING_MIDDLE_GAME[square];
                    break;
                }
            }
        }

        // Black pieces (flip tables)
        for (int pieceType = 6; pieceType < 12; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1; // Clear LSB
                int flippedSquare = FlipSquare(square);

                switch (pieceType - 6) {
                case 0:
                    score -= PAWN_TABLE[flippedSquare];
                    break;
                case 1:
                    score -= KNIGHT_TABLE[flippedSquare];
                    break;
                case 2:
                    score -= BISHOP_TABLE[flippedSquare];
                    break;
                case 3:
                    score -= ROOK_TABLE[flippedSquare];
                    break;
                case 4:
                    score -= QUEEN_TABLE[flippedSquare];
                    break;
                case 5:
                    score -= endgame ? KING_END_GAME[flippedSquare]
                                     : KING_MIDDLE_GAME[flippedSquare];
                    break;
                }
            }
        }

        return score;
    }

    // Main evaluation function
    static int Evaluate(const Board &board) {
        int materialScore = EvaluateMaterial(board);
        int positionScore = EvaluatePosition(board);

        int totalScore = materialScore + positionScore;

        /*std::cout << "Evaluation: " << totalScore << std::endl;*/

        // Return score from current player's perspective
        return board.whiteToMove ? totalScore : -totalScore;
    }
};
