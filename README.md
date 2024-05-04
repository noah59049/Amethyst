A UCI chess engine written in C++.

Board representation/move generation:
- Bitboards for each piece type
- Copy-make move generation
- Magic bitboards for sliding pieces
- Zobrist hashing

Search features:
- Fail-hard negamax search with alpha-beta pruning
- Repetition table
- Iterative Deepening
- Aspiration Windows
- Transposition Table
- Move Ordering:
  - Best move from transposition table
  - Captures with SEE>=0
  - Killer moves
  - All other quiet moves, sorted by history heuristic
  - Captures with SEE<0
- Check move extension
- Late move reductions, except not at root node
- Late move pruning
- Reverse Futility Pruning
- Null move pruning with verification search
- Quiescence search
  - Only examine captures with SEE>=0 (I think this is called SEE pruning)
 
Evaluation:
- Material
- Piece square tables
- Mobility
- Isolated Pawns
- Passed Pawn on rank/file bonus
- King Shelter
- King zone attacks
- Bishop Pair
- Pawn Hash Table
- Evaluation weights tuned using Texel Tuning
