/*
    Tiny Language Compiler (tlc)

    アーキテクチャ依存部共通部分
    Common part of the architecture dependent files

    2021年 木村啓二
*/

#ifndef  ARCH_COMMON_H
#define  ARCH_COMMON_H

#include  <stdio.h>
#include  "ast.h"
#include  "symtab.h"

extern const char MAIN_LABEL[];
extern const char PUTINT_CODE[];
extern const char SECTION_TEXT[];
extern const char CALL_OP[];

extern char reg_name[][10];
extern char param_reg_name[][10];

extern void arch_assign_memory(SymTab *symtab);

extern void gen_func_header(FILE *out, char *name,
                            int frame_size, AST_List *arg_list);
extern void gen_store_params(FILE *out, AST_Node *param, int nump);
extern void gen_func_footer(FILE *out, const char *func_end_label);
extern void gen_exp_ident(FILE *out, AST_Node *idnt);
extern void gen_exp_cnst(FILE *out, AST_Node *c);
extern int  gen_call_prologue(FILE *out, AST_Node *e,
                              int *padsize, int *framesize);
extern void gen_call_set_param(FILE *out, int reg, int nump, int sparams);
extern void gen_call_epilogue(FILE *out, AST_Node *e,
                              int padsize, int framesize);

extern void gen_insn_store_lvar(FILE* out, int reg, int offset);
extern void gen_insn_neg(FILE* out, int dst, int src);
extern void gen_insn_add(FILE* out, int dst, int src1, int src2);
extern void gen_insn_sub(FILE* out, int dst, int src1, int src2);
extern void gen_insn_mul(FILE* out, int dst, int src1, int src2);
extern void gen_insn_ret_asgn(FILE *out, int src);
extern void gen_insn_jmp(FILE *out, const char *label);
extern void gen_insn_cmp(FILE* out, int src1, int src2);
extern void gen_insn_rel(FILE *out, int cond, const char *l_cmp, int reg);
extern void gen_insn_cond_set(FILE* out, int dst, int cond);

#endif  /* ARCH_COMMON_H */
