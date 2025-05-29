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
    enum Piece {
        WP,
        WN,
        WB,
        WR,
        WQ,
        WK,
        BP,
        BN,
        BB,
        BR,
        BQ,
        BK,
        NO_PIECE = -1
    };

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
        const char *pieceNames[] = {
            "White Pawns",   "White Knights", "White Bishops", "White Rooks",
            "White Queens",  "White King",    "Black Pawns",   "Black Knights",
            "Black Bishops", "Black Rooks",   "Black Queens",  "Black King"};

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

    void MakeMove(Move &move) {
        uint64_t fromMask = 1ULL << move.fromSquare;
        uint64_t toMask = 1ULL << move.toSquare;

        // Store previous state for undo
        move.previousWhiteToMove = whiteToMove;
        move.capturedPieceType = -1;

        // Remove captured piece first
        for (int i = 0; i < 12; i++) {
            if (bitboards[i] & toMask) {
                move.capturedPieceType = i;
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
        whiteToMove = !whiteToMove;

        UpdateOccupancies();
    }

    // UndoMove using stored move information
    void UndoMove(const Move &move) {
        uint64_t fromMask = 1ULL << move.fromSquare;
        uint64_t toMask = 1ULL << move.toSquare;

        // Move the piece back from toSquare to fromSquare
        for (int i = 0; i < 12; i++) {
            if (bitboards[i] & toMask) {
                bitboards[i] &= ~toMask;  // Remove from destination
                bitboards[i] |= fromMask; // Place back at origin
                break;
            }
        }

        // Restore captured piece if there was one
        if (move.capturedPieceType != -1) {
            bitboards[move.capturedPieceType] |= toMask;
        }

        // Restore previous game state
        whiteToMove = move.previousWhiteToMove;
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
            GenerateAllMoves(moves, true);
        else
            GenerateAllMoves(moves, false);

        return moves;
    }

    // Generate all legal moves
    void GenerateAllMoves(std::vector<Move> &moves, bool white) {
        moves.clear();

        GeneratePawnMoves(moves, white);
        GenerateKnightMoves(moves, white);
        GenerateBishopMoves(moves, white);
        GenerateRookMoves(moves, white);
        GenerateQueenMoves(moves, white);
        GenerateKingMoves(moves, white);
    }

    void GeneratePawnMoves(std::vector<Move> &moves, bool white) {
        int pawnIndex = white ? WP : BP;
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
                bool isEnemy = white ? (occupancies[1] & target)
                                     : (occupancies[0] & target);
                if (isEnemy) {
                    if (from / 8 == promotionRank) {
                        for (int i = 0; i < 4; ++i) {
                            moves.emplace_back(from, capTo, true, false, false,
                                               true, white ? i : i + 6);
                        }
                    } else {
                        moves.emplace_back(from, capTo, true);
                    }
                }
            }

            pawns &= pawns - 1; // clear LSB
        }
    }

    // Knight moves generation
    void GenerateKnightMoves(std::vector<Move> &moves, bool white) {
        int knightIndex = white ? WN : BN;
        uint64_t knights = bitboards[knightIndex];

        // Knight move patterns (8 possible moves)
        const int knightOffsets[8] = {-17, -15, -10, -6, 6, 10, 15, 17};

        while (knights) {
            int from = __builtin_ctzll(knights);
            uint64_t fromMask = 1ULL << from;

            for (int offset : knightOffsets) {
                int to = from + offset;

                // Check if the move is valid (on the board)
                if (to < 0 || to >= 64)
                    continue;

                // Check if knight doesn't move too far horizontally
                int fromFile = from % 8;
                int toFile = to % 8;
                if (abs(fromFile - toFile) > 2)
                    continue;

                uint64_t toMask = 1ULL << to;

                // Check if target square is occupied by a friendly piece
                bool isFriendly = white ? (occupancies[0] & toMask)
                                        : (occupancies[1] & toMask);
                if (isFriendly)
                    continue;

                // Check if target square is occupied by an enemy piece
                bool isCapture = white ? (occupancies[1] & toMask)
                                       : (occupancies[0] & toMask);

                // Add move
                moves.emplace_back(from, to, isCapture);
            }

            knights &= knights - 1; // clear LSB
        }
    }

    // Bishop moves generation
    void GenerateBishopMoves(std::vector<Move> &moves, bool white) {
        int bishopIndex = white ? WB : BB;
        uint64_t bishops = bitboards[bishopIndex];

        // Direction offsets for diagonals
        const int directions[4] = {-9, -7, 7, 9};

        while (bishops) {
            int from = __builtin_ctzll(bishops);

            for (int direction : directions) {
                int to = from;

                while (true) {
                    to += direction;

                    // Check if the move is valid (on the board)
                    if (to < 0 || to >= 64)
                        break;

                    // Check if bishop doesn't move off the diagonal
                    int fromFile = from % 8;
                    int toFile = to % 8;
                    int fromRank = from / 8;
                    int toRank = to / 8;
                    if (abs(fromFile - toFile) != abs(fromRank - toRank))
                        break;

                    uint64_t toMask = 1ULL << to;

                    // Check if target square is occupied by a friendly piece
                    bool isFriendly = white ? (occupancies[0] & toMask)
                                            : (occupancies[1] & toMask);
                    if (isFriendly)
                        break;

                    // Check if target square is occupied by an enemy piece
                    bool isCapture = white ? (occupancies[1] & toMask)
                                           : (occupancies[0] & toMask);

                    // Add move
                    moves.emplace_back(from, to, isCapture);

                    // Stop if we hit any piece (we can't move through pieces)
                    if (occupancies[2] & toMask)
                        break;
                }
            }

            bishops &= bishops - 1; // clear LSB
        }
    }

    // Rook moves generation
    void GenerateRookMoves(std::vector<Move> &moves, bool white) {
        int rookIndex = white ? WR : BR;
        uint64_t rooks = bitboards[rookIndex];

        // Direction offsets for ranks and files
        const int directions[4] = {-8, -1, 1, 8};

        while (rooks) {
            int from = __builtin_ctzll(rooks);

            for (int direction : directions) {
                int to = from;

                while (true) {
                    to += direction;

                    // Check if the move is valid (on the board)
                    if (to < 0 || to >= 64)
                        break;

                    // Check if rook doesn't move diagonally
                    // For horizontal moves, make sure we don't wrap around
                    if (direction == -1 || direction == 1) {
                        int fromRank = from / 8;
                        int toRank = to / 8;
                        if (fromRank != toRank)
                            break;
                    }

                    uint64_t toMask = 1ULL << to;

                    // Check if target square is occupied by a friendly piece
                    bool isFriendly = white ? (occupancies[0] & toMask)
                                            : (occupancies[1] & toMask);
                    if (isFriendly)
                        break;

                    // Check if target square is occupied by an enemy piece
                    bool isCapture = white ? (occupancies[1] & toMask)
                                           : (occupancies[0] & toMask);

                    // Add move
                    moves.emplace_back(from, to, isCapture);

                    // Stop if we hit any piece (we can't move through pieces)
                    if (occupancies[2] & toMask)
                        break;
                }
            }

            rooks &= rooks - 1; // clear LSB
        }
    }

    // Queen moves generation (combination of rook and bishop moves)
    void GenerateQueenMoves(std::vector<Move> &moves, bool white) {
        int queenIndex = white ? WQ : BQ;
        uint64_t queens = bitboards[queenIndex];

        // Direction offsets for all 8 directions
        const int directions[8] = {-9, -8, -7, -1, 1, 7, 8, 9};

        while (queens) {
            int from = __builtin_ctzll(queens);

            for (int direction : directions) {
                int to = from;

                while (true) {
                    to += direction;

                    // Check if the move is valid (on the board)
                    if (to < 0 || to >= 64)
                        break;

                    // Check move validity based on direction type
                    int fromFile = from % 8;
                    int toFile = to % 8;
                    int fromRank = from / 8;
                    int toRank = to / 8;

                    // For horizontal moves, make sure we don't wrap around
                    if (direction == -1 || direction == 1) {
                        if (fromRank != toRank)
                            break;
                    }

                    // For diagonal moves, ensure we stay on the diagonal
                    if (direction == -9 || direction == -7 || direction == 7 ||
                        direction == 9) {
                        if (abs(fromFile - toFile) != abs(fromRank - toRank))
                            break;
                    }

                    uint64_t toMask = 1ULL << to;

                    // Check if target square is occupied by a friendly piece
                    bool isFriendly = white ? (occupancies[0] & toMask)
                                            : (occupancies[1] & toMask);
                    if (isFriendly)
                        break;

                    // Check if target square is occupied by an enemy piece
                    bool isCapture = white ? (occupancies[1] & toMask)
                                           : (occupancies[0] & toMask);

                    // Add move
                    moves.emplace_back(from, to, isCapture);

                    // Stop if we hit any piece (we can't move through pieces)
                    if (occupancies[2] & toMask)
                        break;
                }
            }

            queens &= queens - 1; // clear LSB
        }
    }

    // King moves generation
    void GenerateKingMoves(std::vector<Move> &moves, bool white) {
        int kingIndex = white ? WK : BK;
        uint64_t kings = bitboards[kingIndex];

        // All 8 directions for king movement
        const int directions[8] = {-9, -8, -7, -1, 1, 7, 8, 9};

        while (kings) {
            int from = __builtin_ctzll(kings);

            for (int direction : directions) {
                int to = from + direction;

                // Check if the move is valid (on the board)
                if (to < 0 || to >= 64)
                    continue;

                // Check edge cases for horizontal and diagonal moves
                int fromFile = from % 8;
                int toFile = to % 8;

                // Prevent wraparound on horizontal moves
                if ((direction == -1 || direction == 1) &&
                    (abs(fromFile - toFile) > 1))
                    continue;

                // Prevent wraparound on diagonal moves
                if ((direction == -9 || direction == -7 || direction == 7 ||
                     direction == 9) &&
                    (abs(fromFile - toFile) > 1))
                    continue;

                uint64_t toMask = 1ULL << to;

                // Check if target square is occupied by a friendly piece
                bool isFriendly = white ? (occupancies[0] & toMask)
                                        : (occupancies[1] & toMask);
                if (isFriendly)
                    continue;

                // Check if target square is occupied by an enemy piece
                bool isCapture = white ? (occupancies[1] & toMask)
                                       : (occupancies[0] & toMask);

                // Add regular move
                moves.emplace_back(from, to, isCapture);
            }

            kings &= kings - 1; // clear LSB
        }
    }
};
