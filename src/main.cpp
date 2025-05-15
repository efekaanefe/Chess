#include "board.h"
#include "gui.h"
#include "move.h"

int main() {
  Board board;
  board.LoadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  // board.PrintBitboards();

  // auto moves = board.GenerateMoves();
  // for (auto &m : moves) {
  //   std::cout << m.ToString() << std::endl;
  // }

  ChessGUI gui(640, 640, &board, 40);
  gui.Run();
  return 0;
}
