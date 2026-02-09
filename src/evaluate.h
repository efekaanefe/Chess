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
    static constexpr int PAWN_TABLE[64] = { 0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
                                            10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
                                            0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
                                            5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0 };

    static constexpr int KNIGHT_TABLE[64] = { -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
                                              0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
                                              15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
                                              -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
                                              5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50 };

    static constexpr int BISHOP_TABLE[64] = { -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
                                              0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
                                              5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
                                              -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
                                              0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20 };

    static constexpr int ROOK_TABLE[64] = { 0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,
                                            -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
                                            -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
                                            -5, 0, 0, 0, 0, 0, 0, -5, 0,  0,  0,  5,  5,  0,  0,  0 };

    static constexpr int QUEEN_TABLE[64] = {
        -20, -10, -10, -5,  -5,  -10, -10, -20, -10, 0,  0, 0,   0,   0,   0,   -10, -10, 0,   5,   5,  5, 5,
        0,   -10, -5,  0,   5,   5,   5,   5,   0,   -5, 0, 0,   5,   5,   5,   5,   0,   -5,  -10, 5,  5, 5,
        5,   5,   0,   -10, -10, 0,   5,   0,   0,   0,  0, -10, -20, -10, -10, -5,  -5,  -10, -10, -20
    };

    static constexpr int KING_MIDDLE_GAME[64] = { -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
                                                  -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
                                                  -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
                                                  -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
                                                  0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20 };

    static constexpr int KING_END_GAME[64] = { -50, -40, -30, -20, -20, -30, -40, -50, -30, -20, -10, 0,   0,
                                               -10, -20, -30, -30, -10, 20,  30,  30,  20,  -10, -30, -30, -10,
                                               30,  40,  40,  30,  -10, -30, -30, -10, 30,  40,  40,  30,  -10,
                                               -30, -30, -10, 20,  30,  30,  20,  -10, -30, -30, -30, 0,   0,
                                               0,   0,   -30, -30, -50, -30, -30, -30, -30, -30, -30, -50 };

    // Precomputed file masks
    static constexpr uint64_t FILE_MASKS[8] = { 0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL,
                                                0x0808080808080808ULL, 0x1010101010101010ULL, 0x2020202020202020ULL,
                                                0x4040404040404040ULL, 0x8080808080808080ULL };

    // Precomputed adjacent file masks for isolated pawn detection
    static constexpr uint64_t ADJACENT_FILE_MASKS[8] = {
        0x0202020202020202ULL, // File A: only B adjacent
        0x0505050505050505ULL, // File B: A and C adjacent
        0x0A0A0A0A0A0A0A0AULL, // File C: B and D adjacent
        0x1414141414141414ULL, // File D: C and E adjacent
        0x2828282828282828ULL, // File E: D and F adjacent
        0x5050505050505050ULL, // File F: E and G adjacent
        0xA0A0A0A0A0A0A0A0ULL, // File G: F and H adjacent
        0x4040404040404040ULL  // File H: only G adjacent
    };

    // Flip square index for black pieces
    static int FlipSquare( int square ) {
        return square ^ 56; // Flips rank
    }

    // Check if position is endgame (few pieces remaining)
    static bool IsEndgame( const Board &board ) {
        int queenCount = __builtin_popcountll( board.bitboards[4] ) + __builtin_popcountll( board.bitboards[10] );

        // If queens are off the board, it's likely an endgame
        if ( queenCount == 0 )
            return true;

        // Count major pieces (queens, rooks) using bitwise OR
        int majorPieceCount = queenCount + __builtin_popcountll( board.bitboards[3] | board.bitboards[9] );

        // If there are 2 or fewer major pieces, it's probably an endgame
        return majorPieceCount <= 2;
    }

    // Optimized pawn structure evaluation using bitwise operations
    static int EvaluatePawnStructure( const Board &board ) {
        int score = 0;
        uint64_t whitePawns = board.bitboards[0];
        uint64_t blackPawns = board.bitboards[6];

        // Evaluate each file using bitwise operations
        for ( int file = 0; file < 8; file++ ) {
            uint64_t fileMask = FILE_MASKS[file];
            uint64_t adjacentMask = ADJACENT_FILE_MASKS[file];

            // Count pawns on this file
            int whiteCount = __builtin_popcountll( whitePawns & fileMask );
            int blackCount = __builtin_popcountll( blackPawns & fileMask );

            // Doubled pawns penalty
            if ( whiteCount > 1 )
                score -= 20 * ( whiteCount - 1 );
            if ( blackCount > 1 )
                score += 20 * ( blackCount - 1 );

            // Isolated pawns penalty (no friendly pawns on adjacent files)
            if ( whiteCount > 0 && !( whitePawns & adjacentMask ) )
                score -= 12;
            if ( blackCount > 0 && !( blackPawns & adjacentMask ) )
                score += 12;
        }

        // Passed pawn evaluation using bitwise shifts
        score += EvaluatePassedPawns( whitePawns, blackPawns, true );
        score -= EvaluatePassedPawns( blackPawns, whitePawns, false );

        return score;
    }

    // Helper function for passed pawn evaluation
    static int EvaluatePassedPawns( uint64_t ourPawns, uint64_t enemyPawns, bool isWhite ) {
        int score = 0;
        uint64_t pawns = ourPawns;

        while ( pawns ) {
            int square = __builtin_ctzll( pawns );
            pawns &= pawns - 1; // Clear the lowest set bit

            int file = square % 8;
            int rank = square / 8;

            // Skip pawns on promotion/back ranks
            if ( ( isWhite && rank >= 6 ) || ( !isWhite && rank <= 1 ) )
                continue;

            // Create mask for squares in front of pawn and adjacent files
            uint64_t frontMask = 0ULL;
            if ( isWhite ) {
                // For white pawns, check squares ahead
                for ( int r = rank + 1; r < 8; r++ ) {
                    for ( int f = std::max( 0, file - 1 ); f <= std::min( 7, file + 1 ); f++ ) {
                        frontMask |= ( 1ULL << ( r * 8 + f ) );
                    }
                }
            } else {
                // For black pawns, check squares behind
                for ( int r = rank - 1; r >= 0; r-- ) {
                    for ( int f = std::max( 0, file - 1 ); f <= std::min( 7, file + 1 ); f++ ) {
                        frontMask |= ( 1ULL << ( r * 8 + f ) );
                    }
                }
            }

            // Check if no enemy pawns block this pawn
            if ( !( enemyPawns & frontMask ) ) {
                int rankFromEnd = isWhite ? ( 7 - rank ) : rank;
                int bonus = rankFromEnd * rankFromEnd * 5; // Quadratic bonus
                score += bonus;
            }
        }
        return score;
    }

    // Optimized piece safety evaluation
    static int EvaluatePieceSafety( Board &board ) {
        int score = 0;

        // Evaluate all white pieces at once
        for ( int pieceType = 0; pieceType < 6; pieceType++ ) {
            score += EvaluatePieceSafetyForType( board, pieceType, true );
        }

        // Evaluate all black pieces at once
        for ( int pieceType = 6; pieceType < 12; pieceType++ ) {
            score -= EvaluatePieceSafetyForType( board, pieceType - 6, false );
        }

        return score;
    }

    // Helper for piece safety evaluation
    static int EvaluatePieceSafetyForType( Board &board, int pieceType, bool isWhite ) {
        int score = 0;
        int boardIndex = isWhite ? pieceType : pieceType + 6;
        uint64_t pieces = board.bitboards[boardIndex];

        while ( pieces ) {
            int square = __builtin_ctzll( pieces );
            pieces &= pieces - 1; // Clear the lowest set bit

            bool isAttacked = MoveGen::IsSquareAttacked( board, square, !isWhite );
            bool isDefended = MoveGen::IsSquareAttacked( board, square, isWhite );

            if ( isAttacked ) {
                if ( !isDefended ) {
                    // Hanging piece - big penalty
                    score -= PIECE_VALUES[pieceType] / 2;
                } else {
                    // Attacked but defended - smaller penalty
                    score -= PIECE_VALUES[pieceType] / 10;
                }
            } else if ( isDefended ) {
                // Defended but not attacked - small bonus
                score += 5;
            }
        }
        return score;
    }

    // Mobility evaluation remains the same (already efficient)
    static int EvaluateMobility( Board &board ) {
        std::vector<Move> whiteMoves;
        MoveGen::GenerateAllPseudoLegal( board, whiteMoves, true );

        std::vector<Move> blackMoves;
        MoveGen::GenerateAllPseudoLegal( board, blackMoves, false );

        // Mobility score is proportional to the square root of move count difference
        int mobilityScore = static_cast<int>( 10 * ( sqrt( whiteMoves.size() ) - sqrt( blackMoves.size() ) ) );

        return mobilityScore;
    }

    // Optimized material evaluation using popcount
    static int EvaluateMaterial( const Board &board, float phase ) {
        int whiteMaterial = 0;
        int blackMaterial = 0;

        // Use array to process all piece types efficiently
        const int pieceValues[] = { 100, 320, 330, 500, 900 }; // Exclude king

        for ( int i = 0; i < 5; i++ ) {
            int whiteCount = __builtin_popcountll( board.bitboards[i] );
            int blackCount = __builtin_popcountll( board.bitboards[i + 6] );

            whiteMaterial += whiteCount * pieceValues[i];
            blackMaterial += blackCount * pieceValues[i];

            // Piece pair bonuses
            if ( i == 1 && whiteCount >= 2 )
                whiteMaterial += 10; // Knight pair
            if ( i == 1 && blackCount >= 2 )
                blackMaterial += 10;
            if ( i == 2 && whiteCount >= 2 )
                whiteMaterial += 30; // Bishop pair
            if ( i == 2 && blackCount >= 2 )
                blackMaterial += 30;
        }

        int score = whiteMaterial - blackMaterial;

        // Tapered evaluation: in endgame, material becomes more important
        return static_cast<int>( score * ( 0.8 + 0.2 * phase ) );
    }

    // Optimized king safety using bitwise operations
    static int EvaluateKingSafety( Board &board, bool endgame ) {
        if ( endgame )
            return 0; // King safety less important in endgame

        int score = 0;

        // White king safety
        int whiteKingSquare = __builtin_ctzll( board.bitboards[5] );
        int whiteKingFile = whiteKingSquare % 8;
        score += EvaluateKingSafetyForColor( board.bitboards[0], whiteKingFile, true );

        // Black king safety
        int blackKingSquare = __builtin_ctzll( board.bitboards[11] );
        int blackKingFile = blackKingSquare % 8;
        score -= EvaluateKingSafetyForColor( board.bitboards[6], blackKingFile, false );

        return score;
    }

    // Helper for king safety evaluation
    static int EvaluateKingSafetyForColor( uint64_t pawns, int kingFile, bool isWhite ) {
        int penalty = 0;

        // Check files around king using bitwise operations
        uint64_t kingAreaFiles = 0ULL;
        for ( int file = std::max( 0, kingFile - 1 ); file <= std::min( 7, kingFile + 1 ); file++ ) {
            kingAreaFiles |= FILE_MASKS[file];
        }

        // Count open files in king area (no pawns)
        for ( int file = std::max( 0, kingFile - 1 ); file <= std::min( 7, kingFile + 1 ); file++ ) {
            if ( !( pawns & FILE_MASKS[file] ) ) {
                penalty += 20;
            }
        }

        return penalty;
    }

    // Optimized piece-square table evaluation
    static int EvaluatePieceSquareTables( const Board &board, bool endgame ) {
        int score = 0;

        // Process white pieces
        score += ProcessPieceSquareForColor( board, 0, 5, endgame, false );

        // Process black pieces (with flipped squares)
        score -= ProcessPieceSquareForColor( board, 6, 11, endgame, true );

        return score;
    }

    // Helper for piece-square table processing
    static int ProcessPieceSquareForColor( const Board &board, int startIdx, int endIdx, bool endgame, bool flip ) {
        int score = 0;
        const int *tables[] = { PAWN_TABLE, KNIGHT_TABLE, BISHOP_TABLE, ROOK_TABLE, QUEEN_TABLE, nullptr };

        for ( int pieceType = startIdx; pieceType <= endIdx; pieceType++ ) {
            uint64_t pieces = board.bitboards[pieceType];
            int tableIdx = pieceType % 6;

            while ( pieces ) {
                int square = __builtin_ctzll( pieces );
                pieces &= pieces - 1; // Clear the lowest set bit

                int evalSquare = flip ? FlipSquare( square ) : square;

                if ( tableIdx == 5 ) { // King
                    score += endgame ? KING_END_GAME[evalSquare] : KING_MIDDLE_GAME[evalSquare];
                } else {
                    score += tables[tableIdx][evalSquare];
                }
            }
        }
        return score;
    }

    // Main evaluation function
    static int Evaluate( Board &board ) {
        // Check for game ending conditions first
        std::vector<Move> moves;
        GameState::GenerateAllLegalMoves( board, moves, board.whiteToMove );
        bool isInCheck = GameState::IsKingInCheck( board, board.whiteToMove );

        if ( moves.empty() ) {
            if ( isInCheck ) {
                // Checkmate
                return board.whiteToMove ? -CHECKMATE : CHECKMATE;
            } else {
                // Stalemate
                return STALEMATE;
            }
        }

        // Check for draw by insufficient material
        if ( IsInsufficientMaterial( board ) ) {
            return DRAW;
        }

        bool endgame = IsEndgame( board );
        float phase = endgame ? 1.0f : 0.0f; // 0.0 = opening, 1.0 = endgame

        // Evaluate all factors
        int materialScore = EvaluateMaterial( board, phase );
        int pieceSquareScore = EvaluatePieceSquareTables( board, endgame );
        int pawnStructureScore = EvaluatePawnStructure( board );
        int mobilityScore = EvaluateMobility( board );
        int safetyScore = EvaluatePieceSafety( board );
        int kingSafetyScore = EvaluateKingSafety( board, endgame );

        // Combine scores with weights
        int totalScore = materialScore + static_cast<int>( pieceSquareScore * 0.3 ) +
                         static_cast<int>( pawnStructureScore * 0.7 ) + static_cast<int>( mobilityScore * 0.3 ) +
                         static_cast<int>( safetyScore * 0.0 );

        std::cout << "Evaluation: " << totalScore << " (Material: " << materialScore
                  << ", Position: " << pieceSquareScore << ", Structure: " << pawnStructureScore
                  << ", Mobility: " << mobilityScore << ", Safety: " << safetyScore << ")" << std::endl;

        // Tapered evaluation: in endgame, material and pawn structure become more important
        totalScore = static_cast<int>( totalScore * ( endgame ? 1.2 : 1.0 ) );

        // Return score from current player's perspective
        // return board.whiteToMove ? totalScore : -totalScore;
        // The search algorithm handles minimax perspective switching
        return totalScore;
    }

  private:
    // Optimized insufficient material check
    static bool IsInsufficientMaterial( const Board &board ) {
        // Use bitwise OR to combine all pieces except kings
        uint64_t whitePieces =
            board.bitboards[0] | board.bitboards[1] | board.bitboards[2] | board.bitboards[3] | board.bitboards[4];
        uint64_t blackPieces =
            board.bitboards[6] | board.bitboards[7] | board.bitboards[8] | board.bitboards[9] | board.bitboards[10];

        int whitePieceCount = __builtin_popcountll( whitePieces );
        int blackPieceCount = __builtin_popcountll( blackPieces );

        // King vs King
        if ( whitePieceCount == 0 && blackPieceCount == 0 )
            return true;

        // King + bishop vs King + bishop with bishops on same color
        if ( whitePieceCount == 1 && blackPieceCount == 1 && board.bitboards[2] && board.bitboards[8] ) {
            int whiteBishopSquare = __builtin_ctzll( board.bitboards[2] );
            int blackBishopSquare = __builtin_ctzll( board.bitboards[8] );
            // Bishops on same color squares
            return ( ( whiteBishopSquare + blackBishopSquare ) % 2 ) == 0;
        }

        return false;
    }
};