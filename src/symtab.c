/*
    Tiny Language Compiler (tlc)

    シンボルテーブル / symbol table

    2016年 木村啓二
*/

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  "arch_common.h"
#include  "ast.h"
#include  "util.h"
#include  "symtab.h"

/* 現在処理関数のシンボルテーブル 
   symbol table of the current processing function */
SymTab current_symtab;

/* 各関数のシンボルテーブルを納める領域のポインタ
   Root of the symbol tables */
SymTab **symtab_array;

/* 関数名のテーブルの先頭（先頭はダミー）/ Head of the symbol table for function names */
SymTab func_symtab;

/* 登録済み関数idの最大値 / maximum id number of the registered functions */
int  max_id;

/* symtab_arrayのサイズ / size of symtab_array */
int  size_symtab_array;

/* 変数identを型typeで現在処理関数のシンボルテーブルに追加する
   既に登録済みなら0を返す
   Add variable ident to the symbol table with its type.
   If it has been already registered, the function return0 0. */
int
append_sym(int type, int symkind, char *ident)
{
    int  ok = 1;
    SymTab *t = NULL;

    if (symkind == SYM_NONE) {
        errexit("Illegal symbol kind.\n", __FILE__, __LINE__);
    } else if (symkind == SYM_FUNC) {
        t = &func_symtab;
    } else {
        t = &current_symtab;
    }
    for ( ; t->next != NULL; t = t->next) {
        if (strcmp(ident, t->next->ident) == 0) {
            ok = 0;
            break;
        }
    }
    if (ok) {
        t->next = xcalloc(1, sizeof(SymTab));
        t->next->type = type;
        t->next->kind = symkind;
        t->next->entry = t->entry+1;
        if ((t->next->ident = strdup(ident)) == NULL) {
            fprintf(stderr, "Not enough memory for strdup.\n");
            abort();
        }
    }
    return ok;
}

/* idで識別される関数のシンボルテーブルより変数identを探す
   idが0の時は現在処理関数
   存在したらそのエントリーのポインタを返す。なければNULL
   Look up a variable ident in the symbol table identified by id.
   id of 0 stands for the current processing function.
   This function returns the pointer of the entry if exists, otherwise NULL.
 */
SymTab*
lookup_sym(int id, int symkind, char *ident)
{
    SymTab *t;
    if (symkind == SYM_NONE) {
        errexit("Illegal symbol kind.\n", __FILE__, __LINE__);
    }
    if (id == 0) {
        if (symkind == SYM_FUNC) {
            t = func_symtab.next;
        } else {
            t = current_symtab.next;
        }
    } else if (id <= max_id) {
        if (symkind == SYM_FUNC) {
            errexit("Illegal symbol kind (for functions).\n",
                    __FILE__, __LINE__);
        }
        t = symtab_array[id];
    } else {
        fprintf(stderr, "Illegal function id(%d).\n", id);
        abort();
    }
    for (; t != NULL; t = t->next) {
        if (strcmp(t->ident, ident) == 0) {
            break;
        }
    }
    return t;
}

/* 現在処理関数をid(1以上)で識別される関数のシンボルテーブルとして登録する
   Register a symboltable with id of a function
 */
#define CHUNK 10

void
commit_current_symtab(int id)
{
    if (id == 0) {
        fprintf(stderr, "Illegal id number.\n");
        abort();
    }
    if (id >= size_symtab_array) {
        size_symtab_array
            = (id > size_symtab_array+CHUNK) ? id : size_symtab_array+CHUNK;
        symtab_array
            = xrealloc(symtab_array, size_symtab_array*sizeof(SymTab*));
    }
    if (max_id < id) {
        max_id = id;
    }
    symtab_array[id] = current_symtab.next;
    current_symtab.next = NULL;
}

/*
  自動変数のオフセット（の絶対値）の最大値を返す
  すでにold %ebpの分のカウントがしてある
  上記の整列補正のためのpad数計算はコード生成側(cg.c)で行う
  Return the maximum size of offset
  It has already includes old %ebp.
  The adjustment explained above is performed in cg.c.
*/
int
get_frame_size(int id)
{
    int  maxo;
    SymTab *t;
    if (id == 0) {
        t = current_symtab.next;
    } else if (id <= max_id) {
        t = symtab_array[id];
    } else {
        fprintf(stderr, "Illegal function id(%d).\n", id);
        abort();
    }
    maxo = 0;
    for (; t != NULL; t = t->next) {
        if ((t->kind == SYM_AUTOVAR || t->kind == SYM_ARG)
            && maxo < -t->offset) {
            maxo = -t->offset;
        }
    }
    return maxo;
}

/*
  変数のスタック上の位置（オフセット）を決める。
  Decide the layout in the stack (offset).
*/
void
assign_memory(void)
{
    int i;

    for (i = 1; i <= max_id; i++) {
        arch_assign_memory(symtab_array[i]);
    }
}

void
dump_symtab(void)
{
    int i;
    SymTab  *t;

    fputs("FuncTab\n", stderr);
    for (t = func_symtab.next; t != NULL; t = t->next) {
        fprintf(stderr, " %s #%d\n", t->ident, t->entry);
    }
    fputs("\nSymTab\n", stderr);
    for (i = 1; i <= max_id; i++) {
        fprintf(stderr, "id(%d)\n", i);
        for (t = symtab_array[i]; t != NULL; t = t->next) {
            fprintf(stderr, " %s #%d, offset(%d)\n",
                    t->ident, t->entry, t->offset);
	}
    }
}
