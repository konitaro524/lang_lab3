/*
    Tiny Language Compiler (tlc)

    レジスタ管理・コード生成
    register management and code generation

    2016年 木村啓二
*/

#include  <assert.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#include  "ast.h"
#include  "arch_common.h"
#include  "cg.h"
#include  "symtab.h"
#include  "util.h"

/*
 * レジスタ割り付け系
 * 方針：
 * 式の構文木で深い方から優先してレジスタを割り付け、レジスタを3つだけ使う
 * （64bitコードではもっと使っても良いがとりあえずこれだけにしておく）
 *
 * 流れ：
 * 1. 各式を深さ優先で探索し、末端から自分までいくつノードがあるかrankに記録する
 * 2. rankの大きい方から優先して探索し、使えるレジスタを割り付ける
 *  いずれも分の巡回までは同じ道筋なので、されぞれpass1/pass2で処理を分ける
 *
 * 簡単のため、式の子は高々2つであることを前提とする
 *
 * 関数呼び出しの際のレジスタの扱い:
 * - 引数を処理する前に（現状では）%eax, %r10d, %r11dをスタックに保存し実引数の評価を行う
 * - 戻り値は%eaxに格納されるので、関数ノードに割り当てられたレジスタが%eaxでなければ値をコピーする
 *
 * Register assignment
 * Basic strategy:
 * - Assign registers to AST in depth-first manner
 * - Use only three registers
 * (We can use more registers for 64bit-code,
 *  but the number of used registers is limited right now.)
 *
 * Flow of assignment:
 * 1. Traverse AST in depth-first manner
 *    and record the number of nodes from the current node to the end node in "rank"
 * 2. Assign available registers by traversing AST from the highest rank
 * Since both have same traverse number, we can use same traverse code.
 *
 * For simplicity, we assume the number of children nodes are at most two.
 *
 * Register usage for function call:
 * - Before processing parameters, (currently) the values in %eax, %r10d,
 *   and %r11d are stored in the stack, then the actual parameters are evaluated.
 * - Since the x86 ABI requires %eax is used for a return value,
 *   if an assigned register for a function node is not %eax,
 *   the value in %eax must be copied to the assigned register.
 *
 */

#define  MAX_REG_NUM 3

static void traverse_ast_func(AST_Node *f, int pass);
static void traverse_ast_stm(AST_Node *s, int pass);
static void traverse_ast_exp(AST_Node *e, int pass);
static int  ranking_ast_exp(AST_Node *e);
static void assign_ast_exp(AST_Node *e);
static void assign_ast_call(AST_Node *e);
static void assign_ast_exp_body(AST_Node *e, int regs[]);

void
assign_regs(void)
{
    AST_List *l;

    TRAVERSE_AST_LIST(l, AST_root, traverse_ast_func(l->elem, 1));
    TRAVERSE_AST_LIST(l, AST_root, traverse_ast_func(l->elem, 2));
}

void
traverse_ast_func(AST_Node *f, int pass)
{
    traverse_ast_stm(f->child[1], pass);
}

void
traverse_ast_stm(AST_Node *s, int pass)
{
    AST_List *l;

    if (s == NULL) {
        return;
    }
    switch (s->sub_kind) {
    case  AST_STM_LIST:
        TRAVERSE_AST_LIST(l, s->list, traverse_ast_stm(l->elem, pass));
        break;
    case  AST_STM_DEC:
        /* Nothing to do */
        break;
    case  AST_STM_ASIGN:
        traverse_ast_exp(s->child[0], pass);
        break;
    case  AST_STM_IF:
        traverse_ast_exp(s->child[0], pass);
        /* then-statement */
        traverse_ast_stm(s->child[1], pass);
        /* else-statement */
        traverse_ast_stm(s->child[2], pass);
        break;
    case  AST_STM_WHILE:
        traverse_ast_exp(s->child[0], pass);
        traverse_ast_stm(s->child[1], pass);
        break;
    case  AST_STM_FOR:
        traverse_ast_exp(s->child[0], pass);
        traverse_ast_exp(s->child[1], pass);
        traverse_ast_exp(s->child[2], pass);
        traverse_ast_stm(s->child[3], pass);
        break;
/* REPORT3
   このあたりにdo-while文ノード用のレジスタ割り付け巡回処理を追加する
   Add traverse code for register assignment to do-while statement
*/

    case AST_STM_DOWHILE:
        traverse_ast_stm(s->child[0], pass);  // AST_STM_WHILEと逆
        traverse_ast_exp(s->child[1], pass);  
        break;

    case  AST_STM_RETURN:
        traverse_ast_exp(s->child[0], pass);
        break;
    default:
        errexit("Invalid statement kind", __FILE__, __LINE__);
    }
}

void
traverse_ast_exp(AST_Node *e, int pass)
{
    if (e == NULL) {
        return;
    }
    if (pass == 1) {
        ranking_ast_exp(e);
    } else if (pass == 2) {
        assign_ast_exp(e);
    } else {
        fputs("Illegal register assignemnt pass.\n", stderr);
        abort();
    }
}

int
ranking_ast_exp(AST_Node *e)
{
    int  r0, r1, maxr;
    AST_List *l;
    
    TRAVERSE_AST_LIST(l, e->list, ranking_ast_exp(l->elem));
    r0 = r1 = 0;
    if (e->sub_kind != AST_EXP_CALL && e->child[0] != NULL) {
        r0 = ranking_ast_exp(e->child[0]);
    }
    if (e->child[1] != NULL) {
        r1 = ranking_ast_exp(e->child[1]);
    }
    maxr = r0 >= r1 ? r0 : r1;
    e->rank = maxr+1; /* 末端でもこれでOK / This is OK for the end node. */
    return e->rank;
}


void
assign_ast_exp(AST_Node *e)
{
    if (e->sub_kind == AST_EXP_CALL) {
        assign_ast_call(e);
    } else {
        int regs[MAX_REG_NUM];	/* 利用可能レジスタのフラグ / flags for available registers */
        memset(regs, 0, sizeof(regs));
        assign_ast_exp_body(e, regs);
    }
}

/*
 * 関数呼び出し前にREGISTERをスタックに保存するのでレジスタ使用状況はリセット
 * Flags for available registers are reset before function call
 * since they are all stored in the stack.
 */
void
assign_ast_call(AST_Node *e)
{
    AST_List *l;
    /* 引き数列の処理 / process parameters */
    TRAVERSE_AST_LIST(l, e->list, assign_ast_exp(l->elem));
}

void
assign_ast_exp_body(AST_Node *e, int regs[])
{
    int  i, i0, i1, r0, r1;

    r0 = r1 = 0;
    if (e->child[0] != NULL) {
        r0 = e->child[0]->rank;
    }
    if (e->child[1] != NULL) {
        r1 = e->child[1]->rank;
    }
    if (r0 >= r1) {
        i0 = 0; i1 = 1;
    } else {
        i0 = 1; i1 = 0;
    }
    if (r0 != 0 || r1 != 0) { /* 子がある / node(s) exists */
        if (e->child[i0] != NULL) {
            assign_ast_exp_body(e->child[i0], regs);
	}
	if (e->child[i1] != NULL) {
            assign_ast_exp_body(e->child[i1], regs);
	}
	if (e->child[0] != NULL) {
            e->reg = e->child[0]->reg;
	}
	if (e->child[1] != NULL) {
            regs[e->child[1]->reg] = 0;
	}
    } else {
        for (i = 0; i < MAX_REG_NUM; i++) {
            if (regs[i] == 0) {
                e->reg = i;
                regs[i] = 1;
                break;
            }
	}
	if (i == MAX_REG_NUM) {
            fputs("Number of registers is not sufficient.\n", stderr);
            abort();
        }
    }
}

/*
 * コード生成系
 * Code generation
 */

static void init_label(void);
static void make_func_last_label(AST_Node *f);
static int  get_label(void);
static char *gen_label(int label);
static void gen_label_stm(FILE *out, int label);
static void gen_header(FILE *out);
static void gen_func(FILE *out, AST_Node *f);
static void gen_put_int(FILE *out);
static void gen_stm(FILE *out, AST_Node *s);
static void gen_stm_asign(FILE *out, AST_Node *s);
static void gen_stm_rel(FILE *out, AST_Node *e, int l_cmp);
static void gen_stm_if(FILE *out, AST_Node *s);
static void gen_stm_while(FILE *out, AST_Node *s);
static void gen_stm_for(FILE *out, AST_Node *s);
static void gen_stm_dowhile(FILE *out, AST_Node *s);
static void gen_stm_return(FILE *out, AST_Node *s);
static void gen_exp(FILE *out, AST_Node *e);
static void gen_exp_asgn(FILE *out, AST_Node *e);
static void gen_exp_rel(FILE *out, AST_Node *rel);
extern void gen_exp_call(FILE *out, AST_Node *e);
extern void gen_exp_call_param(FILE *out, AST_Node *p, int nump, int sparams);
static void gen_exp_n2(FILE *out, AST_Node *e);

static int local_label;		/* 関数内ラベルの番号 / label number in a func */
static char *func_end_label;	/* 関数末尾のラベル / End-label for a func */


void
init_label(void)
{
    local_label = 0;
}

/*
 * 関数末尾のラベル
 * func1()のラベルの場合は_END_func1
 * End-label for a function
 * ex) The label for func1() is END_func1
 */
void
make_func_last_label(AST_Node *f)
{
    char *name;
    int len;

    assert(f->child[0]->sub_kind == AST_EXP_IDENT);
    name = f->child[0]->str;
    len = strlen(name)+6;
    func_end_label = xmalloc(len);
    snprintf(func_end_label, len, "_END_%s", name);
}

int
get_label(void)
{
    return local_label++;
}

char*
gen_label(int label)
{
    static char buf[10];

    snprintf(buf, sizeof(buf), ".L%d", label);
    return buf;
}

void
gen_label_stm(FILE *out, int label)
{
    fprintf(out, "%s:\n", gen_label(label));
}

void
gen_code(FILE *out)
{
    AST_List *l;
    
    gen_header(out);
    init_label();
    TRAVERSE_AST_LIST(l, AST_root, gen_func(out, l->elem));
    gen_put_int(out);
}

void
gen_header(FILE *out)
{
    fprintf(out, "%s", SECTION_TEXT);
}

void
gen_func(FILE *out, AST_Node *f)
{
    AST_List *l;

    assert(f->child[0]->sub_kind == AST_EXP_IDENT);
    make_func_last_label(f);
    gen_func_header(out, f->child[0]->str, get_frame_size(f->id), f->list);
    TRAVERSE_AST_LIST(l, f->child[1]->list, gen_stm(out, l->elem));
    gen_func_footer(out, func_end_label);
    free(func_end_label);
    func_end_label = NULL;
}

void
gen_put_int(FILE *out)
{
    fprintf(out, "%s", PUTINT_CODE);
}

void
gen_stm(FILE *out, AST_Node *s)
{
    AST_List *l;
    
    if (s == NULL) {
        return;
    }
    switch (s->sub_kind) {
    case  AST_STM_LIST:
        TRAVERSE_AST_LIST(l, s->list, gen_stm(out, l->elem));
        break;
    case  AST_STM_DEC:
        /* Nothing to do */
        break;
    case  AST_STM_ASIGN:
        gen_stm_asign(out, s);
        break;
    case  AST_STM_IF:
        gen_stm_if(out, s);
        break;
    case  AST_STM_WHILE:
        gen_stm_while(out, s);
        break;
    case  AST_STM_FOR:
        gen_stm_for(out, s);
        break;
    case  AST_STM_DOWHILE:
        gen_stm_dowhile(out, s);
        break;
    case  AST_STM_RETURN:
        gen_stm_return(out, s);
        break;
    default:
        errexit("Invalid statement kind", __FILE__, __LINE__);
    }
}

void
gen_stm_asign(FILE *out, AST_Node *s)
{
    gen_exp(out, s->child[0]);
}

/*
 * if文やwhile文等の条件式のための適切な条件分岐コードを生成する
 * eは条件式
 * l_cmpは条件が偽だった場合の飛び先ラベル
 * Generate appropriate code for if-statement and while-statement.
 * e is condition expression.
 * l_cmp is the target label for the false condition.
 */
void
gen_stm_rel(FILE *out, AST_Node *e, int l_cmp)
{
    gen_insn_rel(out, e->sub_kind, gen_label(l_cmp), e->reg);
}

void
gen_stm_if(FILE *out, AST_Node *s)
{
    int  l_else = -1, l_end, l_cmp;

    l_cmp = l_end = get_label();
    if (s->child[2] != NULL) { /* else */
        l_cmp = l_else = get_label();
    }

    gen_exp(out, s->child[0]);
    gen_stm_rel(out, s->child[0], l_cmp);
    gen_stm(out, s->child[1]);
    if (s->child[2] != NULL) {
        gen_insn_jmp(out, gen_label(l_end));
        gen_label_stm(out, l_else);
        gen_stm(out, s->child[2]);
    }
    gen_label_stm(out, l_end);
}

void
gen_stm_while(FILE *out, AST_Node *s)
{
    int  l_begin, l_exit;

    l_begin = get_label();
    l_exit = get_label();
    gen_label_stm(out, l_begin);
    gen_exp(out, s->child[0]);
    gen_stm_rel(out, s->child[0], l_exit);
    gen_stm(out, s->child[1]);
    gen_insn_jmp(out, gen_label(l_begin));
    gen_label_stm(out, l_exit);
}

void
gen_stm_for(FILE *out, AST_Node *s)
{
    int  l_begin, l_exit;

    l_begin = get_label();
    l_exit = get_label();
    gen_exp(out, s->child[0]);
    gen_label_stm(out, l_begin);
    gen_exp(out, s->child[1]);
    gen_stm_rel(out, s->child[1], l_exit);
    gen_stm(out, s->child[3]);
    gen_exp(out, s->child[2]);
    gen_insn_jmp(out, gen_label(l_begin));
    gen_label_stm(out, l_exit);
}

void
gen_stm_dowhile(FILE *out, AST_Node *s)
{
    /* REPORT3
       ここにdo-while文のコード生成処理を追加する
       Add code-generation code for do-while
    */

    int  l_begin, l_exit;

    l_begin = get_label();
    l_exit = get_label();
    gen_label_stm(out, l_begin);
    gen_stm(out, s->child[0]);
    gen_exp(out, s->child[1]);
    gen_stm_rel(out, s->child[1], l_exit);
    gen_insn_jmp(out, gen_label(l_begin));
    gen_label_stm(out, l_exit);
}

void
gen_stm_return(FILE *out, AST_Node *s)
{
    gen_exp(out, s->child[0]);
    gen_insn_ret_asgn(out, s->reg);
    gen_insn_jmp(out, func_end_label);
}

void
gen_exp(FILE *out, AST_Node *e)
{
    if (e == NULL) {
        return;
    }
    if (e->sub_kind == AST_EXP_ASGN) {
        gen_exp_asgn(out, e);
    } else if (e->sub_kind == AST_EXP_IDENT) {
        gen_exp_ident(out, e);
    } else if (e->sub_kind == AST_EXP_CNST_INT) {
        gen_exp_cnst(out, e);
    } else if (e->sub_kind == AST_EXP_CALL) {
        gen_exp_call(out, e);
    } else {
        gen_exp_n2(out, e);
    }
}

void
gen_exp_asgn(FILE *out, AST_Node *e)
{
    gen_exp(out, e->child[1]);
    if (e->child[0]->sub_kind != AST_EXP_IDENT) {
        errexit("Invalid destination operand for assign.", __FILE__, __LINE__);
    }
    gen_insn_store_lvar(out, e->child[1]->reg, e->child[0]->symtab->offset);
}

void
gen_exp_rel(FILE *out, AST_Node *e)
{
    gen_insn_cmp(out, e->child[0]->reg, e->child[1]->reg);
    if (e->parent->kind == AST_KIND_STM
        /* REPORT3
           このあたりも修正が必要？
           It seems to add some modification.
        */
        && (e->parent->sub_kind == AST_STM_IF
            || e->parent->sub_kind == AST_STM_WHILE
            || e->parent->sub_kind == AST_STM_FOR
            ||e->parent->sub_kind == AST_STM_DOWHILE)) {
        /* The parent statement generates a branch operation. */
    } else {
        /* AST_EXP_LT, AST_EXP_GT, AST_EXP_LTE,
           AST_EXP_GTE, AST_EXP_EQ, or AST_EXP_NE */
        gen_insn_cond_set(out, e->reg, e->sub_kind);
    }
}

/*
   関数呼び出し手順：
   - スタックポインタの移動
   - 必要なレジスタの退避
   - 実引数の評価・スタックに格納
   - call
   - 戻り値をノードに割り当てられたレジスタに移動
   - スタックポインタを戻す

   Steps for function call
   - adjust the stack-pointer
   - save registers if needed
   - evaluate actual parameters and save them into the stack
   - call
   - copy the return value to the assigned register
*/
void
gen_exp_call(FILE *out, AST_Node *e)
{
    int i;
    int sparams, psize, fsize;
    AST_List *l;
    
    sparams = gen_call_prologue(out, e, &psize, &fsize);
    i = 0;
    TRAVERSE_AST_LIST(l, e->list,
                      gen_exp_call_param(out, l->elem, ++i, sparams));
    assert(e->child[0]->sub_kind == AST_EXP_IDENT);
    gen_call_epilogue(out, e, psize, fsize);
}

void
gen_exp_call_param(FILE *out, AST_Node *p, int nump, int sparams)
{
    gen_exp(out, p);
    gen_call_set_param(out, p->reg, nump, sparams);
}

void
gen_exp_n2(FILE *out, AST_Node *e)
{
    int  i0, i1, r0, r1, src;

    /* レジスタ割り付けと同じ順番で巡回する必要がある
       Traverse order must be same as that of register assignment */
    r0 = r1 = 0;
    src = e->reg;
    if (e->child[0] != NULL) {
        r0 = e->child[0]->rank;
    }
    if (e->child[1] != NULL) {
        r1 = e->child[1]->rank;
        src = e->child[1]->reg;
    }
    if (r0 >= r1) {
        i0 = 0; i1 = 1;
    } else {
        i0 = 1; i1 = 0;
    }
    if (r0 != 0 || r1 != 0) {
        if (e->child[i0] != NULL) {
            gen_exp(out, e->child[i0]);
	}
        if (e->child[i1] != NULL) {
            gen_exp(out, e->child[i1]);
        }
    }
    switch (e->sub_kind) {
    case  AST_EXP_UNARY_PLUS:
        break;			/* nothing to do */
    case  AST_EXP_UNARY_MINUS:
        gen_insn_neg(out, e->reg, e->reg);
        break;
    case  AST_EXP_MUL:
        gen_insn_mul(out, e->reg, e->reg, src);
        break;
    case  AST_EXP_DIV:
        /* "div" is not supported now because of its register restriction. */
        fputs("Sorry, div is not suppoted.\n", stderr);
        exit(-1);
        break;
    case  AST_EXP_ADD:
        gen_insn_add(out, e->reg, e->reg, src);
        break;
    case  AST_EXP_SUB:
        gen_insn_sub(out, e->reg, e->reg, src);
        break;
    case  AST_EXP_LT:
    case  AST_EXP_GT:
    case  AST_EXP_LTE:
    case  AST_EXP_GTE:
    case  AST_EXP_EQ:
    case  AST_EXP_NE:
        gen_exp_rel(out, e);
        break;
    default:
        fprintf(stderr, "Unsupported sub_kind %d\n", e->sub_kind);
    }
}
