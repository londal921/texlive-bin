#include "exkana.h"

char AKASATANA[]={
	0xa4,0xa2, /* あ */
	0xa4,0xab, /* か */
	0xa4,0xb5, /* さ */
	0xa4,0xbf, /* た */
	0xa4,0xca, /* な */
	0xa4,0xcf, /* は */
	0xa4,0xde, /* ま */
	0xa4,0xe4, /* や */
	0xa4,0xe9, /* ら */
	0xa4,0xef, /* わ */
	0
};
char AKASATANAutf8[]={
	0xe3,0x81,0x82, /* あ */
	0xe3,0x81,0x8b, /* か */
	0xe3,0x81,0x95, /* さ */
	0xe3,0x81,0x9f, /* た */
	0xe3,0x81,0xaa, /* な */
	0xe3,0x81,0xaf, /* は */
	0xe3,0x81,0xbe, /* ま */
	0xe3,0x82,0x84, /* や */
	0xe3,0x82,0x89, /* ら */
	0xe3,0x82,0x8f, /* わ */
	0
};
char *akasatana;

char AIUEO[]={
	0xa4,0xa2, /* あ */
	0xa4,0xa4, /* い */
	0xa4,0xa6, /* う */
	0xa4,0xa8, /* え */
	0xa4,0xaa, /* お */
	0xa4,0xab, /* か */
	0xa4,0xad, /* き */
	0xa4,0xaf, /* く */
	0xa4,0xb1, /* け */
	0xa4,0xb3, /* こ */
	0xa4,0xb5, /* さ */
	0xa4,0xb7, /* し */
	0xa4,0xb9, /* す */
	0xa4,0xbb, /* せ */
	0xa4,0xbd, /* そ */
	0xa4,0xbf, /* た */
	0xa4,0xc1, /* ち */
	0xa4,0xc4, /* つ */
	0xa4,0xc6, /* て */
	0xa4,0xc8, /* と */
	0xa4,0xca, /* な */
	0xa4,0xcb, /* に */
	0xa4,0xcc, /* ぬ */
	0xa4,0xcd, /* ね */
	0xa4,0xce, /* の */
	0xa4,0xcf, /* は */
	0xa4,0xd2, /* ひ */
	0xa4,0xd5, /* ふ */
	0xa4,0xd8, /* へ */
	0xa4,0xdb, /* ほ */
	0xa4,0xde, /* ま */
	0xa4,0xdf, /* み */
	0xa4,0xe0, /* む */
	0xa4,0xe1, /* め */
	0xa4,0xe2, /* も */
	0xa4,0xe4, /* や */
	0xa4,0xe6, /* ゆ */
	0xa4,0xe8, /* よ */
	0xa4,0xe9, /* ら */
	0xa4,0xea, /* り */
	0xa4,0xeb, /* る */
	0xa4,0xec, /* れ */
	0xa4,0xed, /* ろ */
	0xa4,0xef, /* わ */
	0xa4,0xf0, /* ゐ */
	0xa4,0xf1, /* ゑ */
	0xa4,0xf2, /* を */
	0xa4,0xf3, /* ん */
	0
};
char *aiueo;

char ONBIKI[]={0xa1,0xbc,0}; /* ー */
char SPACE[]={0xa1,0xa1,0}; /* 全角スペース */
char ALPHAEND[]={0xa3,0xfa,0}; /* ｚ */
char HIRATOP[]={0xa4,0xa1,0}; /* ぁ */
char HIRAEND[]={0xa4,0xf3,0}; /* ん */
char KATATOP[]={0xa5,0xa1,0}; /* ァ */
char KATAEND[]={0xa5,0xf6,0}; /* ヶ */
