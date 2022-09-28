
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif
 {
	char *sval;
	struct {
		int isimpl;
		long int value;
	} oival;
	Location loc;
	SDBinfo *tree;
} YYSTYPE;
extern YYSTYPE yylval;
# define NUM 257
# define NNUM 258
# define FNUM 259
# define FSEARCH 260
# define BSEARCH 261
# define e_CMD 262
# define r_CMD 263
# define b_CMD 264
# define dm_CMD 265
# define cm_CMD 266
# define FINP_CMD 267
# define SHELL_CMD 268
# define SLASHFORM 269
# define EQUALFORM 270
# define QMARKFORM 271
# define PROCPREF 272
# define VNAME 273
# define PNAME 274
# define REGNAME 275
# define PLINENUM 276
# define FLINENUM 277
# define ARROW 278
# define STRING 279
# define SCHAR 280
# define COMMENT 281
