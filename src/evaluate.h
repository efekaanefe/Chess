#pragma once

#include "board.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

class Evaluator {
  public:
    // Constants for game ending conditions
    static constexpr int CHECKMATE = 10000;
    static constexpr int STALEMATE = 0;
    static constexpr int DRAW = 0;

    // Piece values (centipawns)
    static constexpr int PIECE_VALUES[6] = {
        100, // Pawn
        320, // Knight (slightly higher than bishop)
        330, // Bishop (slightly higher than knight)
        500, // Rook
        900, // Queen
        0    // King (not actually used in material eval)
    };

    // Piece-square tables (middlegame)
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
        int majorPieceCount = 0;
        int queenCount = __builtin_popcountll(board.bitboards[4]) +
                         __builtin_popcountll(board.bitboards[10]);

        // If queens are off the board, it's likely an endgame
        if (queenCount == 0)
            return true;

        // Count major pieces (queens, rooks)
        majorPieceCount += queenCount;
        majorPieceCount += __builtin_popcountll(board.bitboards[3]) +
                           __builtin_popcountll(board.bitboards[9]);

        // If there are 2 or fewer major pieces, it's probably an endgame
        return majorPieceCount <= 2;
    }

    // Evaluate pawn structure
    static int EvaluatePawnStructure(const Board &board) {
        int score = 0;
        uint64_t whitePawns = board.bitboards[0];
        uint64_t blackPawns = board.bitboards[6];

        // Masks for each file
        const uint64_t FILE_MASKS[8] = {
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL,
            0x0808080808080808ULL, 0x1010101010101010ULL, 0x2020202020202020ULL,
            0x4040404040404040ULL, 0x8080808080808080ULL};

        // Doubled, isolated, and passed pawn evaluation
        for (int file = 0; file < 8; file++) {
            uint64_t fileMask = FILE_MASKS[file];

            // White pawns
            int whiteCount = __builtin_popcountll(whitePawns & fileMask);
            if (whiteCount > 1)
                score -= 20 * (whiteCount - 1); // Doubled pawns

            // Black pawns
            int blackCount = __builtin_popcountll(blackPawns & fileMask);
            if (blackCount > 1)
                score += 20 * (blackCount - 1); // Doubled pawns

            // Isolated pawns
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
                score -= 12;
            if (blackIsolated && blackCount > 0)
                score += 12;
        }

        // Passed pawn evaluation
        for (int rank = 1; rank <= 6; ++rank) {
            for (int file = 0; file < 8; ++file) {
                int sq = rank * 8 + file;
                uint64_t mask = 1ULL << sq;

                // White passed pawn
                if (whitePawns & mask) {
                    bool isPassed = true;
                    int stopSquare = sq;
                    while (stopSquare < 56) {
                        stopSquare += 8;
                        // Check squares in front and adjacent files
                        for (int f = std::max(0, file - 1);
                             f <= std::min(7, file + 1); ++f) {
                            if (blackPawns &
                                (1ULL << (stopSquare - 8 * rank + f))) {
                                isPassed = false;
                                break;
                            }
                        }
                        if (!isPassed)
                            break;
                    }
                    if (isPassed) {
                        int bonus =
                            (7 - rank) * (7 - rank) * 5; // Quadratic bonus
                        score += bonus;
                    }
                }

                // Black passed pawn
                if (blackPawns & mask) {
                    bool isPassed = true;
                    int stopSquare = sq;
                    while (stopSquare >= 8) {
                        stopSquare -= 8;
                        // Check squares in front and adjacent files
                        for (int f = std::max(0, file - 1);
                             f <= std::min(7, file + 1); ++f) {
                            if (whitePawns &
                                (1ULL << (stopSquare + 8 * (7 - rank) + f))) {
                                isPassed = false;
                                break;
                            }
                        }
                        if (!isPassed)
                            break;
                    }
                    if (isPassed) {
                        int bonus = rank * rank * 5; // Quadratic bonus
                        score -= bonus;
                    }
                }
            }
        }

        return score;
    }

    // Evaluate piece safety
    static int EvaluatePieceSafety(Board &board) {
        int score = 0;

        // Evaluate white pieces
        for (int pieceType = 0; pieceType < 6; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1;

                bool isAttacked = board.IsSquareAttacked(square, false);
                bool isDefended = board.IsSquareAttacked(square, true);

                if (isAttacked) {
                    if (!isDefended) {
                        // Hanging piece - big penalty
                        score -= PIECE_VALUES[pieceType] / 2;
                    } else {
                        // Attacked but defended - smaller penalty
                        score -= PIECE_VALUES[pieceType] / 10;
                    }
                } else if (isDefended) {
                    // Defended but not attacked - small bonus
                    score += 5;
                }
            }
        }

        // Evaluate black pieces
        for (int pieceType = 6; pieceType < 12; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1;

                bool isAttacked = board.IsSquareAttacked(square, true);
                bool isDefended = board.IsSquareAttacked(square, false);

                if (isAttacked) {
                    if (!isDefended) {
                        // Hanging piece - big bonus for us
                        score += PIECE_VALUES[pieceType - 6] / 2;
                    } else {
                        // Attacked but defended - smaller bonus
                        score += PIECE_VALUES[pieceType - 6] / 10;
                    }
                } else if (isDefended) {
                    // Defended but not attacked - small penalty
                    score -= 5;
                }
            }
        }

        return score;
    }

    // Evaluate mobility
    static int EvaluateMobility(Board &board) {
        std::vector<Move> whiteMoves;
        board.GenerateAllMoves(whiteMoves, true);

        std::vector<Move> blackMoves;
        board.GenerateAllMoves(blackMoves, false);

        // Mobility score is proportional to the square root of move count
        // difference to avoid overvaluing mobility in positions with many
        // meaningless moves
        int mobilityScore = static_cast<int>(
            10 * (sqrt(whiteMoves.size()) - sqrt(blackMoves.size())));

        return mobilityScore;
    }

    // Evaluate material balance with tapered evaluation
    static int EvaluateMaterial(const Board &board, float phase) {
        int score = 0;

        // Piece values with small bonuses for piece pairs
        int whiteMaterial = 0;
        int blackMaterial = 0;

        // Pawns
        int whitePawns = __builtin_popcountll(board.bitboards[0]);
        int blackPawns = __builtin_popcountll(board.bitboards[6]);
        whiteMaterial += whitePawns * PIECE_VALUES[0];
        blackMaterial += blackPawns * PIECE_VALUES[0];

        // Knights
        int whiteKnights = __builtin_popcountll(board.bitboards[1]);
        int blackKnights = __builtin_popcountll(board.bitboards[7]);
        whiteMaterial += whiteKnights * PIECE_VALUES[1];
        blackMaterial += blackKnights * PIECE_VALUES[1];
        // Knight pair bonus
        if (whiteKnights >= 2)
            whiteMaterial += 10;
        if (blackKnights >= 2)
            blackMaterial += 10;

        // Bishops
        int whiteBishops = __builtin_popcountll(board.bitboards[2]);
        int blackBishops = __builtin_popcountll(board.bitboards[8]);
        whiteMaterial += whiteBishops * PIECE_VALUES[2];
        blackMaterial += blackBishops * PIECE_VALUES[2];
        // Bishop pair bonus
        if (whiteBishops >= 2)
            whiteMaterial += 30;
        if (blackBishops >= 2)
            blackMaterial += 30;

        // Rooks
        int whiteRooks = __builtin_popcountll(board.bitboards[3]);
        int blackRooks = __builtin_popcountll(board.bitboards[9]);
        whiteMaterial += whiteRooks * PIECE_VALUES[3];
        blackMaterial += blackRooks * PIECE_VALUES[3];

        // Queens
        int whiteQueens = __builtin_popcountll(board.bitboards[4]);
        int blackQueens = __builtin_popcountll(board.bitboards[10]);
        whiteMaterial += whiteQueens * PIECE_VALUES[4];
        blackMaterial += blackQueens * PIECE_VALUES[4];

        score = whiteMaterial - blackMaterial;

        // Tapered evaluation: in endgame, material becomes more important
        return static_cast<int>(score * (0.8 + 0.2 * phase));
    }

    // Evaluate king safety
    static int EvaluateKingSafety(Board &board, bool endgame) {
        int score = 0;

        if (!endgame) {
            // Evaluate white king safety
            int whiteKingSquare = __builtin_ctzll(board.bitboards[5]);
            int whiteKingFile = whiteKingSquare % 8;
            int whiteKingRank = whiteKingSquare / 8;

            // Penalty for open files near king
            uint64_t whitePawns = board.bitboards[0];
            for (int file = std::max(0, whiteKingFile - 1);
                 file <= std::min(7, whiteKingFile + 1); file++) {
                uint64_t fileMask = 0x0101010101010101ULL << file;
                if (!(whitePawns & fileMask)) {
                    score -= 20;
                }
            }

            // Evaluate black king safety
            int blackKingSquare = __builtin_ctzll(board.bitboards[11]);
            int blackKingFile = blackKingSquare % 8;
            int blackKingRank = blackKingSquare / 8;

            // Penalty for open files near king
            uint64_t blackPawns = board.bitboards[6];
            for (int file = std::max(0, blackKingFile - 1);
                 file <= std::min(7, blackKingFile + 1); file++) {
                uint64_t fileMask = 0x0101010101010101ULL << file;
                if (!(blackPawns & fileMask)) {
                    score += 20;
                }
            }
        }

        return score;
    }

    // Evaluate piece-square tables
    static int EvaluatePieceSquareTables(const Board &board, bool endgame) {
        int score = 0;

        // White pieces
        for (int pieceType = 0; pieceType < 6; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1;

                switch (pieceType) {
                case 0: // Pawn
                    score += PAWN_TABLE[square];
                    break;
                case 1: // Knight
                    score += KNIGHT_TABLE[square];
                    break;
                case 2: // Bishop
                    score += BISHOP_TABLE[square];
                    break;
                case 3: // Rook
                    score += ROOK_TABLE[square];
                    break;
                case 4: // Queen
                    score += QUEEN_TABLE[square];
                    break;
                case 5: // King
                    if (endgame) {
                        score += KING_END_GAME[square];
                    } else {
                        score += KING_MIDDLE_GAME[square];
                    }
                    break;
                }
            }
        }

        // Black pieces (flipped)
        for (int pieceType = 6; pieceType < 12; pieceType++) {
            uint64_t pieces = board.bitboards[pieceType];
            while (pieces) {
                int square = __builtin_ctzll(pieces);
                pieces &= pieces - 1;
                int flippedSquare = FlipSquare(square);

                switch (pieceType - 6) {
                case 0: // Pawn
                    score -= PAWN_TABLE[flippedSquare];
                    break;
                case 1: // Knight
                    score -= KNIGHT_TABLE[flippedSquare];
                    break;
                case 2: // Bishop
                    score -= BISHOP_TABLE[flippedSquare];
                    break;
                case 3: // Rook
                    score -= ROOK_TABLE[flippedSquare];
                    break;
                case 4: // Queen
                    score -= QUEEN_TABLE[flippedSquare];
                    break;
                case 5: // King
                    if (endgame) {
                        score -= KING_END_GAME[flippedSquare];
                    } else {
                        score -= KING_MIDDLE_GAME[flippedSquare];
                    }
                    break;
                }
            }
        }

        return score;
    }

    // Main evaluation function
    static int Evaluate(Board &board) {
        // Check for game ending conditions first
        std::vector<Move> moves = board.GenerateMoves();
        bool isInCheck = board.IsInCheck(board.whiteToMove);

        if (moves.empty()) {
            if (isInCheck) {
                // Checkmate
                return board.whiteToMove ? -CHECKMATE : CHECKMATE;
            } else {
                // Stalemate
                return STALEMATE;
            }
        }

        // Check for draw by insufficient material
        if (IsInsufficientMaterial(board)) {
            return DRAW;
        }

        bool endgame = IsEndgame(board);
        float phase = endgame ? 1.0f : 0.0f; // 0.0 = opening, 1.0 = endgame

        // Evaluate all factors
        int materialScore = EvaluateMaterial(board, phase);
        int pieceSquareScore = EvaluatePieceSquareTables(board, endgame);
        int pawnStructureScore = EvaluatePawnStructure(board);
        int mobilityScore = EvaluateMobility(board);
        int safetyScore = EvaluatePieceSafety(board);
        int kingSafetyScore = EvaluateKingSafety(board, endgame);

        // Combine scores with weights
        int totalScore = materialScore + pieceSquareScore * 1 +
                         pawnStructureScore * 0.5 + mobilityScore * 0.5 +
                         safetyScore * 0.1 + kingSafetyScore * 0;


        std::cout << "Evaluation: " << totalScore
                  << " (Material: " << materialScore
                  << ", Position: " << pieceSquareScore
                  << ", Structure: " << pawnStructureScore
                  << ", Mobility: " << mobilityScore
                  << ", Safety: " << safetyScore << ")" << std::endl;


        // Tapered evaluation: in endgame, material and pawn structure become
        // more important
        totalScore = static_cast<int>(totalScore * (endgame ? 1.2 : 1.0));

        // Return score from current player's perspective
        return board.whiteToMove ? totalScore : -totalScore;
    }

  private:
    // Check for insufficient material draw
    static bool IsInsufficientMaterial(const Board &board) {
        int whitePieces = __builtin_popcountll(
            board.bitboards[0] | board.bitboards[1] | board.bitboards[2] |
            board.bitboards[3] | board.bitboards[4]);
        int blackPieces = __builtin_popcountll(
            board.bitboards[6] | board.bitboards[7] | board.bitboards[8] |
            board.bitboards[9] | board.bitboards[10]);

        // King vs King
        if (whitePieces == 0 && blackPieces == 0)
            return true;

        // King + bishop vs King + bishop with bishops on same color
        if (whitePieces == 1 && blackPieces == 1) {
            bool whiteBishop = board.bitboards[2] != 0;
            bool blackBishop = board.bitboards[8] != 0;
            if (whiteBishop && blackBishop) {
                int whiteBishopSquare = __builtin_ctzll(board.bitboards[2]);
                int blackBishopSquare = __builtin_ctzll(board.bitboards[8]);
                // Bishops on same color
                if (((whiteBishopSquare + blackBishopSquare) % 2) == 0) {
                    return true;
                }
            }
        }

        return false;
    }
};
