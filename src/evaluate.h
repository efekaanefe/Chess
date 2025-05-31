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

    // Improved pawn table that rewards advancement
    static constexpr int PAWN_TABLE[64] = {
        0,   0,   0,   0,   0,   0,   0,   0,   // 1st rank (impossible for pawns)
        5,   5,   5,   -5,  -5,  5,   5,   5,   // 2nd rank - starting position
        10,  10,  15,  20,  20,  15,  10,  10,  // 3rd rank - slight advancement bonus
        15,  15,  20,  30,  30,  20,  15,  15,  // 4th rank - good central control
        25,  25,  30,  40,  40,  30,  25,  25,  // 5th rank - strong advancement
        35,  35,  40,  50,  50,  40,  35,  35,  // 6th rank - near promotion
        60,  60,  60,  60,  60,  60,  60,  60,  // 7th rank - about to promote!
        100, 100, 100, 100, 100, 100, 100, 100  // 8th rank - promotion (though handled elsewhere)
    };

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

    // Evaluate passed pawns (pawns with no enemy pawns blocking their path)
    static int EvaluatePassedPawns(const Board &board) {
        int score = 0;
        
        uint64_t whitePawns = board.bitboards[0];
        uint64_t blackPawns = board.bitboards[6];
        
        // Check white passed pawns
        uint64_t pawns = whitePawns;
        while (pawns) {
            int square = __builtin_ctzll(pawns);
            pawns &= pawns - 1;
            
            int file = square % 8;
            int rank = square / 8;
            
            // Create mask for enemy pawns that could block this pawn
            uint64_t blockingMask = 0;
            for (int r = rank + 1; r < 8; r++) {
                for (int f = std::max(0, file - 1); f <= std::min(7, file + 1); f++) {
                    blockingMask |= (1ULL << (r * 8 + f));
                }
            }
            
            // If no black pawns can block this pawn, it's passed
            if (!(blackPawns & blockingMask)) {
                int bonus = (rank - 1) * 20; // More bonus for advanced passed pawns
                score += bonus;
            }
        }
        
        // Check black passed pawns
        pawns = blackPawns;
        while (pawns) {
            int square = __builtin_ctzll(pawns);
            pawns &= pawns - 1;
            
            int file = square % 8;
            int rank = square / 8;
            
            // Create mask for enemy pawns that could block this pawn
            uint64_t blockingMask = 0;
            for (int r = 0; r < rank; r++) {
                for (int f = std::max(0, file - 1); f <= std::min(7, file + 1); f++) {
                    blockingMask |= (1ULL << (r * 8 + f));
                }
            }
            
            // If no white pawns can block this pawn, it's passed
            if (!(whitePawns & blockingMask)) {
                int bonus = (6 - rank) * 20; // More bonus for advanced passed pawns
                score -= bonus;
            }
        }
        
        return score;
    }

    // Evaluate pawn structure penalties
    static int EvaluatePawnStructure(const Board &board) {
        int score = 0;
        
        uint64_t whitePawns = board.bitboards[0];
        uint64_t blackPawns = board.bitboards[6];
        
        // Penalize doubled pawns
        for (int file = 0; file < 8; file++) {
            uint64_t fileMask = 0x0101010101010101ULL << file;
            
            int whitePawnsOnFile = __builtin_popcountll(whitePawns & fileMask);
            int blackPawnsOnFile = __builtin_popcountll(blackPawns & fileMask);
            
            if (whitePawnsOnFile > 1) {
                score -= (whitePawnsOnFile - 1) * 15; // Penalty for doubled pawns
            }
            if (blackPawnsOnFile > 1) {
                score += (blackPawnsOnFile - 1) * 15;
            }
        }
        
        return score;
    }

    // Generate attack bitboard for a piece type

    // Evaluate piece safety (hanging pieces, defended pieces)
    static int EvaluatePieceSafety(Board &board) {
        int score = 0;
        
        // Check all white pieces
        for (int pieceType = 0; pieceType < 6; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1;
                
                bool isAttacked = board.IsSquareAttacked(square, false); // Attacked by black
                bool isDefended = board.IsSquareAttacked(square, true);  // Defended by white
                
                if (isAttacked && !isDefended) {
                    // Hanging piece - major penalty!
                    score -= PIECE_VALUES[pieceType];
                } else if (isAttacked && isDefended) {
                    // Attacked but defended - small penalty based on piece value difference
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
                
                bool isAttacked = board.IsSquareAttacked(square, true);  // Attacked by white
                bool isDefended = board.IsSquareAttacked(square, false); // Defended by black
                
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

    // Evaluate king safety
    static int EvaluateKingSafety(Board &board) {
        int score = 0;
        
        // Find kings
        uint64_t whiteKing = board.bitboards[5];
        uint64_t blackKing = board.bitboards[11];
        
        if (whiteKing) {
            int whiteKingSquare = __builtin_ctzll(whiteKing);
            int attacksOnWhiteKing = 0;
            
            // Count attacks around white king
            for (int dr = -1; dr <= 1; dr++) {
                for (int df = -1; df <= 1; df++) {
                    int rank = (whiteKingSquare / 8) + dr;
                    int file = (whiteKingSquare % 8) + df;
                    if (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
                        int square = rank * 8 + file;
                        if (board.IsSquareAttacked(square, false)) {
                            attacksOnWhiteKing++;
                        }
                    }
                }
            }
            score -= attacksOnWhiteKing * 10; // Penalty for king in danger
        }
        
        if (blackKing) {
            int blackKingSquare = __builtin_ctzll(blackKing);
            int attacksOnBlackKing = 0;
            
            // Count attacks around black king
            for (int dr = -1; dr <= 1; dr++) {
                for (int df = -1; df <= 1; df++) {
                    int rank = (blackKingSquare / 8) + dr;
                    int file = (blackKingSquare % 8) + df;
                    if (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
                        int square = rank * 8 + file;
                        if (board.IsSquareAttacked(square, true)) {
                            attacksOnBlackKing++;
                        }
                    }
                }
            }
            score += attacksOnBlackKing * 10; // Bonus for attacking enemy king
        }
        
        return score;
    }

    // Add mobility bonus to discourage repetitive moves
    static int EvaluateMobility(const Board &board) {
        // This is a simplified mobility evaluation
        // In a real engine, you'd count legal moves for each piece type
        int score = 0;
        
        // Bonus for pieces not on their starting squares (encourages development)
        uint64_t whiteKnights = board.bitboards[1];
        uint64_t blackKnights = board.bitboards[7];
        
        // Starting squares for knights
        uint64_t whiteKnightStart = (1ULL << 1) | (1ULL << 6);  // b1, g1
        uint64_t blackKnightStart = (1ULL << 57) | (1ULL << 62); // b8, g8
        
        // Bonus for developed knights
        score += __builtin_popcountll(whiteKnights & ~whiteKnightStart) * 10;
        score -= __builtin_popcountll(blackKnights & ~blackKnightStart) * 10;
        
        return score;
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
    static int Evaluate(Board &board) {
        int materialScore = EvaluateMaterial(board);
        int positionScore = EvaluatePosition(board);
        int passedPawnScore = EvaluatePassedPawns(board);
        int pawnStructureScore = EvaluatePawnStructure(board);
        int mobilityScore = EvaluateMobility(board);
        int safetyScore = EvaluatePieceSafety(board);
        int kingSafetyScore = EvaluateKingSafety(board);

        int totalScore = materialScore + positionScore + passedPawnScore + 
                        pawnStructureScore + mobilityScore + safetyScore + kingSafetyScore;

        /*std::cout << "Evaluation: " << totalScore 
                  << " (Material: " << materialScore 
                  << ", Position: " << positionScore
                  << ", Passed: " << passedPawnScore
                  << ", Structure: " << pawnStructureScore
                  << ", Mobility: " << mobilityScore
                  << ", Safety: " << safetyScore
                  << ", King Safety: " << kingSafetyScore << ")" << std::endl;*/

        // Return score from current player's perspective
        return board.whiteToMove ? totalScore : -totalScore;
    }
};

