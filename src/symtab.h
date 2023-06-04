/*
    Tiny Language Compiler (tlc)

    シンボルテーブル / symbol table

    2016年 木村啓二
*/

#ifndef  SYMTAB_H
#define  SYMTAB_H

/* 変数の種別 / variable kinds */
enum {
    SYM_NONE,
    SYM_FUNC,			/* 関数 / function  */
    SYM_VAR,			/* 変数全般（仮引数+自動変数）/ variables (formal parameters+auto variables) */
    SYM_ARG,			/* 仮引数 / formal parameters */
    SYM_AUTOVAR			/* 自動変数 / auto variables */
};

typedef struct SymTab {
    int  entry;   /* エントリー番号 / entry id */
    int  kind;    /* 変数種別 / variable kind */
    int  offset;  /* メモリ領域（現在はスタックフレーム）中のオフセット
                     offset of memory (currently stack frame) */
    int  argid;   /* 引数だった場合のid（1から。0なら引数ではない）
                     id when this is a parameter
                     (numbering from 1, 0 means it's not a parameter */
    int  type;	  /* 変数型（現在はintのみ) / type (currently only int) */
    char  *ident; /* 変数名 / variable name (identifier) */
    struct SymTab *next;
} SymTab;

/* 変数identを型typeで現在処理関数のシンボルテーブルに追加する
   既に登録済みなら0を返す
   Add variable ident to the symbol table with its type.
   If it has been already registered, the function return0 0. */
extern  int  append_sym(int type, int symkind, char *ident);

/* idで識別される関数のシンボルテーブルより変数identを探す
   idが0の時は現在処理関数
   存在したらそのエントリーのポインタを返す。なければNULL
   Look up a variable ident in the symbol table identified by id.
   id of 0 stands for the current processing function.
   This function returns the pointer of the entry if exists, otherwise NULL.
 */
extern  SymTab  *lookup_sym(int id, int symkind, char *ident);

/* 現在処理関数をid(1以上)で識別される関数のシンボルテーブルとして登録する
   Register a symboltable with id of a function
*/
extern  void commit_current_symtab(int id);

/* 読み出された関数で必要とするスタックフレームのサイズを返す
   Retruns the stack frame size
*/
extern  int get_frame_size(int id);

/* メモリの割り付け / memory assignment  */
extern  void assign_memory(void);

extern  void dump_symtab(void);

#endif	/* SYMTAB_H */
