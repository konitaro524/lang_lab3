/*
    Tiny Language Compiler (tlc)

    ARM依存部部分
    ARM dependent part

    2021年 木村啓二
*/

#include  <assert.h>
#include  <limits.h>
#include  <string.h>
#include  "arch_common.h"
#include  "symtab.h"
#include  "util.h"

#if defined(TARGET_RASPI)
const char MAIN_LABEL[]   = "main";
const char PUTINT_CODE[]  = /* to be modified */
    "\t.section\t.rodata\n"
    ".LC0:\n"
    "\t.string \"%d\\n\"\n"
    "\t.text\n"
    "put_int:\n"
    "\tstp\tx29, x30, [sp, -32]!\n"
    "\tmov\tx29, sp\n"
    "\tstr\tw0, [sp, 28]\n"
    "\tldr\tw1, [sp, 28]\n"
    "\tadrp\tx0, .LC0\n"
    "\tadd\tx0, x0, :lo12:.LC0\n"
    "\tbl\tprintf\n"
    "\tnop\n"
    "\tldp\tx29, x30, [sp], 32\n"
    "\tret\n";
const char SECTION_TEXT[] =  "\t.text\n";
#elif defined(TARGET_AMAC)
const char MAIN_LABEL[]   = "_main";
const char PUTINT_CODE[]  =
    "\t.text\n"
    "\t.p2align 2\n"
    ".LC0:\n"
    "\t.string \"%d\\n\"\n"
    "\t.text\n"
    "\t.p2align 2\n"
    "put_int:\n"
    "\tsub\tsp, sp, #32\n"
    "\tstp\tx29, x30, [sp, #16]\n"
    "\tadd\tx29, sp, #16\n"
    "\tstur\tw0, [x29, #-4]\n"
    "\tldur\tw9, [x29, #-4]\n"
    "\tmov\tx8, x9\n"
    "\tadrp\tx0, .LC0@PAGE\n"
    "\tadd\tx0, x0, .LC0@PAGEOFF\n"
    "\tmov\tx9, sp\n"
    "\tstr\tx8, [x9]\n"
    "\tbl\t_printf\n"
    "\tldp\tx29, x30, [sp, #16]\n"
    "\tadd\tsp, sp, #32\n"
    "\tret\n";
const char SECTION_TEXT[] =  "\t.text\n\t.p2align 2\n";
#endif
const char CALL_OP[]      =  "bl";


char reg_name[][10] = {"w8", "w9", "w10"};
char param_reg_name[][10] = {"NULL", "w0", "w1", "w2", "w3", "w4", "w5",
                             "w6", "w7" };

/* tlcにおけるARM (64bit)スタックレイアウトメモ
   note for ARM(64bit) stack layout in tlc
   （例/example）
   int func(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
   { int v1, v2, v3; ... }

   最初の8つの引数はレジスタを使って渡される
   -> 1番目から順にw0(1), w1(2), w2(3), w3(4), w4(5), w5(6), w6(7), w7(8)
      9つめ以降はspに近い順から8バイトごとにスタックに保存される
   -> 関数呼び出し後、レジスタで渡された値もスタックに保存される
      レイアウトは以下の図のようになる
   The first six parameters are passed through registers.
   -> From the first parameter, they are passed by
      w0(1), w1(2), w2(3), w3(4), w4(5), w5(6), w6(7), and w7(8), respectively.
      9th and more parameters are once saved in the stack
      each of which occupies 8 bytes.
      The order of them starts from the address pointed by %rsp.
   -> After control is transferred to the function body,
      all parameters are saved in the stack.
      The following figure depicts a stack layout.

(higher address)
... |          | old spを16byte整列にするための隙間(pad)
    |          |
    | Old x30' |  the frame for the previous function
    | Old x29' |<- old sp 
    |  pad     |
    |   w0     |
    |   w1     |関数呼び出し前に保存 / save before function call
    |   w2     |
------------------ <- x29 (old sp just before function call)
    |          |
 -8 |   a10    |
    |          |
-16 |    a9    |
-20 |    v3    |
-24 |    v2    |
-28 |    v1    |
-32 |    a1    |
-36 |    a2    |
-40 |    a3    |
-44 |    a4    |
-48 |    a5    |
-52 |    a6    |
... |          |
    |  old -   | 
sp+8|   x30    |
    |  old -   |
sp+0|   x29    |<- spは16バイト境界に調整 
    |          |
(lower address)


上記の状態を作るため、
- 関数呼び出し直前にw8,w9,w10と実引数用に16byte整列番地にspを合わせる
-> パッドの数をPa、実引数の9つめ以降の数とNa2とすると(Pa+2*Na2+3)*4%16を0にする
    （(Pa+2*Na2+3)%4を0）
- 関数呼び出し直後に、自動変数用とレジスタ渡し引数用の領域確保のため、16byte整列したspを計算
-> パッドの数をPv、自動変数の数をNv、8つめまでの引数の数をNa1とすると
    (Pv+Nv+Na1)*4%16を0にする ((Pv+Nv+Na1)%4を0)
    さらにこのあと保存するx29 (2*4byte), x30 (2*4byte)の分の16byteを足す
- その後、old x29, x30をスタックトップに保存し、old spをx29とする
To make the situation above:
- Before the function call, adjust the sp on the 16byte memory alignment
  for w8, w9, w10, and actual parameters.
-> For adjustment, the following equation is used:
    (Pa+2*Na2+3)%4 = 0
    where Pa is the number of pad,
    Na is the number of actual parameters exceeding 9.
- Then, adjust the sp on the 16byte memory alignment for auto variables
  and parameters passed through registers.
-> For adjustment, the following equation is used:
    (Pv+Nv+Na)*4%16 = 0
    where Pv is the number of pad, Nv is the number of auto variables,
    and Na1 is the number of parameters up to 8.
    Further, 16 bytes for x29 and x30 to be stored at the next step are added.
- old x29 and x30 are pushed, and old sp+16 is stored in x29.
*/

/*
  変数のスタック上の位置（オフセット）を決める。
  引数の数: narg
  自動変数の数: nvar
  引数のid番号（1から）: aid
  変数のid番号（1から）: vid
  - 引数(9-)
    x29+0から8byteごと
    (narg-aid+1)*(-8)
  - 自動変数
    引数（9-）から順に
    (narg-8)*(-8)+(nvar-vid+1)*(-4)
  - 引数(1-8)
    自動変数の手前
    (narg-8)*(-8)+nvar*(-4)+aid*(-4)
  Decide the layout in the stack (offset).
  See the figure above.
*/

void
arch_assign_memory(SymTab *symtab)
{
    int id_arg, id_var, poffset, voffset;
    SymTab *t;

    id_arg = 0; id_var = 0;
    for (t = symtab; t != NULL; t = t->next) {
        /* 変数のサイズはint 4byteで固定 */
        if (t->kind == SYM_ARG) {
            t->argid = ++id_arg;
        } else if (t->kind == SYM_AUTOVAR) {
            t->offset = ++id_var;
            t->argid = 0;
        }
    }
    poffset = (id_arg > 8) ? (id_arg-8)*(-8) : 0;
    voffset = (id_var > 0) ? id_var*(-4) : 0;
    for (t = symtab; t != NULL; t = t->next) {
        if (t->kind == SYM_ARG) {
            if (t->argid < 9) {
                t->offset = poffset+voffset+t->argid*(-4);
            } else {
                t->offset = (id_arg-t->argid+1)*(-8);
            }
        } else if (t->kind == SYM_AUTOVAR) {
            t->offset = poffset+(id_var-t->offset+1)*(-4);
        }
    }
}

static int current_frame_size;

void
gen_func_header(FILE *out, char *name, int frame_size, AST_List *arg_list)
{
    const char *targetn = name;
    AST_List *l;
    int i;
    int pad;

    /* 整列補正用のpad計算。上記のスタックに関するメモを参照
       frame_sizeはメモ中の(Nv+Na1)*4に等しい
       Calculate pad for memory alignment.
       Please look at the note above.
       frame_size is equal to (Nv+Na1)*4 in the note.
    */
    pad = 16 - frame_size%16;
    if (pad == 16) {
        pad = 0;
    }
    current_frame_size = frame_size+pad+16;
    if (strcmp(name, "main") == 0) {
        targetn = MAIN_LABEL;
    }
    fprintf(out,
            "\t.global\t%s\n"
            "%s:\n", targetn, targetn);
    fprintf(out, "\tstp\tx29, x30, [sp, -%d]!\n", current_frame_size);
    fprintf(out, "\tadd\tx29, sp, %d\n", current_frame_size);
    i = 0;
    TRAVERSE_AST_LIST(l, arg_list, gen_store_params(out, l->elem, ++i));
}

void
gen_store_params(FILE *out, AST_Node *param, int nump)
{
    if (nump < 9) {
        AST_Node *pid = param->child[0];
        assert(pid != NULL && pid->symtab != NULL);
        fprintf(out, "\tstr\t%s, [x29, %d]\n",
                param_reg_name[nump], pid->symtab->offset);
    }
}

void
gen_func_footer(FILE *out, const char *func_end_label)
{
    fprintf(out, "%s:\n"
            "\tldp\tx29, x30, [sp], %d\n"
            "\tret\n\n", func_end_label, current_frame_size);
}

void
gen_exp_cnst(FILE *out, AST_Node *c)
{
    if (c->val > SHRT_MAX || c->val < SHRT_MIN) { /* over 16bits */
        fprintf(out,
                "\tmov\t%s, %x\n"
                "\tmovk\t%s, %x, lsl 16\n",
                reg_name[c->reg], 0xffff & c->val,
                reg_name[c->reg], (0xffff0000 & c->val) >> 16);
    } else {
        fprintf(out, "\tmov\t%s, %d\n", reg_name[c->reg], c->val);
    }
}

void
gen_exp_ident(FILE *out, AST_Node *idnt)
{
    fprintf(out, "\tldr\t%s, [x29, %d]\n",
            reg_name[idnt->reg], idnt->symtab->offset);
}

int
gen_call_prologue(FILE *out, AST_Node *e, int *padsize, int *framesize)
{
    int i, psize, fsize, pad, sparams;
    AST_List *l;
    
    sparams = 0;
    TRAVERSE_AST_LIST(l, e->list, ++sparams);
    /* spの整列補正。上記のスタックに関するメモを参照
       Adjust sp for alignment. See the note about stack above. */
    sparams = (sparams > 8) ? sparams - 8 : 0;
    pad = 4; /* pad is always 4 for arm64 */
    psize = sparams * 8;
    fsize = pad+3*4; /* w0, w1, and w2 are all int(4byte). */

    /* padと待避するレジスタの分だけspをずらす
       Adjust sp by pad and saved registers */
    fprintf(out, "\tsub\tsp, sp, #%d\n", fsize);
    for (i = 0; i < 3; i++) {
        if (e->reg != i) {
            fprintf(out, "\tstr\t%s, [sp, %d]\n", reg_name[i], 8-4*i);
        }
    }

    *padsize = psize;
    *framesize = fsize;
    return sparams;
}

void
gen_call_set_param(FILE *out, int reg, int nump, int sparams)
{
    if (nump < 9) {
        if (strcmp(param_reg_name[nump], reg_name[reg]) != 0) {
            fprintf(out, "\tmov\t%s, %s\n",
                    param_reg_name[nump], reg_name[reg]);
        }
    } else {
        fprintf(out, "\tstr\t%s, [sp, %d]\n",
                reg_name[reg], (sparams+8-nump+1)*(-8));
    }
}

void
gen_call_epilogue(FILE *out, AST_Node *e, int padsize, int framesize)
{
    int i;
    fprintf(out, "\tbl\t%s\n", e->child[0]->str);
    /* 戻り値の格納 / copy return value*/
    if (e->parent != NULL && e->parent->kind == AST_KIND_EXP) {
        fprintf(out, "\tmov\t%s, w0\n", reg_name[e->reg]);
    }
    /* %rspを戻す / pop %rsp */
    for (i = 0; i < 3; i++) {
        if (e->reg != i) {
            fprintf(out, "\tldr\t%s, [sp, %d]\n", reg_name[i], 8-4*i);
            /* padsize is not used for arm64. */
        }
    }
    fprintf(out, "\tadd\tsp, sp, %d\n", framesize);
    /* framesize is 16 for arm64. */
}

/*
  For basic operations that can be directly translated
  a corresponding assembly instruction.
  Note that the destination register and the first source register
  must be same for the x86-style assembly language (two-operands).
*/

/*
  store local variable
*/
void
gen_insn_store_lvar(FILE* out, int reg, int offset)
{
    fprintf(out, "\tstr\t%s, [x29, %d]\n", reg_name[reg], offset);
}

void
gen_insn_neg(FILE* out, int dst, int src)
{
    fprintf(out, "\tneg\t%s, %s\n", reg_name[dst], reg_name[src]);
}

void
gen_insn_add(FILE* out, int dst, int src1, int src2)
{
    fprintf(out, "\tadd\t%s, %s, %s\n",
            reg_name[dst], reg_name[src1], reg_name[src2]);
}

void
gen_insn_sub(FILE* out, int dst, int src1, int src2)
{
    fprintf(out, "\tsub\t%s, %s, %s\n",
            reg_name[dst], reg_name[src1], reg_name[src2]);
}

void
gen_insn_mul(FILE* out, int dst, int src1, int src2)
{
    fprintf(out, "\tmul\t%s, %s, %s\n",
            reg_name[dst], reg_name[src1], reg_name[src2]);
}

/* return value is passed through "w0" register. */
void
gen_insn_ret_asgn(FILE *out, int src)
{
    fprintf(out, "\tmov\tw0, %s\n", reg_name[src]);
}

void
gen_insn_jmp(FILE *out, const char *label)
{
    fprintf(out, "\tb\t%s\n", label);
}

void
gen_insn_cmp(FILE* out, int src1, int src2)
{
    fprintf(out, "\tcmp\t%s, %s\n", reg_name[src1], reg_name[src2]);
}

void
gen_insn_rel(FILE *out, int cond, const char *l_cmp, int reg)
{
    switch (cond) {
    case  AST_EXP_LT:
        fprintf(out, "\tb.ge\t%s\n", l_cmp);
        break;
    case  AST_EXP_GT:
        fprintf(out, "\tb.le\t%s\n", l_cmp);
        break;
    case  AST_EXP_LTE:
        fprintf(out, "\tb.gt\t%s\n", l_cmp);
        break;
    case  AST_EXP_GTE:
        fprintf(out, "\tb.lt\t%s\n", l_cmp);
        break;
    case  AST_EXP_EQ:
        fprintf(out, "\tb.ne\t%s\n", l_cmp);
        break;
    case  AST_EXP_NE:
        fprintf(out, "\tb.eq\t%s\n", l_cmp);
        break;
    default:
        /* "0" stands for "false". */
        fprintf(out, "\tcmp\t%s, 0\n", reg_name[reg]);
        fprintf(out, "\tb.eq\t%s\n", l_cmp);
    }
}

void
gen_insn_cond_set(FILE* out, int dst, int cond)
{
    switch (cond) {
    case  AST_EXP_LT:
        fprintf(out, "\tcset\t%s, lt\n", reg_name[dst]);
        break;
    case  AST_EXP_GT:
        fprintf(out, "\tcset\t%s, gt\n", reg_name[dst]);
        break;
    case  AST_EXP_LTE:
        fprintf(out, "\tcset\t%s, le\n", reg_name[dst]);
        break;
    case  AST_EXP_GTE:
        fprintf(out, "\tcset\t%s, ge\n", reg_name[dst]);
        break;
    case  AST_EXP_EQ:
        fprintf(out, "\tcset\t%s, eq\n", reg_name[dst]);
        break;
    case  AST_EXP_NE:
        fprintf(out, "\tcset\t%s, ne\n", reg_name[dst]);
        break;
    default:
        errexit("Invalid relation instruction.", __FILE__, __LINE__);
    }
}
