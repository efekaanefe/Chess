#pragma once

#include "board.h"
#include <cstdint>
#include <vector>

class Evaluator {
  public:
    // Constants for game ending conditions
    static constexpr int CHECKMATE = 20000;
    static constexpr int STALEMATE = 0;

    static constexpr int PIECE_VALUES[6] = {
        100, // Pawn
        310, // Knight
        320, // Bishop
        500, // Rook
        900, // Queen
        0    // King
    };

    static constexpr int WHITE_PAWN_TABLE[64] = {
        0,   0,   0,   0,   0,   0,   0,   0,  98,  134, 61,  95,  68,
        126, 34,  -11, -6,  7,   26,  31,  65, 56,  25,  -20, -14, 13,
        6,   21,  23,  12,  17,  -23, -27, -2, -5,  12,  17,  6,   10,
        -25, -26, -4,  -4,  -10, 3,   3,   33, -12, -35, -1,  -20, -23,
        -15, 24,  38,  -22, 0,   0,   0,   0,  0,   0,   0,   0};

    static constexpr int BLACK_PAWN_TABLE[64] = {
        0,   0,   0,  0,   0,   0,   0,   0,   -22, 0,   0,   0,  0,
        0,   0,   0,  -15, 24,  38,  -23, -20, -1,  -35, -12, 3,  3,
        33,  -10, -4, -4,  -26, -25, 10,  6,   17,  12,  -5,  -2, -27,
        -23, 17,  12, 23,  21,  6,   13,  -14, -20, 25,  56,  65, 31,
        26,  7,   -6, -11, 34,  126, 68,  95,  61,  134, 98,  0};

    static constexpr int KNIGHT_TABLE[64] = {
        -167, -89, -34, -49, 61,   -97, -15, -107, -73, -41, 72,  36,  23,
        62,   7,   -17, -47, 60,   37,  65,  84,   129, 73,  44,  -9,  17,
        19,   53,  37,  69,  18,   22,  -13, 4,    16,  13,  28,  19,  21,
        -8,   -23, -9,  12,  10,   19,  17,  25,   -16, -29, -53, -12, -3,
        -1,   18,  -14, -19, -105, -21, -58, -33,  -17, -28, -19, -23};

    static constexpr int BISHOP_TABLE[64] = {
        -29, 4,  -82, -37, -25, -42, 7,  -8,  -26, 16, -18, -13, 30,
        59,  18, -47, -16, 37,  43,  40, 35,  50,  37, -2,  -4,  5,
        19,  50, 37,  37,  7,   -2,  -6, 13,  13,  26, 34,  12,  10,
        4,   0,  15,  15,  15,  14,  27, 18,  10,  4,  15,  16,  0,
        7,   21, 13,  4,   -12, 7,   2,  -13, 1,   13, 4,   -16};

    static constexpr int ROOK_TABLE[64] = {
        32, 10, 22, 37, 33, 17, 23, 29, 36, 37, 38, 43, 45, 42, 37, 29,
        8,  14, 25, 30, 30, 25, 18, 8,  0,  7,  17, 22, 22, 17, 8,  1,
        -2, 4,  10, 14, 14, 10, 4,  -1, -3, 1,  6,  9,  9,  6,  1,  -2,
        -3, 1,  4,  7,  7,  4,  1,  -3, 8,  8,  8,  13, 13, 8,  8,  8};

    static constexpr int QUEEN_TABLE[64] = {
        -28, -4,  -2,  -11, -9,  -10, -6,  -28, -33, -4,  -3,  -7,  -6,
        -2,  -7,  -33, -20, -9,  -2,  -2,  -3,  -5,  -6,  -20, -15, -4,
        -1,  1,   1,   0,   -5,  -15, -12, -3,  -1,  3,   2,   1,   -2,
        -12, -13, -5,  -3,  -1,  0,   -2,  -4,  -13, -20, -11, -4,  -3,
        -4,  -6,  -9,  -20, -28, -15, -10, -6,  -7,  -9,  -13, -28};

    static constexpr int KING_MIDDLE_GAME[64] = {
        -65, -54, -48, -43, -52, -28, -41, -67, -40, -24, -24, -19, -15,
        -14, -25, -40, -13, 3,   8,   15,  15,  3,   1,   -9,  7,   16,
        23,  23,  23,  23,  16,  7,   23,  23,  27,  27,  27,  27,  23,
        23,  30,  30,  30,  30,  30,  30,  30,  30,  33,  35,  36,  36,
        36,  35,  33,  33,  7,   10,  9,   9,   9,   10,  7,   7};

    static constexpr int KING_END_GAME[64] = {
        -74, -35, -7, -2, -2, -7, -35, -74, -42, -20, -7, 0,  0,  -7, -20, -42,
        -21, -7,  6,  4,  4,  6,  -7,  -21, -7,  4,   13, 14, 14, 13, 4,   -7,
        0,   4,   14, 20, 20, 14, 4,   0,   0,   7,   14, 20, 20, 14, 7,   0,
        -7,  4,   13, 14, 14, 13, 4,   -7,  -21, -7,  6,  4,  4,  6,  -7,  -21};

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
        return pieceCount <= 10;
    }

    // Evaluate pawn structure using pure bitboard operations
    // Evaluate pawn structure penalties
    static int EvaluatePawnStructure(const Board &board) {
        int score = 0;

        uint64_t whitePawns = board.bitboards[0];
        uint64_t blackPawns = board.bitboards[6];

        // Masks for each file
        const uint64_t FILE_MASKS[8] = {
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL,
            0x0808080808080808ULL, 0x1010101010101010ULL, 0x2020202020202020ULL,
            0x4040404040404040ULL, 0x8080808080808080ULL};

        for (int file = 0; file < 8; file++) {
            uint64_t fileMask = FILE_MASKS[file];

            int whiteCount = __builtin_popcountll(whitePawns & fileMask);
            int blackCount = __builtin_popcountll(blackPawns & fileMask);

            // Doubled pawns penalty
            if (whiteCount > 1)
                score -= 20 * (whiteCount - 1);
            if (blackCount > 1)
                score += 20 * (blackCount - 1);

            // Isolated pawn penalty: check adjacent files
            bool whiteIsolated = true;
            bool blackIsolated = true;

            if (file > 0) {
                whiteIsolated &= !(whitePawns & FILE_MASKS[file - 1]);
                blackIsolated &= !(blackPawns & FILE_MASKS[file - 1]);
            }
            if (file < 7) {
                whiteIsolated &= !(whitePawns & FILE_MASKS[file + 1]);
                blackIsolated &= !(blackPawns & FILE_MASKS[file + 1]);
            }

            if (whiteIsolated && whiteCount > 0)
                score -= 15;
            if (blackIsolated && blackCount > 0)
                score += 15;
        }

        // Passed pawn bonus
        for (int rank = 1; rank <= 6; ++rank) {
            for (int file = 0; file < 8; ++file) {
                int sq = rank * 8 + file;
                uint64_t mask = 1ULL << sq;

                // White passed pawn
                if (whitePawns & mask) {
                    bool blocked = false;
                    for (int r = rank + 1; r <= 7 && !blocked; ++r) {
                        for (int f = std::max(0, file - 1);
                             f <= std::min(7, file + 1); ++f) {
                            if (blackPawns & (1ULL << (r * 8 + f)))
                                blocked = true;
                        }
                    }
                    if (!blocked)
                        score += (7 - rank) * 5 +
                                 10; // more bonus the closer it is to promotion
                }

                // Black passed pawn
                if (blackPawns & mask) {
                    bool blocked = false;
                    for (int r = rank - 1; r >= 0 && !blocked; --r) {
                        for (int f = std::max(0, file - 1);
                             f <= std::min(7, file + 1); ++f) {
                            if (whitePawns & (1ULL << (r * 8 + f)))
                                blocked = true;
                        }
                    }
                    if (!blocked)
                        score -= rank * 5 + 10;
                }
            }
        }

        return score;
    }

    // Evaluate piece safety (hanging pieces, defended pieces)
    static int EvaluatePieceSafety(Board &board) {
        int score = 0;

        // Check all white pieces
        for (int pieceType = 0; pieceType < 6; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1;

                bool isAttacked =
                    board.IsSquareAttacked(square, false); // Attacked by black
                bool isDefended =
                    board.IsSquareAttacked(square, true); // Defended by white

                if (isAttacked && !isDefended) {
                    // Hanging piece - major penalty!
                    score -= PIECE_VALUES[pieceType];
                } else if (isAttacked && isDefended) {
                    // Attacked but defended - small penalty based on piece
                    // value difference
                    score -= PIECE_VALUES[pieceType] / 10;
                } else if (isDefended) {
                    // Well defended piece - small bonus
                    score += 5;
                }
            }
        }

        // Check all black pieces
        for (int pieceType = 6; pieceType < 12; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1;

                bool isAttacked =
                    board.IsSquareAttacked(square, true); // Attacked by white
                bool isDefended =
                    board.IsSquareAttacked(square, false); // Defended by black

                if (isAttacked && !isDefended) {
                    // Hanging piece - major bonus for us!
                    score += PIECE_VALUES[pieceType - 6];
                } else if (isAttacked && isDefended) {
                    // Attacked but defended - small bonus
                    score += PIECE_VALUES[pieceType - 6] / 10;
                } else if (isDefended) {
                    // Well defended enemy piece - small penalty
                    score -= 5;
                }
            }
        }

        return score;
    }

    // Evaluate mobility (number of legal moves)
    static float EvaluateMobility(Board &board) {
        std::vector<Move> whiteMoves;
        board.GenerateAllMoves(whiteMoves, true);

        std::vector<Move> blackMoves;
        board.GenerateAllMoves(blackMoves, false);

        long long whiteCount = static_cast<long long>(whiteMoves.size());
        long long blackCount = static_cast<long long>(blackMoves.size());

        long long diff = whiteCount - blackCount;

        return static_cast<float>(diff);
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

    // EvaluatePosition function to use tables as multipliers
    static int EvaluatePosition(const Board &board) {
        int score = 0;
        bool endgame = IsEndgame(board);

        // White pieces
        for (int pieceType = 0; pieceType < 5; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1; // Clear LSB

                int baseValue = PIECE_VALUES[pieceType];
                float multiplier = 1.0f;

                switch (pieceType) {
                case 0:
                    multiplier = WHITE_PAWN_TABLE[square];
                    break;
                case 1:
                    multiplier = KNIGHT_TABLE[square];
                    break;
                case 2:
                    multiplier = BISHOP_TABLE[square];
                    break;
                case 3:
                    multiplier = ROOK_TABLE[square];
                    break;
                case 4:
                    multiplier = QUEEN_TABLE[square];
                    break;
                case 5:
                    // For king, use a different approach since values can be
                    // negative
                    if (endgame) {
                        multiplier = (KING_END_GAME[square]);
                    } else {
                        multiplier = (KING_MIDDLE_GAME[square]);
                    }
                    multiplier = std::max(0.1f, multiplier);
                    break;
                }
                baseValue = 1;
                score += static_cast<int>(baseValue * multiplier);
            }
        }

        // Black pieces
        for (int pieceType = 6; pieceType < 11; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1; // Clear LSB

                int baseValue = PIECE_VALUES[pieceType - 6];
                float multiplier = 1.0f;

                switch (pieceType - 6) {
                case 0:
                    multiplier = BLACK_PAWN_TABLE[square];
                    break;
                case 1:
                    multiplier = KNIGHT_TABLE[FlipSquare(square)];
                    break;
                case 2:
                    multiplier = BISHOP_TABLE[FlipSquare(square)];
                    break;
                case 3:
                    multiplier = ROOK_TABLE[FlipSquare(square)];
                    break;
                case 4:
                    multiplier = QUEEN_TABLE[FlipSquare(square)];
                    break;
                case 5: {
                    int flippedSquare = FlipSquare(square);
                    if (endgame) {
                        multiplier = (KING_END_GAME[flippedSquare]);
                    } else {
                        multiplier = (KING_MIDDLE_GAME[flippedSquare]);
                    }
                    multiplier = std::max(0.1f, multiplier);
                } break;
                }

                baseValue = 1;
                score -= static_cast<int>(baseValue * multiplier);
            }
        }

        return score;
    }

    // Main evaluation function
    static int Evaluate(Board &board) {

        // Check for game ending conditions
        std::vector<Move> moves = board.GenerateMoves();
        bool isInCheck = board.IsInCheck(board.whiteToMove);

        if (moves.empty()) {
            if (isInCheck) {
                if (board.whiteToMove) {
                    return -CHECKMATE; // Black wins
                } else {
                    return CHECKMATE; // White wins
                }
            } else {
                return STALEMATE;
            }
        }

        int materialScore = EvaluateMaterial(board) * 1;
        int positionScore = EvaluatePosition(board) * 0.1;
        int pawnStructureScore = EvaluatePawnStructure(board) * 1;
        int mobilityScore = EvaluateMobility(board) * 2;
        int safetyScore = EvaluatePieceSafety(board) * 0;

        int totalScore = materialScore + positionScore + pawnStructureScore +
                         mobilityScore + safetyScore;

        if (std::abs(totalScore) > 500) {

            std::cout << "Evaluation: " << totalScore
                      << " (Material: " << materialScore
                      << ", Position: " << positionScore
                      << ", Structure: " << pawnStructureScore
                      << ", Mobility: " << mobilityScore
                      << ", Safety: " << safetyScore << ")" << std::endl;
        }

        // Return score from current player's perspective
        return board.whiteToMove ? totalScore : -totalScore;
    }
};
