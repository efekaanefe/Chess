#pragma once

#include <stdint.h>
#include <string>

class Move {
  public:
    int fromSquare; // 0–63
    int toSquare;   // 0–63
    bool isCapture = false;
    bool isEnPassant = false;
    bool isCastling = false;
    bool isPromotion = false;
    bool isRookMove = false;  // New: indicates if this is a rook move
    int promotedPiece = -1;
    
    // Castling-specific fields
    int rookFrom = -1; // Rook's starting square for castling
    int rookTo = -1;   // Rook's destination square for castling

    // Added for undo functionality
    int capturedPieceType = -1;      // Store what piece was captured (if any)
    bool previousWhiteToMove = true; // Store previous turn state
    uint8_t previousCastlingRights;

    Move(int from, int to, bool capture = false, bool enpassant = false,
         bool castling = false, bool promotion = false, int promoPiece = -1,
         bool rookMove = false, int rookFrom = -1, int rookTo = -1)
        : fromSquare(from), toSquare(to), isCapture(capture),
          isEnPassant(enpassant), isCastling(castling), isPromotion(promotion),
          isRookMove(rookMove), promotedPiece(promoPiece), 
          rookFrom(rookFrom), rookTo(rookTo), previousCastlingRights(0) {}

    std::string ToString() const {
        char files[] = "abcdefgh";
        int fromRank = fromSquare / 8 + 1;
        int fromFile = fromSquare % 8;
        int toRank = toSquare / 8 + 1;
        int toFile = toSquare % 8;

        std::string s;
        s += files[fromFile];
        s += std::to_string(fromRank);
        s += files[toFile];
        s += std::to_string(toRank);
        if (isPromotion) {
            s += "=PNBRQK"[promotedPiece % 6]; // crude promotion label
        }
        if (isCastling) {
            s += " (castle)";
        }
        if (isRookMove && !isCastling) {  // Don't mark castling as rook move
            s += " (rook)";
        }
        return s;
    }
};
