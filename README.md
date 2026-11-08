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
- Hand-crafted evaluation with its own tuner, currently not published

## Building

You need CMake 3.26 or newer, Ninja, and a C++20 compiler.

```sh
cmake --preset release
cmake --build --preset release
```

This builds two executables in `build/release/`:

- `amethyst_chess3`: the engine
- `amethyst_chess3_test`: the test suite

For a debug build, use `--preset debug` in both commands. The output then goes to `build/debug/`.

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

## AI use

I used AI for a small part of this project:

- Updating `.gitignore` and adding `CMakePresets.json` when I moved the project from CLion to VSCode (Claude Code)
- Writing a few of the tests (ChatGPT)
- Writing this README (Claude Code)

I wrote everything else myself, including the move generator, search, evaluation and tuner.
