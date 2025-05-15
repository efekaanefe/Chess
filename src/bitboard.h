// bitboard.h
#pragma once
#include <cstdint>
#include <string>

// Bitboard is represented as a 64-bit unsigned integer
using Bitboard = uint64_t;

// Constants for board representation
namespace BitboardConstants {
    constexpr Bitboard EMPTY = 0ULL;
    constexpr Bitboard UNIVERSE = ~0ULL;
    constexpr Bitboard FILE_A = 0x0101010101010101ULL;
    constexpr Bitboard FILE_H = 0x8080808080808080ULL;
    constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
    constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;
}

class BitboardOps {
public:
    // Print a bitboard as an 8x8 grid
    static std::string toString(Bitboard b);
    
    // Check if a specific square is set in the bitboard (0-63)
    static bool getBit(Bitboard b, int square);
    
    // Set a specific square in the bitboard
    static Bitboard setBit(Bitboard b, int square);
    
    // Clear a specific square in the bitboard
    static Bitboard clearBit(Bitboard b, int square);
    
    // Get the number of set bits (population count)
    static int popCount(Bitboard b);
    
    // Get the index of the least significant set bit
    static int getLSB(Bitboard b);
    
    // Get and remove the least significant set bit
    static int popLSB(Bitboard& b);
    
    // Shift operations for different directions
    static Bitboard northOne(Bitboard b);
    static Bitboard southOne(Bitboard b);
    static Bitboard eastOne(Bitboard b);
    static Bitboard westOne(Bitboard b);
    static Bitboard northEastOne(Bitboard b);
    static Bitboard northWestOne(Bitboard b);
    static Bitboard southEastOne(Bitboard b);
    static Bitboard southWestOne(Bitboard b);
};

