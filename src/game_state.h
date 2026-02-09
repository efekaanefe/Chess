#pragma once

// ============================================================================
// Game State - Header-Only Implementation
// ============================================================================
// Functions for detecting check, checkmate, stalemate, and move legality.
// NOTE: This file must be included AFTER board.h defines the Board class.

#include "move_generator.h"
#include <vector>

// Forward declaration
class Board;

namespace GameState {

// ============================================================================
// Square Attack Detection
// ============================================================================

inline bool IsSquareAttacked( const Board &board, int square, bool byWhite ) {
    return MoveGen::IsSquareAttacked( board, square, byWhite );
}

// ============================================================================
// King In Check
// ============================================================================

inline bool IsKingInCheck( const Board &board, bool white ) {
    int kingIndex = white ? WK : BK;
    uint64_t kingBitboard = board.bitboards[kingIndex];

    if ( kingBitboard == 0 )
        return false;

    int kingSquare = BitScanForward( kingBitboard );
    return IsSquareAttacked( board, kingSquare, !white );
}

// ============================================================================
// Move Legality
// ============================================================================

inline bool IsMoveLegal( Board &board, Move &move, bool white ) {
    board.MakeMove( move );
    bool kingInCheck = IsKingInCheck( board, white );
    board.UndoMove( move );
    return !kingInCheck;
}

// ============================================================================
// Generate All Legal Moves
// ============================================================================

inline void GenerateAllLegalMoves( Board &board, std::vector<Move> &moves, bool white ) {
    moves.clear();

    std::vector<Move> pseudoLegal;
    MoveGen::GenerateAllPseudoLegal( board, pseudoLegal, white );

    for ( Move &move : pseudoLegal ) {
        if ( IsMoveLegal( board, move, white ) ) {
            moves.push_back( move );
        }
    }
}

// ============================================================================
// Checkmate Detection
// ============================================================================

inline bool IsCheckmate( Board &board, bool white ) {
    std::vector<Move> legalMoves;
    GenerateAllLegalMoves( board, legalMoves, white );
    return legalMoves.empty() && IsKingInCheck( board, white );
}

// ============================================================================
// Stalemate Detection
// ============================================================================

inline bool IsStalemate( Board &board, bool white ) {
    std::vector<Move> legalMoves;
    GenerateAllLegalMoves( board, legalMoves, white );
    return legalMoves.empty() && !IsKingInCheck( board, white );
}

} // namespace GameState
