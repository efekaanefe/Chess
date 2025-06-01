#pragma once

#include "board.h"
#include "evaluate.h"
#include <algorithm>
#include <climits>
#include <iomanip>
#include <iostream>
#include <vector>

class SearchEngine {
  public:
    static constexpr int MAX_DEPTH = 50;
    static const int MAX_EVAL = 100000; // Large value for alpha-beta bounds

    SearchEngine() : nodesSearched(0), nextMove(0, 0) {}

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
    Move nextMove;

    // Negamax Alpha-Beta implementation matching Python structure
    int FindMoveNegaMaxAlphaBeta(Board &gs, std::vector<Move> &validMoves,
                                 int depth, int alpha, int beta,
                                 int turnMultiplier, int maxDepth) {
        nodesSearched++;

        if (depth == 0) {
            return Quiescence(gs, alpha, beta, turnMultiplier);
        }

        int maxScore = -MAX_EVAL;
        Move localBestMove = validMoves.empty() ? Move(0, 0) : validMoves[0];

        for (auto &move : validMoves) {
            gs.MakeMove(move);
            std::vector<Move> nextMoves = gs.GenerateMoves();

            // Negamax recursive call - negate result and swap alpha/beta
            int score = -FindMoveNegaMaxAlphaBeta(
                gs, nextMoves, depth - 1, -beta, -alpha, -turnMultiplier, maxDepth);

            if (score > maxScore) {
                maxScore = score;
                localBestMove = move;

                // Store best move at root level (check if this is the initial
                // depth)
                if (depth == maxDepth) {
                    nextMove = move;
                    std::cout << "Best move updated: " << move.ToString()
                              << " Score: " << std::fixed
                              << std::setprecision(5) << score << std::endl;
                }
            }

            gs.UndoMove(move);

            // Alpha-beta pruning logic
            if (maxScore > alpha) {
                alpha = maxScore;
            }

            if (alpha >= beta) {
                break; // Beta cutoff
            }
        }

        return maxScore;
    }

    // Move ordering functions
    void OrderMoves(Board &board, std::vector<Move> &moves) {
        std::sort(moves.begin(), moves.end(),
                  [&](const Move &a, const Move &b) {
                      // Prioritize captures (MVV-LVA: Most Valuable Victim -
                      // Least Valuable Attacker)
                      if (a.isCapture && !b.isCapture)
                          return true;
                      if (!a.isCapture && b.isCapture)
                          return false;

                      if (a.isCapture && b.isCapture) {
                          int victimA = GetPieceValue(board, a.toSquare);
                          int victimB = GetPieceValue(board, b.toSquare);
                          int attackerA = GetPieceValue(board, a.fromSquare);
                          int attackerB = GetPieceValue(board, b.fromSquare);

                          return (victimA - attackerA) > (victimB - attackerB);
                      }

                      // Prioritize promotions
                      if (a.isPromotion && !b.isPromotion)
                          return true;
                      if (!a.isPromotion && b.isPromotion)
                          return false;

                      return false;
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

    // Quiescence search for tactical positions
    int Quiescence(Board &board, int alpha, int beta, int turnMultiplier) {
        nodesSearched++;

        int standPat = turnMultiplier * Evaluator::Evaluate(board);

        if (standPat >= beta) {
            return beta;
        }
        if (alpha < standPat) {
            alpha = standPat;
        }

        std::vector<Move> moves = board.GenerateMoves();
        std::vector<Move> captures;

        // Only consider captures in quiescence
        for (const auto &move : moves) {
            if (move.isCapture) {
                captures.push_back(move);
            }
        }

        OrderMoves(board, captures);

        for (auto &move : captures) {
            board.MakeMove(move);
            int score = -Quiescence(board, -beta, -alpha, -turnMultiplier);
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

  public:
    SearchResult FindBestMove(Board &board, int maxDepth = 6) {
        nodesSearched = 0;

        std::vector<Move> rootMoves = board.GenerateMoves();
        if (rootMoves.empty()) {
            return SearchResult();
        }

        OrderMoves(board, rootMoves);

        // Search at root level directly
        Move bestMove = rootMoves[0];
        int bestScore = -MAX_EVAL;
        int turnMultiplier = board.whiteToMove ? 1 : -1;

        for (auto &move : rootMoves) {
            board.MakeMove(move);
            std::vector<Move> nextMoves = board.GenerateMoves();

            // Search remaining depth
            int score =
                -FindMoveNegaMaxAlphaBeta(board, nextMoves, maxDepth - 1,
                                          -MAX_EVAL, MAX_EVAL, -turnMultiplier, maxDepth);

            board.UndoMove(move);

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
                std::cout << "Root: " << move.ToString() << " Score: " << score
                          << std::endl;
            }
        }

        return SearchResult(bestMove, bestScore, maxDepth, nodesSearched);
    }
};
