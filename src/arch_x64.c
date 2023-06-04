/*
    Tiny Language Compiler (tlc)

    x86-64依存部部分
    x86-64 dependent part

    2021年 木村啓二
*/

#include  <assert.h>
#include  <string.h>
#include  "arch_common.h"
#include  "symtab.h"
#include  "util.h"

/*
 * x86-64のターゲット依存部分
 * MakefileのTARGET_FLAGを適切に選択する
 * Target dependent part, particularly for x86-64.
 * We need to set TARGET_FLAG in Makefile appropriately.
 */
#if defined(TARGET_LINUX) || defined(TARGET_CYGWIN)
#if defined(TARGET_LINUX)
const char MAIN_LABEL[]   = "main";
const char PUTINT_CODE[]  =
    "\t.section\t.rodata\n"
    ".LC0:\n"
    "\t.string \"%d\\n\"\n"
    "\t.text\n"
    "put_int:\n"
    "\tpushq\t%rbp\n"
    "\tmovq\t%rsp, %rbp\n"
    "\tsubq\t$16,%rsp\n"
    "\tmovl\t%edi, -4(%rbp)\n"
    "\tmovl\t-4(%rbp), %esi\n"
    "\tleaq\t.LC0(%rip), %rdi\n"
    "\tmovl\t$0, %eax\n"
    "\tcall\tprintf@PLT\n"
    "\tleave\n"
    "\tret\n";
#elif defined(TARGET_CYGWIN)
const char MAIN_LABEL[]   = "main";
const char PUTINT_CODE[]  =
    "\t.section\t.rodata\n"
    ".LC0:\n"
    "\t.string \"%d\\n\"\n"
    "\t.text\n"
    "put_int:\n"
    "\tpushq\t%rbp\n"
    "\tmovq\t%rsp, %rbp\n"
    "\tsubq\t$32,%rsp\n"
    "\tmovl\t%ecx, %edx\n"
    "\tleaq\t.LC0(%rip), %rcx\n"
    "\tcall\tprintf\n"
    "\taddq\t$32, %rsp\n"
    "\tpopq\t%rbp\n"
    "\tret\n";
#endif
const char SECTION_TEXT[] =  "\t.text\n";
const char CALL_OP[]      =  "call";

#elif defined(TARGET_MAC)
const char MAIN_LABEL[]   = "_main";
const char SECTION_TEXT[] = "\t.section\t__TEXT,__text\n";
const char CALL_OP[]      =  "calll";
const char PUTINT_CODE[]  =
    "\t.section\t__TEXT,__cstring\n"
    ".LC0:\n"
    "\t.string \"%d\\n\"\n"
    "\t.section\t__TEXT,__text\n"
    "put_int:\n"
    "\tpushq\t%rbp\n"
    "\tmovq\t%rsp, %rbp\n"
    "\tsubq\t$16,%rsp\n"
    "\tleaq\t.LC0(%rip), %rax\n"
    "\tmovl\t%edi, -4(%rbp)\n"
    "\tmovl\t-4(%rbp), %esi\n"
    "\tmovq\t%rax, %rdi\n"
    "\tmovb\t$0, %al\n"
    "\tcallq\t_printf\n"
    "\tmovl\t%eax, -8(%rbp)\n"
    "\taddq\t$16, %rsp\n"
    "\tpopq\t%rbp\n"
    "\tretq\n";
#elif defined(TARGET_CYGWIN)
#endif

char reg_name[][10] = {"%eax", "%r10d", "%r11d"};
#ifdef  TARGET_CYGWIN
char param_reg_name[][10] = {"NULL", "%ecx", "%edx", "%r8d", "%r9d",
				    "%edi", "%esi" };
#else
char param_reg_name[][10] = {"NULL", "%edi", "%esi", "%edx", "%ecx",
				    "%r8d", "%r9d" };
#endif

/* tlcにおけるx86 (64bit)スタックレイアウトメモ
   note for x86(64bit) stack layout in tlc
   （例/example）
   int func(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
   { int v1, v2, v3; ... }

   最初の6つの引数はレジスタを使って渡される
   -> 1番目から順に%edi(1), %esi(2), %edx(3), %ecx(4), %r8d(5), %r9d(6)
      7つめ以降はrspに近い順から8バイトごとにスタックに保存される
   -> 関数呼び出し後、レジスタで渡された値もスタックに保存される
      レイアウトは以下の図のようになる
   The first six parameters are passed through registers.
   -> From the first parameter, they are passed by
      %edi(1), %esi(2), %edx(3), %ecx(4), %r8d(5), and %r9d(6), respectively.
      7th and more parameters are once saved in the stack
      each of which occupies 8 bytes.
      The order of them starts from the address pointed by %rsp.
   -> After control is transferred to the function body,
      all parameters are saved in the stack.
      The following figure depicts a stack layout.
   ** CYGWINの場合 ** (obsoleted)
   CYGWINでは最初の4つの引数を、%ecx, %edx, $r8d, %r9dの順でレジスタ経由で渡す
   それ以降はスタックで渡す。tlc64ではMacやLinuxに合わせて6つまでをレジスタで渡す
   ことにする。5つめに%edi, 6つめに%esiを使用する。

(higher address)
... |          | old %rspを16byte整列にするための隙間(pad)
    |   %eax   |
    |   %r10d  |関数呼び出し前に保存 / save before function call
    |   %r11d   |
------------------
    |          |
+24 |    a8    | (+8)
    |          |
+16 |    a7    | (+0) <- 関数呼び出し前のold %rspオフセット
    | return - |           old %rspは16バイト境界に整列される
+8  |  address |
    | old -    |
+0  |   %rbp   | <- %rbp
-4  |    v3    |
-8  |    v2    |
-12 |    v1    |
-16 |    a1    |
-20 |    a2    |
-24 |    a3    |
-28 |    a4    |
-32 |    a5    |
-36 |    a6    |
... |          | 
    |          |<- %rspは16バイト境界に調整 
(lower address)


上記の状態を作るため、
- 関数呼び出し直前に%eax,%r10d,%r11dと実引数用に16byte整列番地に%rspを合わせる
-> パッドの数をPa、実引数の7つめ以降の数とNa2とすると(Pa+2*Na2+3)*4%16を0にする
     ((Pa+2*Na2+3)%4を0)
- 関数呼び出し直後に、まずold %rbpをpushし、old %rspを%rbpにコピーする
- その後、自動変数用とレジスタ渡し引数用の領域確保のため、16byte整列した%rspを計算
-> パッドの数をPv、自動変数の数をNv、6つめまでの引数の数をNa1とすると
    (Pv+Nv+Na1)*4%16を0にする ((Pv+Nv+Na1)%4を0)
To make the situation above:
- Before the function call, adjust the %rsp on the 16byte memory alignment
  for %eax, %r10d, %r11d, and actual parameters.
-> For adjustment, the following equation is used:
    (Pa+2*N12+3)%4 = 0
    where Pa is the number of pad,
    Na is the number of actual parameters exceeding 7.
- After transferring the function body, old %rbp is pushed,
   and old %rsp is copied to %rbp.
- Then, adjust the %rsp on the 16byte memory alignment for auto variables
  and parameters passed through registers.
-> For adjustment, the following equation is used:
    (Pv+Nv+Na)*4%16 = 0
    where Pv is the number of pad, Nv is the number of auto variables,
    and Na1 is the number of parameters up to 6.
*/

/*
  変数のスタック上の位置（オフセット）を決める。
  - 引数(1-6)
    自動変数の手前（なので、自動変数の位置を決めてから割り当て）
  - 引数(7-)
    +16（old %rbpとreturn addressの後）に8byteごと
  - 自動変数
    -4から順に
  Decide the layout in the stack (offset).
  See the figure above.
*/

void
arch_assign_memory(SymTab *symtab)
{
    int id_arg, id_var;
    SymTab *t;

    id_arg = 0; id_var = 0;
    for (t = symtab; t != NULL; t = t->next) {
        /* 変数のサイズはint 4byteで固定 */
        if (t->kind == SYM_ARG) {
            t->argid = ++id_arg;
        } else if (t->kind == SYM_AUTOVAR) {
            t->offset = (++id_var)*(-4);
            t->argid = 0;
        }
    }
    for (t = symtab; t != NULL; t = t->next) {
        if (t->kind == SYM_ARG) {
            if (t->argid < 7) {
                t->offset = (id_var+t->argid)*(-4);
            } else {
                t->offset = 16+(t->argid-7)*8;
            }
        }
    }
}

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
       Please look at the note in symtab.c
       frame_size is equal to (Nv+Na1)*4 in the note.
    */
    pad = 16 - frame_size%16;
    if (pad == 16) {
        pad = 0;
    }
    if (strcmp(name, "main") == 0) {
        targetn = MAIN_LABEL;
    }
    fprintf(out,
            "\t.globl\t%s\n"
            "%s:\n", targetn, targetn);
    fputs("\tpushq\t%rbp\n"
          "\tmovq\t%rsp, %rbp\n", out);
    i = 0;
    TRAVERSE_AST_LIST(l, arg_list, gen_store_params(out, l->elem, ++i));
    if (frame_size+pad > 0) {
        fprintf(out, "\tsubq\t$%d, %%rsp\n", frame_size+pad);
    }
}

void
gen_store_params(FILE *out, AST_Node *param, int nump)
{
    if (nump < 7) {
        AST_Node *pid = param->child[0];
        assert(pid != NULL && pid->symtab != NULL);
        fprintf(out, "\tmovl\t%s, %d(%%rbp)\n",
                param_reg_name[nump], pid->symtab->offset);
    }
}

void
gen_func_footer(FILE *out, const char *func_end_label)
{
    fprintf(out, "%s:\n"
            "\tleave\n"
            "\tret\n\n", func_end_label);
}

void
gen_exp_cnst(FILE *out, AST_Node *c)
{
    fprintf(out, "\tmovl\t$%d, %s\n", c->val, reg_name[c->reg]);
}

void
gen_exp_ident(FILE *out, AST_Node *idnt)
{
    fprintf(out, "\tmovl\t%d(%%rbp), %s\n",
            idnt->symtab->offset, reg_name[idnt->reg]);
}

int
gen_call_prologue(FILE *out, AST_Node *e, int *padsize, int *framesize)
{
    int i, psize, fsize, pad, sparams;
    AST_List *l;
    
    sparams = 0;
    TRAVERSE_AST_LIST(l, e->list, ++sparams);
    /* %rspの整列補正。上記のスタックに関するメモを参照
       Adjust %rsp for alignment. See the note about stack above. */
    sparams = (sparams > 6) ? sparams - 6 : 0;
    pad = 4-(sparams*2+3)%4; /* sparams*2はスタック上で2要素分占めるため / for 2elemnts */
    if (pad == 4) {
        pad = 0;
    }
    pad *= 4; psize = sparams * 8;
    fsize = pad+psize+3*4; /* %eax, %ecx, and %edx are all int(4byte)
			      The size of a parameters passed through stack is 8byte. */

    /* 実引数とpadと待避するレジスタの分だけ%rspをずらす
       Adjust %rsp by total size of the actual parameters, pad, and saved registers */
    fprintf(out, "\tsubq\t$%d, %%rsp\n", fsize);
    for (i = 0; i < 3; i++) {
        if (e->reg != i) {
            fprintf(out, "\tmovl\t%s, %d(%%rsp)\n",
                    reg_name[i], psize+12-4*(i+1));
        }
    }

    *padsize = psize;
    *framesize = fsize;
    return sparams;
}

void
gen_call_set_param(FILE *out, int reg, int nump, int sparams)
{
    /* sparms is not used for x64. */
    if (nump < 7) {
        fprintf(out, "\tmovl\t%s, %s\n",
                reg_name[reg], param_reg_name[nump]);
    } else {
        fprintf(out, "\tmovl\t%s, %d(%%rsp)\n",	reg_name[reg], (nump-7)*8);
    }
}

void
gen_call_epilogue(FILE *out, AST_Node *e, int padsize, int framesize)
{
    int i;
    fprintf(out, "\tcall\t%s\n", e->child[0]->str);
    /* 戻り値の格納 / copy return value*/
    if (e->reg != 0) {
        fprintf(out, "\tmovl\t%s, %s\n", reg_name[0], reg_name[e->reg]);
    }
    /* %rspを戻す / pop %rsp */
    for (i = 0; i < 3; i++) {
        if (e->reg != i) {
            fprintf(out, "\tmovl\t%d(%%rsp), %s\n",
                    padsize+12-4*(i+1), reg_name[i]);
        }
    }
    fprintf(out, "\taddq\t$%d, %%rsp\n", framesize);
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
    fprintf(out, "\tmovl\t%s, %d(%%rbp)\n", reg_name[reg], offset);
}

void
gen_insn_neg(FILE* out, int dst, int src)
{
    assert(dst == src);
    fprintf(out, "\tnegl\t%s\n", reg_name[dst]);
}

void
gen_insn_add(FILE* out, int dst, int src1, int src2)
{
    assert(dst == src1);
    fprintf(out, "\taddl\t%s, %s\n", reg_name[src2], reg_name[dst]);
}

void
gen_insn_sub(FILE* out, int dst, int src1, int src2)
{
    assert(dst == src1);
    fprintf(out, "\tsubl\t%s, %s\n", reg_name[src2], reg_name[dst]);
}

void
gen_insn_mul(FILE* out, int dst, int src1, int src2)
{
    assert(dst == src1);
    fprintf(out, "\timull\t%s, %s\n", reg_name[src2], reg_name[dst]);
}

void
gen_insn_ret_asgn(FILE *out, int src)
{
    if (src != 0) {
        fprintf(out, "\tmovl\t%s, %s\n", reg_name[src], reg_name[0]);
    }
}

void
gen_insn_jmp(FILE *out, const char *label)
{
    fprintf(out, "\tjmp\t%s\n", label);
}

void
gen_insn_cmp(FILE* out, int src1, int src2)
{
    fprintf(out, "\tcmpl\t%s, %s\n", reg_name[src2], reg_name[src1]);
}

void
gen_insn_rel(FILE *out, int cond, const char *l_cmp, int reg)
{
    switch (cond) {
    case  AST_EXP_LT:
        fprintf(out, "\tjge\t%s\n", l_cmp);
        break;
    case  AST_EXP_GT:
        fprintf(out, "\tjle\t%s\n", l_cmp);
        break;
    case  AST_EXP_LTE:
        fprintf(out, "\tjg\t%s\n", l_cmp);
        break;
    case  AST_EXP_GTE:
        fprintf(out, "\tjl\t%s\n", l_cmp);
        break;
    case  AST_EXP_EQ:
        fprintf(out, "\tjne\t%s\n", l_cmp);
        break;
    case  AST_EXP_NE:
        fprintf(out, "\tje\t%s\n", l_cmp);
        break;
    default:
        /* "0" stands for "false". */
        fprintf(out, "\tcmpl\t$0,%s\n", reg_name[reg]);
        fprintf(out, "\tje\t%s\n", l_cmp);
    }
}

void
gen_insn_cond_set(FILE* out, int dst, int cond)
{
    switch (cond) {
    case  AST_EXP_LT:
        fputs("\tsetl\t%al\n", out);
        break;
    case  AST_EXP_GT:
        fputs("\tsetg\t%al\n", out);
        break;
    case  AST_EXP_LTE:
        fputs("\tsetle\t%al\n", out);
        break;
    case  AST_EXP_GTE:
        fputs("\tsetge\t%al\n", out);
        break;
    case  AST_EXP_EQ:
        fputs("\tsete\t%al\n", out);
        break;
    case  AST_EXP_NE:
        fputs("\tsetne\t%al\n", out);
        break;
    default:
        errexit("Invalid relation instruction.", __FILE__, __LINE__);
    }
    fprintf(out, "\tmovzbl\t%%al, %s\n", reg_name[dst]);
}
