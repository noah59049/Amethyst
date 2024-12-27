#include <sstream>
#include <string>
#include <cassert>
#include <iostream>
#include <bit>

#include "chessboard.h"
#include "logarithm.h"
#include "attacks.h"
#include "bitmasks.h"
#include "zobrist.h"
#include "hce.h"

const static std::string whitePieceChars = "PNBRQK";
const static std::string blackPieceChars = "pnbrqk";
const static std::string emptySpaceChars = "/12345678";
const static std::string fileChars = "abcdefgh";
const static std::string rankChars = "12345678";

std::string moveToLAN(move_t move) {
    square_t from = mvs::getFrom(move);
    square_t to = mvs::getTo(move);
    std::string output;
    output += fileChars[squares::getFile(from)];
    output += rankChars[squares::getRank(from)];
    output += fileChars[squares::getFile(to)];
    output += rankChars[squares::getRank(to)];

    if (mvs::isPromotion(move)) {
        output += blackPieceChars[mvs::getPromotedPiece(move)];
    }

    return output;
}

inline square_t rotateSquare(square_t fenSquare) {
    // FEN (kind of implicitly) uses
    // a8 = 0
    // h8 = 7
    // a1 = 56
    // h1 = 63

    // However, I use
    // a1 = 0
    // a8 = 7
    // h1 = 56
    // h8 = 63

    // So we need to rotate the square
    square_t myRank = 7 - fenSquare / 8;
    square_t myFile = fenSquare & 7;

    // return myFile * 8 + myRank;
    return squares::squareFromFileRank(myFile, myRank);
}

inline bitboard_t fenSquareToBitboard(square_t fenSquare) {
    return 1ULL << rotateSquare(fenSquare);
}

ChessBoard::ChessBoard(const std::string &fen) {
    pieceTypes = {0,0,0,0,0,0};
    colors = {0,0};
    stm = 0;
    epCastlingRights = 0;
    halfmove = 0;
    pieceGivingCheck = NOT_IN_CHECK_CODE;
    // Initializing stm, epCastlingRights, pieceGivingCheck and halfmove are not strictly necessary
    // But they are nice to have because otherwise we might have UB
    // Also otherwise CLion gives warnings

    // Step 1: Split the FEN into 6 parts
    // Step 1.1: Initialize the strings with parts of the FEN
    std::string fenPieces;
    std::string fenSTM;
    std::string fenCastlingRights;
    std::string fenEPRights;
    std::string fenHalfmove;
    std::string fenFullmove;

    // Step 1.2: Initialize the stringstream
    std::stringstream ss(fen);

    // Step 1.3: Put all the parts of the stringstream into FEN parts
    ss >> fenPieces;
    ss >> fenSTM;
    ss >> fenCastlingRights;
    ss >> fenEPRights;
    ss >> fenHalfmove;
    ss >> fenFullmove;

    // Step 2: Fill in pieceTypes and colors
    square_t fenSquare = 0;
    for (char chr : fenPieces) {
        bitboard_t toAdd = fenSquareToBitboard(fenSquare);
        auto whitePieceType = whitePieceChars.find(chr);
        auto blackPieceType = blackPieceChars.find(chr);
        auto numEmptySpaces = emptySpaceChars.find(chr);
        
        if (whitePieceType != std::string::npos) {
            pieceTypes[whitePieceType] |= toAdd;
            colors[sides::WHITE] |= toAdd;
            fenSquare++;
        }
        else if (blackPieceType != std::string::npos) {
            pieceTypes[blackPieceType] |= toAdd;
            colors[sides::BLACK] |= toAdd;
            fenSquare++;
        }
        else if (numEmptySpaces != std::string::npos) {
            fenSquare += numEmptySpaces;
        }
        else {
            assert(false);
        }
    }

    // Step 3: Initialize stm
    if (fenSTM == "w")
        stm = sides::WHITE;
    else if (fenSTM == "b")
        stm = sides::BLACK;
    else
        assert(false);

    // Step 4: Initialize castling rights
    for (char c : fenCastlingRights) {
        switch (c) {
            case 'K':
                epCastlingRights |= rights::WHITE_SC;
                break;
            case 'Q':
                epCastlingRights |= rights::WHITE_LC;
                break;
            case 'k':
                epCastlingRights |= rights::BLACK_SC;
                break;
            case 'q':
                epCastlingRights |= rights::BLACK_LC;
            case '-':
                break;
            default:
                assert(false);
        }
    }

    // Step 5: Initialize en passant rights
    auto epRightsFile = fileChars.find(fenEPRights[0]);
    if (epRightsFile != std::string::npos) {
        rights::addEPRights(epCastlingRights,epRightsFile);
    }

    // Step 6: Initialize halfmove
    std::stringstream halfmoveSS(fenHalfmove);
    halfmoveSS >> halfmove;


    // Step 7: Initialize fullmove
    fullmove = 0;
    std::stringstream fullmoveSS(fenFullmove);
    fullmoveSS >> fullmove;

    // Step 8: Initialize zobrist code
    zobristCode = calcZobristCode();

    // Step 9: Initialize piece giving check
    updatePieceGivingCheck();
}

ChessBoard ChessBoard::fromFEN(const std::string& fen) {
    return ChessBoard(fen);
}

ChessBoard ChessBoard::startpos() {
    return ChessBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

std::string ChessBoard::toFEN() const {
    std::string fen;

    // Step 1: Add pieces to FEN
    for (square_t rank = 7; rank != square_t(-1); rank--) {
        int numEmptySquares = 0;
        for (square_t file = 0; file < 8; file++) {
            bitboard_t squareMask = 1ULL << squares::squareFromFileRank(file, rank);
            char chr = ' ';
            if (colors[sides::WHITE] & squareMask) {
                for (piece_t piece = pcs::PAWN; piece <= pcs::KING; piece++) {
                    if (pieceTypes[piece] & squareMask) {
                        chr = whitePieceChars[piece];
                        break;
                    }
                }
            }
            else if (colors[sides::BLACK] & squareMask) {
                for (piece_t piece = pcs::PAWN; piece <= pcs::KING; piece++) {
                    if (pieceTypes[piece] & squareMask) {
                        chr = blackPieceChars[piece];
                        break;
                    }
                }
            }
            // We are done finding the piece
            if (chr == ' ') {
                numEmptySquares++;
                if (file == 7)
                    fen += std::string(1,emptySpaceChars[numEmptySquares]);
            }
            else { // there is an actual piece there
                if (numEmptySquares != 0) {
                    fen += std::string(1,emptySpaceChars[numEmptySquares]);
                    numEmptySquares = 0;
                }
                fen += std::string(1,chr);
            }
        } // end for loop over file
        if (rank != 0)
            fen += "/";
    } // end for loop over rank

    // Step 2: Add stm to FEN
    // Note: We add a space on either side of stm
    // This is because it's easier to add the spaces here than somewhere else
    if (stm == sides::WHITE)
        fen += " w ";
    else
        fen += " b ";

    // Step 3: Add castling rights to FEN
    if (epCastlingRights & rights::WHITE_SC)
        fen += "K";
    if (epCastlingRights & rights::WHITE_LC)
        fen += "Q";
    if (epCastlingRights & rights::BLACK_SC)
        fen += "k";
    if (epCastlingRights & rights::BLACK_LC)
        fen += "q";
    const auto allCastlingRights = rights::WHITE_SC | rights::WHITE_LC | rights::BLACK_SC | rights::BLACK_LC;
    if (!(epCastlingRights & allCastlingRights))
        fen += "-";
    fen += " ";

    // Step 3: Add en passant rights
    if (rights::isEPPossible(epCastlingRights)) {
        fen += fileChars[rights::extractEPRights(epCastlingRights)];
        if (stm == sides::WHITE)
            fen += "6";
        else
            fen += "3";
    }
    else {
        fen += "-";
    }
    fen += " ";

    // Step 4: Halfmove
    fen += std::to_string(halfmove);
    fen += " ";

    // Step 5: Fullmove
    fen += std::to_string(fullmove);

    // Step 6: Return the fen
    return fen;
}

bool ChessBoard::areBitboardsCorrect() const {
    bool correct = true;

    // Check #1: White and black sides pieces don't overlap.
    if (colors[sides::WHITE] & colors[sides::BLACK]) {
        std::cout << "FAILED bitboards correct test: Bitboards for sides overlap" << std::endl;
        correct = false;
    }
    bitboard_t allColors = colors[sides::WHITE] | colors[sides::BLACK];

    // Check #2: Piece types don't overlap
    bitboard_t allPieceTypes = 0;
    for (piece_t pieceType = pcs::PAWN; pieceType <= pcs::KING; pieceType++) {
        if (allPieceTypes & pieceTypes[pieceType]) {
            std::cout << "FAILED bitboards correct test: Bitboards for piece types overlap" << std::endl;
            correct = false;
        }
        allPieceTypes |= pieceTypes[pieceType];
    }

    // Check #3: Union of all piece types is equal to union of both colors
    if (allPieceTypes != allColors) {
        std::cout << "FAILED bitboards correct test: Union of all piece types is not equal to union of both colors" << std::endl;
        correct = false;
    }

    // Check #4: Both sides have exactly one king
    if (std::popcount(colors[sides::WHITE] & pieceTypes[pcs::KING]) != 1) {
        std::cout << "FAILED bitboards correct test: White does not have exactly one king" << std::endl;
        correct = false;
    }
    if (std::popcount(colors[sides::BLACK] & pieceTypes[pcs::KING]) != 1) {
        std::cout << "FAILED bitboards correct test: Black does not have exactly one king" << std::endl;
        correct = false;
    }

    // Check #5: No pawns on the first or last ranks
    if (pieceTypes[pcs::PAWN] & masks::FIRST_AND_LAST_RANK) {
        std::cout << "FAILED bitboards correct test: Pawns on the first or last rank" << std::endl;
        correct = false;
    }

    // Check #6: Zobrist is correct
    if (zobristCode != calcZobristCode()) {
        std::cout << "FAILED bitboards correct test: zobrist code is not correct" << std::endl;
        std::cout << std::hex;
        std::cout << "incremental zobrist code is 0x" << zobristCode << ", calculated zobrist code is 0x" << calcZobristCode() << ", and their difference is 0x" << (zobristCode ^ calcZobristCode()) << std::endl;
        correct = false;
    }

    // Final result: Print out info if the test failed
    if (!correct)
        printAllBitboards();

    return correct;
}

void ChessBoard::printAllBitboards() const {
    std::cout << "Results of ChessBoard::printAllBitboards() are:" << std::endl;
    std::cout << "FEN is " << toFEN() << std::endl;
    std::cout << "white bitboard is " << colors[sides::WHITE] << std::endl;
    std::cout << "black bitboard is " << colors[sides::BLACK] << std::endl;
    for (piece_t piece1 = pcs::PAWN; piece1 <= pcs::KING; piece1++) {
        std::cout << "piece bitboard for piece code " << uint16_t(piece1) << " is " << pieceTypes[piece1] << std::endl;
    }
    std::cout << "epCastlingRights is: " << uint16_t(epCastlingRights) << std::endl;
    std::cout << std::endl;
}

void ChessBoard::makemove(move_t move) {
    // Step 0: Get info from move
    const piece_t piece = mvs::getPiece(move);
    const square_t from = mvs::getFrom(move);
    const square_t to = mvs::getTo(move);
    const bitboard_t delta = 1ULL << from | 1ULL << to;

    // Step 1: Actually move the piece
    pieceTypes[piece] ^= delta;
    colors[stm] ^= delta;

    // Step 1.5: zobrist code for moving the piece
    zobristCode ^= zb::getPieceZobrist(from,stm,piece) ^ zb::getPieceZobrist(to,stm,piece);

    // Step 2: Promotions
    if (mvs::isPromotion(move)) {
        piece_t promoPiece = mvs::getPromotedPiece(move);
        pieceTypes[promoPiece] ^= 1ULL << to;
        pieceTypes[pcs::PAWN] ^= 1ULL << to;

        // Step 2.5: zobrist code for promotion
        zobristCode ^= zb::getPieceZobrist(to,stm,pcs::PAWN) ^ zb::getPieceZobrist(to,stm,promoPiece);
    }

    // Step 3: EP
    if (mvs::isEP(move)) {
        square_t epSquare = to;
        stm == sides::WHITE ? epSquare-- : epSquare++;
        pieceTypes[pcs::PAWN] ^= 1ULL << epSquare;
        colors[stm ^ 1] ^= 1ULL << epSquare;

        // Step 3.5: zobrist code for EP
        zobristCode ^= zb::getPieceZobrist(epSquare, stm ^ 1, pcs::PAWN);
    }

    // Step 4: Regular captures
    else if (mvs::isCapture(move)) {
        piece_t capturedPiece = mvs::getCapturedPiece(move);
        pieceTypes[capturedPiece] ^= 1ULL << to;
        colors[stm ^ 1] ^= 1ULL << to;

        // Step 4.5: zobrist code for regular captures
        zobristCode ^= zb::getPieceZobrist(to, stm ^ 1, capturedPiece);
    }

    // Step 5: Castling
    if (mvs::isShortCastle(move)) {
        bitboard_t deltaRooks;
        zobrist_t deltaZobrist;
        if (stm == sides::WHITE) {
            deltaRooks = 1ULL << squares::h1 | 1ULL << squares::f1;
            deltaZobrist = zb::getPieceZobrist(squares::h1,stm,pcs::ROOK) ^ zb::getPieceZobrist(squares::f1,stm,pcs::ROOK);
        }
        else {
            deltaRooks = 1ULL << squares::h8 | 1ULL << squares::f8;
            deltaZobrist = zb::getPieceZobrist(squares::h8,stm,pcs::ROOK) ^ zb::getPieceZobrist(squares::f8,stm,pcs::ROOK);
        }

        pieceTypes[pcs::ROOK] ^= deltaRooks;
        colors[stm] ^= deltaRooks;

        // Step 5.5: zobrist code for castling
        zobristCode ^= deltaZobrist;
    }

    else if (mvs::isLongCastle(move)) {
        bitboard_t deltaRooks;
        zobrist_t deltaZobrist;
        if (stm == sides::WHITE) {
            deltaRooks = 1ULL << squares::d1 | 1ULL << squares::a1;
            deltaZobrist = zb::getPieceZobrist(squares::d1,stm,pcs::ROOK) ^ zb::getPieceZobrist(squares::a1,stm,pcs::ROOK);
        }
        else {
            deltaRooks = 1ULL << squares::d8 | 1ULL << squares::a8;
            deltaZobrist = zb::getPieceZobrist(squares::d8,stm,pcs::ROOK) ^ zb::getPieceZobrist(squares::a8,stm,pcs::ROOK);
        }

        pieceTypes[pcs::ROOK] ^= deltaRooks;
        colors[stm] ^= deltaRooks;

        // Step 5.5: zobrist code for castling
        zobristCode ^= deltaZobrist;
    }

    // Step 5.99: Zobrist code for epCastlingRights before changes
    zobristCode ^= zb::getRightsZobrist(epCastlingRights);

    // Step 6: Deal with EP Rights
    rights::removeEPRights(epCastlingRights);
    if (mvs::isDoublePawnPush(move)) {
        rights::addEPRights(epCastlingRights, squares::getFile(from));
    }

    // Step 7: Deal with castling rights
    epCastlingRights &= rights::CASTLING_RIGHTS_PRESERVED[from] & rights::CASTLING_RIGHTS_PRESERVED[to];

    // Step 7.01: Zobrist code for epCastlingRights after changes
    zobristCode ^= zb::getRightsZobrist(epCastlingRights);

    // Step 8: Adjust the halfmove
    if (mvs::isCapture(move) or piece == pcs::PAWN)
        halfmove = 0;
    else
        halfmove++;

    // Step 9: Adjust the fullmove
    if (stm == sides::BLACK)
        fullmove += 1;

    // Step 10: Adjust the stm
    stm ^= 1;

    // Step 10.5: Zobrist code for stm
    zobristCode ^= zb::stmZobrist;

    // Step 11: Update the piece giving check
    updatePieceGivingCheck();
}

move_t ChessBoard::parseLANMove(const std::string &move) const {
    // Step 1: Get the from and to squares
    square_t fromFile = move[0] - 'a';
    square_t fromRank = move[1] - '1';
    square_t toFile = move[2] - 'a';
    square_t toRank = move[3] - '1';
    square_t from = squares::squareFromFileRank(fromFile, fromRank);
    square_t to = squares::squareFromFileRank(toFile, toRank);

    // Step 2: Get the moving piece
    bitboard_t fromBB = 1ULL << from;
    piece_t piece;
    for (piece = pcs::PAWN; piece <= pcs::KING; piece++) {
        if (pieceTypes[piece] & fromBB)
            break;
    }

    // Step 2.5: If there isn't any piece on the start square, throw an error
    if (piece > pcs::KING) {
        std::cout << "Error in ChessBoard::makeLANMove: No piece on start square" << std::endl;
        std::cout << "FEN is " << toFEN() << std::endl;
        std::cout << "move is " << move << std::endl;
        exit(1);
    }

    // Step 3: Get the captured piece
    // Note that we consider a move a capture here if there is a piece on the "to" square
    // This means that en passant is not counted as a capture
    // We will handle en passant later
    bitboard_t toBB = 1ULL << to;
    piece_t capturedPiece;
    bool isCapture = false;
    for (capturedPiece = pcs::PAWN; capturedPiece < pcs::KING; capturedPiece++) {
        if (pieceTypes[capturedPiece] & toBB) {
            isCapture = true;
            break;
        }
    }
    if (!isCapture)
        capturedPiece = 0;

    // Step 4: Get the promoted piece
    bool isPromo = move.size() - 4;
    piece_t promoPiece = isPromo ? blackPieceChars.find(move[4]) : 0;

    // Step 5: Get the flag

    // Step 5.1: Captures / quiets flag
    move_t flag = isCapture ? flags::CAPTURE_FLAG : flags::QUIET_FLAG;

    // Step 5.2: Promo flag
    if (isPromo)
        flag = isCapture ? flags::KNIGHT_CAP_PROMO_FLAG + promoPiece - pcs::KNIGHT : flags::KNIGHT_PROMO_FLAG + promoPiece - pcs::KNIGHT;

    // Step 5.3: Castle flag
    if (piece == pcs::KING and fromFile == 4 and toFile == 6)
        flag = flags::SHORT_CASTLE_FLAG;
    else if (piece == pcs::KING and fromFile == 4 and toFile == 2)
        flag = flags::LONG_CASTLE_FLAG;

    // Step 5.4: En passant flags
    if (piece == pcs::PAWN and fromFile != toFile and !isCapture)
        flag = flags::EN_PASSANT_FLAG;

    // Step 5.5: Double pawn push flags
    if (piece == pcs::PAWN and (from == to + 2 or to == from + 2))
        flag = flags::DOUBLE_PAWN_PUSH_FLAG;

    return mvs::constructMove(from,to,flag,piece,capturedPiece);
}

bool ChessBoard::canTheKingBeTaken() const {
    // Can stm take nstm's king
    // This is the same as
    // If nstm's king were a [piece], could it take any of stm's [piece]s
    // and sub in pawn, knight, bishop, rook, queen, king for [piece]
    const side_t nstm = stm ^ 1;
    const bitboard_t allPieces = colors[sides::WHITE] ^ colors[sides::BLACK];
    const square_t kingSquare = log2ll(pieceTypes[pcs::KING] & colors[nstm]);

    for (piece_t piece = pcs::PAWN; piece <= pcs::KING; piece++) {
        bitboard_t attackedSquares = getAttackedSquares(kingSquare, piece, allPieces, nstm);
        if (attackedSquares & pieceTypes[piece] & colors[stm])
            return true;
    }

    return false;
}

bool ChessBoard::isPseudolegal(move_t move) const {
    if (move == 0)
        return false;

    // Step 0: Get info from move
    const piece_t piece = mvs::getPiece(move);
    const square_t from = mvs::getFrom(move);
    const square_t to = mvs::getTo(move);
    const bitboard_t fromBB = 1ULL << from;
    const bitboard_t toBB = 1ULL << to;
    const bitboard_t allPieces = colors[sides::WHITE] | colors[sides::BLACK];

    // Step 1: Check that there is a piece on the start square
    if ((fromBB & pieceTypes[piece] & colors[stm]) == 0) {
        return false;
    }

    // Step 2: Make sure that if the move is a capture, the correct piece is actually captured
    // Also make sure that if the move is not a capture, a piece is not captured
    if (mvs::isCapture(move) and !mvs::isEP(move)) {
        const piece_t capturedPiece = mvs::getCapturedPiece(move);
        if ((colors[stm ^ 1] & pieceTypes[capturedPiece] & toBB) == 0)
            return false;
    }
    else {
        if (allPieces & toBB)
            return false;
    }

    // Step 3: Handle castling
    if (mvs::isShortCastle(move)) {
        // We must not be in check (isLegal doesn't check this)
        if (isInCheck())
            return false;
        // We must have castling rights
        if (!rights::canSideCastleShort(stm, epCastlingRights))
            return false;
        // Squares in between king and rook must be empty
        if ((allPieces >> 7 * stm & masks::E1_THROUGH_H1) != masks::E1_H1)
            return false;
    }

    if (mvs::isLongCastle(move)) {
        // We must not be in check (isLegal doesn't check this)
        if (isInCheck())
            return false;
        // We must have castling rights
        if (!rights::canSideCastleLong(stm, epCastlingRights))
            return false;
        // Squares in between king and rook must be empty
        if ((allPieces >> 7 * stm & masks::E1_THROUGH_A1) != masks::E1_A1)
            return false;
    }

    // Step 4: Handle en passant
    if (mvs::isEP(move)) {
        // The destination square must be empty (we already checked this)
        if (allPieces & toBB)
            exit(1);
        // The piece moving must be a pawn (check not needed because all EP moves generated have piece == PAWN)
        if (piece != pcs::PAWN)
            exit(1);
        // The piece moving must be on the fifth rank for white or fourth rank for black
        if (squares::getRank(from) != 4 - stm)
            return false;
        // En passant must be possible
        if (!rights::isEPPossible(epCastlingRights))
            return false;
        // The destination file must be equal to the pawn that last moved two squares
        if (squares::getFile(to) != rights::extractEPRights(epCastlingRights))
            return false;
    }

    // Step 5: Handle promotions
    if (mvs::isPromotion(move)) {
        // The piece moving must be a pawn (check not needed because all promotions generated are pawn moves)
        if (piece != pcs::PAWN)
            exit(1);
    }

    // Step 6: Handle double pawn pushes
    if (mvs::isDoublePawnPush(move)) {
        // There must be nothing in front of the pawn
        // The start square must be on the correct rank
        if (stm == sides::WHITE) {
            return (fromBB & masks::SECOND_RANK  and ~allPieces & fromBB << 1);
        }
        else {
            return (fromBB & masks::SEVENTH_RANK and ~allPieces & fromBB >> 1);
        }
    }

    // Step 7: Handle single pawn pushes
    else if (piece == pcs::PAWN and !mvs::isCapture(move)) {
        // The pawn must move forwards one square
        // This avoids issues where the other side moves a pawn the other direction
        if (to - from != 1 - 2 * stm)
            return false;
    }

    // Step 8: Make sure that the piece attacks the destination square
    else if (!mvs::isCastle(move)){
        return getAttackedSquares(from,piece,allPieces,stm) & toBB;
    }

    return true;
}

bool ChessBoard::isLegal(move_t move) const {
    ChessBoard newBoard = *this;
    if (mvs::isCastle(move)) {
        newBoard.makemove(mvs::castleToKingSlide(move));
        if (newBoard.canTheKingBeTaken())
            return false;
        newBoard = *this;
    }
    newBoard.makemove(move);
    return !newBoard.canTheKingBeTaken();
}

[[nodiscard]] bool ChessBoard::isGoodSEE(move_t move) const {
    // Step 1: Deal with underpromotions (always bad) and queen promo-captures (always good)
    if (mvs::isPromotion(move)) {
        if (mvs::getPromotedPiece(move) != pcs::QUEEN)
            return false;
        else if (mvs::isCapture(move))
            return true;
    }

    const side_t nstm = stm ^ 1;
    const square_t to = mvs::getTo(move);
    const piece_t movingPiece = mvs::getPiece(move);
    const piece_t capturedPiece = mvs::getCapturedPiece(move); // this is just PAWN if no piece was captured, which is fine
    if (movingPiece == pcs::KING)
        return true;
    if (mvs::isCapture(move) and capturedPiece >= movingPiece)
        return true;
    int diff = 1;
    const bitboard_t fromBB = 1ULL << mvs::getFrom(move);
    bitboard_t blockers = colors[0] ^ colors[1] ^ fromBB;

    for (piece_t piece = pcs::PAWN; piece <= pcs::KING; piece++) {
        bitboard_t nstmAttackers = getAttackedSquares(to,piece,blockers,stm) & colors[nstm] & pieceTypes[piece];
        bitboard_t stmAttackers = getAttackedSquares(to,piece,blockers,nstm) & colors[stm] & pieceTypes[piece] & ~fromBB;
        diff += std::popcount(stmAttackers) - std::popcount(nstmAttackers);

        if (nstmAttackers and piece < movingPiece)
            return false;
        if (diff > 1)
            return true;
        else if (diff < 0)
            return false;

        blockers ^= nstmAttackers ^ stmAttackers;
    }
    return diff;
}

void ChessBoard::updatePieceGivingCheck() {
    const side_t nstm = stm ^ 1;
    const bitboard_t allPieces = colors[sides::WHITE] ^ colors[sides::BLACK];
    const square_t kingSquare = log2ll(pieceTypes[pcs::KING] & colors[stm]);

    pieceGivingCheck = NOT_IN_CHECK_CODE;

    for (piece_t piece = pcs::PAWN; piece <= pcs::QUEEN; piece++) {
        bitboard_t attackedSquares = getAttackedSquares(kingSquare, piece, allPieces, stm);
        bitboard_t checkersBB = attackedSquares & pieceTypes[piece] & colors[nstm];
        if (checkersBB == 0) {
            continue;
        }
        else if (pieceGivingCheck != NOT_IN_CHECK_CODE or std::popcount(checkersBB) > 1) {
            pieceGivingCheck = DOUBLE_CHECK_CODE;
            return;
        }
        else {
            pieceGivingCheck = log2ll(checkersBB);
        }
    } // end for loop over piece type
} // end updatePieceGivingCheck method

piece_t ChessBoard::getPieceAt(square_t square) const {
    const bitboard_t squareBB = 1ULL << square;
    for (piece_t piece = pcs::PAWN; piece <= pcs::KING; piece++) {
        if (squareBB & pieceTypes[piece])
            return piece;
    }
    assert(false);
    exit(1);
}

MoveList ChessBoard::getPseudoLegalMoves() const {
    MoveList moves;

    const bitboard_t allPieces = colors[sides::WHITE] | colors[sides::BLACK];
    const side_t nstm = stm ^ 1;

    bitboard_t remainingFrom; // This will be used as the bitboard of all of a type of piece that can move
    bitboard_t fromBB; // This is the bitboard of the one piece that we are considering moving at this time.
    // In other words, popcount(fromBB) will always be 1
    square_t from; // The from square of the move we are considering
    bitboard_t remainingTo; // This is the bitboard of the remaining squares the pieces can move to
    bitboard_t toBB; // This is the bitboard of the one square we are considering moving to
    // In other words, popcount(toBB) will always be 1
    square_t to; // The to square of the move we are considering
    piece_t capturedPiece; // The piece that is captued in the specific move we're making

    // Step 1: EP
    if (rights::isEPPossible(epCastlingRights)) {
        square_t epFile = rights::extractEPRights(epCastlingRights);
        to = squares::squareFromFileRank(epFile, 5 - 3 * stm);
        for (square_t direction = 0; direction < 2; direction++) {
            // direction = 0 means captures towards the a-file
            // direction = 1 means captures towards the h-file
            from = squares::squareFromFileRank(epFile + 1 - 2 * direction, 4 - stm);
            if (epFile != (7 & direction - 1) and 1ULL << from & colors[stm] & pieceTypes[pcs::PAWN]) {
                moves.push_back(mvs::constructMove(from, to, flags::EN_PASSANT_FLAG, pcs::PAWN, 0));
            } // end if this EP move is pseudolegal
        } // end for loop over direction
    } // end if en passant rights exist

    // Step 3: Pawn captures
    for (square_t direction = 0; direction < 2; direction++) {
        // direction = 0 means captures towards the a-file
        // direction = 1 means captures towards the h-file
        remainingFrom = colors[nstm] & masks::NOT_H_FILE << 8 * direction;
        if (direction == 0)
            remainingFrom <<= 7 + stm + stm;
        else
            remainingFrom >>= 9 - stm - stm;
        remainingFrom &= pieceTypes[pcs::PAWN] & colors[stm];
        while (remainingFrom) {
            fromBB = remainingFrom & -remainingFrom;
            remainingFrom -= fromBB;
            from = log2ll(fromBB);
            to = from - 7 + direction * 16 - stm - stm;
            capturedPiece = getPieceAt(to);
            if (1U << squares::getRank(to) & 0b10000001) { // if getRank(to) is 0 or 7
                moves.push_back(mvs::constructMove(from, to, flags::QUEEN_CAP_PROMO_FLAG, pcs::PAWN, capturedPiece));
                moves.push_back(mvs::constructMove(from, to, flags::ROOK_CAP_PROMO_FLAG, pcs::PAWN, capturedPiece));
                moves.push_back(mvs::constructMove(from, to, flags::BISHOP_CAP_PROMO_FLAG, pcs::PAWN, capturedPiece));
                moves.push_back(mvs::constructMove(from, to, flags::KNIGHT_CAP_PROMO_FLAG, pcs::PAWN, capturedPiece));
            }
            else {
                moves.push_back(mvs::constructMove(from, to, flags::CAPTURE_FLAG, pcs::PAWN, capturedPiece));
            } // end else (not promotion)
        } // end while remainingFrom
    } // End for loop over direction

    // Step 4: Pawn single pushes
    remainingFrom = (stm == sides::BLACK) ? ~allPieces << 1 : ~allPieces >> 1;
    remainingFrom &= pieceTypes[pcs::PAWN] & colors[stm];
    while (remainingFrom) {
        fromBB = remainingFrom & -remainingFrom;
        remainingFrom -= fromBB;
        from = log2ll(fromBB);
        to = from + 1 - stm - stm;
        if (1U << squares::getRank(to) & 0b10000001) { // if getRank(to) is 0 or 7
            moves.push_back(mvs::constructMove(from, to, flags::QUEEN_PROMO_FLAG, pcs::PAWN, 0));
            moves.push_back(mvs::constructMove(from, to, flags::ROOK_PROMO_FLAG, pcs::PAWN, 0));
            moves.push_back(mvs::constructMove(from, to, flags::BISHOP_PROMO_FLAG, pcs::PAWN, 0));
            moves.push_back(mvs::constructMove(from, to, flags::KNIGHT_PROMO_FLAG, pcs::PAWN, 0));
        }
        else {
            moves.push_back(mvs::constructMove(from, to, flags::QUIET_FLAG, pcs::PAWN, 0));
        } // end else (not promotion)
    } // end while remainingFrom

    // Step 5: Pawn double pushes
    if (stm == sides::WHITE)
        remainingFrom = masks::SECOND_RANK & ~allPieces >> 1 & ~allPieces >> 2;
    else
        remainingFrom = masks::SEVENTH_RANK & ~allPieces << 1 & ~allPieces << 2;
    remainingFrom &= pieceTypes[pcs::PAWN] & colors[stm];
    while (remainingFrom) {
        fromBB = remainingFrom & -remainingFrom;
        remainingFrom -= fromBB;
        from = log2ll(fromBB);
        to = from + 2 - 4 * stm;
        moves.push_back(mvs::constructMove(from, to, flags::DOUBLE_PAWN_PUSH_FLAG, pcs::PAWN, 0));
    } // end while remainingFrom

    // Step 6: Pieces moving
    for (piece_t piece = pcs::KNIGHT; piece <= pcs::KING; piece++) {
        remainingFrom = colors[stm] & pieceTypes[piece];
        while (remainingFrom) {
            fromBB = remainingFrom & -remainingFrom;
            remainingFrom -= fromBB;
            from = log2ll(fromBB);
            remainingTo = getAttackedSquares(from, piece, allPieces, stm);
            while (remainingTo) {
                toBB = remainingTo & -remainingTo;
                remainingTo -= toBB;
                if (toBB & allPieces) {
                    if (toBB & colors[nstm]) {
                        to = log2ll(toBB);
                        capturedPiece = getPieceAt(to);
                        moves.push_back(mvs::constructMove(from, to, flags::CAPTURE_FLAG, piece, capturedPiece));
                    } // end if move is a capture
                } // end if there is a piece at end square
                else {
                    to = log2ll(toBB);
                    moves.push_back(mvs::constructMove(from, to, flags::QUIET_FLAG, piece, 0));
                } // end if move is quiet
            } // end while remainingTo
        } // end while remainingFrom
    } // end for loop over pieceType

    // Step 7: Castling
    // Note: This does assume white = 0, black = 1 to make the code block shorter
    if (!isInCheck()) {
        // Short castle
        if (rights::canSideCastleShort(stm,epCastlingRights) and
            (allPieces & masks::E1_THROUGH_H1 << 7 * stm) == masks::E1_H1 << 7 * stm) {
            moves.push_back(mvs::constructShortCastle(stm));
        }

        // Long castle
        if (rights::canSideCastleLong(stm,epCastlingRights) and
            (allPieces & masks::E1_THROUGH_A1 << 7 * stm) == masks::E1_A1 << 7 * stm) {
            moves.push_back(mvs::constructLongCastle(stm));
        } // end if can castle long
    } // end if not in check

    // Step 8: Return the pseudolegal moves
    return moves;
}

zobrist_t ChessBoard::calcZobristCode() const {
    zobrist_t zobrist = 0;

    // Step 1: loop over all piece types and add their zobrist codes
    bitboard_t remainingPieces;
    bitboard_t squareBB;
    square_t square;
    for (piece_t piece = pcs::PAWN; piece <= pcs::KING; piece++) {
        remainingPieces = pieceTypes[piece];
        while (remainingPieces) {
            squareBB = remainingPieces & -remainingPieces;
            remainingPieces -= squareBB;
            square = log2ll(squareBB);
            for (side_t side = 0; side < 2; side++) {
                if (colors[side] & squareBB)
                    zobrist ^= zb::getPieceZobrist(square,side,piece);
            } // end for loop over side
        } // end while remaining pieces
    } // end for loop over piece type

    // Step 2: add rights zobrist
    zobrist ^= zb::getRightsZobrist(epCastlingRights);

    // Step 3: Add stm zobrist
    zobrist ^= zb::stmZobrist * stm;

    // Step 4: Return the zobrist code
    return zobrist;
}

zobrist_t ChessBoard::getZobristCode() const {
    return zobristCode;
}

eval_t ChessBoard::getEval() const {
    // Step 0: initialize eval and phase
    packed_eval_t packedEval = 0;
    phase_t phase = 0;

    // Step 2: Loop through all the piece types, adding up their values
    // We don't add the king's material value since every side always has exactly 1 king
    for (piece_t piece = pcs::PAWN; piece <= pcs::KING; piece++) {
        for (side_t side = 0; side < 2; side++) {
            bitboard_t remainingPieces = pieceTypes[piece] & colors[side];
            phase += hce::PHASE_PIECE_VALUES[piece] * std::popcount(remainingPieces);
            bitboard_t squareBB;
            square_t square;
            while (remainingPieces) {
                squareBB = remainingPieces & -remainingPieces;
                remainingPieces -= squareBB;
                square = log2ll(squareBB);
                if (side == sides::WHITE)
                    packedEval += hce::real_psts[piece][square];
                else
                    packedEval -= hce::real_psts[piece][square ^ 7];
            } // end while remainingPieces
        } // end for loop over side
    } // end for loop over piece type

    // Step 3: Unpack the eval
    eval_t whiteRelativeEval = hce::evalFromPacked(packedEval, phase);

    // Step 4: Return the eval from the perspective of stm
    return (stm == sides::WHITE) ? whiteRelativeEval : -whiteRelativeEval;
}