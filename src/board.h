#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <bitset>
#include <cstdint>

class Board {
public:
    // Bitboards for 6 piece types × 2 colors
    uint64_t bitboards[12] = {0};

    // Enum for indexing
    enum Piece {
        WP, WN, WB, WR, WQ, WK,
        BP, BN, BB, BR, BQ, BK
    };

    // Piece letter to index
    std::unordered_map<char, Piece> pieceMap = {
        {'P', WP}, {'N', WN}, {'B', WB}, {'R', WR}, {'Q', WQ}, {'K', WK},
        {'p', BP}, {'n', BN}, {'b', BB}, {'r', BR}, {'q', BQ}, {'k', BK}
    };

    Board() = default;

    void LoadFEN(const std::string& fen) {
        size_t index = 0;
        int rank = 7;
        int file = 0;

        while (index < fen.size() && fen[index] != ' ') {
            char c = fen[index];

            if (c == '/') {
                rank--;
                file = 0;
            } else if (isdigit(c)) {
                file += c - '0';
            } else {
                int square = rank * 8 + file;
                if (pieceMap.count(c)) {
                    bitboards[pieceMap[c]] |= (1ULL << square);
                }
                file++;
            }
            index++;
        }
    }

    void PrintBitboards() const {
        const char* pieceNames[] = {
            "White Pawns", "White Knights", "White Bishops", "White Rooks", "White Queens", "White King",
            "Black Pawns", "Black Knights", "Black Bishops", "Black Rooks", "Black Queens", "Black King"
        };

        for (int i = 0; i < 12; ++i) {
            std::cout << pieceNames[i] << ":\n";
            std::bitset<64> b(bitboards[i]);
            for (int rank = 7; rank >= 0; --rank) {
                for (int file = 0; file < 8; ++file) {
                    std::cout << b[rank * 8 + file];
                }
                std::cout << '\n';
            }
            std::cout << '\n';
        }
    }
};

