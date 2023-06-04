/*
    Tiny Language Compiler (tlc)

    AST related files

    2016年 木村啓二
*/

#ifndef  AST_H
#define  AST_H

/* ASTの主種別 / AST main kinds */
enum {
    AST_KIND_NONE,
    AST_KIND_FUNC,
    AST_KIND_STM,
    AST_KIND_EXP
};

/* ASTの副種別 / AST sub kinds */
enum {
    AST_SUB_NONE,
    /* 文用の副種別 / for statements */
    AST_STM_LIST,
    AST_STM_DEC,
    AST_STM_ASIGN,
    AST_STM_IF,
    AST_STM_WHILE,
    AST_STM_FOR,
    AST_STM_DOWHILE,
    AST_STM_RETURN,
    /* 式用の副種別 / for expressions */
    AST_EXP_ASGN,
    AST_EXP_IDENT,
    AST_EXP_CNST_INT,
    AST_EXP_PRIME,
    AST_EXP_CALL,
    AST_EXP_PARAM,
    AST_EXP_UNARY_PLUS,
    AST_EXP_UNARY_MINUS,
    AST_EXP_MUL,
    AST_EXP_DIV,
    AST_EXP_ADD,
    AST_EXP_SUB,
    AST_EXP_LT,
    AST_EXP_GT,
    AST_EXP_LTE,
    AST_EXP_GTE,
    AST_EXP_EQ,
    AST_EXP_NE
};

/* 型 / variable types */
enum {
    TYPE_NONE,
    TYPE_INT
};

#define  AST_NUM_CHILDLEN  4

typedef struct AST_Node {
    int  kind;		/* 主種別 / main kind */
    int  sub_kind;	/* 副種別 / sub kind */
    int  lineno;
    int  id;
    int  val;		/* value for AST_EXP_CNST_INT */
    char *str;		/* string for AST_EXP_IDENT */
    int  reg;		/* assigned register */
    int  rank;		/* レジスタ割り付けとコード生成時の巡回優先度
                           Priority for register assignment and code generation */
    /* AST_EXP_IDENTの時のsymtab（予定 / for future extension） */
    struct AST_List *parent_list;
    struct AST_Node *parent;
    struct AST_Node *child[AST_NUM_CHILDLEN];
    struct AST_List *list;
    struct SymTab   *symtab;
} AST_Node;

/*
 * AST双方向リスト / doubly linked list for AST
 */
typedef struct AST_List {
    struct AST_Node *elem;
    struct AST_List *prev;
    struct AST_List *next;
    struct AST_Node *parent;
} AST_List;

/* ASTの根  / root of AST */
extern AST_List *AST_root;

#define TRAVERSE_AST_LIST(E, BEGIN, PROC) \
    do { (E) = (BEGIN); if ((E) != NULL) { do {	\
        PROC; \
        (E) = (E)->next; } while ((E) != (BEGIN)); }} while (0);

#define REV_TRAVERSE_AST_LIST(E, BEGIN, PROC) \
    do { (E) = (BEGIN); if ((E) != NULL) { do { (E) = (E)->prev; \
        PROC; \
    } while ((E) != (BEGIN)); }} while (0);

extern AST_Node *create_AST_Node(int kind, int sub_kind);
extern AST_Node *create_AST_Exp(int sub_kind);
extern AST_Node *create_AST_Stm(int sub_kind, int line);

/* リストlにノードnの要素を追加し、追加した要素のポインタを返す。
   lはリストの先頭要素である。また、lはNULLでも良い。
   Append node n to list l, then return the pointer of appended element.
   l is the first element of the list, and it can be NULL.
 */
extern AST_List *append_AST_List(AST_List *l, AST_Node *n);

extern void dump_ast();

#endif	/* AST_H */
