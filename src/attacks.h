#pragma once

#include "bitboard.h"
#include <cstdlib>
#include <cstring>

// ============================================================================
// Attack Tables
// ============================================================================
// Precomputed attack bitboards for non-sliding pieces.
// Sliding pieces (bishop, rook, queen) use on-the-fly generation.

namespace Attacks {

// Attack tables: [color][square] for pawns, [square] for others
inline uint64_t pawnAttacks[2][64];
inline uint64_t knightAttacks[64];
inline uint64_t kingAttacks[64];
inline bool initialized = false;

// ============================================================================
// Initialization
// ============================================================================

inline void Init() {
    if ( initialized )
        return;

    memset( pawnAttacks, 0, sizeof( pawnAttacks ) );
    memset( knightAttacks, 0, sizeof( knightAttacks ) );
    memset( kingAttacks, 0, sizeof( kingAttacks ) );

    for ( int sq = 0; sq < 64; ++sq ) {
        int rank = RankOf( sq );
        int file = FileOf( sq );

        // -----------------------------------------------------------------
        // Pawn attacks
        // -----------------------------------------------------------------
        // White pawns attack diagonally upward (+7, +9)
        //     . X .
        //     . P .
        if ( rank < 7 ) {
            if ( file > 0 )
                pawnAttacks[WHITE_SIDE][sq] |= 1ULL << ( sq + 7 );
            if ( file < 7 )
                pawnAttacks[WHITE_SIDE][sq] |= 1ULL << ( sq + 9 );
        }
        // Black pawns attack diagonally downward (-7, -9)
        //     . p .
        //     . X .
        if ( rank > 0 ) {
            if ( file > 0 )
                pawnAttacks[BLACK_SIDE][sq] |= 1ULL << ( sq - 9 );
            if ( file < 7 )
                pawnAttacks[BLACK_SIDE][sq] |= 1ULL << ( sq - 7 );
        }

        // -----------------------------------------------------------------
        // Knight attacks
        // -----------------------------------------------------------------
        // Knights move in an "L" shape: 2+1 or 1+2 squares
        //     . X . X .
        //     X . . . X
        //     . . N . .
        //     X . . . X
        //     . X . X .
        const int knightDirs[] = { -17, -15, -10, -6, 6, 10, 15, 17 };
        for ( int dir : knightDirs ) {
            int target = sq + dir;
            if ( target >= 0 && target < 64 ) {
                int targetFile = FileOf( target );
                if ( abs( file - targetFile ) <= 2 ) {
                    knightAttacks[sq] |= 1ULL << target;
                }
            }
        }

        // -----------------------------------------------------------------
        // King attacks
        // -----------------------------------------------------------------
        // King moves one square in any direction
        //     X X X
        //     X K X
        //     X X X
        const int kingDirs[] = { -9, -8, -7, -1, 1, 7, 8, 9 };
        for ( int dir : kingDirs ) {
            int target = sq + dir;
            if ( target >= 0 && target < 64 ) {
                int targetFile = FileOf( target );
                if ( abs( file - targetFile ) <= 1 ) {
                    kingAttacks[sq] |= 1ULL << target;
                }
            }
        }
    }

    initialized = true;
}

// ============================================================================
// Sliding Piece Attacks (computed on-the-fly with blockers)
// ============================================================================

// Get bishop attacks from a square with given blocker configuration
inline uint64_t GetBishopAttacks( int square, uint64_t blockers ) {
    uint64_t attacks = 0ULL;
    int r = RankOf( square );
    int f = FileOf( square );

    // Four diagonal directions: NE, NW, SE, SW
    int dirs[][2] = { { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };

    for ( auto &d : dirs ) {
        for ( int tr = r + d[0], tf = f + d[1]; tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7; tr += d[0], tf += d[1] ) {
            int sq = MakeSquare( tr, tf );
            attacks |= 1ULL << sq;
            if ( blockers & ( 1ULL << sq ) )
                break;
        }
    }
    return attacks;
}

// Get rook attacks from a square with given blocker configuration
inline uint64_t GetRookAttacks( int square, uint64_t blockers ) {
    uint64_t attacks = 0ULL;
    int r = RankOf( square );
    int f = FileOf( square );

    // Four straight directions: N, S, E, W
    int dirs[][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };

    for ( auto &d : dirs ) {
        for ( int tr = r + d[0], tf = f + d[1]; tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7; tr += d[0], tf += d[1] ) {
            int sq = MakeSquare( tr, tf );
            attacks |= 1ULL << sq;
            if ( blockers & ( 1ULL << sq ) )
                break;
        }
    }
    return attacks;
}

// Get queen attacks (union of bishop and rook attacks)
inline uint64_t GetQueenAttacks( int square, uint64_t blockers ) {
    return GetBishopAttacks( square, blockers ) | GetRookAttacks( square, blockers );
}

} // namespace Attacks
