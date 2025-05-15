#pragma once

#include "move.h"
#include <bitset>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class Board {
public:
  // Bitboards for 6 piece types × 2 colors
  uint64_t bitboards[12] = {0};

  // Enum for indexing
  enum Piece { WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK, NO_PIECE = -1 };

  bool whiteToMove = true;
  uint64_t occupancies[3]; // white, black, all

  // Piece letter to index
  std::unordered_map<char, Piece> pieceMap = {
      {'P', WP}, {'N', WN}, {'B', WB}, {'R', WR}, {'Q', WQ}, {'K', WK},
      {'p', BP}, {'n', BN}, {'b', BB}, {'r', BR}, {'q', BQ}, {'k', BK}};

  Board() = default;

  void LoadFEN(const std::string &fen) {
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
    UpdateOccupancies();
  }

  void PrintBitboards() const {
    const char *pieceNames[] = {"White Pawns", "White Knights", "White Bishops",
                                "White Rooks", "White Queens",  "White King",
                                "Black Pawns", "Black Knights", "Black Bishops",
                                "Black Rooks", "Black Queens",  "Black King"};

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

  void MakeMove(const Move &move) {
    uint64_t fromMask = 1ULL << move.fromSquare;
    uint64_t toMask = 1ULL << move.toSquare;

    // Remove captured piece first
    for (int i = 0; i < 12; i++) {
      if (bitboards[i] & toMask) {
        bitboards[i] &= ~toMask;
      }
    }

    // Move the piece
    for (int i = 0; i < 12; i++) {
      if (bitboards[i] & fromMask) {
        bitboards[i] &= ~fromMask;
        bitboards[i] |= toMask;
        break;
      }
    }

    UpdateOccupancies();
  }

  // Possible move generation below
  void UpdateOccupancies() {
    occupancies[0] = occupancies[1] = occupancies[2] = 0ULL;
    for (int i = 0; i < 6; ++i) {
      occupancies[0] |= bitboards[i];
    }
    for (int i = 6; i < 12; ++i) {
      occupancies[1] |= bitboards[i];
    }
    occupancies[2] = occupancies[0] | occupancies[1];
  }

  std::vector<Move> GenerateMoves() {
    std::vector<Move> moves;

    if (whiteToMove)
      GeneratePawnMoves(moves, true);
    else
      GeneratePawnMoves(moves, false);

    return moves;
  }

  void GeneratePawnMoves(std::vector<Move> &moves, bool white) {
    int pawnIndex = white ? 0 : 6;
    uint64_t pawns = bitboards[pawnIndex];

    int direction = white ? 8 : -8;
    int doublePushRank = white ? 1 : 6;
    int promotionRank = white ? 6 : 1;

    while (pawns) {
      int from = __builtin_ctzll(pawns);
      uint64_t fromMask = 1ULL << from;
      int to = from + direction;

      // Single forward push
      if (!(occupancies[2] & (1ULL << to))) {
        if (from / 8 == promotionRank) {
          // Promotion
          for (int i = 0; i < 4; ++i) {
            moves.emplace_back(from, to, false, false, false, true,
                               white ? i : i + 6);
          }
        } else {
          moves.emplace_back(from, to);
        }

        // Double push
        if (from / 8 == doublePushRank &&
            !(occupancies[2] & (1ULL << (from + 2 * direction)))) {
          moves.emplace_back(from, from + 2 * direction);
        }
      }

      // Captures
      int deltas[2] = {7, 9};
      for (int d : deltas) {
        int capTo = white ? from + d : from - d;
        if (capTo < 0 || capTo >= 64)
          continue;
        uint64_t target = 1ULL << capTo;
        bool isEnemy =
            white ? (occupancies[1] & target) : (occupancies[0] & target);
        if (isEnemy) {
          if (from / 8 == promotionRank) {
            for (int i = 0; i < 4; ++i) {
              moves.emplace_back(from, capTo, true, false, false, true,
                                 white ? i : i + 6);
            }
          } else {
            moves.emplace_back(from, capTo, true);
          }
        }
      }

      pawns &= pawns - 1; // clear LSB
    }
  }
};
