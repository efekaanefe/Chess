#pragma once

#include "board.h"
#include "evaluate.h"
#include <algorithm>
#include <climits>
#include <vector>

class SearchEngine {
  public:
    static constexpr int MATE_SCORE = 30000;
    static constexpr int MAX_DEPTH = 50;

    struct SearchResult {
        Move bestMove;
        int score;
        int depth;
        int nodesSearched;

        SearchResult() : bestMove(0, 0), score(0), depth(0), nodesSearched(0) {}
        SearchResult(Move move, int s, int d, int nodes)
            : bestMove(move), score(s), depth(d), nodesSearched(nodes) {}
    };

  private:
    int nodesSearched;

    // Check if position is checkmate or stalemate
    bool IsGameOver(Board &board, const std::vector<Move> &moves) {
        return moves.empty();
    }

    // Improved quiescence search
    int Quiescence(Board &board, int alpha, int beta) {
        nodesSearched++;

        int standPat = Evaluator::Evaluate(board);

        // Return from white's perspective
        if (!board.whiteToMove) {
            standPat = -standPat;
        }

        if (standPat >= beta) {
            return beta;
        }
        if (alpha < standPat) {
            alpha = standPat;
        }

        std::vector<Move> moves = board.GenerateMoves();
        std::vector<Move> captures;

        for (const auto &move : moves) {
            if (move.isCapture) {
                captures.push_back(move);
            }
        }

        OrderCaptures(board, captures);

        for (auto &move : captures) {
            board.MakeMove(move);
            int score = -Quiescence(board, -beta, -alpha);
            board.UndoMove(move);

            if (score >= beta) {
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        return alpha;
    }

    void OrderCaptures(Board &board, std::vector<Move> &captures) {
        std::sort(captures.begin(), captures.end(),
                  [&](const Move &a, const Move &b) {
                      int victimA = GetPieceValue(board, a.toSquare);
                      int victimB = GetPieceValue(board, b.toSquare);
                      int attackerA = GetPieceValue(board, a.fromSquare);
                      int attackerB = GetPieceValue(board, b.fromSquare);

                      return (victimA - attackerA) > (victimB - attackerB);
                  });
    }

    int GetPieceValue(Board &board, int square) {
        uint64_t mask = 1ULL << square;
        for (int i = 0; i < 12; i++) {
            if (board.bitboards[i] & mask) {
                return Evaluator::PIECE_VALUES[i % 6];
            }
        }
        return 0;
    }

    void OrderMoves(Board &board, std::vector<Move> &moves) {
        std::sort(moves.begin(), moves.end(),
                  [&](const Move &a, const Move &b) {
                      // Prioritize captures
                      if (a.isCapture && !b.isCapture)
                          return true;
                      if (!a.isCapture && b.isCapture)
                          return false;

                      if (a.isCapture && b.isCapture) {
                          int victimA = GetPieceValue(board, a.toSquare);
                          int victimB = GetPieceValue(board, b.toSquare);
                          return victimA > victimB;
                      }

                      if (a.isPromotion && !b.isPromotion)
                          return true;
                      if (!a.isPromotion && b.isPromotion)
                          return false;

                      return false;
                  });
    }

    // Fixed alpha-beta search with proper perspective handling
    int AlphaBeta(Board &board, int depth, int alpha, int beta, bool maximizing) {
        nodesSearched++;

        if (depth == 0) {
            return Quiescence(board, alpha, beta);
        }

        std::vector<Move> moves = board.GenerateMoves();

        if (moves.empty()) {
            // Check if in check to distinguish mate from stalemate
            if (board.IsInCheck(board.whiteToMove)) {
                // Checkmate - return mate score adjusted by depth to prefer faster mates
                return maximizing ? -MATE_SCORE + (MAX_DEPTH - depth)
                                  : MATE_SCORE - (MAX_DEPTH - depth);
            } else {
                // Stalemate
                return 0;
            }
        }

        OrderMoves(board, moves);

        if (maximizing) {
            int maxEval = INT_MIN;
            for (auto &move : moves) {
                board.MakeMove(move);
                int eval = AlphaBeta(board, depth - 1, alpha, beta, false);
                board.UndoMove(move);

                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);

                if (beta <= alpha) {
                    break; // Beta cutoff
                }
            }
            return maxEval;
        } else {
            int minEval = INT_MAX;
            for (auto &move : moves) {
                board.MakeMove(move);
                int eval = AlphaBeta(board, depth - 1, alpha, beta, true);
                board.UndoMove(move);

                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);

                if (beta <= alpha) {
                    break; // Alpha cutoff
                }
            }
            return minEval;
        }
    }

  public:
    // Fixed iterative deepening search
    SearchResult FindBestMove(Board &board, int maxDepth = 6) {
        Move bestMove = Move(0, 0);
        int bestScore = -MATE_SCORE; // Start with worst possible score
        nodesSearched = 0;

        std::vector<Move> rootMoves = board.GenerateMoves();
        if (rootMoves.empty()) {
            return SearchResult();
        }

        OrderMoves(board, rootMoves);
        bestMove = rootMoves[0];

        for (int depth = 1; depth <= maxDepth; depth++) {
            int currentBestScore = -MATE_SCORE;
            Move currentBestMove = rootMoves[0];
            int alpha = -MATE_SCORE;
            int beta = MATE_SCORE;

            for (auto &move : rootMoves) {
                board.MakeMove(move);
                // Use negamax approach: negate the result and swap alpha/beta
                int score = -AlphaBeta(board, depth - 1, -beta, -alpha, false);
                board.UndoMove(move);

                if (score > currentBestScore) {
                    currentBestScore = score;
                    currentBestMove = move;
                }

                alpha = std::max(alpha, score);

                // Early exit for forced mate
                if (score >= MATE_SCORE - MAX_DEPTH) {
                    break;
                }
            }

            // Always update the best move found at this depth
            bestScore = currentBestScore;
            bestMove = currentBestMove;

            std::cout << "Depth " << depth << ": " << bestMove.ToString()
                      << " (score: " << bestScore << ")" << std::endl;

            // Early termination for forced mates
            if (abs(bestScore) >= MATE_SCORE - MAX_DEPTH) {
                break;
            }
        }

        return SearchResult(bestMove, bestScore, maxDepth, nodesSearched);
    }

    // Fixed single-depth search
    SearchResult Search(Board &board, int depth) {
        nodesSearched = 0;
        std::vector<Move> moves = board.GenerateMoves();

        if (moves.empty()) {
            return SearchResult();
        }

        OrderMoves(board, moves);

        Move bestMove = moves[0];
        int bestScore = -MATE_SCORE;

        for (auto &move : moves) {
            board.MakeMove(move);
            // Use negamax: negate the result since opponent is minimizing
            int score = -AlphaBeta(board, depth - 1, -MATE_SCORE, MATE_SCORE, false);
            board.UndoMove(move);

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }

        return SearchResult(bestMove, bestScore, depth, nodesSearched);
    }
};
