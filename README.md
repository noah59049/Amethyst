A UCI chess engine written in C++.

Board representation/move generation:
- Bitboards for each piece type (using uint64_t data type)
- Copy-make move generation
- Magic bitboards for sliding pieces
- Zobrist hashing

Search features:
- Negamax search with alpha-beta pruning
- Repetition table
- Iterative Deepening
- Aspiration Windows
- Transposition Table
- Move Ordering:
  - Best move from transposition table at current depth
  - Best move from transposition table at depth-1
  - Captures with SEE>=0
  - Killer moves
  - All other quiet moves, sorted by history heuristic
  - Captures with SEE<0
- Check move extension
- Late move reductions, except not at root node
- Null move reductions
  - If null move fails high, do search with depth-4. This avoids zugzwang.
- Quiescence search
  - Only examine captures with SEE>=0
 
Evaluation:
- Material
- Piece square tables
