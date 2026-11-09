# Amethyst

Amethyst is a UCI-compatible chess engine written in C++20 by Noah Holbrook. It uses a hand-crafted evaluation (HCE), not a neural network, so it plays in any UCI chess GUI (Cute Chess, Arena, BanksiaGUI, and others) or from the command line.

## Strength

Amethyst has not been released or tested on CCRL yet. My estimate is around **2600–2700 Elo on the CCRL scale**.

## Features

**Search**
- Iterative deepening with aspiration windows
- Principal variation search (PVS) with a transposition table
- Quiescence search
- Reverse futility pruning, null move pruning, late move pruning and late move reductions
- Internal iterative reductions
- Move ordering with MVV-LVA, SEE and history heuristics
- Correction history

**Evaluation**
- Hand-crafted evaluation with its own tuner at https://github.com/noah59049/amethyst-tuner. That tuner is designed to work with the shared library at `hcetuner.cpp`

## Building

You need CMake 3.26 or newer, Ninja, and a C++20 compiler.

```sh
cmake --preset release
cmake --build --preset release
```

This builds the following in `build/release/`:

- `amethyst_chess3`: the engine
- `amethyst_chess3_test`: the test suite
- `libamethyst_tuner.dylib`: the tuner library (see below)

For a debug build, use `--preset debug` in both commands. The output then goes to `build/debug/`.

### Building the tuner library

[hcetuner.cpp](hcetuner.cpp) compiles to a shared library that the Python tuner ([amethyst-tuner](https://github.com/noah59049/amethyst-tuner)) calls to compute features from training positions. The full build above already includes it. To build only the library:

```sh
cmake --preset release
cmake --build --preset release --target amethyst_tuner
```

The output is `build/release/libamethyst_tuner.dylib` on macOS, `libamethyst_tuner.so` on Linux, or `amethyst_tuner.dll` on Windows.

The library reads its training positions from a file named `Noah.epd` in the current working directory of the process that loads it. Each line is a FEN followed by the game result as a number in square brackets. `[1.0]` = white wins, `[0.5]` = draw, `[0.0]` = black wins.

## Running

Start the engine:

```sh
./build/release/amethyst_chess3
```

To use it in a GUI, add `build/release/amethyst_chess3` as a UCI engine.

The engine reads UCI commands from standard input. Here is a short session:

```
uci
isready
position startpos moves e2e4 e7e5
go wtime 60000 btime 60000 winc 1000 binc 1000
quit
```

Supported `go` parameters: `wtime`, `btime`, `winc`, `binc`, `movestogo`, `depth`, `nodes`, `mate`, `movetime` and `infinite`.

### Extra commands

| Command      | Description                                           |
|--------------|-------------------------------------------------------|
| `bench`      | Runs a fixed benchmark search and reports node count and speed |
| `staticeval` | Prints the static evaluation of the current position  |

### UCI options

| Option | Default | Range   |
|--------|---------|---------|
| `Hash` | 16 MB   | 1–1024  |

Amethyst is currently single-threaded.

## Running the tests

```sh
./build/release/amethyst_chess3_test
```

## Code overview

Like most chess engines, Amethyst does three jobs:

1. **Represent the board and generate moves.** It stores the position and lists every legal move.
2. **Evaluate positions.** It gives a score to a position without looking ahead, such as "White is up about a pawn".
3. **Search.** It looks many moves ahead, using the evaluation at the end of each line, and picks the move that leads to the best result.

Everything else is support code: speaking the UCI protocol to GUIs, remembering positions it has already searched, and testing.

Most of the engine uses **bitboards**. A bitboard is a 64-bit integer with one bit per square. For example, one bitboard holds "all white pieces" and another holds "all knights". Combining them with bitwise operations answers questions like "which squares does this rook attack?" quickly. Squares are numbered with a1 = 0, a8 = 7, h1 = 56 and h8 = 63, so each group of 8 consecutive squares is one file, not one rank.

### Where to start reading

- [main.cpp](main.cpp) starts the UCI loop and does nothing else.
- [uci.cpp](uci.cpp) reads commands from the GUI, such as `position` and `go`, and sends them to the rest of the engine.
- [search.cpp](search.cpp) chooses the move. Each search function is split into numbered steps in the comments.
- [chessboard.h](chessboard.h) defines the board class that most other files use.

### Board representation and move generation

| File | What it does |
|------|--------------|
| [typedefs.h](typedefs.h) | Short type names used everywhere, such as `bitboard_t`, `move_t`, `square_t` and `eval_t` |
| [flags.h](flags.h) | Constants for pieces, sides and squares, plus functions for packing a move into a 32-bit integer and reading it back (from-square, to-square, moving piece, captured piece, promotion, and so on) |
| [bitmasks.h](bitmasks.h) | Fixed bitboards for useful square sets, such as the edge files or the squares that must be empty to castle |
| [chessboard.h](chessboard.h) / [chessboard.cpp](chessboard.cpp) | The `ChessBoard` class. It loads and writes FEN strings, makes moves, checks legality, detects check, and keeps the position's Zobrist hash up to date |
| [attacks.h](attacks.h) / [attacks.cpp](attacks.cpp) | Works out which squares a piece attacks. Sliding pieces (bishops, rooks, queens) use magic bitboards, a precomputed lookup table |
| [logarithm.h](logarithm.h) | Finds the index of a bitboard's lowest set bit, which turns a bitboard into a square |
| [zobrist.h](zobrist.h) | Random numbers for Zobrist hashing, which gives each position a 64-bit fingerprint so the engine can recognise it later |
| [movelist.h](movelist.h) | `MoveList`, a fixed-size array of moves |
| [movegenerator.h](movegenerator.h) / [movegenerator.cpp](movegenerator.cpp) | Hands moves to the search one at a time, in stages: the transposition table's best move first, then good captures, then quiet moves, then bad captures. Moves likely to be best go first, which lets the search skip more of the tree |
| [moveorder.h](moveorder.h) / [moveorder.cpp](moveorder.cpp) | Scores captures with MVV-LVA (Most Valuable Victim, Least Valuable Attacker), so capturing a queen with a pawn is tried before capturing a pawn with a queen |

### Evaluation

| File | What it does |
|------|--------------|
| [hce.h](hce.h) / [hce.cpp](hce.cpp) | The hand-crafted evaluation. It adds up piece-square tables (a value for each piece on each square, with separate tables depending on which side your king is on) and mobility (how many squares each piece can move to). Every term has a middlegame and an endgame value, which are blended according to how much material is left |
| [hcetuner.cpp](hcetuner.cpp) | Built as a shared library. It loads training positions and exports their features (piece placement, mobility, game phase, game result) so an outside tuning script can fit the evaluation weights |

### Search

| File | What it does |
|------|--------------|
| [search.h](search.h) / [search.cpp](search.cpp) | `rootSearch` runs iterative deepening: it searches 1 move deep, then 2, then 3, until time runs out. `negamax` is the main recursive alpha-beta search. `qsearch` is quiescence search: at depth 0, it continues searching until no captures are left to avoid horizon effect / evaluating positions in the middle of a trade |
| [searchglobals.h](searchglobals.h) / [searchglobals.cpp](searchglobals.cpp) | State shared during a search: time and depth limits, the transposition table, and `ThreadData` (node count, history tables, search stack) |
| [tt.h](tt.h) / [tt.cpp](tt.cpp) | The transposition table, a hash table of positions already searched, storing their score, depth and best move. The same position is often reached by different move orders, and this avoids searching it again |
| [repetitiontable.h](repetitiontable.h) / [repetitiontable.cpp](repetitiontable.cpp) | Records positions from the game so far to detect draws by repetition |
| [corrhist.h](corrhist.h) / [corrhist.cpp](corrhist.cpp) | Correction history. It tracks how far the static evaluation has been off for a given pawn structure and adjusts future evaluations to match |

### UCI and tooling

| File | What it does |
|------|--------------|
| [uci.h](uci.h) / [uci.cpp](uci.cpp) | The UCI command loop |
| [uciopt.h](uciopt.h) / [uciopt.cpp](uciopt.cpp) | Current values and allowed ranges for UCI options such as `Hash` |
| [bench.h](bench.h) / [bench.cpp](bench.cpp) | The `bench` command: searches a fixed set of positions and reports total nodes and speed. Developers use it to check that a change does not alter search behaviour unexpectedly |
| [perft.h](perft.h) / [perft.cpp](perft.cpp) | Perft counts every position reachable to a given depth. Comparing the counts with known correct values is the standard way to test a move generator |
| [tests.cpp](tests.cpp) | The test suite (perft, move encoding, and other checks), built as `amethyst_chess3_test` |

## AI use

I used AI for a small part of this project:

- Updating `.gitignore` and adding `CMakePresets.json` when I moved the project from CLion to VSCode (Claude Code)
- Writing a few of the tests in `tests.cpp` (ChatGPT)
- Writing this README (Claude Code)

I wrote everything else myself, including the move generator, search, evaluation and tuner.
