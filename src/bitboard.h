#pragma once

#include <cstdint>

// ============================================================================
// Piece Types and Colors
// ============================================================================

enum Piece {
    WP,
    WN,
    WB,
    WR,
    WQ,
    WK, // White pieces (0-5)
    BP,
    BN,
    BB,
    BR,
    BQ,
    BK, // Black pieces (6-11)
    NO_PIECE = -1
};

enum PieceColor { WHITE_SIDE = 0, BLACK_SIDE = 1 };

// ============================================================================
// Square Constants
// ============================================================================
// Board layout (Little-Endian Rank-File mapping):
//
//   8 | 56 57 58 59 60 61 62 63
//   7 | 48 49 50 51 52 53 54 55
//   6 | 40 41 42 43 44 45 46 47
//   5 | 32 33 34 35 36 37 38 39
//   4 | 24 25 26 27 28 29 30 31
//   3 | 16 17 18 19 20 21 22 23
//   2 |  8  9 10 11 12 13 14 15
//   1 |  0  1  2  3  4  5  6  7
//     +-------------------------
//       a  b  c  d  e  f  g  h

constexpr int A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7;
constexpr int A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15;
constexpr int A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23;
constexpr int A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31;
constexpr int A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39;
constexpr int A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47;
constexpr int A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55;
constexpr int A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63;

// ============================================================================
// File and Rank Masks
// ============================================================================
//
// FILE_A (0x0101010101010101):     RANK_1 (0x00000000000000FF):
//   1 . . . . . . .                  1 1 1 1 1 1 1 1  <- rank 1
//   1 . . . . . . .                  . . . . . . . .
//   1 . . . . . . .                  . . . . . . . .
//   1 . . . . . . .                  . . . . . . . .
//   1 . . . . . . .                  . . . . . . . .
//   1 . . . . . . .                  . . . . . . . .
//   1 . . . . . . .                  . . . . . . . .
//   1 . . . . . . .                  . . . . . . . .
//   ^
//   a-file

constexpr uint64_t FILE_A = 0x0101010101010101ULL;
constexpr uint64_t FILE_B = 0x0202020202020202ULL;
constexpr uint64_t FILE_C = 0x0404040404040404ULL;
constexpr uint64_t FILE_D = 0x0808080808080808ULL;
constexpr uint64_t FILE_E = 0x1010101010101010ULL;
constexpr uint64_t FILE_F = 0x2020202020202020ULL;
constexpr uint64_t FILE_G = 0x4040404040404040ULL;
constexpr uint64_t FILE_H = 0x8080808080808080ULL;

constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

// Combined file masks for knight move generation
constexpr uint64_t NOT_FILE_A = ~FILE_A;
constexpr uint64_t NOT_FILE_H = ~FILE_H;
constexpr uint64_t NOT_FILE_AB = ~( FILE_A | FILE_B );
constexpr uint64_t NOT_FILE_GH = ~( FILE_G | FILE_H );

// ============================================================================
// Castling Rights (bit flags)
// ============================================================================

constexpr uint8_t CASTLE_WK = 1; // White kingside
constexpr uint8_t CASTLE_WQ = 2; // White queenside
constexpr uint8_t CASTLE_BK = 4; // Black kingside
constexpr uint8_t CASTLE_BQ = 8; // Black queenside
constexpr uint8_t CASTLE_ALL = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;

// ============================================================================
// Bit Manipulation Utilities
// ============================================================================

// Get index of least significant bit (0-63)
inline int BitScanForward( uint64_t bb ) { return __builtin_ctzll( bb ); }

// Remove and return the least significant bit index
inline int PopLSB( uint64_t &bb ) {
    int sq = __builtin_ctzll( bb );
    bb &= bb - 1;
    return sq;
}

// Count number of set bits
inline int PopCount( uint64_t bb ) { return __builtin_popcountll( bb ); }

// Get square from rank and file (0-indexed)
inline constexpr int MakeSquare( int rank, int file ) { return rank * 8 + file; }

// Extract rank from square
inline constexpr int RankOf( int sq ) { return sq / 8; }

// Extract file from square
inline constexpr int FileOf( int sq ) { return sq % 8; }
