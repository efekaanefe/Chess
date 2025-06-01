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
        10,  // Pawn
        30,  // Knight
        30,  // Bishop
        50,  // Rook
        100, // Queen
        1000 // King
    };

    static constexpr int KNIGHT_TABLE[64] = {
        1, 1, 1, 1, 1, 1, 1, 1, // rank 8 (index 56-63)
        1, 2, 2, 2, 2, 2, 2, 1, // rank 7 (index 48-55)
        1, 2, 3, 3, 3, 3, 2, 1, // rank 6 (index 40-47)
        1, 2, 3, 4, 4, 3, 2, 1, // rank 5 (index 32-39)
        1, 2, 3, 4, 4, 3, 2, 1, // rank 4 (index 24-31)
        1, 2, 3, 3, 3, 3, 2, 1, // rank 3 (index 16-23)
        1, 2, 2, 2, 2, 2, 2, 1, // rank 2 (index 8-15)
        1, 1, 1, 1, 1, 1, 1, 1  // rank 1 (index 0-7)
    };

    static constexpr int BISHOP_TABLE[64] = {
        4, 3, 2, 1, 1, 2, 3, 4, // rank 8
        3, 4, 3, 2, 2, 3, 4, 3, // rank 7
        2, 3, 4, 3, 3, 4, 3, 2, // rank 6
        1, 2, 3, 4, 4, 3, 2, 1, // rank 5
        1, 2, 3, 4, 4, 3, 2, 1, // rank 4
        2, 3, 4, 3, 3, 4, 3, 2, // rank 3
        3, 4, 3, 2, 2, 3, 4, 3, // rank 2
        4, 3, 2, 1, 1, 2, 3, 4  // rank 1
    };

    static constexpr int QUEEN_TABLE[64] = {
        1, 1, 1, 3, 1, 1, 1, 1, // rank 8
        1, 2, 3, 3, 3, 1, 1, 1, // rank 7
        1, 4, 3, 3, 3, 4, 2, 1, // rank 6
        1, 2, 3, 3, 3, 2, 2, 1, // rank 5
        1, 2, 3, 3, 3, 2, 2, 1, // rank 4
        1, 4, 3, 3, 3, 4, 2, 1, // rank 3
        1, 2, 3, 3, 3, 1, 1, 1, // rank 2
        1, 1, 1, 3, 1, 1, 1, 1  // rank 1
    };

    static constexpr int ROOK_TABLE[64] = {
        4, 3, 4, 4, 4, 4, 3, 4, // rank 8
        4, 4, 4, 4, 4, 4, 4, 4, // rank 7
        1, 1, 2, 3, 3, 2, 1, 1, // rank 6
        1, 2, 3, 4, 4, 3, 2, 1, // rank 5
        1, 2, 3, 4, 4, 3, 2, 1, // rank 4
        1, 1, 2, 3, 3, 2, 1, 1, // rank 3
        4, 4, 4, 4, 4, 4, 4, 4, // rank 2
        4, 3, 4, 4, 4, 4, 3, 4  // rank 1
    };

    static constexpr int WHITE_PAWN_TABLE[64] = {
        10, 10, 10, 10, 10, 10, 10, 10, // rank 8
        8,  8,  8,  8,  8,  8,  8,  8,  // rank 7
        5,  6,  6,  7,  7,  6,  6,  5,  // rank 6
        2,  3,  3,  5,  5,  3,  3,  2,  // rank 5
        1,  2,  3,  4,  4,  3,  2,  1,  // rank 4
        1,  1,  2,  3,  3,  2,  1,  1,  // rank 3
        1,  1,  1,  0,  0,  1,  1,  1,  // rank 2
        0,  0,  0,  0,  0,  0,  0,  0   // rank 1
    };

    static constexpr int BLACK_PAWN_TABLE[64] = {
        0,  0,  0,  0,  0,  0,  0,  0, // rank 8
        1,  1,  1,  0,  0,  1,  1,  1, // rank 7
        1,  1,  2,  3,  3,  2,  1,  1, // rank 6
        1,  2,  3,  4,  4,  3,  2,  1, // rank 5
        2,  3,  3,  5,  5,  3,  3,  2, // rank 4
        5,  6,  6,  7,  7,  6,  6,  5, // rank 3
        8,  8,  8,  8,  8,  8,  8,  8, // rank 2
        10, 10, 10, 10, 10, 10, 10, 10 // rank 1
    };

    static constexpr float KING_MIDDLE_GAME[64] = {
        -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0, -3.0, -4.0, -4.0, -5.0,
        -5.0, -4.0, -4.0, -3.0, -3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0,
        -2.0, -3.0, -3.0, -4.0, -4.0, -3.0, -3.0, -2.0, -1.0, -2.0, -2.0, -2.0,
        -2.0, -2.0, -2.0, -1.0, 2.0,  2.0,  0.0,  0.0,  0.0,  0.0,  2.0,  2.0,
        2.0,  3.0,  1.0,  0.0,  0.0,  1.0,  3.0,  2.0};

    static constexpr float KING_END_GAME[64] = {
        -5.0, -4.0, -3.0, -2.0, -2.0, -3.0, -4.0, -5.0, -3.0, -2.0, -1.0,
        0.0,  0.0,  -1.0, -2.0, -3.0, -3.0, -1.0, 2.0,  3.0,  3.0,  2.0,
        -1.0, -3.0, -3.0, -1.0, 3.0,  4.0,  4.0,  3.0,  -1.0, -3.0, -3.0,
        -1.0, 3.0,  4.0,  4.0,  3.0,  -1.0, -3.0, -3.0, -1.0, 2.0,  3.0,
        3.0,  2.0,  -1.0, -3.0, -3.0, -3.0, 0.0,  0.0,  0.0,  0.0,  -3.0,
        -3.0, -5.0, -3.0, -3.0, -3.0, -3.0, -3.0, -3.0, -5.0};

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
            uint64_t whiteFile = whitePawns & fileMask;
            uint64_t blackFile = blackPawns & fileMask;

            int whiteCount = __builtin_popcountll(whiteFile);
            int blackCount = __builtin_popcountll(blackFile);

            // Doubled pawns
            if (whiteCount > 1)
                score -= (whiteCount - 1) * 15;
            if (blackCount > 1)
                score += (blackCount - 1) * 15;

            // Isolated pawns
            bool whiteIsolated = true;
            bool blackIsolated = true;

            if (file > 0) {
                if (whitePawns & FILE_MASKS[file - 1])
                    whiteIsolated = false;
                if (blackPawns & FILE_MASKS[file - 1])
                    blackIsolated = false;
            }
            if (file < 7) {
                if (whitePawns & FILE_MASKS[file + 1])
                    whiteIsolated = false;
                if (blackPawns & FILE_MASKS[file + 1])
                    blackIsolated = false;
            }

            if (whiteIsolated && whiteCount > 0)
                score -= whiteCount * 10;
            if (blackIsolated && blackCount > 0)
                score += blackCount * 10;
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
        for (int pieceType = 0; pieceType < 5; pieceType++) {
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
        for (int pieceType = 6; pieceType < 11; pieceType++) {
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
        for (int pieceType = 0; pieceType < 6; pieceType++) {
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

                score += static_cast<int>(baseValue * multiplier);
            }
        }

        // Black pieces
        for (int pieceType = 6; pieceType < 12; pieceType++) {
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

        int materialScore = EvaluateMaterial(board)*5;
        int positionScore = EvaluatePosition(board)*0.1;
        int pawnStructureScore = EvaluatePawnStructure(board);
        int mobilityScore = EvaluateMobility(board) * 2;
        int safetyScore = EvaluatePieceSafety(board);

        int totalScore = materialScore + positionScore + pawnStructureScore +
                         mobilityScore + safetyScore;

        /*if (std::abs(totalScore) > 500) {*/
        /**/
        /*    std::cout << "Evaluation: " << totalScore*/
        /*              << " (Material: " << materialScore*/
        /*              << ", Position: " << positionScore*/
        /*              << ", Structure: " << pawnStructureScore*/
        /*              << ", Mobility: " << mobilityScore*/
        /*              << ", Safety: " << safetyScore << ")" << std::endl;*/
        /*}*/

        // Return score from current player's perspective
        return board.whiteToMove ? totalScore : -totalScore;
    }
};
