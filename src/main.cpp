#include "gui.h"
#include "board.h"

int main() {

  Board board;
  board.LoadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  board.PrintBitboards();

  ChessGUI gui(640, 640, &board, 40);
  gui.Run();
  return 0;
}
