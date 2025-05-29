#pragma once

#include <string>

class Move {
public:
    int fromSquare;   // 0–63
    int toSquare;     // 0–63
    bool isCapture = false;
    bool isEnPassant = false;
    bool isCastling = false;
    bool isPromotion = false;
    int promotedPiece = -1;  // NOTE: use enum in board for readibility 0–11 for piece type or -1 if not a promotion
    
    // Added for undo functionality
    int capturedPieceType = -1;  // Store what piece was captured (if any)
    bool previousWhiteToMove = true;  // Store previous turn state

    Move(int from, int to,
         bool capture = false,
         bool enpassant = false,
         bool castling = false,
         bool promotion = false,
         int promoPiece = -1)
        : fromSquare(from),
          toSquare(to),
          isCapture(capture),
          isEnPassant(enpassant),
          isCastling(castling),
          isPromotion(promotion),
          promotedPiece(promoPiece)
    {}

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
            s += "=PNBRQK"[promotedPiece % 6];  // crude promotion label
        }
        return s;
    }
};


