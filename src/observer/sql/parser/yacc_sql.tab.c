/* A Bison parser, made by GNU Bison 3.8.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30801

/* Bison version string.  */
#define YYBISON_VERSION "3.8.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 2 "yacc_sql.y"


#include "sql/parser/parse_defs.h"
#include "sql/parser/yacc_sql.tab.h"
#include "sql/parser/lex.yy.h"
// #include "common/log/log.h" // 包含C++中的头文件

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
int yydebug=1;
typedef struct ParserContext {
  Query * ssql;
  size_t select_length;
  size_t condition_length;
  size_t from_length;
  // 多行插入：将元组记录到insert_tuples中，元组个数记录到insert_count中
  // 解析时，当前正在读入的元组即为insert_tuples[insert_count]
  // 当一个元组被解析完毕后，insert_count自增，继续解析后续的元组
  size_t insert_count; // 在解析过程中，该数值随着读入的tuple个数增加而增加，始终指向当前未读取完的（即最后一个）元组
  LexTuple insert_tuples[MAX_TUPLES_ONE_INSERTION];
  // keuin: 为减少代码修改范围，values和value_length保留给单值操作使用，保持不变
  // 读取时，同时读取到两个缓冲区里。读取完毕使用时，根据语句类型选择使用单值或多值缓冲区（旧的使用单值缓冲区的代码不用变）
  size_t value_length;
  Value values[MAX_NUM];
  Condition conditions[MAX_NUM];
  CompOp comp;
  char id[MAX_NUM];
} ParserContext;

// 在解析子查询时，存储父查询信息的临时空间
// 注意：目前此临时空间的设计只支持一层嵌套查询，如需支持多层嵌套，需修改此处的设计！
static struct {
	Query *query;
	Condition *condition;
	size_t condition_length;
	CompOp op;
} sub_query_old_ctx;

//获取子串
char *substr(const char *s,int n1,int n2)/*从s中提取下标为n1~n2的字符组成一个新字符串，然后返回这个新串的首地址*/
{
  char *sp = malloc(sizeof(char) * (n2 - n1 + 2));
  int i, j = 0;
  for (i = n1; i <= n2; i++) {
    sp[j++] = s[i];
  }
  sp[j] = 0;
  return sp;
}

void yyerror(yyscan_t scanner, const char *str)
{
  ParserContext *context = (ParserContext *)(yyget_extra(scanner));
  query_reset(context->ssql);
  context->ssql->flag = SCF_ERROR;
  context->condition_length = 0;
  context->from_length = 0;
  context->select_length = 0;
  context->value_length = 0;
  context->insert_count = 0;
  context->ssql->sstr.insertion.tuple_count = 0;
  printf("parse sql failed. error=%s", str);
}

// INSERT语句中每个tuple的语义动作
static void insert_tuple_add(ParserContext *CONTEXT) {
	// 一个tuple读完了，把他复制到tuple list里，供多值insert使用
	// 逐个复制当前tuple的每个值
	LexTuple *copy_target = &CONTEXT->insert_tuples[CONTEXT->insert_count];
	size_t value_count = CONTEXT->value_length;
	for (size_t i = 0; i < value_count; ++i) {
		copy_target->values[i] = CONTEXT->values[i];
	}
	copy_target->count = value_count;
	CONTEXT->insert_count++;
	CONTEXT->value_length = 0; // 当前元组读取完毕，丢弃元组读入缓冲区中的所有标量值，防止前一个元组的值出现在下一个元组里
}

ParserContext *get_context(yyscan_t scanner)
{
  return (ParserContext *)yyget_extra(scanner);
}

#define CONTEXT get_context(scanner)


#line 160 "yacc_sql.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "yacc_sql.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SEMICOLON = 3,                  /* SEMICOLON  */
  YYSYMBOL_CREATE = 4,                     /* CREATE  */
  YYSYMBOL_DROP = 5,                       /* DROP  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_TABLES = 7,                     /* TABLES  */
  YYSYMBOL_INDEX = 8,                      /* INDEX  */
  YYSYMBOL_SELECT = 9,                     /* SELECT  */
  YYSYMBOL_DESC = 10,                      /* DESC  */
  YYSYMBOL_SHOW = 11,                      /* SHOW  */
  YYSYMBOL_SYNC = 12,                      /* SYNC  */
  YYSYMBOL_INSERT = 13,                    /* INSERT  */
  YYSYMBOL_DELETE = 14,                    /* DELETE  */
  YYSYMBOL_UPDATE = 15,                    /* UPDATE  */
  YYSYMBOL_LBRACE = 16,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 17,                    /* RBRACE  */
  YYSYMBOL_COMMA = 18,                     /* COMMA  */
  YYSYMBOL_TRX_BEGIN = 19,                 /* TRX_BEGIN  */
  YYSYMBOL_TRX_COMMIT = 20,                /* TRX_COMMIT  */
  YYSYMBOL_TRX_ROLLBACK = 21,              /* TRX_ROLLBACK  */
  YYSYMBOL_INT_T = 22,                     /* INT_T  */
  YYSYMBOL_STRING_T = 23,                  /* STRING_T  */
  YYSYMBOL_TEXT_T = 24,                    /* TEXT_T  */
  YYSYMBOL_FLOAT_T = 25,                   /* FLOAT_T  */
  YYSYMBOL_HELP = 26,                      /* HELP  */
  YYSYMBOL_EXIT = 27,                      /* EXIT  */
  YYSYMBOL_DOT = 28,                       /* DOT  */
  YYSYMBOL_INTO = 29,                      /* INTO  */
  YYSYMBOL_VALUES = 30,                    /* VALUES  */
  YYSYMBOL_FROM = 31,                      /* FROM  */
  YYSYMBOL_WHERE = 32,                     /* WHERE  */
  YYSYMBOL_AND = 33,                       /* AND  */
  YYSYMBOL_SET = 34,                       /* SET  */
  YYSYMBOL_ON = 35,                        /* ON  */
  YYSYMBOL_LOAD = 36,                      /* LOAD  */
  YYSYMBOL_DATA = 37,                      /* DATA  */
  YYSYMBOL_DATE_T = 38,                    /* DATE_T  */
  YYSYMBOL_INFILE = 39,                    /* INFILE  */
  YYSYMBOL_EQ = 40,                        /* EQ  */
  YYSYMBOL_LT = 41,                        /* LT  */
  YYSYMBOL_GT = 42,                        /* GT  */
  YYSYMBOL_LE = 43,                        /* LE  */
  YYSYMBOL_GE = 44,                        /* GE  */
  YYSYMBOL_NE = 45,                        /* NE  */
  YYSYMBOL_IN = 46,                        /* IN  */
  YYSYMBOL_MAX = 47,                       /* MAX  */
  YYSYMBOL_MIN = 48,                       /* MIN  */
  YYSYMBOL_COUNT = 49,                     /* COUNT  */
  YYSYMBOL_AVG = 50,                       /* AVG  */
  YYSYMBOL_ORDER = 51,                     /* ORDER  */
  YYSYMBOL_BY = 52,                        /* BY  */
  YYSYMBOL_ASC = 53,                       /* ASC  */
  YYSYMBOL_INNER = 54,                     /* INNER  */
  YYSYMBOL_JOIN = 55,                      /* JOIN  */
  YYSYMBOL_UNIQUE = 56,                    /* UNIQUE  */
  YYSYMBOL_GROUP = 57,                     /* GROUP  */
  YYSYMBOL_NULLABLE = 58,                  /* NULLABLE  */
  YYSYMBOL_NULL_K = 59,                    /* NULL_K  */
  YYSYMBOL_NOT = 60,                       /* NOT  */
  YYSYMBOL_IS = 61,                        /* IS  */
  YYSYMBOL_ADD = 62,                       /* ADD  */
  YYSYMBOL_SUB = 63,                       /* SUB  */
  YYSYMBOL_DIV = 64,                       /* DIV  */
  YYSYMBOL_NUMBER = 65,                    /* NUMBER  */
  YYSYMBOL_FLOAT = 66,                     /* FLOAT  */
  YYSYMBOL_ID = 67,                        /* ID  */
  YYSYMBOL_PATH = 68,                      /* PATH  */
  YYSYMBOL_SSS = 69,                       /* SSS  */
  YYSYMBOL_STAR = 70,                      /* STAR  */
  YYSYMBOL_STRING_V = 71,                  /* STRING_V  */
  YYSYMBOL_YYACCEPT = 72,                  /* $accept  */
  YYSYMBOL_commands = 73,                  /* commands  */
  YYSYMBOL_command = 74,                   /* command  */
  YYSYMBOL_exit = 75,                      /* exit  */
  YYSYMBOL_help = 76,                      /* help  */
  YYSYMBOL_sync = 77,                      /* sync  */
  YYSYMBOL_begin = 78,                     /* begin  */
  YYSYMBOL_commit = 79,                    /* commit  */
  YYSYMBOL_rollback = 80,                  /* rollback  */
  YYSYMBOL_drop_table = 81,                /* drop_table  */
  YYSYMBOL_show_tables = 82,               /* show_tables  */
  YYSYMBOL_desc_table = 83,                /* desc_table  */
  YYSYMBOL_create_index = 84,              /* create_index  */
  YYSYMBOL_drop_index = 85,                /* drop_index  */
  YYSYMBOL_create_table = 86,              /* create_table  */
  YYSYMBOL_attr_def_list = 87,             /* attr_def_list  */
  YYSYMBOL_attr_def = 88,                  /* attr_def  */
  YYSYMBOL_number = 89,                    /* number  */
  YYSYMBOL_type = 90,                      /* type  */
  YYSYMBOL_ID_get = 91,                    /* ID_get  */
  YYSYMBOL_insert = 92,                    /* insert  */
  YYSYMBOL_93_1 = 93,                      /* $@1  */
  YYSYMBOL_tuple = 94,                     /* tuple  */
  YYSYMBOL_tuple_list = 95,                /* tuple_list  */
  YYSYMBOL_96_2 = 96,                      /* $@2  */
  YYSYMBOL_value_list = 97,                /* value_list  */
  YYSYMBOL_value = 98,                     /* value  */
  YYSYMBOL_delete = 99,                    /* delete  */
  YYSYMBOL_update = 100,                   /* update  */
  YYSYMBOL_select = 101,                   /* select  */
  YYSYMBOL_relations = 102,                /* relations  */
  YYSYMBOL_join_list = 103,                /* join_list  */
  YYSYMBOL_rel_list = 104,                 /* rel_list  */
  YYSYMBOL_groups = 105,                   /* groups  */
  YYSYMBOL_group_list = 106,               /* group_list  */
  YYSYMBOL_orders = 107,                   /* orders  */
  YYSYMBOL_order_list = 108,               /* order_list  */
  YYSYMBOL_ATT = 109,                      /* ATT  */
  YYSYMBOL_E_ = 110,                       /* E_  */
  YYSYMBOL_E = 111,                        /* E  */
  YYSYMBOL_T = 112,                        /* T  */
  YYSYMBOL_F = 113,                        /* F  */
  YYSYMBOL_AGG = 114,                      /* AGG  */
  YYSYMBOL_select_attr = 115,              /* select_attr  */
  YYSYMBOL_attr_list = 116,                /* attr_list  */
  YYSYMBOL_where = 117,                    /* where  */
  YYSYMBOL_condition_list = 118,           /* condition_list  */
  YYSYMBOL_condition = 119,                /* condition  */
  YYSYMBOL_120_3 = 120,                    /* $@3  */
  YYSYMBOL_121_4 = 121,                    /* $@4  */
  YYSYMBOL_122_5 = 122,                    /* $@5  */
  YYSYMBOL_subQuery = 123,                 /* subQuery  */
  YYSYMBOL_124_6 = 124,                    /* $@6  */
  YYSYMBOL_comOp = 125,                    /* comOp  */
  YYSYMBOL_load_data = 126                 /* load_data  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   359

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  72
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  55
/* YYNRULES -- Number of rules.  */
#define YYNRULES  161
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  326

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   326


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   185,   185,   187,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   223,   228,   233,   239,   245,   251,   257,
     263,   269,   276,   282,   290,   297,   306,   308,   312,   323,
     334,   345,   358,   361,   362,   363,   364,   365,   368,   377,
     377,   394,   396,   397,   397,   401,   403,   408,   411,   414,
     418,   424,   434,   444,   458,   461,   463,   466,   468,   472,
     474,   479,   484,   489,   494,   499,   504,   506,   511,   516,
     521,   526,   531,   536,   538,   543,   548,   553,   558,   563,
     568,   570,   575,   580,   585,   590,   595,   601,   606,   611,
     616,   624,   628,   631,   634,   638,   641,   644,   648,   652,
     655,   658,   661,   666,   671,   676,   681,   686,   691,   696,
     701,   706,   711,   717,   718,   721,   723,   724,   726,   728,
     732,   734,   739,   744,   744,   773,   773,   799,   799,   825,
     850,   892,   892,   931,   932,   933,   934,   935,   936,   937,
     938,   942
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SEMICOLON", "CREATE",
  "DROP", "TABLE", "TABLES", "INDEX", "SELECT", "DESC", "SHOW", "SYNC",
  "INSERT", "DELETE", "UPDATE", "LBRACE", "RBRACE", "COMMA", "TRX_BEGIN",
  "TRX_COMMIT", "TRX_ROLLBACK", "INT_T", "STRING_T", "TEXT_T", "FLOAT_T",
  "HELP", "EXIT", "DOT", "INTO", "VALUES", "FROM", "WHERE", "AND", "SET",
  "ON", "LOAD", "DATA", "DATE_T", "INFILE", "EQ", "LT", "GT", "LE", "GE",
  "NE", "IN", "MAX", "MIN", "COUNT", "AVG", "ORDER", "BY", "ASC", "INNER",
  "JOIN", "UNIQUE", "GROUP", "NULLABLE", "NULL_K", "NOT", "IS", "ADD",
  "SUB", "DIV", "NUMBER", "FLOAT", "ID", "PATH", "SSS", "STAR", "STRING_V",
  "$accept", "commands", "command", "exit", "help", "sync", "begin",
  "commit", "rollback", "drop_table", "show_tables", "desc_table",
  "create_index", "drop_index", "create_table", "attr_def_list",
  "attr_def", "number", "type", "ID_get", "insert", "$@1", "tuple",
  "tuple_list", "$@2", "value_list", "value", "delete", "update", "select",
  "relations", "join_list", "rel_list", "groups", "group_list", "orders",
  "order_list", "ATT", "E_", "E", "T", "F", "AGG", "select_attr",
  "attr_list", "where", "condition_list", "condition", "$@3", "$@4", "$@5",
  "subQuery", "$@6", "comOp", "load_data", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-254)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-76)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -254,    11,  -254,    40,   159,    75,   -31,    33,  -254,    27,
      32,    28,    93,    37,  -254,  -254,  -254,  -254,  -254,    63,
     105,   121,   127,   130,   109,    62,   114,  -254,     2,  -254,
    -254,    36,  -254,  -254,   154,  -254,  -254,  -254,  -254,  -254,
    -254,  -254,  -254,  -254,  -254,  -254,  -254,  -254,  -254,  -254,
    -254,  -254,  -254,  -254,  -254,  -254,  -254,  -254,   165,  -254,
      87,     9,   167,  -254,  -254,   115,   116,   176,   119,   122,
     160,  -254,   195,    87,   -29,  -254,   195,   183,  -254,  -254,
     150,   151,   185,    26,    39,   189,   164,   166,    66,   173,
     174,   175,   199,    93,  -254,    65,   214,  -254,    93,    93,
      93,    93,   228,   213,   201,  -254,  -254,    75,  -254,  -254,
     202,   219,   224,   210,  -254,   242,   211,   242,  -254,  -254,
     212,    16,    17,   262,    25,   263,    58,   102,   247,   181,
      35,  -254,  -254,   216,  -254,   -29,   -29,  -254,  -254,   217,
     218,   251,   195,   195,   233,   224,   272,   110,  -254,   254,
    -254,  -254,   100,  -254,  -254,   266,  -254,   232,  -254,   234,
    -254,  -254,   235,  -254,  -254,   237,   278,   238,   239,   278,
    -254,   110,   279,   240,   241,   279,  -254,  -254,   233,  -254,
     282,   229,   295,   245,  -254,  -254,   264,   170,  -254,    93,
     230,   280,   170,   242,   242,  -254,  -254,   308,   299,   300,
     301,   302,  -254,   192,   101,  -254,   280,  -254,   193,   177,
    -254,   214,   217,   305,  -254,  -254,  -254,  -254,  -254,    55,
     256,   309,   267,   310,   311,   313,   317,  -254,  -254,  -254,
    -254,  -254,  -254,  -254,   281,   271,   131,   110,  -254,   224,
    -254,  -254,  -254,  -254,   265,  -254,  -254,  -254,  -254,   278,
     268,   278,   278,   278,  -254,   233,   279,   269,   279,   279,
     279,  -254,  -254,   282,  -254,   273,  -254,   274,   320,   275,
    -254,   170,   322,   272,  -254,   182,  -254,   318,   324,  -254,
    -254,    93,    87,   280,  -254,  -254,   194,  -254,  -254,  -254,
     197,  -254,  -254,  -254,  -254,   326,  -254,  -254,   327,   310,
    -254,  -254,    93,  -254,  -254,   325,   328,  -254,   278,   278,
     279,   279,  -254,  -254,  -254,   311,    87,   329,   330,  -254,
    -254,  -254,  -254,  -254,   331,  -254
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,    75,     1,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,     0,    36,    37,    38,    34,    33,     0,
       0,     0,     0,     0,     0,     0,     0,    70,     0,    67,
      68,   108,    69,   107,     0,    20,    19,    14,    15,    16,
      17,     9,    10,    11,    12,    13,     8,     5,   122,     7,
       6,     4,    24,    25,    27,    28,    22,    23,   121,    29,
      30,   114,   117,    21,    18,     0,     0,     0,     0,     0,
     108,   121,   135,   111,   114,   117,   135,     0,    41,    40,
       0,     0,     0,     0,    86,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   120,     0,    77,     3,     0,     0,
       0,     0,     0,     0,     0,    39,    44,     0,   134,   133,
       0,     0,   138,     0,   118,    86,     0,    86,    87,   101,
       0,     0,     0,     0,     0,     0,     0,   100,     0,    86,
       0,   109,   110,     0,    74,   112,   113,   116,   115,     0,
       0,     0,   135,   135,    75,   138,     0,     0,    71,     0,
      91,   105,    86,    89,   103,     0,   124,     0,   126,     0,
     130,   127,     0,   129,   132,     0,   100,     0,     0,   100,
      94,     0,    86,     0,     0,    86,    80,   119,    75,    58,
      46,     0,     0,     0,   137,   136,    93,     0,    59,   151,
       0,   140,     0,    86,    86,    88,   102,     0,     0,     0,
       0,     0,    98,   100,   100,    96,   140,    84,    86,    86,
      82,    77,     0,     0,    53,    54,    57,    55,    56,    49,
       0,     0,    79,    65,    62,     0,     0,   153,   154,   155,
     156,   157,   158,   143,     0,   159,     0,     0,   139,   138,
      92,   106,    90,   104,     0,   123,   125,   128,   131,   100,
       0,   100,   100,   100,    95,    75,    86,     0,    86,    86,
      86,    81,    78,    46,    45,     0,    51,     0,     0,     0,
      73,     0,     0,     0,    60,     0,   152,     0,     0,   147,
     160,   151,   142,   140,    72,   161,   100,    99,    97,    76,
      86,    85,    83,    47,    52,     0,    50,    42,     0,    65,
      61,    63,     0,   151,   151,     0,     0,   141,   100,   100,
      86,    86,    48,    43,    66,    62,   150,     0,     0,   151,
     149,    64,   144,   146,     0,   148
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -254,  -254,  -254,  -254,  -254,  -254,  -254,  -254,  -254,  -254,
    -254,  -254,  -254,  -254,  -254,    86,   138,  -254,  -254,  -254,
    -254,  -254,    78,    38,  -254,    53,  -178,  -254,  -254,   128,
     246,     7,   144,   135,    -1,   172,    12,    54,    -4,     1,
       3,     6,    -2,  -254,   -59,  -140,  -196,  -152,  -254,  -254,
    -254,  -253,  -254,    84,  -254
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,   213,   180,   295,   219,   181,
      47,   224,   188,   274,   315,   272,    48,    49,    50,    51,
      52,    96,   134,    54,   118,    56,   119,    71,    59,    83,
      74,    75,    63,    77,   108,   148,   238,   191,   277,   278,
     305,   225,   226,   236,    64
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      55,    72,    60,    76,    61,   186,    73,    62,    53,   223,
     255,     2,   -31,    57,   239,     3,     4,   109,    93,   206,
       5,     6,     7,     8,     9,    10,    11,    12,   306,    13,
      14,    15,    16,   156,   158,   100,    78,    17,    18,   -75,
      79,   101,   161,   114,   157,   159,    65,    19,    66,   115,
     317,   318,   177,   162,   -75,    58,    80,    13,    20,    21,
      22,    23,    24,    81,    95,    25,   324,   116,    26,    70,
      27,   265,    33,   100,    28,   164,    29,    30,    31,   101,
      32,    33,    94,   184,   185,   283,   165,   307,    98,    99,
      25,    12,   117,   299,   130,    82,    67,    98,    99,   284,
      85,   135,   136,   142,    84,   143,   137,   138,    73,    12,
     193,   252,   166,   266,   150,   267,   153,    91,    13,   167,
     167,    86,    20,    21,    22,    23,   189,   151,   176,   154,
     168,   123,   131,   124,    27,   132,   125,    87,    28,   170,
      29,    30,    70,    88,    32,    33,    89,   281,   190,    98,
      99,   195,    27,   194,   253,   169,    28,    97,    29,    30,
      70,    90,    32,    33,   196,    68,    92,    69,   -26,    27,
     -32,   207,   190,    28,   210,    29,    30,    70,   202,    32,
      33,   205,   102,   103,   104,   211,   105,   259,    95,   106,
      27,   172,   240,   242,    28,   173,    29,    30,    70,   173,
      32,    33,   249,   256,   308,   241,   243,   310,   261,   174,
     167,   173,   167,   107,   110,   173,   254,   111,   112,   113,
     250,   257,   227,   228,   229,   230,   231,   232,   120,    27,
     260,   121,   133,   122,   175,    29,    30,   282,   190,    32,
     126,   127,   128,   235,   139,   251,   258,   309,   140,   146,
     311,   214,   215,   216,   217,   150,   147,   153,   291,   292,
      13,   151,   289,   154,   287,   288,   129,   218,   141,   144,
     227,   228,   229,   230,   231,   232,   233,   149,   152,   160,
     163,   155,   171,   178,   179,   182,   183,    25,   187,   195,
     234,   235,    98,    99,   192,   197,   167,   173,   196,   198,
     212,   199,   200,   316,   201,   203,   204,   208,   209,   240,
     242,   220,   221,   237,   244,    24,   245,   246,   247,   248,
     241,   243,   264,   268,    26,   269,     5,   279,   271,   273,
     275,   280,   285,   296,   303,   286,   290,   297,   294,   300,
     304,   319,   298,   312,   313,   320,   322,   323,   325,   293,
     263,   301,   314,   321,   276,   262,   145,   270,   222,   302
};

static const yytype_int16 yycheck[] =
{
       1,     5,     1,     5,     1,   145,     5,     1,     1,   187,
     206,     0,     3,     1,   192,     4,     5,    76,    16,   171,
       9,    10,    11,    12,    13,    14,    15,    16,   281,    18,
      19,    20,    21,    17,    17,    64,    67,    26,    27,     3,
       7,    70,    17,    17,    28,    28,     6,    36,     8,    10,
     303,   304,    17,    28,    18,     1,    29,    18,    47,    48,
      49,    50,    51,    31,    28,    54,   319,    28,    57,    67,
      59,    16,    70,    64,    63,    17,    65,    66,    67,    70,
      69,    70,    28,   142,   143,   237,    28,   283,    62,    63,
      54,    16,    53,   271,    93,    67,    56,    62,    63,   239,
      37,    98,    99,   107,    67,   107,   100,   101,   107,    16,
      10,    10,    10,    58,   115,    60,   117,    55,    18,    18,
      18,    16,    47,    48,    49,    50,    16,   115,   129,   117,
      28,    65,    67,    67,    59,    70,    70,    16,    63,   127,
      65,    66,    67,    16,    69,    70,    16,    16,   147,    62,
      63,   152,    59,    53,    53,    53,    63,     3,    65,    66,
      67,    52,    69,    70,   152,     6,    52,     8,     3,    59,
       3,   172,   171,    63,   175,    65,    66,    67,   166,    69,
      70,   169,    67,    67,     8,   178,    67,    10,    28,    67,
      59,    10,   193,   194,    63,    18,    65,    66,    67,    18,
      69,    70,    10,    10,    10,   193,   194,    10,   209,    28,
      18,    18,    18,    18,    31,    18,   204,    67,    67,    34,
      28,    28,    40,    41,    42,    43,    44,    45,    39,    59,
      53,    67,    18,    67,    53,    65,    66,   236,   237,    69,
      67,    67,    67,    61,    16,    53,    53,    53,    35,    30,
      53,    22,    23,    24,    25,   256,    32,   258,   259,   260,
      18,   249,   255,   251,   252,   253,    67,    38,    67,    67,
      40,    41,    42,    43,    44,    45,    46,    67,    67,    17,
      17,    69,    35,    67,    67,    67,    35,    54,    16,   290,
      60,    61,    62,    63,    40,    29,    18,    18,   286,    67,
      18,    67,    67,   302,    67,    67,    67,    67,    67,   310,
     311,    16,    67,    33,     6,    51,    17,    17,    17,    17,
     308,   309,    17,    67,    57,    16,     9,    46,    18,    18,
      17,    60,    67,    59,    16,    67,    67,    17,    65,    17,
      16,    16,    67,    17,    17,    17,    17,    17,    17,   263,
     212,   273,   299,   315,   226,   211,   110,   222,   186,   275
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    73,     0,     4,     5,     9,    10,    11,    12,    13,
      14,    15,    16,    18,    19,    20,    21,    26,    27,    36,
      47,    48,    49,    50,    51,    54,    57,    59,    63,    65,
      66,    67,    69,    70,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    92,    98,    99,
     100,   101,   102,   103,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   126,     6,     8,    56,     6,     8,
      67,   109,   110,   111,   112,   113,   114,   115,    67,     7,
      29,    31,    67,   111,    67,    37,    16,    16,    16,    16,
      52,    55,    52,    16,   109,    28,   103,     3,    62,    63,
      64,    70,    67,    67,     8,    67,    67,    18,   116,   116,
      31,    67,    67,    34,    17,    10,    28,    53,   106,   108,
      39,    67,    67,    65,    67,    70,    67,    67,    67,    67,
     111,    67,    70,    18,   104,   112,   112,   113,   113,    16,
      35,    67,   110,   114,    67,   102,    30,    32,   117,    67,
     106,   108,    67,   106,   108,    69,    17,    28,    17,    28,
      17,    17,    28,    17,    17,    28,    10,    18,    28,    53,
     108,    35,    10,    18,    28,    53,   106,    17,    67,    67,
      88,    91,    67,    35,   116,   116,   117,    16,    94,    16,
     111,   119,    40,    10,    53,   106,   108,    29,    67,    67,
      67,    67,   108,    67,    67,   108,   119,   106,    67,    67,
     106,   103,    18,    87,    22,    23,    24,    25,    38,    90,
      16,    67,   107,    98,    93,   123,   124,    40,    41,    42,
      43,    44,    45,    46,    60,    61,   125,    33,   118,    98,
     106,   108,   106,   108,     6,    17,    17,    17,    17,    10,
      28,    53,    10,    53,   108,   118,    10,    28,    53,    10,
      53,   106,   104,    88,    17,    16,    58,    60,    67,    16,
     105,    18,    97,    18,    95,    17,   101,   120,   121,    46,
      60,    16,   111,   119,   117,    67,    67,   108,   108,   103,
      67,   106,   106,    87,    65,    89,    59,    17,    67,    98,
      17,    94,   125,    16,    16,   122,   123,   118,    10,    53,
      10,    53,    17,    17,    97,    96,   111,   123,   123,    16,
      17,    95,    17,    17,   123,    17
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    72,    73,    73,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    84,    85,    86,    87,    87,    88,    88,
      88,    88,    89,    90,    90,    90,    90,    90,    91,    93,
      92,    94,    95,    96,    95,    97,    97,    98,    98,    98,
      98,    99,   100,   101,   102,   103,   103,   104,   104,   105,
     105,   105,   105,   105,   105,   105,   106,   106,   106,   106,
     106,   106,   106,   107,   107,   107,   107,   107,   107,   107,
     108,   108,   108,   108,   108,   108,   108,   109,   109,   109,
     109,   110,   111,   111,   111,   112,   112,   112,   113,   113,
     113,   113,   113,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   115,   115,   116,   116,   116,   117,   117,
     118,   118,   119,   120,   119,   121,   119,   122,   119,   119,
     119,   124,   123,   125,   125,   125,   125,   125,   125,   125,
     125,   126
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     2,     8,     9,     3,     7,     0,     3,     5,     2,
       4,     3,     1,     1,     1,     1,     1,     1,     1,     0,
       7,     4,     0,     0,     4,     0,     3,     1,     1,     1,
       1,     4,     7,     7,     3,     0,     7,     0,     4,     0,
       4,     6,     5,     7,     5,     7,     0,     3,     5,     4,
       6,     4,     6,     0,     4,     6,     5,     7,     5,     7,
       0,     3,     5,     4,     6,     4,     6,     1,     1,     3,
       3,     1,     3,     3,     1,     3,     3,     1,     3,     4,
       2,     1,     1,     6,     4,     6,     4,     4,     6,     4,
       4,     6,     4,     2,     2,     0,     3,     3,     0,     3,
       0,     3,     3,     0,     6,     0,     6,     0,     7,     5,
       5,     0,     2,     1,     1,     1,     1,     1,     1,     1,
       2,     7
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (scanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, scanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *scanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *scanner)
{
  YY_USE (yyvaluep);
  YY_USE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 33: /* exit: EXIT  */
#line 223 "yacc_sql.y"
         {
        CONTEXT->ssql->flag=SCF_EXIT;//"exit";
    }
#line 1516 "yacc_sql.tab.c"
    break;

  case 34: /* help: HELP  */
#line 228 "yacc_sql.y"
         {
        CONTEXT->ssql->flag=SCF_HELP;//"help";
    }
#line 1524 "yacc_sql.tab.c"
    break;

  case 35: /* sync: SYNC  */
#line 233 "yacc_sql.y"
         {
      CONTEXT->ssql->flag = SCF_SYNC;
    }
#line 1532 "yacc_sql.tab.c"
    break;

  case 36: /* begin: TRX_BEGIN  */
#line 239 "yacc_sql.y"
              {
      CONTEXT->ssql->flag = SCF_BEGIN;
    }
#line 1540 "yacc_sql.tab.c"
    break;

  case 37: /* commit: TRX_COMMIT  */
#line 245 "yacc_sql.y"
               {
      CONTEXT->ssql->flag = SCF_COMMIT;
    }
#line 1548 "yacc_sql.tab.c"
    break;

  case 38: /* rollback: TRX_ROLLBACK  */
#line 251 "yacc_sql.y"
                 {
      CONTEXT->ssql->flag = SCF_ROLLBACK;
    }
#line 1556 "yacc_sql.tab.c"
    break;

  case 39: /* drop_table: DROP TABLE ID  */
#line 257 "yacc_sql.y"
                  {
        CONTEXT->ssql->flag = SCF_DROP_TABLE;//"drop_table";
        drop_table_init(&CONTEXT->ssql->sstr.drop_table, (yyvsp[0].string));
    }
#line 1565 "yacc_sql.tab.c"
    break;

  case 40: /* show_tables: SHOW TABLES  */
#line 263 "yacc_sql.y"
                {
      CONTEXT->ssql->flag = SCF_SHOW_TABLES;
    }
#line 1573 "yacc_sql.tab.c"
    break;

  case 41: /* desc_table: DESC ID  */
#line 269 "yacc_sql.y"
            {
      CONTEXT->ssql->flag = SCF_DESC_TABLE;
      desc_table_init(&CONTEXT->ssql->sstr.desc_table, (yyvsp[0].string));
    }
#line 1582 "yacc_sql.tab.c"
    break;

  case 42: /* create_index: CREATE INDEX ID ON ID LBRACE ID RBRACE  */
#line 277 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
			create_index_init(&CONTEXT->ssql->sstr.create_index, (yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string), false);
		}
#line 1591 "yacc_sql.tab.c"
    break;

  case 43: /* create_index: CREATE UNIQUE INDEX ID ON ID LBRACE ID RBRACE  */
#line 283 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
                        create_index_init(&CONTEXT->ssql->sstr.create_index, (yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string), true);
		}
#line 1600 "yacc_sql.tab.c"
    break;

  case 44: /* drop_index: DROP INDEX ID  */
#line 291 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_DROP_INDEX;//"drop_index";
			drop_index_init(&CONTEXT->ssql->sstr.drop_index, (yyvsp[0].string));
		}
#line 1609 "yacc_sql.tab.c"
    break;

  case 45: /* create_table: CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE  */
#line 298 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_CREATE_TABLE;//"create_table";
			// CONTEXT->ssql->sstr.create_table.attribute_count = CONTEXT->value_length;
			create_table_init_name(&CONTEXT->ssql->sstr.create_table, (yyvsp[-4].string));
			//临时变量清零	
			CONTEXT->value_length = 0;
		}
#line 1621 "yacc_sql.tab.c"
    break;

  case 47: /* attr_def_list: COMMA attr_def attr_def_list  */
#line 308 "yacc_sql.y"
                                   {    }
#line 1627 "yacc_sql.tab.c"
    break;

  case 48: /* attr_def: ID_get type LBRACE number RBRACE  */
#line 313 "yacc_sql.y"
                {
			AttrInfo attribute;
			attr_info_init(&attribute, CONTEXT->id, (yyvsp[-3].number), (yyvsp[-1].number), false);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name =(char*)malloc(sizeof(char));
			// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id); 
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type = $2;  
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length = $4;
			CONTEXT->value_length++;
		}
#line 1642 "yacc_sql.tab.c"
    break;

  case 49: /* attr_def: ID_get type  */
#line 324 "yacc_sql.y"
                {
			AttrInfo attribute;
			attr_info_init(&attribute, CONTEXT->id, (yyvsp[0].number), 4, false);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name=(char*)malloc(sizeof(char));
			// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id); 
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type=$2;  
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length=4; // default attribute length
			CONTEXT->value_length++;
		}
#line 1657 "yacc_sql.tab.c"
    break;

  case 50: /* attr_def: ID_get type NOT NULL_K  */
#line 335 "yacc_sql.y"
                        {
				AttrInfo attribute;
				attr_info_init(&attribute, CONTEXT->id, (yyvsp[-2].number), 4, false);
				create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name=(char*)malloc(sizeof(char));
				// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id);
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type=$2;
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length=4; // default attribute length
				CONTEXT->value_length++;
			}
#line 1672 "yacc_sql.tab.c"
    break;

  case 51: /* attr_def: ID_get type NULLABLE  */
#line 346 "yacc_sql.y"
                        {
				AttrInfo attribute;
				attr_info_init(&attribute, CONTEXT->id, (yyvsp[-1].number), 4, true);
				create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name=(char*)malloc(sizeof(char));
				// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id);
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type=$2;
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length=4; // default attribute length
				CONTEXT->value_length++;
			}
#line 1687 "yacc_sql.tab.c"
    break;

  case 52: /* number: NUMBER  */
#line 358 "yacc_sql.y"
                       {(yyval.number) = (yyvsp[0].number);}
#line 1693 "yacc_sql.tab.c"
    break;

  case 53: /* type: INT_T  */
#line 361 "yacc_sql.y"
              { (yyval.number)=INTS; }
#line 1699 "yacc_sql.tab.c"
    break;

  case 54: /* type: STRING_T  */
#line 362 "yacc_sql.y"
                  { (yyval.number)=CHARS; }
#line 1705 "yacc_sql.tab.c"
    break;

  case 55: /* type: FLOAT_T  */
#line 363 "yacc_sql.y"
                 { (yyval.number)=FLOATS; }
#line 1711 "yacc_sql.tab.c"
    break;

  case 56: /* type: DATE_T  */
#line 364 "yacc_sql.y"
                { (yyval.number)=DATE; }
#line 1717 "yacc_sql.tab.c"
    break;

  case 57: /* type: TEXT_T  */
#line 365 "yacc_sql.y"
                { (yyval.number)=TEXTS; }
#line 1723 "yacc_sql.tab.c"
    break;

  case 58: /* ID_get: ID  */
#line 369 "yacc_sql.y"
        {
		char *temp=(yyvsp[0].string); 
		snprintf(CONTEXT->id, sizeof(CONTEXT->id), "%s", temp);
	}
#line 1732 "yacc_sql.tab.c"
    break;

  case 59: /* $@1: %empty  */
#line 377 "yacc_sql.y"
                                { insert_tuple_add(CONTEXT); }
#line 1738 "yacc_sql.tab.c"
    break;

  case 60: /* insert: INSERT INTO ID VALUES tuple $@1 tuple_list  */
#line 378 "yacc_sql.y"
                {
			// CONTEXT->values[CONTEXT->value_length++] = *$6;

			CONTEXT->ssql->flag=SCF_INSERT;//"insert";
			// CONTEXT->ssql->sstr.insertion.relation_name = $3;
			// CONTEXT->ssql->sstr.insertion.value_num = CONTEXT->value_length;
			// for(i = 0; i < CONTEXT->value_length; i++){
			// 	CONTEXT->ssql->sstr.insertion.values[i] = CONTEXT->values[i];
      // }
			inserts_init(&CONTEXT->ssql->sstr.insertion, (yyvsp[-4].string), CONTEXT->insert_tuples, CONTEXT->insert_count);

      //临时变量清零
      CONTEXT->insert_count=0;
    }
#line 1757 "yacc_sql.tab.c"
    break;

  case 63: /* $@2: %empty  */
#line 397 "yacc_sql.y"
                  {
		insert_tuple_add(CONTEXT);
	}
#line 1765 "yacc_sql.tab.c"
    break;

  case 66: /* value_list: COMMA value value_list  */
#line 403 "yacc_sql.y"
                              {
  		// CONTEXT->values[CONTEXT->value_length++] = *$2;
	  }
#line 1773 "yacc_sql.tab.c"
    break;

  case 67: /* value: NUMBER  */
#line 408 "yacc_sql.y"
          {	
  		value_init_integer(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].number));
		}
#line 1781 "yacc_sql.tab.c"
    break;

  case 68: /* value: FLOAT  */
#line 411 "yacc_sql.y"
          {
  		value_init_float(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].floats));
		}
#line 1789 "yacc_sql.tab.c"
    break;

  case 69: /* value: SSS  */
#line 414 "yacc_sql.y"
         {
			(yyvsp[0].string) = substr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
  		value_init_string(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].string));
		}
#line 1798 "yacc_sql.tab.c"
    break;

  case 70: /* value: NULL_K  */
#line 418 "yacc_sql.y"
                        {
		value_init_null(&CONTEXT->values[CONTEXT->value_length++]);
		}
#line 1806 "yacc_sql.tab.c"
    break;

  case 71: /* delete: DELETE FROM ID where  */
#line 425 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_DELETE;//"delete";
			deletes_init_relation(&CONTEXT->ssql->sstr.deletion, (yyvsp[-1].string));
			deletes_set_conditions(&CONTEXT->ssql->sstr.deletion, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;	
    }
#line 1818 "yacc_sql.tab.c"
    break;

  case 72: /* update: UPDATE ID SET ID EQ value where  */
#line 435 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_UPDATE;//"update";
			Value *value = &CONTEXT->values[0];
			updates_init(&CONTEXT->ssql->sstr.update, (yyvsp[-5].string), (yyvsp[-3].string), value, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;
		}
#line 1830 "yacc_sql.tab.c"
    break;

  case 73: /* select: SELECT select_attr FROM relations where orders groups  */
#line 445 "yacc_sql.y"
                {
			// CONTEXT->ssql->sstr.selection.relations[CONTEXT->from_length++]=$4;
			selects_append_conditions(&CONTEXT->ssql->sstr.selection, CONTEXT->conditions, CONTEXT->condition_length);

			CONTEXT->ssql->flag=SCF_SELECT;//"select";
			// CONTEXT->ssql->sstr.selection.attr_num = CONTEXT->select_length;
			//临时变量清零
			CONTEXT->condition_length=0;
			CONTEXT->from_length=0;
			CONTEXT->select_length=0;
			CONTEXT->value_length = 0;
	}
#line 1847 "yacc_sql.tab.c"
    break;

  case 74: /* relations: ID join_list rel_list  */
#line 458 "yacc_sql.y"
                             {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-2].string));
	}
#line 1855 "yacc_sql.tab.c"
    break;

  case 76: /* join_list: INNER JOIN ID ON condition condition_list join_list  */
#line 463 "yacc_sql.y"
                                                     {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-4].string));
}
#line 1863 "yacc_sql.tab.c"
    break;

  case 78: /* rel_list: COMMA ID join_list rel_list  */
#line 468 "yacc_sql.y"
                                  {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-2].string));
	}
#line 1871 "yacc_sql.tab.c"
    break;

  case 80: /* groups: GROUP BY ID group_list  */
#line 474 "yacc_sql.y"
                        {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 1881 "yacc_sql.tab.c"
    break;

  case 81: /* groups: GROUP BY ID DOT ID group_list  */
#line 479 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 1891 "yacc_sql.tab.c"
    break;

  case 82: /* groups: GROUP BY ID ASC group_list  */
#line 484 "yacc_sql.y"
                            {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 1901 "yacc_sql.tab.c"
    break;

  case 83: /* groups: GROUP BY ID DOT ID ASC group_list  */
#line 489 "yacc_sql.y"
                                   {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 1911 "yacc_sql.tab.c"
    break;

  case 84: /* groups: GROUP BY ID DESC group_list  */
#line 494 "yacc_sql.y"
                             {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 1921 "yacc_sql.tab.c"
    break;

  case 85: /* groups: GROUP BY ID DOT ID DESC group_list  */
#line 499 "yacc_sql.y"
                                    {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 1931 "yacc_sql.tab.c"
    break;

  case 87: /* group_list: COMMA ID group_list  */
#line 506 "yacc_sql.y"
                      {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 1941 "yacc_sql.tab.c"
    break;

  case 88: /* group_list: COMMA ID DOT ID group_list  */
#line 511 "yacc_sql.y"
                              {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 1951 "yacc_sql.tab.c"
    break;

  case 89: /* group_list: COMMA ID ASC group_list  */
#line 516 "yacc_sql.y"
                           {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 1961 "yacc_sql.tab.c"
    break;

  case 90: /* group_list: COMMA ID DOT ID ASC group_list  */
#line 521 "yacc_sql.y"
                                  {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 1971 "yacc_sql.tab.c"
    break;

  case 91: /* group_list: COMMA ID DESC group_list  */
#line 526 "yacc_sql.y"
                            {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 1981 "yacc_sql.tab.c"
    break;

  case 92: /* group_list: COMMA ID DOT ID DESC group_list  */
#line 531 "yacc_sql.y"
                                   {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 1991 "yacc_sql.tab.c"
    break;

  case 94: /* orders: ORDER BY ID order_list  */
#line 538 "yacc_sql.y"
                        {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2001 "yacc_sql.tab.c"
    break;

  case 95: /* orders: ORDER BY ID DOT ID order_list  */
#line 543 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2011 "yacc_sql.tab.c"
    break;

  case 96: /* orders: ORDER BY ID ASC order_list  */
#line 548 "yacc_sql.y"
                            {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2021 "yacc_sql.tab.c"
    break;

  case 97: /* orders: ORDER BY ID DOT ID ASC order_list  */
#line 553 "yacc_sql.y"
                                   {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2031 "yacc_sql.tab.c"
    break;

  case 98: /* orders: ORDER BY ID DESC order_list  */
#line 558 "yacc_sql.y"
                             {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2041 "yacc_sql.tab.c"
    break;

  case 99: /* orders: ORDER BY ID DOT ID DESC order_list  */
#line 563 "yacc_sql.y"
                                    {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2051 "yacc_sql.tab.c"
    break;

  case 101: /* order_list: COMMA ID order_list  */
#line 570 "yacc_sql.y"
                      {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2061 "yacc_sql.tab.c"
    break;

  case 102: /* order_list: COMMA ID DOT ID order_list  */
#line 575 "yacc_sql.y"
                              {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2071 "yacc_sql.tab.c"
    break;

  case 103: /* order_list: COMMA ID ASC order_list  */
#line 580 "yacc_sql.y"
                           {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2081 "yacc_sql.tab.c"
    break;

  case 104: /* order_list: COMMA ID DOT ID ASC order_list  */
#line 585 "yacc_sql.y"
                                  {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2091 "yacc_sql.tab.c"
    break;

  case 105: /* order_list: COMMA ID DESC order_list  */
#line 590 "yacc_sql.y"
                            {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2101 "yacc_sql.tab.c"
    break;

  case 106: /* order_list: COMMA ID DOT ID DESC order_list  */
#line 595 "yacc_sql.y"
                                   {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2111 "yacc_sql.tab.c"
    break;

  case 107: /* ATT: STAR  */
#line 601 "yacc_sql.y"
            {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*");
			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
#line 2121 "yacc_sql.tab.c"
    break;

  case 108: /* ATT: ID  */
#line 606 "yacc_sql.y"
             {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[0].string));
			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
#line 2131 "yacc_sql.tab.c"
    break;

  case 109: /* ATT: ID DOT ID  */
#line 611 "yacc_sql.y"
                    {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-2].string), (yyvsp[0].string));
			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
#line 2141 "yacc_sql.tab.c"
    break;

  case 110: /* ATT: ID DOT STAR  */
#line 616 "yacc_sql.y"
                      {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-2].string), "*");
			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
            	}
#line 2151 "yacc_sql.tab.c"
    break;

  case 111: /* E_: E  */
#line 624 "yacc_sql.y"
         {
		selects_append_attribute_plus(&CONTEXT->ssql->sstr.selection);
	}
#line 2159 "yacc_sql.tab.c"
    break;

  case 112: /* E: E ADD T  */
#line 628 "yacc_sql.y"
               {
		e_e_t(Add);
	}
#line 2167 "yacc_sql.tab.c"
    break;

  case 113: /* E: E SUB T  */
#line 631 "yacc_sql.y"
                 {
		e_e_t(Sub);
	}
#line 2175 "yacc_sql.tab.c"
    break;

  case 114: /* E: T  */
#line 634 "yacc_sql.y"
           {
		e_t();
	}
#line 2183 "yacc_sql.tab.c"
    break;

  case 115: /* T: T STAR F  */
#line 638 "yacc_sql.y"
                {
		t_t_f(Mul);
	}
#line 2191 "yacc_sql.tab.c"
    break;

  case 116: /* T: T DIV F  */
#line 641 "yacc_sql.y"
                 {
		t_t_f(Div);
	}
#line 2199 "yacc_sql.tab.c"
    break;

  case 117: /* T: F  */
#line 644 "yacc_sql.y"
           {
		t_f();
	}
#line 2207 "yacc_sql.tab.c"
    break;

  case 118: /* F: LBRACE E RBRACE  */
#line 648 "yacc_sql.y"
                       {
		f_e();
	}
#line 2215 "yacc_sql.tab.c"
    break;

  case 119: /* F: SUB LBRACE E RBRACE  */
#line 652 "yacc_sql.y"
                           {
			set_sub();
	}
#line 2223 "yacc_sql.tab.c"
    break;

  case 120: /* F: SUB ATT  */
#line 655 "yacc_sql.y"
                {
        		set_sub();
        	}
#line 2231 "yacc_sql.tab.c"
    break;

  case 121: /* F: ATT  */
#line 658 "yacc_sql.y"
            {
		//set_sub();
	}
#line 2239 "yacc_sql.tab.c"
    break;

  case 122: /* F: value  */
#line 661 "yacc_sql.y"
              {
        	Value *value = &CONTEXT->values[CONTEXT->value_length - 1];
		    relation_value_append(value);
        }
#line 2248 "yacc_sql.tab.c"
    break;

  case 123: /* AGG: MAX LBRACE ID DOT ID RBRACE  */
#line 666 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2258 "yacc_sql.tab.c"
    break;

  case 124: /* AGG: MAX LBRACE ID RBRACE  */
#line 671 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2268 "yacc_sql.tab.c"
    break;

  case 125: /* AGG: MIN LBRACE ID DOT ID RBRACE  */
#line 676 "yacc_sql.y"
                                {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2278 "yacc_sql.tab.c"
    break;

  case 126: /* AGG: MIN LBRACE ID RBRACE  */
#line 681 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2288 "yacc_sql.tab.c"
    break;

  case 127: /* AGG: COUNT LBRACE ID RBRACE  */
#line 686 "yacc_sql.y"
                           {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2298 "yacc_sql.tab.c"
    break;

  case 128: /* AGG: COUNT LBRACE ID DOT ID RBRACE  */
#line 691 "yacc_sql.y"
                                  {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2308 "yacc_sql.tab.c"
    break;

  case 129: /* AGG: COUNT LBRACE STAR RBRACE  */
#line 696 "yacc_sql.y"
                                  {
        	RelAttr attr;
        	relation_agg_attr_init(&attr, NULL, "*",Count);
                selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
            }
#line 2318 "yacc_sql.tab.c"
    break;

  case 130: /* AGG: COUNT LBRACE NUMBER RBRACE  */
#line 701 "yacc_sql.y"
                                       {
                    	RelAttr attr;
                    	relation_agg_attr_init(&attr, NULL, "*",Count);
                            selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
                        }
#line 2328 "yacc_sql.tab.c"
    break;

  case 131: /* AGG: AVG LBRACE ID DOT ID RBRACE  */
#line 706 "yacc_sql.y"
                                {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2338 "yacc_sql.tab.c"
    break;

  case 132: /* AGG: AVG LBRACE ID RBRACE  */
#line 711 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2348 "yacc_sql.tab.c"
    break;

  case 139: /* where: WHERE condition condition_list  */
#line 728 "yacc_sql.y"
                                     {	
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 2356 "yacc_sql.tab.c"
    break;

  case 141: /* condition_list: AND condition condition_list  */
#line 734 "yacc_sql.y"
                                   {
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 2364 "yacc_sql.tab.c"
    break;

  case 142: /* condition: E comOp E  */
#line 739 "yacc_sql.y"
             {
    		Condition condition;
        	condition_init_ex(&condition, CONTEXT->comp);
        	CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
#line 2374 "yacc_sql.tab.c"
    break;

  case 143: /* $@3: %empty  */
#line 744 "yacc_sql.y"
               { CONTEXT->comp = CONTAINED_BY; }
#line 2380 "yacc_sql.tab.c"
    break;

  case 144: /* condition: E IN $@3 LBRACE subQuery RBRACE  */
#line 745 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr* left_attr;
			//relation_attr_init(&left_attr, NULL, $1);
			//relation_attr_get(left_attr);

			Unevaluated *uneval = malloc(sizeof(Unevaluated));
			assert(uneval != NULL);
			uneval->type = UE_SELECT;
			Selects *sub_sel = CONTEXT->ssql->sstr.selection.sub_selection;
			assert(sub_sel != NULL);
			uneval->data.select = *(sub_sel);
			free(sub_sel);
			CONTEXT->ssql->sstr.selection.sub_selection = NULL; // 这个递归结构好像没啥用，直接扬了吧

			Value rval; // right value
			rval.type = UNEVALUATED;
			rval.data = uneval;

			RelAttr right_attr;
            right_attr.value=rval;
            right_attr.extype=val;

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, NULL, NULL, 0, &right_attr, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2413 "yacc_sql.tab.c"
    break;

  case 145: /* $@4: %empty  */
#line 773 "yacc_sql.y"
               { CONTEXT->comp = CONTAINED_BY; }
#line 2419 "yacc_sql.tab.c"
    break;

  case 146: /* condition: E IN $@4 LBRACE subQuery RBRACE  */
#line 774 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr *left_attr;
			//relation_attr_get(left_attr);

			Unevaluated *uneval = malloc(sizeof(Unevaluated));
			assert(uneval != NULL);
			uneval->type = UE_SELECT;
			Selects *sub_sel = CONTEXT->ssql->sstr.selection.sub_selection;
			assert(sub_sel != NULL);
			uneval->data.select = *(sub_sel);
			free(sub_sel);
			CONTEXT->ssql->sstr.selection.sub_selection = NULL; // 这个递归结构好像没啥用，直接扬了吧

			Value rval; // right value
			rval.type = UNEVALUATED;
			rval.data = uneval;
			RelAttr right_attr;
			right_attr.value=rval;
			right_attr.extype=val;
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, NULL, NULL, 0, &right_attr, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2449 "yacc_sql.tab.c"
    break;

  case 147: /* $@5: %empty  */
#line 799 "yacc_sql.y"
                   { CONTEXT->comp = NOT_CONTAINED_BY; }
#line 2455 "yacc_sql.tab.c"
    break;

  case 148: /* condition: E NOT IN $@5 LBRACE subQuery RBRACE  */
#line 800 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr *left_attr;
                        //relation_attr_get(left_attr);

			Unevaluated *uneval = malloc(sizeof(Unevaluated));
			assert(uneval != NULL);
			uneval->type = UE_SELECT;
			Selects *sub_sel = CONTEXT->ssql->sstr.selection.sub_selection;
			assert(sub_sel != NULL);
			uneval->data.select = *(sub_sel);
			free(sub_sel);
			CONTEXT->ssql->sstr.selection.sub_selection = NULL; // 这个递归结构好像没啥用，直接扬了吧

			Value rval; // right value
			rval.type = UNEVALUATED;
			rval.data = uneval;
			RelAttr right_attr;
			right_attr.value=rval;
			right_attr.extype=val;
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, NULL, NULL, 0, &right_attr, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2485 "yacc_sql.tab.c"
    break;

  case 149: /* condition: E comOp LBRACE subQuery RBRACE  */
#line 826 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr *left_attr;
                        //relation_attr_get(left_attr);
			Unevaluated *uneval = malloc(sizeof(Unevaluated));
			assert(uneval != NULL);
			uneval->type = UE_SELECT;
			Selects *sub_sel = CONTEXT->ssql->sstr.selection.sub_selection;
			assert(sub_sel != NULL);
			uneval->data.select = *(sub_sel);
			free(sub_sel);
			CONTEXT->ssql->sstr.selection.sub_selection = NULL; // 这个递归结构好像没啥用，直接扬了吧

			Value rval; // right value
			rval.type = UNEVALUATED;
			rval.data = uneval;
			RelAttr right_attr;
			right_attr.value=rval;
			right_attr.extype=val;
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, NULL, NULL, 0, &right_attr, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2514 "yacc_sql.tab.c"
    break;

  case 150: /* condition: LBRACE subQuery RBRACE comOp E  */
#line 851 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr* left_attr;
			//relation_attr_get(left_attr);//TODO
			Unevaluated *uneval = malloc(sizeof(Unevaluated));
			assert(uneval != NULL);
			uneval->type = UE_SELECT;
			Selects *sub_sel = CONTEXT->ssql->sstr.selection.sub_selection;
			assert(sub_sel != NULL);
			uneval->data.select = *(sub_sel);
			free(sub_sel);
			CONTEXT->ssql->sstr.selection.sub_selection = NULL; // 这个递归结构好像没啥用，直接扬了吧
			Value rval; // right value
			rval.type = UNEVALUATED;
			rval.data = uneval;

			RelAttr right_attr;
			right_attr.value=rval;
			right_attr.extype=val;

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, &right_attr, &rval, 1, NULL, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2544 "yacc_sql.tab.c"
    break;

  case 151: /* $@6: %empty  */
#line 892 "yacc_sql.y"
                {
		    // 保存上下文
			sub_query_old_ctx.query = CONTEXT->ssql;
			sub_query_old_ctx.op = CONTEXT->comp;
			sub_query_old_ctx.condition_length = CONTEXT->condition_length;
			sub_query_old_ctx.condition = malloc(sizeof(CONTEXT->conditions));
			memcpy(sub_query_old_ctx.condition, CONTEXT->conditions, sizeof(CONTEXT->conditions));
			// assert(sub_query_old_ctx.condition != NULL);

			CONTEXT->ssql = calloc(1, sizeof(Query)); // 临时的Query对象，在恢复上下文时被销毁
			CONTEXT->condition_length = 0;
			memset(CONTEXT->conditions, '\0', sizeof(CONTEXT->conditions)); // 清零缓冲区，便于debug

		}
#line 2563 "yacc_sql.tab.c"
    break;

  case 152: /* subQuery: $@6 select  */
#line 907 "yacc_sql.y"
                {
		    // 恢复上下文
		    // 现在ssql中保存的是子查询的上下文，CONTEXT内的诸如condition_length等变量已被select的语义动作
		    // 给移动到CONTEXT->ssql->sstr.selection内了，现只需将其转移到新的缓冲区中，并free掉、替换为父查询语句的上下文

			// 把子查询复制出来
			Selects *sub_sel = malloc(sizeof(Selects));
			assert(sub_sel != NULL);
		    *sub_sel = CONTEXT->ssql->sstr.selection;

			// 还原上下文到子查询运行前
			free(CONTEXT->ssql);
			CONTEXT->ssql = sub_query_old_ctx.query;
			CONTEXT->comp = sub_query_old_ctx.op;
			CONTEXT->condition_length = sub_query_old_ctx.condition_length;
			memcpy(CONTEXT->conditions, sub_query_old_ctx.condition, sizeof(CONTEXT->conditions));
			free(sub_query_old_ctx.condition);

			// 更新父查询的子查询指针
			CONTEXT->ssql->sstr.selection.sub_selection = sub_sel;
		}
#line 2589 "yacc_sql.tab.c"
    break;

  case 153: /* comOp: EQ  */
#line 931 "yacc_sql.y"
             { CONTEXT->comp = EQUAL_TO; }
#line 2595 "yacc_sql.tab.c"
    break;

  case 154: /* comOp: LT  */
#line 932 "yacc_sql.y"
         { CONTEXT->comp = LESS_THAN; }
#line 2601 "yacc_sql.tab.c"
    break;

  case 155: /* comOp: GT  */
#line 933 "yacc_sql.y"
         { CONTEXT->comp = GREAT_THAN; }
#line 2607 "yacc_sql.tab.c"
    break;

  case 156: /* comOp: LE  */
#line 934 "yacc_sql.y"
         { CONTEXT->comp = LESS_EQUAL; }
#line 2613 "yacc_sql.tab.c"
    break;

  case 157: /* comOp: GE  */
#line 935 "yacc_sql.y"
         { CONTEXT->comp = GREAT_EQUAL; }
#line 2619 "yacc_sql.tab.c"
    break;

  case 158: /* comOp: NE  */
#line 936 "yacc_sql.y"
         { CONTEXT->comp = NOT_EQUAL; }
#line 2625 "yacc_sql.tab.c"
    break;

  case 159: /* comOp: IS  */
#line 937 "yacc_sql.y"
         { CONTEXT->comp = IS_CompOP; }
#line 2631 "yacc_sql.tab.c"
    break;

  case 160: /* comOp: IS NOT  */
#line 938 "yacc_sql.y"
             {CONTEXT->comp = IS_NOT_CompOP;}
#line 2637 "yacc_sql.tab.c"
    break;

  case 161: /* load_data: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 943 "yacc_sql.y"
                {
		  CONTEXT->ssql->flag = SCF_LOAD_DATA;
			load_data_init(&CONTEXT->ssql->sstr.load_data, (yyvsp[0].string), (yyvsp[-3].string));
		}
#line 2646 "yacc_sql.tab.c"
    break;


#line 2650 "yacc_sql.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (scanner, YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 948 "yacc_sql.y"

//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);

int sql_parse(const char *s, Query *sqls){
	ParserContext context;
	memset(&context, 0, sizeof(context));

	yyscan_t scanner;
	yylex_init_extra(&context, &scanner);
	context.ssql = sqls;
	scan_string(s, scanner);
	int result = yyparse(scanner);
	yylex_destroy(scanner);
	return result;
}
