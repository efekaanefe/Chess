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

    // Evaluate pawn structure using pure bitboard operations
    static int EvaluatePawnStructure(const Board &board) {
        int score = 0;
        
        uint64_t whitePawns = board.bitboards[0];
        uint64_t blackPawns = board.bitboards[6];
        
        // === DOUBLED PAWNS ===
        // Count pawns on each file and subtract total pawns to get doubled count
        const uint64_t FILE_MASKS[8] = {
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL,
            0x0808080808080808ULL, 0x1010101010101010ULL, 0x2020202020202020ULL,
            0x4040404040404040ULL, 0x8080808080808080ULL
        };
        
        int whiteDoubled = 0, blackDoubled = 0;
        for (int file = 0; file < 8; file++) {
            int whiteOnFile = __builtin_popcountll(whitePawns & FILE_MASKS[file]);
            int blackOnFile = __builtin_popcountll(blackPawns & FILE_MASKS[file]);
            if (whiteOnFile > 1) whiteDoubled += whiteOnFile - 1;
            if (blackOnFile > 1) blackDoubled += blackOnFile - 1;
        }
        score -= whiteDoubled * 15 - blackDoubled * 15;
        
        // === ISOLATED PAWNS ===
        // Pawns with no friendly pawns on adjacent files
        uint64_t whiteAdjSupport = ((whitePawns & 0xFEFEFEFEFEFEFEFEULL) >> 1) | 
                                ((whitePawns & 0x7F7F7F7F7F7F7F7FULL) << 1);
        uint64_t blackAdjSupport = ((blackPawns & 0xFEFEFEFEFEFEFEFEULL) >> 1) | 
                                ((blackPawns & 0x7F7F7F7F7F7F7F7FULL) << 1);
        
        uint64_t whiteIsolated = whitePawns & ~whiteAdjSupport;
        uint64_t blackIsolated = blackPawns & ~blackAdjSupport;
        
        score -= __builtin_popcountll(whiteIsolated) * 10;
        score += __builtin_popcountll(blackIsolated) * 10;
        
        // === PROTECTED PAWNS ===
        // Pawns defended by other pawns
        uint64_t whiteProtection = ((whitePawns & 0xFEFEFEFEFEFEFEFEULL) >> 9) | 
                                ((whitePawns & 0x7F7F7F7F7F7F7F7FULL) >> 7);
        uint64_t blackProtection = ((blackPawns & 0xFEFEFEFEFEFEFEFEULL) << 7) | 
                                ((blackPawns & 0x7F7F7F7F7F7F7F7FULL) << 9);
        
        uint64_t whiteProtected = whitePawns & whiteProtection;
        uint64_t blackProtected = blackPawns & blackProtection;
        
        score += __builtin_popcountll(whiteProtected) * 8;
        score -= __builtin_popcountll(blackProtected) * 8;
        
        // === PASSED PAWNS ===
        // White passed pawns: no black pawns in front or diagonally in front
        uint64_t whitePassed = 0;
        uint64_t blackFrontSpan = blackPawns;
        // Fill downward to create "front span" for black pawns
        blackFrontSpan |= blackFrontSpan >> 8;
        blackFrontSpan |= blackFrontSpan >> 16;
        blackFrontSpan |= blackFrontSpan >> 32;
        // Add diagonal coverage
        uint64_t blackControl = blackFrontSpan | 
                            ((blackFrontSpan & 0xFEFEFEFEFEFEFEFEULL) >> 1) |
                            ((blackFrontSpan & 0x7F7F7F7F7F7F7F7FULL) << 1);
        whitePassed = whitePawns & ~blackControl;
        
        // Black passed pawns: no white pawns behind or diagonally behind  
        uint64_t blackPassed = 0;
        uint64_t whiteFrontSpan = whitePawns;
        // Fill upward to create "front span" for white pawns
        whiteFrontSpan |= whiteFrontSpan << 8;
        whiteFrontSpan |= whiteFrontSpan << 16;
        whiteFrontSpan |= whiteFrontSpan << 32;
        // Add diagonal coverage
        uint64_t whiteControl = whiteFrontSpan | 
                            ((whiteFrontSpan & 0xFEFEFEFEFEFEFEFEULL) >> 1) |
                            ((whiteFrontSpan & 0x7F7F7F7F7F7F7F7FULL) << 1);
        blackPassed = blackPawns & ~whiteControl;
        
        score += __builtin_popcountll(whitePassed) * 25;
        score -= __builtin_popcountll(blackPassed) * 25;
        
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
