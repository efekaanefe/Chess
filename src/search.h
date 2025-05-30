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
        return moves.empty(); // Simplified - assumes legal move generation
    }

    // Quiescence search to avoid horizon effect
    int Quiescence(Board &board, int alpha, int beta) {
        nodesSearched++;

        int standPat = Evaluator::Evaluate(board);

        if (standPat >= beta) {
            return beta;
        }
        if (alpha < standPat) {
            alpha = standPat;
        }

        // Generate only capture moves for quiescence
        std::vector<Move> moves = board.GenerateMoves();
        std::vector<Move> captures;

        for (const auto &move : moves) {
            if (move.isCapture) {
                captures.push_back(move);
            }
        }

        // Order captures by Most Valuable Victim - Least Valuable Attacker
        // (MVV-LVA)
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

    // Order captures by MVV-LVA
    void OrderCaptures(Board &board, std::vector<Move> &captures) {
        std::sort(captures.begin(), captures.end(),
                  [&](const Move &a, const Move &b) {
                      // Get piece values for ordering
                      int victimA = GetPieceValue(board, a.toSquare);
                      int victimB = GetPieceValue(board, b.toSquare);
                      int attackerA = GetPieceValue(board, a.fromSquare);
                      int attackerB = GetPieceValue(board, b.fromSquare);

                      // MVV-LVA: prioritize high-value victims and low-value
                      // attackers
                      return (victimA - attackerA) > (victimB - attackerB);
                  });
    }

    // Get piece value at square for move ordering
    int GetPieceValue(Board &board, int square) {
        uint64_t mask = 1ULL << square;
        for (int i = 0; i < 12; i++) {
            if (board.bitboards[i] & mask) {
                return Evaluator::PIECE_VALUES[i % 6];
            }
        }
        return 0;
    }

    // Basic move ordering
    void OrderMoves(Board &board, std::vector<Move> &moves) {
        std::sort(moves.begin(), moves.end(),
                  [&](const Move &a, const Move &b) {
                      // Prioritize captures
                      if (a.isCapture && !b.isCapture)
                          return true;
                      if (!a.isCapture && b.isCapture)
                          return false;

                      // Among captures, use MVV-LVA
                      if (a.isCapture && b.isCapture) {
                          int victimA = GetPieceValue(board, a.toSquare);
                          int victimB = GetPieceValue(board, b.toSquare);
                          return victimA > victimB;
                      }

                      // Prioritize promotions
                      if (a.isPromotion && !b.isPromotion)
                          return true;
                      if (!a.isPromotion && b.isPromotion)
                          return false;

                      return false; // Keep original order for other moves
                  });
    }

    // Alpha-beta search
    int AlphaBeta(Board &board, int depth, int alpha, int beta,
                  bool maximizing) {
        nodesSearched++;

        if (depth == 0) {
            return Quiescence(board, alpha, beta);
        }

        std::vector<Move> moves = board.GenerateMoves();

        if (moves.empty()) {
            // TODO: Check if in check to distinguish checkmate from stalemate
            return maximizing ? -MATE_SCORE + (MAX_DEPTH - depth)
                              : MATE_SCORE - (MAX_DEPTH - depth);
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
    // Find best move using iterative deepening
    SearchResult FindBestMove(Board &board, int maxDepth = 6) {
        Move bestMove = Move(0, 0); // Default to an invalid move initially
        int bestScore = INT_MIN;
        nodesSearched = 0;

        std::vector<Move> rootMoves = board.GenerateMoves();
        if (rootMoves.empty()) {
            return SearchResult(); // No moves, return default result
        }

        OrderMoves(board, rootMoves);

        // Initialize bestMove with the first legal move as a fallback.
        // This ensures selectedMove is always a valid move if rootMoves is not
        // empty.
        bestMove = rootMoves[0];

        // Iterative deepening
        for (int depth = 1; depth <= maxDepth; depth++) {
            int alpha = INT_MIN;
            int beta = INT_MAX;
            Move currentBest = rootMoves[0]; // Initialize currentBest with the
                                             // first move for this depth
            int currentBestScore = INT_MIN;

            for (auto &move : rootMoves) {
                board.MakeMove(move);
                // The score here is from the perspective of the *next* player
                // after 'move' is made. So we negate it to get it from the
                // current player's perspective.
                int score = -AlphaBeta(
                    board, depth - 1, -beta, -alpha,
                    false); // Pass false as 'maximizing' for the opponent
                board.UndoMove(move);

                // This 'score' is from the root player's perspective
                if (score > currentBestScore) {
                    currentBestScore = score;
                    currentBest = move;
                }

                alpha = std::max(alpha, score);
                if (beta <= alpha) {
                    // If a cutoff happens at the root, the best move found so
                    // far is still valid.
                    break;
                }
            }

            bestMove = currentBest;
            bestScore = currentBestScore;

            // Print search info
            std::cout << "Depth " << depth << ": " << bestMove.ToString()
                      << " (score: " << bestScore << ")" << std::endl;

            // If a mate is found at the current depth, no need to search deeper
            if (std::abs(bestScore) >= MATE_SCORE - (MAX_DEPTH - depth)) {
                std::cout << "Mate found at depth " << depth
                          << ", stopping iterative deepening." << std::endl;
                break;
            }
        }

        return SearchResult(bestMove, bestScore, maxDepth, nodesSearched);
    }

    // Quick search for a given depth (similar logic)
    SearchResult Search(Board &board, int depth) {
        nodesSearched = 0;
        std::vector<Move> moves = board.GenerateMoves();

        if (moves.empty()) {
            return SearchResult();
        }

        OrderMoves(board, moves);

        Move bestMove = moves[0]; // Initialize with first move
        int bestScore = INT_MIN;

        for (auto &move : moves) {
            board.MakeMove(move);
            int score = -AlphaBeta(board, depth - 1, INT_MIN, INT_MAX,
                                   false); // Opponent is minimizing
            board.UndoMove(move);

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }

        return SearchResult(bestMove, bestScore, depth, nodesSearched);
    }
};
