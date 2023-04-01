#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include <vector>
#include <bitset>
#include <windows.h>
#include <cctype>

#include "constants.h"
#include "bitboard.h"

#include "king.h"
#include "knight.h"
#include "pawn.h"
#include "slider.h"
#include "magic.h"

#include "evaluation.h"

using namespace std;

struct gameState
{
    bool hasKingMoved[2];
    bool hasKingSideRookMoved[2];
    bool hasQueenSideRookMoved[2];
    int enPassantSquare;
};

struct moveInfo
{
    int pieceType;
    int startSquare;
    int finishSquare;
    bool enPassant;
    int capturedPieceType;
    int promotionPieceType;
};

class Board {
    public:
        bool turn=0;

        U64 pieces[12]={};

        U64 occupied[2]={0,0};
        U64 attacked[2]={0,0};

        vector<gameState> stateHistory;
        vector<moveInfo> moveHistory;

        vector<moveInfo> moveBuffer;

        gameState current = {
            .hasKingMoved={false,false},
            .hasKingSideRookMoved={false,false},
            .hasQueenSideRookMoved={false,false},
        };

        const int _nKing=0;
        const int _nQueens=2;
        const int _nRooks=4;
        const int _nBishops=6;
        const int _nKnights=8;
        const int _nPawns=10;

        Board()
        {
            //default constructor for regular games.
            pieces[_nKing]=WHITE_KING;
            pieces[_nKing+1]=BLACK_KING;

            pieces[_nQueens]=WHITE_QUEENS;
            pieces[_nQueens+1]=BLACK_QUEENS;

            pieces[_nRooks]=WHITE_ROOKS;
            pieces[_nRooks+1]=BLACK_ROOKS;

            pieces[_nBishops]=WHITE_BISHOPS;
            pieces[_nBishops+1]=BLACK_BISHOPS;

            pieces[_nKnights]=WHITE_KNIGHTS;
            pieces[_nKnights+1]=BLACK_KNIGHTS;

            pieces[_nPawns]=WHITE_PAWNS;
            pieces[_nPawns+1]=BLACK_PAWNS;

            updateOccupied();
            updateAttacked(0); updateAttacked(1);
        };

        void appendMove(int pieceType, int startSquare, int finishSquare)
        {
            //will automatically detect promotion and generate all permutations.
            //will automatically detect captured piece (including en passant).
            moveInfo newMove = {
                .pieceType=pieceType,
                .startSquare=startSquare,
                .finishSquare=finishSquare,
                .enPassant=false,
                .capturedPieceType=-1,
                .promotionPieceType=-1,
            };

            //check for captured pieces (including en passant).
            if ((convertToBitboard(finishSquare) & occupied[(pieceType+1) & 1])!=0)
            {
                //check for captures.
                U64 x = convertToBitboard(finishSquare);
                for (int i=_nQueens+((pieceType+1) & 1);i<12;i+=2)
                {
                    if ((x & pieces[i]) != 0)
                    {
                        newMove.capturedPieceType=i;
                        break;
                    }
                }
            }
            else if ((pieceType >> 1 == _nPawns >> 1 && finishSquare >> 3 == 5-3*(pieceType & 1)) && (finishSquare%8 != startSquare%8))
            {
                //en-passant capture.
                newMove.enPassant=true;

                //check for captured piece.
                U64 x = convertToBitboard(finishSquare-8+16*(pieceType%2));
                for (int i=_nQueens+((pieceType+1) & 1);i<12;i+=2)
                {
                    if ((x & pieces[i]) != 0)
                    {
                        newMove.capturedPieceType=i;
                        break;
                    }
                }
            }

            //check for pawn promotion.
            if (pieceType >> 1 == _nPawns >> 1 && finishSquare >> 3 == 7-7*(pieceType & 1))
            {
                //promotion.
                for (int i=_nQueens+(pieceType & 1);i<_nPawns;i+=2)
                {
                    newMove.promotionPieceType=i;
                    moveBuffer.push_back(newMove);
                }
            }
            else
            {
                //append normally.
                moveBuffer.push_back(newMove);
            }
        }

        void updateOccupied()
        {
            occupied[0] = pieces[_nKing] | pieces[_nQueens] | pieces[_nRooks] | pieces[_nBishops] | pieces[_nKnights] | pieces[_nPawns];
            occupied[1] = pieces[_nKing+1] | pieces[_nQueens+1] | pieces[_nRooks+1] | pieces[_nBishops+1] | pieces[_nKnights+1] | pieces[_nPawns+1];
        }

        void updateAttacked(bool side)
        {
            //king.
            attacked[(int)(side)] = kingAttacks(pieces[_nKing+(int)(side)]);

            //queen.
            U64 b = occupied[0] | occupied[1];
            U64 temp = pieces[_nQueens+(int)(side)];
            while (temp)
            {
                attacked[(int)(side)] |= magicQueenAttacks(b,popLSB(temp));
            }

            //rooks.
            temp = pieces[_nRooks+(int)(side)];
            while (temp)
            {
                attacked[(int)(side)] |= magicRookAttacks(b,popLSB(temp));
            }

            //bishops.
            temp = pieces[_nBishops+(int)(side)];
            while (temp)
            {
                attacked[(int)(side)] |= magicBishopAttacks(b,popLSB(temp));
            }

            //knights.
            attacked[(int)(side)] |= knightAttacks(pieces[_nKnights+(int)(side)]);

            //pawns.
            attacked[(int)(side)] |= pawnAttacks(pieces[_nPawns+(int)(side)],(int)(side));
        }

        bool isInCheck(bool side)
        {
            //check if the king's square is attacked.
            int kingPos = __builtin_ctzll(pieces[_nKing+(int)(side)]);
            U64 b = occupied[0] | occupied[1];

            bool inCheck = kingAttacks(pieces[_nKing+(int)(side)]) & pieces[_nKing+(int)(!side)];
            inCheck |= magicRookAttacks(b,kingPos) & (pieces[_nRooks+(int)(!side)] | pieces[_nQueens+(int)(!side)]);
            inCheck |= magicBishopAttacks(b,kingPos) & (pieces[_nBishops+(int)(!side)] | pieces[_nQueens+(int)(!side)]);
            inCheck |= knightAttacks(pieces[_nKing+(int)(side)]) & pieces[_nKnights+(int)(!side)];
            inCheck |= pawnAttacks(pieces[_nKing+(int)(side)],side) & pieces[_nPawns+(int)(!side)];

            return inCheck;
        }

        pair<bool,bool> canCastle(bool side)
        {
            pair<bool,bool> res={true,true};

            //king and rook must not have previously moved.
            res.first=!current.hasKingSideRookMoved[(int)(side)] && !current.hasKingMoved[(int)(side)];
            res.second=!current.hasQueenSideRookMoved[(int)(side)] && !current.hasKingMoved[(int)(side)];

            //rooks must not have been captured.
            res.first &= (bool)(pieces[_nRooks+(int)(side)] & KING_ROOK_POS[(int)(side)]);
            res.second &= (bool)(pieces[_nRooks+(int)(side)] & QUEEN_ROOK_POS[(int)(side)]);

            //no pieces between king and rook.
            res.first &= !(bool)(KING_CASTLE_OCCUPIED[(int)(side)] & (occupied[0] | occupied[1]));
            res.second &= !(bool)(QUEEN_CASTLE_OCCUPIED[(int)(side)] & (occupied[0] | occupied[1]));

            //king squares cannot be under attack.
            res.first &= !(bool)(KING_CASTLE_ATTACKED[(int)(side)] & attacked[(int)(!side)]);
            res.second &= !(bool)(QUEEN_CASTLE_ATTACKED[(int)(side)] & attacked[(int)(!side)]);

            return res;
        }

        void display()
        {
            //display the current position in console.

            const char symbols[12]={'K','k','Q','q','R','r','B','b','N','n','P','p'};

            HANDLE hConsole=GetStdHandle(STD_OUTPUT_HANDLE);

            const int colours[2]={7,8};
            const int standardColour=15;

            vector<vector<string> > grid;
            for (int i=0;i<8;i++)
            {
                grid.push_back(vector<string>());
                for (int j=0;j<8;j++)
                {
                    grid.back().push_back("[ ]");
                }
            }

            string temp=bitset<64>(0).to_string();
            for (int i=0;i<12;i++)
            {
                temp=bitset<64>(pieces[i]).to_string();
                for (int j=0;j<64;j++)
                {
                    if (temp[j]=='0') {continue;}

                    grid[(63-j)/8][(63-j)%8][1]=symbols[i];
                }
            }

            for (int i=7;i>=0;i--)
            {
                for (int j=0;j<8;j++)
                {
                    SetConsoleTextAttribute(hConsole,colours[(i+j)%2]);
                    cout << grid[i][j][0];
                    SetConsoleTextAttribute(hConsole,standardColour);

                    if (grid[i][j][1]!=' ')
                    {
                        if (isupper(grid[i][j][1])==true)
                        {
                            SetConsoleTextAttribute(hConsole,colours[0]);
                        }
                        else
                        {
                            SetConsoleTextAttribute(hConsole,colours[1]);
                        }
                        cout << char(toupper(grid[i][j][1]));

                        SetConsoleTextAttribute(hConsole,standardColour);
                    }
                    else {cout << ' ';}

                    SetConsoleTextAttribute(hConsole,colours[(i+j)%2]);
                    cout << grid[i][j][2];
                    SetConsoleTextAttribute(hConsole,standardColour);
                } cout << " " << i+1 << endl;
            }
            cout << " A  B  C  D  E  F  G  H" << endl;
        }

        void generatePseudoMoves(bool side)
        {
            //generate all pseudo-legal moves.
            //assume occupancy and attack bitboards are up-to-date.
            updateOccupied();
            updateAttacked(!turn);

            //king.

            //get position of king.
            int kingPos = __builtin_ctzll(pieces[_nKing+(int)(side)]);

            //get position of move-squares.
            U64 x = kingAttacks(pieces[_nKing+(int)(side)]);
            x &= ~attacked[!turn];
            x &= ~occupied[(int)(side)];

            while (x!=0)
            {
                int finishSquare = popLSB(x);
                appendMove(_nKing+(int)(side), kingPos, finishSquare);
            }

            //castling.
            pair<bool,bool> rights = canCastle(side);
            if (rights.first==true)
            {
                //kingside castle.
                appendMove(_nKing+(int)(side), kingPos, kingPos+2);
            }
            if (rights.second==true)
            {
                //queenside castle.
                appendMove(_nKing+(int)(side), kingPos, kingPos-2);
            }

            U64 p = (occupied[0] | occupied[1]);
            //queen.
            U64 queens = pieces[_nQueens+(int)(side)];

            while (queens!=0)
            {
                int queenPos = popLSB(queens);

                U64 x = magicQueenAttacks(p,queenPos);
                x &= ~occupied[(int)(side)];

                while (x!=0)
                {
                    int finishSquare = popLSB(x);
                    appendMove(_nQueens+(int)(side), queenPos, finishSquare);
                }
            }

            //rook.
            U64 rooks = pieces[_nRooks+(int)(side)];

            while (rooks!=0)
            {
                int rookPos = popLSB(rooks);

                U64 x = magicRookAttacks(p,rookPos);
                x &= ~occupied[(int)(side)];

                while (x!=0)
                {
                    int finishSquare = popLSB(x);
                    appendMove(_nRooks+(int)(side), rookPos, finishSquare);
                }
            }

            //bishops.
            U64 bishops = pieces[_nBishops+(int)(side)];

            while (bishops!=0)
            {
                int bishopPos = popLSB(bishops);

                U64 x = magicBishopAttacks(p,bishopPos);
                x &= ~occupied[(int)(side)];

                while (x!=0)
                {
                    int finishSquare = popLSB(x);
                    appendMove(_nBishops+(int)(side), bishopPos, finishSquare);
                }
            }

            //knights.
            U64 knights = pieces[_nKnights+(int)(side)];

            while (knights!=0)
            {
                int knightPos = popLSB(knights);

                U64 x = knightAttacks(convertToBitboard(knightPos));
                x &= ~occupied[(int)(side)];

                while (x!=0)
                {
                    int finishSquare = popLSB(x);
                    appendMove(_nKnights+(int)(side), knightPos, finishSquare);
                }
            }

            //pawns.
            U64 pawns = pieces[_nPawns+(int)(side)];

            while (pawns!=0)
            {
                int pawnPos = popLSB(pawns);
                U64 pawnPosBoard = convertToBitboard(pawnPos);

                //en passant square included.
                U64 x = pawnAttacks(pawnPosBoard,side);
                x &= ~occupied[(int)(side)];

                if (current.enPassantSquare != -1)
                {
                    x &= (occupied[(int)(!side)] | convertToBitboard(current.enPassantSquare));
                }
                else
                {
                    x &= occupied[(int)(!side)];
                }

                //move forward.
                if (side==0)
                {
                    x |= (pawnPosBoard << 8) & (~p);
                    x |= ((((pawnPosBoard & FILE_2) << 8) & (~p)) << 8) & (~p);
                }
                else
                {
                    x |= (pawnPosBoard >> 8 & (~p));
                    x |= ((((pawnPosBoard & FILE_7) >> 8) & (~p)) >> 8) & (~p);
                }

                while (x!=0)
                {
                    int finishSquare = popLSB(x);
                    appendMove(_nPawns+(int)(side), pawnPos,finishSquare);
                }
            }
        }

        void movePieces(moveInfo currentMove)
        {
            //remove piece from start square;
            pieces[currentMove.pieceType] -= convertToBitboard(currentMove.startSquare);

            //add piece to end square, accounting for promotion.
            int finishPieceType = currentMove.promotionPieceType>=_nQueens ? currentMove.promotionPieceType : currentMove.pieceType;
            pieces[finishPieceType] += convertToBitboard(currentMove.finishSquare);

            //remove any captured pieces.
            if (currentMove.capturedPieceType!=-1)
            {
                pieces[currentMove.capturedPieceType] -= convertToBitboard(currentMove.finishSquare+(int)(currentMove.enPassant)*(-8+16*(currentMove.pieceType & 1)));
            }

            //if castles, then move the rook too.
            if (currentMove.pieceType >> 1 == _nKing >> 1 && abs(currentMove.finishSquare-currentMove.startSquare)==2)
            {
                if (currentMove.finishSquare-currentMove.startSquare==2)
                {
                    //kingside.
                    pieces[_nRooks+(currentMove.pieceType & 1)] -= KING_ROOK_POS[currentMove.pieceType & 1];
                    pieces[_nRooks+(currentMove.pieceType & 1)] += KING_ROOK_POS[currentMove.pieceType & 1] >> 2;
                }
                else
                {
                    //queenside.
                    pieces[_nRooks+(currentMove.pieceType & 1)] -= QUEEN_ROOK_POS[currentMove.pieceType & 1];
                    pieces[_nRooks+(currentMove.pieceType & 1)] += QUEEN_ROOK_POS[currentMove.pieceType & 1] << 3;
                }
            }
        }

        void unMovePieces(moveInfo currentMove)
        {
            //remove piece from destination square.
            int finishPieceType = currentMove.promotionPieceType>=_nQueens ? currentMove.promotionPieceType : currentMove.pieceType;
            pieces[finishPieceType] -= convertToBitboard(currentMove.finishSquare);

            //add piece to start square.
            pieces[currentMove.pieceType] += convertToBitboard(currentMove.startSquare);

            //add back captured pieces.
            if (currentMove.capturedPieceType!=-1)
            {
                pieces[currentMove.capturedPieceType] += convertToBitboard(currentMove.finishSquare+(int)(currentMove.enPassant)*(-8+16*(currentMove.pieceType & 1)));
            }

            //if castles move the rook back.
            if (currentMove.pieceType >> 1 == _nKing >> 1 && abs(currentMove.finishSquare-currentMove.startSquare)==2)
            {
                if (currentMove.finishSquare-currentMove.startSquare==2)
                {
                    //kingside.
                    pieces[_nRooks+(currentMove.pieceType & 1)] -= KING_ROOK_POS[currentMove.pieceType & 1] >> 2;
                    pieces[_nRooks+(currentMove.pieceType & 1)] += KING_ROOK_POS[currentMove.pieceType & 1];
                }
                else
                {
                    //queenside.
                    pieces[_nRooks+(currentMove.pieceType & 1)] -= QUEEN_ROOK_POS[currentMove.pieceType & 1] << 3;
                    pieces[_nRooks+(currentMove.pieceType & 1)] += QUEEN_ROOK_POS[currentMove.pieceType & 1];
                }
            }
        }

        bool makeMove(moveInfo currentMove)
        {
            //move pieces.
            movePieces(currentMove);

            //check if the move was legal.
            updateOccupied();
            //updateAttacked(!turn);

            //if ((pieces[_nKing+(int)(turn)] & attacked[(int)(!turn)]) != 0)
            if (isInCheck(turn))
            {
                //illegal move.
                unMovePieces(currentMove);
                return false;
            }
            else
            {
                //update history.
                stateHistory.push_back(current);
                moveHistory.push_back(currentMove);

                //update game-state.

                //if double-pawn push, set en-passant square.
                //otherwise, set en-passant square to -1.
                if (currentMove.pieceType >> 1 == _nPawns >> 1 && abs(currentMove.finishSquare-currentMove.startSquare) == 16)
                {
                    //en-passant is possible immediately.
                    current.enPassantSquare = currentMove.finishSquare-8+16*(currentMove.pieceType & 1);
                }
                else
                {
                    current.enPassantSquare = -1;
                }

                turn = !turn;
                current.hasKingMoved[currentMove.pieceType & 1] |= (currentMove.pieceType >> 1 == _nKing >> 1);
                current.hasKingSideRookMoved[currentMove.pieceType & 1] |= ((currentMove.pieceType >> 1) == (_nRooks >> 1)) && (currentMove.startSquare == (7 + 56 * (currentMove.pieceType & 1)));
                current.hasQueenSideRookMoved[currentMove.pieceType & 1] |= ((currentMove.pieceType >> 1) == (_nRooks >> 1)) && (currentMove.startSquare == (0 + 56 * (currentMove.pieceType & 1)));

                return true;
            }
        }

        void unmakeMove()
        {
            //unmake most recent move and update gameState.
            turn = !turn;
            current = stateHistory.back();
            unMovePieces(moveHistory.back());

            stateHistory.pop_back();
            moveHistory.pop_back();
        }

        int evalPieces(int pieceType)
        {
            int total=0;
            U64 temp = pieces[pieceType] ^ (63ull * ((U64)pieceType & 1ull));
            while (temp)
            {
                //only mid-game eval at the moment.
                total += PIECE_VALUES_START[pieceType >> 1] + PIECE_TABLES_START[pieceType >> 1][popLSB(temp)];
            }
            return total;
        }
};

#endif // BOARD_H_INCLUDED
