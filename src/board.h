#pragma once

#include "attacks.h"
#include "bitboard.h"
#include "move.h"
#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>

// ============================================================================
// Board - Position State Only
// ============================================================================
// This class manages the board state: piece positions, castling rights, turn.
// Move generation and game logic are handled by separate modules.

class Board {
  public:
    // ========================================================================
    // State
    // ========================================================================

    uint64_t bitboards[12] = { 0 };  // 6 piece types × 2 colors
    uint64_t occupancies[3] = { 0 }; // [0]=white, [1]=black, [2]=all
    bool whiteToMove = true;
    uint8_t castlingRights = CASTLE_ALL;

    // Piece character to index mapping
    std::unordered_map<char, Piece> pieceMap = { { 'P', WP }, { 'N', WN }, { 'B', WB }, { 'R', WR },
                                                 { 'Q', WQ }, { 'K', WK }, { 'p', BP }, { 'n', BN },
                                                 { 'b', BB }, { 'r', BR }, { 'q', BQ }, { 'k', BK } };

    // ========================================================================
    // Construction
    // ========================================================================

    Board() { Attacks::Init(); }

    // ========================================================================
    // FEN Loading
    // ========================================================================

    void LoadFEN( const std::string &fen ) {
        size_t index = 0;
        int rank = 7;
        int file = 0;

        while ( index < fen.size() && fen[index] != ' ' ) {
            char c = fen[index];

            if ( c == '/' ) {
                rank--;
                file = 0;
            } else if ( isdigit( c ) ) {
                file += c - '0';
            } else {
                int square = rank * 8 + file;
                if ( pieceMap.count( c ) ) {
                    bitboards[pieceMap[c]] |= ( 1ULL << square );
                }
                file++;
            }
            index++;
        }
        UpdateOccupancies();
    }

    void Reset() {
        for ( int i = 0; i < 12; ++i ) {
            bitboards[i] = 0;
        }
        occupancies[0] = occupancies[1] = occupancies[2] = 0;
        whiteToMove = true;
        castlingRights = CASTLE_ALL;
        LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    }

    // ========================================================================
    // Occupancy Update
    // ========================================================================

    void UpdateOccupancies() {
        occupancies[0] = occupancies[1] = occupancies[2] = 0ULL;
        for ( int i = 0; i < 6; ++i ) {
            occupancies[0] |= bitboards[i]; // White pieces
        }
        for ( int i = 6; i < 12; ++i ) {
            occupancies[1] |= bitboards[i]; // Black pieces
        }
        occupancies[2] = occupancies[0] | occupancies[1];
    }

    // ========================================================================
    // Make Move
    // ========================================================================

    void MakeMove( Move &move ) {
        uint64_t fromMask = 1ULL << move.fromSquare;
        uint64_t toMask = 1ULL << move.toSquare;

        // Store previous state for undo
        move.previousWhiteToMove = whiteToMove;
        move.previousCastlingRights = castlingRights;
        move.capturedPieceType = NO_PIECE;

        // Update castling rights if king or rook moves
        if ( move.fromSquare == E1 )
            castlingRights &= ~( CASTLE_WK | CASTLE_WQ );
        if ( move.fromSquare == H1 )
            castlingRights &= ~CASTLE_WK;
        if ( move.fromSquare == A1 )
            castlingRights &= ~CASTLE_WQ;
        if ( move.fromSquare == E8 )
            castlingRights &= ~( CASTLE_BK | CASTLE_BQ );
        if ( move.fromSquare == H8 )
            castlingRights &= ~CASTLE_BK;
        if ( move.fromSquare == A8 )
            castlingRights &= ~CASTLE_BQ;

        // Remove captured piece
        for ( int i = 0; i < 12; i++ ) {
            if ( bitboards[i] & toMask ) {
                move.capturedPieceType = i;
                bitboards[i] &= ~toMask;
                break;
            }
        }

        // Handle promotion
        if ( move.isPromotion && move.promotedPiece != NO_PIECE ) {
            int pawnIndex = whiteToMove ? WP : BP;
            bitboards[pawnIndex] &= ~fromMask;
            bitboards[move.promotedPiece] |= toMask;
        }
        // Handle castling
        else if ( move.isCastling ) {
            int kingIndex = whiteToMove ? WK : BK;
            int rookIndex = whiteToMove ? WR : BR;

            bitboards[kingIndex] ^= fromMask | toMask;

            if ( whiteToMove ) {
                if ( move.toSquare == G1 ) {
                    bitboards[rookIndex] ^= ( 1ULL << H1 ) | ( 1ULL << F1 );
                    move.rookFrom = H1;
                    move.rookTo = F1;
                } else {
                    bitboards[rookIndex] ^= ( 1ULL << A1 ) | ( 1ULL << D1 );
                    move.rookFrom = A1;
                    move.rookTo = D1;
                }
            } else {
                if ( move.toSquare == G8 ) {
                    bitboards[rookIndex] ^= ( 1ULL << H8 ) | ( 1ULL << F8 );
                    move.rookFrom = H8;
                    move.rookTo = F8;
                } else {
                    bitboards[rookIndex] ^= ( 1ULL << A8 ) | ( 1ULL << D8 );
                    move.rookFrom = A8;
                    move.rookTo = D8;
                }
            }
            move.isRookMove = true;
        }
        // Regular move
        else {
            for ( int i = 0; i < 12; i++ ) {
                if ( bitboards[i] & fromMask ) {
                    bitboards[i] ^= fromMask | toMask;
                    break;
                }
            }
        }

        whiteToMove = !whiteToMove;
        UpdateOccupancies();
    }

    // ========================================================================
    // Undo Move
    // ========================================================================

    void UndoMove( const Move &move ) {
        uint64_t fromMask = 1ULL << move.fromSquare;
        uint64_t toMask = 1ULL << move.toSquare;

        whiteToMove = move.previousWhiteToMove;

        if ( move.isPromotion && move.promotedPiece != NO_PIECE ) {
            bitboards[move.promotedPiece] &= ~toMask;
            int pawnIndex = whiteToMove ? WP : BP;
            bitboards[pawnIndex] |= fromMask;
        } else if ( move.isCastling ) {
            int kingIndex = whiteToMove ? WK : BK;
            int rookIndex = whiteToMove ? WR : BR;

            bitboards[kingIndex] ^= fromMask | toMask;
            if ( move.isRookMove ) {
                bitboards[rookIndex] ^= ( 1ULL << move.rookFrom ) | ( 1ULL << move.rookTo );
            }
        } else {
            for ( int i = 0; i < 12; i++ ) {
                if ( bitboards[i] & toMask ) {
                    bitboards[i] ^= fromMask | toMask;
                    break;
                }
            }
        }

        if ( move.capturedPieceType != NO_PIECE ) {
            bitboards[move.capturedPieceType] |= toMask;
        }

        castlingRights = move.previousCastlingRights;
        UpdateOccupancies();
    }

    // ========================================================================
    // Debug
    // ========================================================================

    void PrintBitboards() const {
        const char *pieceNames[] = { "White Pawns",   "White Knights", "White Bishops", "White Rooks",
                                     "White Queens",  "White King",    "Black Pawns",   "Black Knights",
                                     "Black Bishops", "Black Rooks",   "Black Queens",  "Black King" };

        for ( int i = 0; i < 12; ++i ) {
            std::cout << pieceNames[i] << ":\n";
            std::bitset<64> b( bitboards[i] );
            for ( int rank = 7; rank >= 0; --rank ) {
                for ( int file = 0; file < 8; ++file ) {
                    std::cout << b[rank * 8 + file];
                }
                std::cout << '\n';
            }
            std::cout << '\n';
        }
    }
};

// ============================================================================
// Include implementations that depend on Board
// ============================================================================

#include "game_state.h"
#include "move_generator.h"
