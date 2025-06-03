#pragma once

#include "move.h"
#include <bitset>
#include <cstdint>
#include <cstring>
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

    static constexpr int A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6,
                         H1 = 7;
    static constexpr int A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61,
                         G8 = 62, H8 = 63;

    uint8_t castlingRights = WK | WQ | BK | BQ;

    bool whiteToMove = true;
    uint64_t occupancies[3]; // white, black, all

    // Piece letter to index
    std::unordered_map<char, Piece> pieceMap = {
        {'P', WP}, {'N', WN}, {'B', WB}, {'R', WR}, {'Q', WQ}, {'K', WK},
        {'p', BP}, {'n', BN}, {'b', BB}, {'r', BR}, {'q', BQ}, {'k', BK}};

    // Static attack tables
    uint64_t pawnAttacks[2][64];    // [color][square]
    uint64_t knightAttacks[64];     // [square]
    uint64_t kingAttacks[64];       // [square]
    bool tablesInitialized = false; // Initialize to false

    // Initialize attack tables
    void InitializeAttackTables() {
        if (tablesInitialized)
            return;

        // Clear tables
        memset(pawnAttacks, 0, sizeof(pawnAttacks));
        memset(knightAttacks, 0, sizeof(knightAttacks));
        memset(kingAttacks, 0, sizeof(kingAttacks));

        // Precompute pawn attacks
        for (int sq = 0; sq < 64; ++sq) {
            int rank = sq / 8, file = sq % 8;

            // White pawn attacks (attacking upward)
            if (rank < 7) {
                if (file > 0)
                    pawnAttacks[0][sq] |= 1ULL << (sq + 7); // Left diagonal
                if (file < 7)
                    pawnAttacks[0][sq] |= 1ULL << (sq + 9); // Right diagonal
            }

            // Black pawn attacks (attacking downward)
            if (rank > 0) {
                if (file > 0)
                    pawnAttacks[1][sq] |= 1ULL << (sq - 9); // Left diagonal
                if (file < 7)
                    pawnAttacks[1][sq] |= 1ULL << (sq - 7); // Right diagonal
            }
        }

        // Precompute knight and king attacks
        static const int knightDirs[] = {-17, -15, -10, -6, 6, 10, 15, 17};
        static const int kingDirs[] = {-9, -8, -7, -1, 1, 7, 8, 9};

        for (int sq = 0; sq < 64; ++sq) {
            int file = sq % 8;

            // Knight attacks
            for (int dir : knightDirs) {
                int target = sq + dir;
                if (target >= 0 && target < 64) {
                    int targetFile = target % 8;
                    if (abs(file - targetFile) <= 2) { // Prevent wraparound
                        knightAttacks[sq] |= 1ULL << target;
                    }
                }
            }

            // King attacks
            for (int dir : kingDirs) {
                int target = sq + dir;
                if (target >= 0 && target < 64) {
                    int targetFile = target % 8;
                    if (abs(file - targetFile) <= 1) { // Prevent wraparound
                        kingAttacks[sq] |= 1ULL << target;
                    }
                }
            }
        }

        tablesInitialized = true;
    }

    // Constructor
    Board() { InitializeAttackTables(); }

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

    void Reset() {
        // Clear all bitboards
        for (int i = 0; i < 12; ++i) {
            bitboards[i] = 0;
        }

        // Clear occupancies
        occupancies[0] = 0; // white
        occupancies[1] = 0; // black
        occupancies[2] = 0; // all

        // Reset to white's turn
        whiteToMove = true;

        // Load starting position
        LoadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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
        move.previousCastlingRights = castlingRights;
        move.capturedPieceType = NO_PIECE;

        // Update castling rights if king or rook moves (do this BEFORE making
        // the move)
        if (move.fromSquare == E1)
            castlingRights &= ~(WK | WQ);
        if (move.fromSquare == H1)
            castlingRights &= ~WK;
        if (move.fromSquare == A1)
            castlingRights &= ~WQ;
        if (move.fromSquare == E8)
            castlingRights &= ~(BK | BQ);
        if (move.fromSquare == H8)
            castlingRights &= ~BK;
        if (move.fromSquare == A8)
            castlingRights &= ~BQ;

        // Remove captured piece first
        for (int i = 0; i < 12; i++) {
            if (bitboards[i] & toMask) {
                move.capturedPieceType = i;
                bitboards[i] &= ~toMask;
                break;
            }
        }

        // Handle promotion
        if (move.isPromotion && move.promotedPiece != NO_PIECE) {
            int pawnIndex =
                whiteToMove ? WP : BP; // Use current turn before toggling
            // Remove pawn
            bitboards[pawnIndex] &= ~fromMask;
            // Add promoted piece
            bitboards[move.promotedPiece] |= toMask;
        }
        // Handle castling
        else if (move.isCastling) {
            int kingIndex = whiteToMove ? WK : BK;
            int rookIndex = whiteToMove ? WR : BR;

            // Move king
            bitboards[kingIndex] ^= fromMask | toMask;

            // Move rook
            if (whiteToMove) {
                if (move.toSquare == G1) { // Kingside
                    bitboards[rookIndex] ^= (1ULL << H1) | (1ULL << F1);
                    move.rookFrom = H1;
                    move.rookTo = F1;
                } else { // Queenside
                    bitboards[rookIndex] ^= (1ULL << A1) | (1ULL << D1);
                    move.rookFrom = A1;
                    move.rookTo = D1;
                }
            } else {
                if (move.toSquare == G8) { // Kingside
                    bitboards[rookIndex] ^= (1ULL << H8) | (1ULL << F8);
                    move.rookFrom = H8;
                    move.rookTo = F8;
                } else { // Queenside
                    bitboards[rookIndex] ^= (1ULL << A8) | (1ULL << D8);
                    move.rookFrom = A8;
                    move.rookTo = D8;
                }
            }
            move.isRookMove = true;
        }
        // Regular move
        else {
            // Find and move the piece
            for (int i = 0; i < 12; i++) {
                if (bitboards[i] & fromMask) {
                    bitboards[i] ^= fromMask | toMask;
                    break;
                }
            }
        }

        // Toggle turn
        whiteToMove = !whiteToMove;

        // Update occupancy bitboards
        UpdateOccupancies();
    }

    void UndoMove(const Move &move) {
        uint64_t fromMask = 1ULL << move.fromSquare;
        uint64_t toMask = 1ULL << move.toSquare;

        // First toggle turn back to original
        whiteToMove = move.previousWhiteToMove;

        // Handle promotion
        if (move.isPromotion && move.promotedPiece != NO_PIECE) {
            // Remove promoted piece
            bitboards[move.promotedPiece] &= ~toMask;
            // Restore pawn
            int pawnIndex = whiteToMove ? WP : BP; // Use restored turn
            bitboards[pawnIndex] |= fromMask;
        }
        // Handle castling
        else if (move.isCastling) {
            int kingIndex = whiteToMove ? WK : BK;
            int rookIndex = whiteToMove ? WR : BR;

            // Move king back
            bitboards[kingIndex] ^= fromMask | toMask;

            // Move rook back
            if (move.isRookMove) {
                bitboards[rookIndex] ^=
                    (1ULL << move.rookFrom) | (1ULL << move.rookTo);
            }
        }
        // Regular move
        else {
            // Move the piece back
            for (int i = 0; i < 12; i++) {
                if (bitboards[i] & toMask) {
                    bitboards[i] ^= fromMask | toMask;
                    break;
                }
            }
        }

        // Restore captured piece, if any
        if (move.capturedPieceType != NO_PIECE) {
            bitboards[move.capturedPieceType] |= toMask;
        }

        // Restore castling rights
        castlingRights = move.previousCastlingRights;

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

        // Generate all pseudo-legal moves first
        std::vector<Move> pseudoLegalMoves;
        GeneratePawnMoves(pseudoLegalMoves, white);
        GenerateKnightMoves(pseudoLegalMoves, white);
        GenerateBishopMoves(pseudoLegalMoves, white);
        GenerateRookMoves(pseudoLegalMoves, white);
        GenerateQueenMoves(pseudoLegalMoves, white);
        GenerateKingMoves(pseudoLegalMoves, white);

        // Filter out illegal moves (moves that leave king in check)
        for (Move &move : pseudoLegalMoves) {
            if (IsMoveLegal(move, white)) {
                moves.push_back(move);
            }
        }
    }

    // Check if a move is legal (doesn't leave own king in check)
    bool IsMoveLegal(Move &move, bool white) {
        // Make the move temporarily
        MakeMove(move);

        // Check if our king is in check after this move
        bool kingInCheck = IsKingInCheck(white);

        // Undo the move
        UndoMove(move);

        // Move is legal if it doesn't leave our king in check
        return !kingInCheck;
    }

    // Check if the king of the specified color is in check
    bool IsKingInCheck(bool white) {
        int kingIndex = white ? 5 : 11; // WK or BK
        uint64_t kingBitboard = bitboards[kingIndex];

        if (kingBitboard == 0)
            return false; // No king found

        // Find king position
        int kingSquare = __builtin_ctzll(kingBitboard);

        // Check if king square is attacked by opponent
        return IsSquareAttacked(kingSquare, !white);
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
            if (to >= 0 && to < 64 && !(occupancies[2] & (1ULL << to))) {
                if (from / 8 == promotionRank) {
                    // Promotion
                    for (int i = 0; i < 4; ++i) {
                        int queenPiece = white ? WQ : BQ;
                        moves.emplace_back(from, to, false, false, false, true,
                                           queenPiece);
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

                // Check for file wrapping
                int fromFile = from % 8;
                int toFile = capTo % 8;
                if (abs(fromFile - toFile) != 1)
                    continue;

                uint64_t target = 1ULL << capTo;
                bool isEnemy = white ? (occupancies[1] & target)
                                     : (occupancies[0] & target);
                if (isEnemy) {
                    if (from / 8 == promotionRank) {
                        for (int i = 0; i < 4; ++i) {
                            int queenPiece = white ? WQ : BQ;
                            moves.emplace_back(from, capTo, true, false, false,
                                               true, queenPiece);
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

                    // Check if bishop doesn't move off the diagonal (file
                    // wrapping check)
                    int fromFile = from % 8;
                    int toFile = to % 8;
                    int fileDiff = abs(fromFile - toFile);
                    int rankDiff = abs((from / 8) - (to / 8));

                    if (fileDiff != rankDiff || fileDiff > 7)
                        break;

                    uint64_t toMask = 1ULL << to;

                    // Check if target square is occupied by a friendly
                    // piece
                    bool isFriendly = white ? (occupancies[0] & toMask)
                                            : (occupancies[1] & toMask);
                    if (isFriendly)
                        break;

                    // Check if target square is occupied by an enemy piece
                    bool isCapture = white ? (occupancies[1] & toMask)
                                           : (occupancies[0] & toMask);

                    // Add move
                    moves.emplace_back(from, to, isCapture);

                    // Stop if we hit any piece (we can't move through
                    // pieces)
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

                    // Check if target square is occupied by a friendly
                    // piece
                    bool isFriendly = white ? (occupancies[0] & toMask)
                                            : (occupancies[1] & toMask);
                    if (isFriendly)
                        break;

                    // Check if target square is occupied by an enemy piece
                    bool isCapture = white ? (occupancies[1] & toMask)
                                           : (occupancies[0] & toMask);

                    // Add move
                    moves.emplace_back(from, to, isCapture);

                    // Stop if we hit any piece (we can't move through
                    // pieces)
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
                    // and don't wrap
                    if (direction == -9 || direction == -7 || direction == 7 ||
                        direction == 9) {
                        int fileDiff = abs(fromFile - toFile);
                        int rankDiff = abs(fromRank - toRank);
                        if (fileDiff != rankDiff || fileDiff > 7)
                            break;
                    }

                    uint64_t toMask = 1ULL << to;

                    // Check if target square is occupied by a friendly
                    // piece
                    bool isFriendly = white ? (occupancies[0] & toMask)
                                            : (occupancies[1] & toMask);
                    if (isFriendly)
                        break;

                    // Check if target square is occupied by an enemy piece
                    bool isCapture = white ? (occupancies[1] & toMask)
                                           : (occupancies[0] & toMask);

                    // Add move
                    moves.emplace_back(from, to, isCapture);

                    // Stop if we hit any piece (we can't move through
                    // pieces)
                    if (occupancies[2] & toMask)
                        break;
                }
            }

            queens &= queens - 1; // clear LSB
        }
    }

    void GenerateKingMoves(std::vector<Move> &moves, bool white) {
        int kingIndex = white ? WK : BK; // WK or BK
        uint64_t kings = bitboards[kingIndex];

        int kingSquare = -1;
        if (kings)
            kingSquare = __builtin_ctzll(kings);
        else
            return;

        // Directions for normal king moves (unchanged)
        const int directions[8] = {-9, -8, -7, -1, 1, 7, 8, 9};
        for (int direction : directions) {
            int to = kingSquare + direction;

            if (to < 0 || to >= 64)
                continue;

            int fromFile = kingSquare % 8;
            int toFile = to % 8;

            if (abs(fromFile - toFile) > 1)
                continue;

            uint64_t toMask = 1ULL << to;

            bool isFriendly =
                white ? (occupancies[0] & toMask) : (occupancies[1] & toMask);
            if (isFriendly)
                continue;

            if (IsSquareAttacked(to, !white))
                continue;

            bool isCapture =
                white ? (occupancies[1] & toMask) : (occupancies[0] & toMask);
            moves.emplace_back(kingSquare, to, isCapture);
        }

        // Castling moves
        if (white) {
            // White kingside castling (E1 to G1)
            if (castlingRights & WK) {
                uint64_t f1 = 1ULL << 5;
                uint64_t g1 = 1ULL << 6;

                bool emptyBetween = !(occupancies[2] & (f1 | g1));
                bool safeSquares = !IsSquareAttacked(4, false) &&
                                   !IsSquareAttacked(5, false) &&
                                   !IsSquareAttacked(6, false);

                if (emptyBetween && safeSquares) {
                    moves.emplace_back(4, 6, false, false, true); // castling
                }
            }

            // White queenside castling (E1 to C1)
            if (castlingRights & WQ) {
                uint64_t d1 = 1ULL << 3;
                uint64_t c1 = 1ULL << 2;
                uint64_t b1 = 1ULL << 1;

                bool emptyBetween = !(occupancies[2] & (d1 | c1 | b1));
                bool safeSquares = !IsSquareAttacked(4, false) &&
                                   !IsSquareAttacked(3, false) &&
                                   !IsSquareAttacked(2, false);

                if (emptyBetween && safeSquares) {
                    moves.emplace_back(4, 2, false, false, true); // castling
                }
            }
        } else {
            // Black kingside castling (E8 to G8)
            if (castlingRights & BK) {
                uint64_t f8 = 1ULL << 61;
                uint64_t g8 = 1ULL << 62;

                bool emptyBetween = !(occupancies[2] & (f8 | g8));
                bool safeSquares = !IsSquareAttacked(60, true) &&
                                   !IsSquareAttacked(61, true) &&
                                   !IsSquareAttacked(62, true);

                if (emptyBetween && safeSquares) {
                    moves.emplace_back(60, 62, false, false, true); // castling
                }
            }

            // Black queenside castling (E8 to C8)
            if (castlingRights & BQ) {
                uint64_t d8 = 1ULL << 59;
                uint64_t c8 = 1ULL << 58;
                uint64_t b8 = 1ULL << 57;

                bool emptyBetween = !(occupancies[2] & (d8 | c8 | b8));
                bool safeSquares = !IsSquareAttacked(60, true) &&
                                   !IsSquareAttacked(59, true) &&
                                   !IsSquareAttacked(58, true);

                if (emptyBetween && safeSquares) {
                    moves.emplace_back(60, 58, false, false, true); // castling
                }
            }
        }
    }

    // Helper method to check if the game is in checkmate or stalemate
    bool IsCheckmate(bool white) {
        std::vector<Move> legalMoves;
        GenerateAllMoves(legalMoves, white);
        return legalMoves.empty() && IsKingInCheck(white);
    }

    bool IsStalemate(bool white) {
        std::vector<Move> legalMoves;
        GenerateAllMoves(legalMoves, white);
        return legalMoves.empty() && !IsKingInCheck(white);
    }

    bool IsInCheck(bool forWhiteKing) {
        int kingSquare;
        if (forWhiteKing) {
            // Find the white king's square
            uint64_t whiteKingBitboard = bitboards[WK];
            if (whiteKingBitboard == 0) {
                return false;
            }
            kingSquare = __builtin_ctzll(whiteKingBitboard);

            // Check if the white king's square is attacked by black pieces
            return IsSquareAttacked(kingSquare,
                                    false); // 'false' indicates by Black
        } else {
            // Find the black king's square
            uint64_t blackKingBitboard = bitboards[BK];
            if (blackKingBitboard == 0) {
                // Similar to above, should not happen.
                return false;
            }
            kingSquare = __builtin_ctzll(blackKingBitboard);

            // Check if the black king's square is attacked by white pieces
            return IsSquareAttacked(kingSquare,
                                    true); // 'true' indicates by White
        }
    }

    // Fixed square attack detection
    bool IsSquareAttacked(int square, bool byWhite) {
        int base = byWhite ? 0 : 6;

        uint64_t pawns = bitboards[base];
        while (pawns) {
            int pawnSq = __builtin_ctzll(pawns);
            // Use byWhite as the color index (0 for white, 1 for black)
            if (pawnAttacks[byWhite ? 0 : 1][pawnSq] & (1ULL << square))
                return true;
            pawns &= pawns - 1;
        }

        // Check knights
        if (knightAttacks[square] & bitboards[base + 1])
            return true;

        // Check king
        if (kingAttacks[square] & bitboards[base + 5])
            return true;

        // Check bishops and queen (diagonal attacks)
        uint64_t diagonalAttacks = GetBishopAttacks(square, occupancies[2]);
        if (diagonalAttacks & (bitboards[base + 2] | bitboards[base + 4]))
            return true;

        // Check rooks and queen (straight attacks)
        uint64_t straightAttacks = GetRookAttacks(square, occupancies[2]);
        if (straightAttacks & (bitboards[base + 3] | bitboards[base + 4]))
            return true;

        return false;
    }

    uint64_t GetBishopAttacks(int square, uint64_t blockers) {
        uint64_t attacks = 0ULL;
        int r = square / 8, f = square % 8;
        // Four diagonal directions: NE, NW, SE, SW
        int dirs[][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

        for (auto &d : dirs) {
            for (int tr = r + d[0], tf = f + d[1];
                 tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7;
                 tr += d[0], tf += d[1]) {
                int sq = tr * 8 + tf;
                attacks |= 1ULL << sq;
                if (blockers & (1ULL << sq))
                    break;
            }
        }
        return attacks;
    }

    uint64_t GetRookAttacks(int square, uint64_t blockers) {
        uint64_t attacks = 0ULL;
        int r = square / 8, f = square % 8;
        // Four straight directions: N, S, E, W
        int dirs[][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

        for (auto &d : dirs) {
            for (int tr = r + d[0], tf = f + d[1];
                 tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7;
                 tr += d[0], tf += d[1]) {
                int sq = tr * 8 + tf;
                attacks |= 1ULL << sq;
                if (blockers & (1ULL << sq))
                    break;
            }
        }
        return attacks;
    }
};
