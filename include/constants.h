#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

typedef unsigned long long U64;
typedef unsigned int U32;

const U64 WHITE_KING=16ull;
const U64 WHITE_QUEENS=8ull;
const U64 WHITE_ROOKS=129ull;
const U64 WHITE_BISHOPS=36ull;
const U64 WHITE_KNIGHTS=66ull;
const U64 WHITE_PAWNS=65280ull;

const U64 BLACK_KING=1152921504606846976ull;
const U64 BLACK_QUEENS=576460752303423488ull;
const U64 BLACK_ROOKS=9295429630892703744ull;
const U64 BLACK_BISHOPS=2594073385365405696ull;
const U64 BLACK_KNIGHTS=4755801206503243776ull;
const U64 BLACK_PAWNS=71776119061217280ull;

const U64 NOT_A_FILE=0xfefefefefefefefe;
const U64 NOT_H_FILE=0x7f7f7f7f7f7f7f7f;
const U64 NOT_AB_FILE=~(217020518514230019ull);
const U64 NOT_GH_FILE=~(13889313184910721216ull);

const U64 FILE_2=65280ull;
const U64 FILE_7=71776119061217280ull;

const U64 KING_CASTLE_OCCUPIED[2]={96ull,6917529027641081856ull};
const U64 QUEEN_CASTLE_OCCUPIED[2]={14ull,1008806316530991104ull};

const U64 KING_CASTLE_ATTACKED[2]={112ull,8070450532247928832ull};
const U64 QUEEN_CASTLE_ATTACKED[2]={28ull,2017612633061982208ull};

const U64 KING_ROOK_POS[2]={128ull,9223372036854775808ull};
const U64 QUEEN_ROOK_POS[2]={1ull,72057594037927936ull};
const int KING_ROOK_SQUARE[2]={7,63};
const int QUEEN_ROOK_SQUARE[2]={0,56};

//MOVE INFO - unsigned int (U32)

const U32 MOVEINFO_PIECETYPE_MASK = 15;
const U32 MOVEINFO_PIECETYPE_OFFSET = 0;

const U32 MOVEINFO_STARTSQUARE_MASK = 1008;
const U32 MOVEINFO_STARTSQUARE_OFFSET = 4;

const U32 MOVEINFO_FINISHSQUARE_MASK = 64512;
const U32 MOVEINFO_FINISHSQUARE_OFFSET = 10;

const U32 MOVEINFO_ENPASSANT_MASK = 65536;
const U32 MOVEINFO_ENPASSANT_OFFSET = 16;

const U32 MOVEINFO_CAPTUREDPIECETYPE_MASK = 1966080;
const U32 MOVEINFO_CAPTUREDPIECETYPE_OFFSET = 17;

const U32 MOVEINFO_FINISHPIECETYPE_MASK = 31457280;
const U32 MOVEINFO_FINISHPIECETYPE_OFFSET = 21;

const U32 MOVEINFO_SHOULDCHECK_MASK = 33554432;
const U32 MOVEINFO_SHOULDCHECK_OFFSET = 25;

#endif // CONSTANTS_H_INCLUDED
