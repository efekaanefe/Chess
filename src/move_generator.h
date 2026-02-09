#pragma once

// ============================================================================
// Move Generator - Header-Only Implementation
// ============================================================================
// Stateless move generation functions that operate on a Board reference.
// All functions append pseudo-legal moves to the provided vector.
// NOTE: This file must be included AFTER board.h defines the Board class.

#include "attacks.h"
#include "move.h"
#include <vector>

// Forward declaration
class Board;

namespace MoveGen {

// ============================================================================
// Attack Detection
// ============================================================================

inline bool IsSquareAttacked( const Board &board, int square, bool byWhite ) {
    int base = byWhite ? 0 : 6;

    // Pawn attacks: reverse lookup
    if ( Attacks::pawnAttacks[byWhite ? 1 : 0][square] & board.bitboards[base] )
        return true;

    // Knight attacks
    if ( Attacks::knightAttacks[square] & board.bitboards[base + 1] )
        return true;

    // King attacks
    if ( Attacks::kingAttacks[square] & board.bitboards[base + 5] )
        return true;

    // Bishop/Queen (diagonal attacks)
    uint64_t diagonalAttacks = Attacks::GetBishopAttacks( square, board.occupancies[2] );
    if ( diagonalAttacks & ( board.bitboards[base + 2] | board.bitboards[base + 4] ) )
        return true;

    // Rook/Queen (straight attacks)
    uint64_t straightAttacks = Attacks::GetRookAttacks( square, board.occupancies[2] );
    if ( straightAttacks & ( board.bitboards[base + 3] | board.bitboards[base + 4] ) )
        return true;

    return false;
}

// ============================================================================
// Pawn Moves
// ============================================================================

inline void GeneratePawnMoves( const Board &board, std::vector<Move> &moves, bool white ) {
    uint64_t pawns = board.bitboards[white ? WP : BP];
    uint64_t empty = ~board.occupancies[2];
    uint64_t enemies = board.occupancies[white ? 1 : 0];

    int promotionPieces[4] = { white ? WQ : BQ, white ? WR : BR, white ? WB : BB, white ? WN : BN };

    if ( white ) {
        uint64_t singlePush = ( pawns << 8 ) & empty;
        uint64_t doublePush = ( ( pawns & RANK_2 ) << 16 ) & empty & ( empty << 8 );
        uint64_t captureLeft = ( ( pawns & ~FILE_A ) << 7 ) & enemies;
        uint64_t captureRight = ( ( pawns & ~FILE_H ) << 9 ) & enemies;

        // Single pushes (non-promotion)
        uint64_t nonPromo = singlePush & ~RANK_8;
        while ( nonPromo ) {
            int to = PopLSB( nonPromo );
            moves.emplace_back( to - 8, to );
        }

        // Promotions
        uint64_t promo = singlePush & RANK_8;
        while ( promo ) {
            int to = PopLSB( promo );
            for ( int piece : promotionPieces ) {
                moves.emplace_back( to - 8, to, false, false, false, true, piece );
            }
        }

        // Double pushes
        while ( doublePush ) {
            int to = PopLSB( doublePush );
            moves.emplace_back( to - 16, to );
        }

        // Captures (non-promotion)
        uint64_t capLeftNonPromo = captureLeft & ~RANK_8;
        while ( capLeftNonPromo ) {
            int to = PopLSB( capLeftNonPromo );
            moves.emplace_back( to - 7, to, true );
        }

        uint64_t capRightNonPromo = captureRight & ~RANK_8;
        while ( capRightNonPromo ) {
            int to = PopLSB( capRightNonPromo );
            moves.emplace_back( to - 9, to, true );
        }

        // Capture promotions
        uint64_t capLeftPromo = captureLeft & RANK_8;
        while ( capLeftPromo ) {
            int to = PopLSB( capLeftPromo );
            for ( int piece : promotionPieces ) {
                moves.emplace_back( to - 7, to, true, false, false, true, piece );
            }
        }

        uint64_t capRightPromo = captureRight & RANK_8;
        while ( capRightPromo ) {
            int to = PopLSB( capRightPromo );
            for ( int piece : promotionPieces ) {
                moves.emplace_back( to - 9, to, true, false, false, true, piece );
            }
        }

    } else {
        uint64_t singlePush = ( pawns >> 8 ) & empty;
        uint64_t doublePush = ( ( pawns & RANK_7 ) >> 16 ) & empty & ( empty >> 8 );
        uint64_t captureLeft = ( ( pawns & ~FILE_A ) >> 9 ) & enemies;
        uint64_t captureRight = ( ( pawns & ~FILE_H ) >> 7 ) & enemies;

        uint64_t nonPromo = singlePush & ~RANK_1;
        while ( nonPromo ) {
            int to = PopLSB( nonPromo );
            moves.emplace_back( to + 8, to );
        }

        uint64_t promo = singlePush & RANK_1;
        while ( promo ) {
            int to = PopLSB( promo );
            for ( int piece : promotionPieces ) {
                moves.emplace_back( to + 8, to, false, false, false, true, piece );
            }
        }

        while ( doublePush ) {
            int to = PopLSB( doublePush );
            moves.emplace_back( to + 16, to );
        }

        uint64_t capLeftNonPromo = captureLeft & ~RANK_1;
        while ( capLeftNonPromo ) {
            int to = PopLSB( capLeftNonPromo );
            moves.emplace_back( to + 9, to, true );
        }

        uint64_t capRightNonPromo = captureRight & ~RANK_1;
        while ( capRightNonPromo ) {
            int to = PopLSB( capRightNonPromo );
            moves.emplace_back( to + 7, to, true );
        }

        uint64_t capLeftPromo = captureLeft & RANK_1;
        while ( capLeftPromo ) {
            int to = PopLSB( capLeftPromo );
            for ( int piece : promotionPieces ) {
                moves.emplace_back( to + 9, to, true, false, false, true, piece );
            }
        }

        uint64_t capRightPromo = captureRight & RANK_1;
        while ( capRightPromo ) {
            int to = PopLSB( capRightPromo );
            for ( int piece : promotionPieces ) {
                moves.emplace_back( to + 7, to, true, false, false, true, piece );
            }
        }
    }
}

// ============================================================================
// Knight Moves
// ============================================================================

inline void GenerateKnightMoves( const Board &board, std::vector<Move> &moves, bool white ) {
    uint64_t knights = board.bitboards[white ? WN : BN];
    uint64_t friendly = board.occupancies[white ? 0 : 1];
    uint64_t enemies = board.occupancies[white ? 1 : 0];

    while ( knights ) {
        int from = PopLSB( knights );
        uint64_t targets = Attacks::knightAttacks[from] & ~friendly;

        while ( targets ) {
            int to = PopLSB( targets );
            bool isCapture = enemies & ( 1ULL << to );
            moves.emplace_back( from, to, isCapture );
        }
    }
}

// ============================================================================
// Bishop Moves
// ============================================================================

inline void GenerateBishopMoves( const Board &board, std::vector<Move> &moves, bool white ) {
    uint64_t bishops = board.bitboards[white ? WB : BB];
    uint64_t friendly = board.occupancies[white ? 0 : 1];
    uint64_t enemies = board.occupancies[white ? 1 : 0];

    while ( bishops ) {
        int from = PopLSB( bishops );
        uint64_t attacks = Attacks::GetBishopAttacks( from, board.occupancies[2] );
        uint64_t targets = attacks & ~friendly;

        while ( targets ) {
            int to = PopLSB( targets );
            bool isCapture = enemies & ( 1ULL << to );
            moves.emplace_back( from, to, isCapture );
        }
    }
}

// ============================================================================
// Rook Moves
// ============================================================================

inline void GenerateRookMoves( const Board &board, std::vector<Move> &moves, bool white ) {
    uint64_t rooks = board.bitboards[white ? WR : BR];
    uint64_t friendly = board.occupancies[white ? 0 : 1];
    uint64_t enemies = board.occupancies[white ? 1 : 0];

    while ( rooks ) {
        int from = PopLSB( rooks );
        uint64_t attacks = Attacks::GetRookAttacks( from, board.occupancies[2] );
        uint64_t targets = attacks & ~friendly;

        while ( targets ) {
            int to = PopLSB( targets );
            bool isCapture = enemies & ( 1ULL << to );
            moves.emplace_back( from, to, isCapture );
        }
    }
}

// ============================================================================
// Queen Moves
// ============================================================================

inline void GenerateQueenMoves( const Board &board, std::vector<Move> &moves, bool white ) {
    uint64_t queens = board.bitboards[white ? WQ : BQ];
    uint64_t friendly = board.occupancies[white ? 0 : 1];
    uint64_t enemies = board.occupancies[white ? 1 : 0];

    while ( queens ) {
        int from = PopLSB( queens );
        uint64_t attacks = Attacks::GetQueenAttacks( from, board.occupancies[2] );
        uint64_t targets = attacks & ~friendly;

        while ( targets ) {
            int to = PopLSB( targets );
            bool isCapture = enemies & ( 1ULL << to );
            moves.emplace_back( from, to, isCapture );
        }
    }
}

// ============================================================================
// King Moves
// ============================================================================

inline void GenerateKingMoves( const Board &board, std::vector<Move> &moves, bool white ) {
    uint64_t kings = board.bitboards[white ? WK : BK];
    if ( !kings )
        return;

    int kingSquare = BitScanForward( kings );
    uint64_t friendly = board.occupancies[white ? 0 : 1];
    uint64_t enemies = board.occupancies[white ? 1 : 0];

    uint64_t targets = Attacks::kingAttacks[kingSquare] & ~friendly;

    while ( targets ) {
        int to = PopLSB( targets );
        if ( !IsSquareAttacked( board, to, !white ) ) {
            bool isCapture = enemies & ( 1ULL << to );
            moves.emplace_back( kingSquare, to, isCapture );
        }
    }

    // Castling
    if ( white ) {
        if ( board.castlingRights & CASTLE_WK ) {
            bool emptyBetween = !( board.occupancies[2] & ( ( 1ULL << F1 ) | ( 1ULL << G1 ) ) );
            bool safe = !IsSquareAttacked( board, E1, false ) && !IsSquareAttacked( board, F1, false ) &&
                        !IsSquareAttacked( board, G1, false );
            if ( emptyBetween && safe ) {
                moves.emplace_back( E1, G1, false, false, true );
            }
        }
        if ( board.castlingRights & CASTLE_WQ ) {
            bool emptyBetween = !( board.occupancies[2] & ( ( 1ULL << D1 ) | ( 1ULL << C1 ) | ( 1ULL << B1 ) ) );
            bool safe = !IsSquareAttacked( board, E1, false ) && !IsSquareAttacked( board, D1, false ) &&
                        !IsSquareAttacked( board, C1, false );
            if ( emptyBetween && safe ) {
                moves.emplace_back( E1, C1, false, false, true );
            }
        }
    } else {
        if ( board.castlingRights & CASTLE_BK ) {
            bool emptyBetween = !( board.occupancies[2] & ( ( 1ULL << F8 ) | ( 1ULL << G8 ) ) );
            bool safe = !IsSquareAttacked( board, E8, true ) && !IsSquareAttacked( board, F8, true ) &&
                        !IsSquareAttacked( board, G8, true );
            if ( emptyBetween && safe ) {
                moves.emplace_back( E8, G8, false, false, true );
            }
        }
        if ( board.castlingRights & CASTLE_BQ ) {
            bool emptyBetween = !( board.occupancies[2] & ( ( 1ULL << D8 ) | ( 1ULL << C8 ) | ( 1ULL << B8 ) ) );
            bool safe = !IsSquareAttacked( board, E8, true ) && !IsSquareAttacked( board, D8, true ) &&
                        !IsSquareAttacked( board, C8, true );
            if ( emptyBetween && safe ) {
                moves.emplace_back( E8, C8, false, false, true );
            }
        }
    }
}

// ============================================================================
// All Pseudo-Legal Moves
// ============================================================================

inline void GenerateAllPseudoLegal( const Board &board, std::vector<Move> &moves, bool white ) {
    GeneratePawnMoves( board, moves, white );
    GenerateKnightMoves( board, moves, white );
    GenerateBishopMoves( board, moves, white );
    GenerateRookMoves( board, moves, white );
    GenerateQueenMoves( board, moves, white );
    GenerateKingMoves( board, moves, white );
}

} // namespace MoveGen
