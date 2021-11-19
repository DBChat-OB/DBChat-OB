/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

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


#line 159 "yacc_sql.tab.c"

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

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_YACC_SQL_TAB_H_INCLUDED
# define YY_YY_YACC_SQL_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    SEMICOLON = 258,
    CREATE = 259,
    DROP = 260,
    TABLE = 261,
    TABLES = 262,
    INDEX = 263,
    SELECT = 264,
    DESC = 265,
    SHOW = 266,
    SYNC = 267,
    INSERT = 268,
    DELETE = 269,
    UPDATE = 270,
    LBRACE = 271,
    RBRACE = 272,
    COMMA = 273,
    TRX_BEGIN = 274,
    TRX_COMMIT = 275,
    TRX_ROLLBACK = 276,
    INT_T = 277,
    STRING_T = 278,
    FLOAT_T = 279,
    HELP = 280,
    EXIT = 281,
    DOT = 282,
    INTO = 283,
    VALUES = 284,
    FROM = 285,
    WHERE = 286,
    AND = 287,
    SET = 288,
    ON = 289,
    LOAD = 290,
    DATA = 291,
    DATE_T = 292,
    INFILE = 293,
    EQ = 294,
    LT = 295,
    GT = 296,
    LE = 297,
    GE = 298,
    NE = 299,
    IN = 300,
    MAX = 301,
    MIN = 302,
    COUNT = 303,
    AVG = 304,
    ORDER = 305,
    BY = 306,
    ASC = 307,
    INNER = 308,
    JOIN = 309,
    UNIQUE = 310,
    GROUP = 311,
    ADD = 312,
    SUB = 313,
    DIV = 314,
    NULLABLE = 315,
    NULL_K = 316,
    NOT = 317,
    IS = 318,
    NUMBER = 319,
    FLOAT = 320,
    ID = 321,
    PATH = 322,
    SSS = 323,
    STAR = 324,
    STRING_V = 325
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 158 "yacc_sql.y"

  struct _Attr *attr;
  struct _Condition *condition1;
  struct _Value *value1;
  char *string;
  int number;
  float floats;
	char *position;

#line 292 "yacc_sql.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (void *scanner);

#endif /* !YY_YY_YACC_SQL_TAB_H_INCLUDED  */



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
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
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

#if ! defined yyoverflow || YYERROR_VERBOSE

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


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
#define YYLAST   376

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  71
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  55
/* YYNRULES -- Number of rules.  */
#define YYNRULES  166
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  344

#define YYUNDEFTOK  2
#define YYMAXUTOK   325


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

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
      65,    66,    67,    68,    69,    70
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   184,   184,   186,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   221,   226,   231,   237,   243,   249,   255,   261,
     267,   274,   280,   288,   295,   304,   306,   310,   321,   332,
     343,   356,   359,   360,   361,   362,   365,   374,   374,   391,
     393,   394,   394,   398,   400,   405,   408,   411,   415,   421,
     431,   441,   455,   458,   460,   463,   465,   469,   471,   476,
     481,   486,   491,   496,   501,   503,   508,   513,   518,   523,
     528,   533,   535,   540,   545,   550,   555,   560,   565,   567,
     572,   577,   582,   587,   592,   598,   602,   605,   608,   612,
     615,   618,   622,   625,   630,   635,   640,   645,   650,   655,
     660,   665,   670,   675,   680,   685,   690,   695,   701,   702,
     705,   707,   708,   710,   712,   716,   718,   723,   742,   762,
     782,   804,   825,   846,   865,   865,   889,   889,   913,   913,
     937,   937,   961,   985,  1009,  1033,  1073,  1073,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1123
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SEMICOLON", "CREATE", "DROP", "TABLE",
  "TABLES", "INDEX", "SELECT", "DESC", "SHOW", "SYNC", "INSERT", "DELETE",
  "UPDATE", "LBRACE", "RBRACE", "COMMA", "TRX_BEGIN", "TRX_COMMIT",
  "TRX_ROLLBACK", "INT_T", "STRING_T", "FLOAT_T", "HELP", "EXIT", "DOT",
  "INTO", "VALUES", "FROM", "WHERE", "AND", "SET", "ON", "LOAD", "DATA",
  "DATE_T", "INFILE", "EQ", "LT", "GT", "LE", "GE", "NE", "IN", "MAX",
  "MIN", "COUNT", "AVG", "ORDER", "BY", "ASC", "INNER", "JOIN", "UNIQUE",
  "GROUP", "ADD", "SUB", "DIV", "NULLABLE", "NULL_K", "NOT", "IS",
  "NUMBER", "FLOAT", "ID", "PATH", "SSS", "STAR", "STRING_V", "$accept",
  "commands", "command", "exit", "help", "sync", "begin", "commit",
  "rollback", "drop_table", "show_tables", "desc_table", "create_index",
  "drop_index", "create_table", "attr_def_list", "attr_def", "number",
  "type", "ID_get", "insert", "$@1", "tuple", "tuple_list", "$@2",
  "value_list", "value", "delete", "update", "select", "relations",
  "join_list", "rel_list", "groups", "group_list", "orders", "order_list",
  "E_", "E", "T", "F", "AGG", "select_attr", "attr_list", "where",
  "condition_list", "condition", "$@3", "$@4", "$@5", "$@6", "subQuery",
  "$@7", "comOp", "load_data", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325
};
# endif

#define YYPACT_NINF (-260)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-74)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -260,   216,  -260,    13,    51,    67,   -50,    28,  -260,    21,
      58,   -14,    78,    27,  -260,  -260,  -260,  -260,  -260,    45,
      73,    79,    82,    91,    57,    65,    61,  -260,  -260,  -260,
       9,  -260,  -260,   117,  -260,  -260,  -260,  -260,  -260,  -260,
    -260,  -260,  -260,  -260,  -260,  -260,  -260,  -260,  -260,  -260,
    -260,  -260,  -260,  -260,  -260,  -260,  -260,  -260,   -35,     8,
     127,  -260,  -260,    71,    72,   143,    94,    95,   135,   149,
     -35,   -45,  -260,   149,   154,  -260,  -260,   119,   137,   163,
      22,    20,   169,   145,   148,    18,   151,   152,   153,   157,
     -38,   191,  -260,    78,    78,    78,    78,   199,   190,   167,
    -260,  -260,    67,  -260,  -260,   172,   211,   208,   183,  -260,
     239,   195,   239,  -260,  -260,   200,    15,    17,   253,    31,
     256,    36,    23,   242,    33,  -260,  -260,   213,  -260,   -45,
     -45,  -260,  -260,   217,   220,   255,   149,   149,   237,   208,
     275,    89,  -260,   254,  -260,  -260,    19,  -260,  -260,   264,
    -260,   228,  -260,   229,  -260,  -260,   231,  -260,  -260,   232,
     281,   241,   243,   281,  -260,    89,   290,   246,   247,   290,
    -260,   237,  -260,   296,    32,   299,   250,  -260,  -260,   267,
     210,  -260,  -260,   261,   204,   286,   210,   239,   239,  -260,
    -260,   313,   303,   308,   309,   310,  -260,   100,   107,  -260,
     286,  -260,   170,   130,  -260,   191,   217,   311,  -260,  -260,
    -260,  -260,     4,   263,   314,   276,   315,   316,   318,   322,
     270,  -260,  -260,  -260,  -260,  -260,  -260,  -260,   292,   277,
     105,   140,    89,  -260,   208,  -260,  -260,  -260,  -260,   272,
    -260,  -260,  -260,  -260,   281,   274,   281,   281,   281,  -260,
     237,   290,   278,   290,   290,   290,  -260,  -260,   296,  -260,
     279,  -260,   280,   325,   282,  -260,   210,   328,   275,  -260,
     204,  -260,   150,   330,  -260,  -260,  -260,  -260,  -260,   320,
    -260,   286,  -260,  -260,   131,  -260,  -260,  -260,   146,  -260,
    -260,  -260,  -260,   332,  -260,  -260,   333,   315,  -260,  -260,
     285,  -260,   307,   113,  -260,   337,   338,   288,  -260,   281,
     281,   290,   290,  -260,  -260,  -260,   316,   329,   341,  -260,
    -260,   331,  -260,   342,  -260,  -260,  -260,  -260,   294,  -260,
     345,   346,   298,  -260,   348,  -260,   349,  -260,  -260,  -260,
    -260,  -260,   350,  -260
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,    73,     1,     0,     0,     0,     0,     0,    34,     0,
       0,     0,     0,     0,    35,    36,    37,    33,    32,     0,
       0,     0,     0,     0,     0,     0,     0,    68,    65,    66,
     114,    67,   113,     0,    20,    19,    14,    15,    16,    17,
       9,    10,    11,    12,    13,     8,     5,   117,     7,     6,
       4,    24,    25,    26,    27,    22,    23,    31,    28,   108,
     111,    21,    18,     0,     0,     0,     0,     0,   114,   130,
     105,   108,   111,   130,     0,    40,    39,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,     3,     0,     0,     0,     0,     0,     0,     0,
      38,    43,     0,   129,   128,     0,     0,   133,     0,   112,
      84,     0,    84,    85,    99,     0,     0,     0,     0,     0,
       0,     0,    98,     0,    84,   115,   116,     0,    72,   106,
     107,   110,   109,     0,     0,     0,   130,   130,    73,   133,
       0,     0,    69,     0,    89,   103,    84,    87,   101,     0,
     119,     0,   121,     0,   125,   122,     0,   124,   127,     0,
      98,     0,     0,    98,    92,     0,    84,     0,     0,    84,
      78,    73,    56,    45,     0,     0,     0,   132,   131,    91,
       0,    57,   156,     0,     0,   135,     0,    84,    84,    86,
     100,     0,     0,     0,     0,     0,    96,    98,    98,    94,
     135,    82,    84,    84,    80,    75,     0,     0,    52,    53,
      54,    55,    48,     0,     0,    77,    63,    60,     0,     0,
       0,   158,   159,   160,   161,   162,   163,   144,     0,   164,
       0,     0,     0,   134,   133,    90,   104,    88,   102,     0,
     118,   120,   123,   126,    98,     0,    98,    98,    98,    93,
      73,    84,     0,    84,    84,    84,    79,    76,    45,    44,
       0,    50,     0,     0,     0,    71,     0,     0,     0,    58,
       0,   157,     0,     0,   148,   165,   156,   139,   137,   140,
     138,   135,    70,   166,    98,    97,    95,    74,    84,    83,
      81,    46,    51,     0,    49,    41,     0,    63,    59,    61,
       0,   146,     0,     0,   156,     0,     0,     0,   136,    98,
      98,    84,    84,    47,    42,    64,    60,   154,     0,   150,
     156,     0,   141,     0,   156,   152,   142,    62,     0,   156,
       0,     0,     0,   145,     0,   155,     0,   156,   153,   143,
     149,   147,     0,   151
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -260,  -260,  -260,  -260,  -260,  -260,  -260,  -260,  -260,  -260,
    -260,  -260,  -260,  -260,  -260,   104,   162,  -260,  -260,  -260,
    -260,  -260,   101,    54,  -260,    74,  -140,  -260,  -260,   155,
     268,     5,   171,   160,    -1,   193,    12,    -3,    14,     3,
       6,    -2,  -260,   -63,  -131,  -195,  -156,  -260,  -260,  -260,
    -260,  -259,  -260,  -166,  -260
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,   207,   173,   293,   212,   174,
      46,   217,   181,   269,   316,   267,    47,    48,    49,    50,
      51,    91,   128,    53,   113,    55,   114,    57,    70,    71,
      72,    61,    74,   103,   142,   233,   185,   273,   318,   305,
     330,   218,   219,   230,    62
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      54,   184,    69,    73,    59,   250,    52,    60,   179,   200,
     104,   -29,   -73,    56,    95,    58,    75,   306,   231,    63,
     260,    64,    93,    94,    96,   184,    80,   -73,   125,   187,
     110,   126,   150,   160,   152,    76,    90,    13,    13,   109,
     216,   161,   151,   166,   153,   323,   234,   111,   155,    77,
     162,   167,    79,   158,   208,   209,   210,    66,   156,    67,
     168,   331,    25,   159,   261,   334,   262,    95,    65,   211,
     336,   188,   112,   177,   178,   163,   281,    96,   342,    93,
      94,    82,   118,    12,   119,   169,   308,   120,    78,    83,
     278,   280,   184,    81,    12,    84,   129,   130,    85,   136,
     137,   131,   132,   282,   300,   182,   303,    86,    87,   144,
     244,   147,    89,    20,    21,    22,    23,   247,   161,    88,
      92,   276,   145,   170,   148,   161,   297,   245,    27,   320,
     -30,    28,    29,    68,   164,    31,    32,    97,    98,    27,
     254,   309,    28,    29,    68,   189,    31,    32,   167,   161,
      27,    99,   246,    28,    29,   183,   311,    31,   190,   248,
     100,   101,    90,   322,   167,   201,    27,   102,   204,    28,
      29,   277,   196,    31,    27,   199,   205,    28,    29,   321,
     251,    31,   255,   310,   105,   106,   235,   237,   167,   221,
     222,   223,   224,   225,   226,   301,   108,   252,   312,   236,
     238,    27,   256,   107,    28,    29,   279,   115,    31,   127,
     249,   116,   302,   229,   117,   133,     2,   121,   122,   123,
       3,     4,   253,   124,   134,     5,     6,     7,     8,     9,
      10,    11,    12,   135,    13,    14,    15,    16,   138,   141,
     140,    17,    18,   221,   222,   223,   224,   225,   226,   143,
     144,    19,   147,   289,   290,   287,   145,    13,   148,   285,
     286,   146,    20,    21,    22,    23,    24,   229,   149,    25,
     154,    27,    26,   157,    28,    29,   165,    27,    31,   171,
      28,    29,    30,   172,    31,    32,   175,   189,   220,   176,
      25,   180,   191,   186,   192,   193,   190,   194,   195,   161,
     221,   222,   223,   224,   225,   226,   227,   197,   167,   198,
     235,   237,   202,   203,   206,   213,   214,    24,   232,   239,
     240,   236,   238,   228,   229,   241,   242,   243,   259,   263,
     264,     5,    26,   266,   268,   270,   272,   274,   283,   275,
     284,   294,   295,   292,   288,   298,   304,   307,   296,   313,
     314,   317,   319,   324,   326,   325,   328,   329,   332,   333,
     335,   337,   291,   338,   339,   340,   341,   343,   258,   299,
     327,   315,   215,   139,   271,   265,   257
};

static const yytype_int16 yycheck[] =
{
       1,   141,     5,     5,     1,   200,     1,     1,   139,   165,
      73,     3,     3,     1,    59,     1,    66,   276,   184,     6,
      16,     8,    57,    58,    69,   165,    12,    18,    66,    10,
      10,    69,    17,    10,    17,     7,    27,    18,    18,    17,
     180,    18,    27,    10,    27,   304,   186,    27,    17,    28,
      27,    18,    66,    17,    22,    23,    24,     6,    27,     8,
      27,   320,    53,    27,    60,   324,    62,    59,    55,    37,
     329,    52,    52,   136,   137,    52,   232,    69,   337,    57,
      58,    36,    64,    16,    66,    52,   281,    69,    30,    16,
     230,   231,   232,    66,    16,    16,    93,    94,    16,   102,
     102,    95,    96,   234,   270,    16,   272,    16,    51,   110,
      10,   112,    51,    46,    47,    48,    49,    10,    18,    54,
       3,    16,   110,   124,   112,    18,   266,    27,    61,    16,
       3,    64,    65,    66,   122,    68,    69,    66,    66,    61,
      10,    10,    64,    65,    66,   146,    68,    69,    18,    18,
      61,     8,    52,    64,    65,    66,    10,    68,   146,    52,
      66,    66,    27,   303,    18,   166,    61,    18,   169,    64,
      65,    66,   160,    68,    61,   163,   171,    64,    65,    66,
      10,    68,    52,    52,    30,    66,   187,   188,    18,    39,
      40,    41,    42,    43,    44,    45,    33,    27,    52,   187,
     188,    61,   203,    66,    64,    65,    66,    38,    68,    18,
     198,    66,    62,    63,    66,    16,     0,    66,    66,    66,
       4,     5,    52,    66,    34,     9,    10,    11,    12,    13,
      14,    15,    16,    66,    18,    19,    20,    21,    66,    31,
      29,    25,    26,    39,    40,    41,    42,    43,    44,    66,
     251,    35,   253,   254,   255,   250,   244,    18,   246,   247,
     248,    66,    46,    47,    48,    49,    50,    63,    68,    53,
      17,    61,    56,    17,    64,    65,    34,    61,    68,    66,
      64,    65,    66,    66,    68,    69,    66,   288,    27,    34,
      53,    16,    28,    39,    66,    66,   284,    66,    66,    18,
      39,    40,    41,    42,    43,    44,    45,    66,    18,    66,
     311,   312,    66,    66,    18,    16,    66,    50,    32,     6,
      17,   309,   310,    62,    63,    17,    17,    17,    17,    66,
      16,     9,    56,    18,    18,    17,    66,    45,    66,    62,
      66,    61,    17,    64,    66,    17,    16,    27,    66,    17,
      17,    66,    45,    16,    66,    17,    27,    16,    27,    17,
      66,    16,   258,    17,    66,    17,    17,    17,   206,   268,
     316,   297,   179,   105,   219,   215,   205
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    72,     0,     4,     5,     9,    10,    11,    12,    13,
      14,    15,    16,    18,    19,    20,    21,    25,    26,    35,
      46,    47,    48,    49,    50,    53,    56,    61,    64,    65,
      66,    68,    69,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    91,    97,    98,    99,
     100,   101,   102,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   125,     6,     8,    55,     6,     8,    66,   108,
     109,   110,   111,   112,   113,    66,     7,    28,    30,    66,
     109,    66,    36,    16,    16,    16,    16,    51,    54,    51,
      27,   102,     3,    57,    58,    59,    69,    66,    66,     8,
      66,    66,    18,   114,   114,    30,    66,    66,    33,    17,
      10,    27,    52,   105,   107,    38,    66,    66,    64,    66,
      69,    66,    66,    66,    66,    66,    69,    18,   103,   110,
     110,   111,   111,    16,    34,    66,   108,   112,    66,   101,
      29,    31,   115,    66,   105,   107,    66,   105,   107,    68,
      17,    27,    17,    27,    17,    17,    27,    17,    17,    27,
      10,    18,    27,    52,   107,    34,    10,    18,    27,    52,
     105,    66,    66,    87,    90,    66,    34,   114,   114,   115,
      16,    93,    16,    66,    97,   117,    39,    10,    52,   105,
     107,    28,    66,    66,    66,    66,   107,    66,    66,   107,
     117,   105,    66,    66,   105,   102,    18,    86,    22,    23,
      24,    37,    89,    16,    66,   106,    97,    92,   122,   123,
      27,    39,    40,    41,    42,    43,    44,    45,    62,    63,
     124,   124,    32,   116,    97,   105,   107,   105,   107,     6,
      17,    17,    17,    17,    10,    27,    52,    10,    52,   107,
     116,    10,    27,    52,    10,    52,   105,   103,    87,    17,
      16,    60,    62,    66,    16,   104,    18,    96,    18,    94,
      17,   100,    66,   118,    45,    62,    16,    66,    97,    66,
      97,   117,   115,    66,    66,   107,   107,   102,    66,   105,
     105,    86,    64,    88,    61,    17,    66,    97,    17,    93,
     124,    45,    62,   124,    16,   120,   122,    27,   116,    10,
      52,    10,    52,    17,    17,    96,    95,    66,   119,    45,
      16,    66,    97,   122,    16,    17,    66,    94,    27,    16,
     121,   122,    27,    17,   122,    66,   122,    16,    17,    66,
      17,    17,   122,    17
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    71,    72,    72,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    83,    84,    85,    86,    86,    87,    87,    87,
      87,    88,    89,    89,    89,    89,    90,    92,    91,    93,
      94,    95,    94,    96,    96,    97,    97,    97,    97,    98,
      99,   100,   101,   102,   102,   103,   103,   104,   104,   104,
     104,   104,   104,   104,   105,   105,   105,   105,   105,   105,
     105,   106,   106,   106,   106,   106,   106,   106,   107,   107,
     107,   107,   107,   107,   107,   108,   109,   109,   109,   110,
     110,   110,   111,   111,   111,   111,   111,   111,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   113,   113,
     114,   114,   114,   115,   115,   116,   116,   117,   117,   117,
     117,   117,   117,   117,   118,   117,   119,   117,   120,   117,
     121,   117,   117,   117,   117,   117,   123,   122,   124,   124,
     124,   124,   124,   124,   124,   124,   125
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       2,     8,     9,     3,     7,     0,     3,     5,     2,     4,
       3,     1,     1,     1,     1,     1,     1,     0,     7,     4,
       0,     0,     4,     0,     3,     1,     1,     1,     1,     4,
       7,     7,     3,     0,     7,     0,     4,     0,     4,     6,
       5,     7,     5,     7,     0,     3,     5,     4,     6,     4,
       6,     0,     4,     6,     5,     7,     5,     7,     0,     3,
       5,     4,     6,     4,     6,     1,     3,     3,     1,     3,
       3,     1,     3,     1,     1,     3,     3,     1,     6,     4,
       6,     4,     4,     6,     4,     4,     6,     4,     2,     2,
       0,     3,     3,     0,     3,     0,     3,     3,     3,     3,
       3,     5,     5,     7,     0,     6,     0,     8,     0,     7,
       0,     9,     5,     7,     5,     7,     0,     2,     1,     1,
       1,     1,     1,     1,     1,     2,     7
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, void *scanner)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (scanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, void *scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep, scanner);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule, void *scanner)
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
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              , scanner);
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
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void *scanner)
{
  YYUSE (yyvaluep);
  YYUSE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
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

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
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
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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
  case 32:
#line 221 "yacc_sql.y"
         {
        CONTEXT->ssql->flag=SCF_EXIT;//"exit";
    }
#line 1725 "yacc_sql.tab.c"
    break;

  case 33:
#line 226 "yacc_sql.y"
         {
        CONTEXT->ssql->flag=SCF_HELP;//"help";
    }
#line 1733 "yacc_sql.tab.c"
    break;

  case 34:
#line 231 "yacc_sql.y"
         {
      CONTEXT->ssql->flag = SCF_SYNC;
    }
#line 1741 "yacc_sql.tab.c"
    break;

  case 35:
#line 237 "yacc_sql.y"
              {
      CONTEXT->ssql->flag = SCF_BEGIN;
    }
#line 1749 "yacc_sql.tab.c"
    break;

  case 36:
#line 243 "yacc_sql.y"
               {
      CONTEXT->ssql->flag = SCF_COMMIT;
    }
#line 1757 "yacc_sql.tab.c"
    break;

  case 37:
#line 249 "yacc_sql.y"
                 {
      CONTEXT->ssql->flag = SCF_ROLLBACK;
    }
#line 1765 "yacc_sql.tab.c"
    break;

  case 38:
#line 255 "yacc_sql.y"
                  {
        CONTEXT->ssql->flag = SCF_DROP_TABLE;//"drop_table";
        drop_table_init(&CONTEXT->ssql->sstr.drop_table, (yyvsp[0].string));
    }
#line 1774 "yacc_sql.tab.c"
    break;

  case 39:
#line 261 "yacc_sql.y"
                {
      CONTEXT->ssql->flag = SCF_SHOW_TABLES;
    }
#line 1782 "yacc_sql.tab.c"
    break;

  case 40:
#line 267 "yacc_sql.y"
            {
      CONTEXT->ssql->flag = SCF_DESC_TABLE;
      desc_table_init(&CONTEXT->ssql->sstr.desc_table, (yyvsp[0].string));
    }
#line 1791 "yacc_sql.tab.c"
    break;

  case 41:
#line 275 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
			create_index_init(&CONTEXT->ssql->sstr.create_index, (yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string), false);
		}
#line 1800 "yacc_sql.tab.c"
    break;

  case 42:
#line 281 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
                        create_index_init(&CONTEXT->ssql->sstr.create_index, (yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string), true);
		}
#line 1809 "yacc_sql.tab.c"
    break;

  case 43:
#line 289 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_DROP_INDEX;//"drop_index";
			drop_index_init(&CONTEXT->ssql->sstr.drop_index, (yyvsp[0].string));
		}
#line 1818 "yacc_sql.tab.c"
    break;

  case 44:
#line 296 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_CREATE_TABLE;//"create_table";
			// CONTEXT->ssql->sstr.create_table.attribute_count = CONTEXT->value_length;
			create_table_init_name(&CONTEXT->ssql->sstr.create_table, (yyvsp[-4].string));
			//临时变量清零	
			CONTEXT->value_length = 0;
		}
#line 1830 "yacc_sql.tab.c"
    break;

  case 46:
#line 306 "yacc_sql.y"
                                   {    }
#line 1836 "yacc_sql.tab.c"
    break;

  case 47:
#line 311 "yacc_sql.y"
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
#line 1851 "yacc_sql.tab.c"
    break;

  case 48:
#line 322 "yacc_sql.y"
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
#line 1866 "yacc_sql.tab.c"
    break;

  case 49:
#line 333 "yacc_sql.y"
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
#line 1881 "yacc_sql.tab.c"
    break;

  case 50:
#line 344 "yacc_sql.y"
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
#line 1896 "yacc_sql.tab.c"
    break;

  case 51:
#line 356 "yacc_sql.y"
                       {(yyval.number) = (yyvsp[0].number);}
#line 1902 "yacc_sql.tab.c"
    break;

  case 52:
#line 359 "yacc_sql.y"
              { (yyval.number)=INTS; }
#line 1908 "yacc_sql.tab.c"
    break;

  case 53:
#line 360 "yacc_sql.y"
                  { (yyval.number)=CHARS; }
#line 1914 "yacc_sql.tab.c"
    break;

  case 54:
#line 361 "yacc_sql.y"
                 { (yyval.number)=FLOATS; }
#line 1920 "yacc_sql.tab.c"
    break;

  case 55:
#line 362 "yacc_sql.y"
                { (yyval.number)=DATE; }
#line 1926 "yacc_sql.tab.c"
    break;

  case 56:
#line 366 "yacc_sql.y"
        {
		char *temp=(yyvsp[0].string); 
		snprintf(CONTEXT->id, sizeof(CONTEXT->id), "%s", temp);
	}
#line 1935 "yacc_sql.tab.c"
    break;

  case 57:
#line 374 "yacc_sql.y"
                                { insert_tuple_add(CONTEXT); }
#line 1941 "yacc_sql.tab.c"
    break;

  case 58:
#line 375 "yacc_sql.y"
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
#line 1960 "yacc_sql.tab.c"
    break;

  case 61:
#line 394 "yacc_sql.y"
                  {
		insert_tuple_add(CONTEXT);
	}
#line 1968 "yacc_sql.tab.c"
    break;

  case 64:
#line 400 "yacc_sql.y"
                              {
  		// CONTEXT->values[CONTEXT->value_length++] = *$2;
	  }
#line 1976 "yacc_sql.tab.c"
    break;

  case 65:
#line 405 "yacc_sql.y"
          {	
  		value_init_integer(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].number));
		}
#line 1984 "yacc_sql.tab.c"
    break;

  case 66:
#line 408 "yacc_sql.y"
          {
  		value_init_float(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].floats));
		}
#line 1992 "yacc_sql.tab.c"
    break;

  case 67:
#line 411 "yacc_sql.y"
         {
			(yyvsp[0].string) = substr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
  		value_init_string(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].string));
		}
#line 2001 "yacc_sql.tab.c"
    break;

  case 68:
#line 415 "yacc_sql.y"
                        {
		value_init_null(&CONTEXT->values[CONTEXT->value_length++]);
		}
#line 2009 "yacc_sql.tab.c"
    break;

  case 69:
#line 422 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_DELETE;//"delete";
			deletes_init_relation(&CONTEXT->ssql->sstr.deletion, (yyvsp[-1].string));
			deletes_set_conditions(&CONTEXT->ssql->sstr.deletion, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;	
    }
#line 2021 "yacc_sql.tab.c"
    break;

  case 70:
#line 432 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_UPDATE;//"update";
			Value *value = &CONTEXT->values[0];
			updates_init(&CONTEXT->ssql->sstr.update, (yyvsp[-5].string), (yyvsp[-3].string), value, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;
		}
#line 2033 "yacc_sql.tab.c"
    break;

  case 71:
#line 442 "yacc_sql.y"
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
#line 2050 "yacc_sql.tab.c"
    break;

  case 72:
#line 455 "yacc_sql.y"
                             {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-2].string));
	}
#line 2058 "yacc_sql.tab.c"
    break;

  case 74:
#line 460 "yacc_sql.y"
                                                     {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-4].string));
}
#line 2066 "yacc_sql.tab.c"
    break;

  case 76:
#line 465 "yacc_sql.y"
                                  {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-2].string));
	}
#line 2074 "yacc_sql.tab.c"
    break;

  case 78:
#line 471 "yacc_sql.y"
                        {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2084 "yacc_sql.tab.c"
    break;

  case 79:
#line 476 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2094 "yacc_sql.tab.c"
    break;

  case 80:
#line 481 "yacc_sql.y"
                            {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2104 "yacc_sql.tab.c"
    break;

  case 81:
#line 486 "yacc_sql.y"
                                   {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2114 "yacc_sql.tab.c"
    break;

  case 82:
#line 491 "yacc_sql.y"
                             {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2124 "yacc_sql.tab.c"
    break;

  case 83:
#line 496 "yacc_sql.y"
                                    {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2134 "yacc_sql.tab.c"
    break;

  case 85:
#line 503 "yacc_sql.y"
                      {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2144 "yacc_sql.tab.c"
    break;

  case 86:
#line 508 "yacc_sql.y"
                              {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2154 "yacc_sql.tab.c"
    break;

  case 87:
#line 513 "yacc_sql.y"
                           {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2164 "yacc_sql.tab.c"
    break;

  case 88:
#line 518 "yacc_sql.y"
                                  {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2174 "yacc_sql.tab.c"
    break;

  case 89:
#line 523 "yacc_sql.y"
                            {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2184 "yacc_sql.tab.c"
    break;

  case 90:
#line 528 "yacc_sql.y"
                                   {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2194 "yacc_sql.tab.c"
    break;

  case 92:
#line 535 "yacc_sql.y"
                        {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2204 "yacc_sql.tab.c"
    break;

  case 93:
#line 540 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2214 "yacc_sql.tab.c"
    break;

  case 94:
#line 545 "yacc_sql.y"
                            {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2224 "yacc_sql.tab.c"
    break;

  case 95:
#line 550 "yacc_sql.y"
                                   {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2234 "yacc_sql.tab.c"
    break;

  case 96:
#line 555 "yacc_sql.y"
                             {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2244 "yacc_sql.tab.c"
    break;

  case 97:
#line 560 "yacc_sql.y"
                                    {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2254 "yacc_sql.tab.c"
    break;

  case 99:
#line 567 "yacc_sql.y"
                      {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2264 "yacc_sql.tab.c"
    break;

  case 100:
#line 572 "yacc_sql.y"
                              {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2274 "yacc_sql.tab.c"
    break;

  case 101:
#line 577 "yacc_sql.y"
                           {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2284 "yacc_sql.tab.c"
    break;

  case 102:
#line 582 "yacc_sql.y"
                                  {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2294 "yacc_sql.tab.c"
    break;

  case 103:
#line 587 "yacc_sql.y"
                            {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2304 "yacc_sql.tab.c"
    break;

  case 104:
#line 592 "yacc_sql.y"
                                   {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2314 "yacc_sql.tab.c"
    break;

  case 105:
#line 598 "yacc_sql.y"
 {
	selects_append_attribute_plus(&CONTEXT->ssql->sstr.selection);
}
#line 2322 "yacc_sql.tab.c"
    break;

  case 106:
#line 602 "yacc_sql.y"
               {
		e_e_t(Add);
	}
#line 2330 "yacc_sql.tab.c"
    break;

  case 107:
#line 605 "yacc_sql.y"
                 {
		e_e_t(Sub);
	}
#line 2338 "yacc_sql.tab.c"
    break;

  case 108:
#line 608 "yacc_sql.y"
           {
		e_t();
	}
#line 2346 "yacc_sql.tab.c"
    break;

  case 109:
#line 612 "yacc_sql.y"
                {
		t_t_f(Mul);
	}
#line 2354 "yacc_sql.tab.c"
    break;

  case 110:
#line 615 "yacc_sql.y"
                 {
		t_t_f(Div);
	}
#line 2362 "yacc_sql.tab.c"
    break;

  case 111:
#line 618 "yacc_sql.y"
           {
		t_f();
	}
#line 2370 "yacc_sql.tab.c"
    break;

  case 112:
#line 622 "yacc_sql.y"
                       {
		f_e();
	}
#line 2378 "yacc_sql.tab.c"
    break;

  case 113:
#line 625 "yacc_sql.y"
             {
        			RelAttr attr;
        			relation_attr_init(&attr, NULL, "*");
        			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
        }
#line 2388 "yacc_sql.tab.c"
    break;

  case 114:
#line 630 "yacc_sql.y"
             {
        			RelAttr attr;
        			relation_attr_init(&attr, NULL, (yyvsp[0].string));
        			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
        }
#line 2398 "yacc_sql.tab.c"
    break;

  case 115:
#line 635 "yacc_sql.y"
                    {
        			RelAttr attr;
        			relation_attr_init(&attr, (yyvsp[-2].string), (yyvsp[0].string));
        			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
        }
#line 2408 "yacc_sql.tab.c"
    break;

  case 116:
#line 640 "yacc_sql.y"
                      {
        			RelAttr attr;
        			relation_attr_init(&attr, (yyvsp[-2].string), "*");
        			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
        }
#line 2418 "yacc_sql.tab.c"
    break;

  case 117:
#line 645 "yacc_sql.y"
              {
        }
#line 2425 "yacc_sql.tab.c"
    break;

  case 118:
#line 650 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2435 "yacc_sql.tab.c"
    break;

  case 119:
#line 655 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2445 "yacc_sql.tab.c"
    break;

  case 120:
#line 660 "yacc_sql.y"
                                {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2455 "yacc_sql.tab.c"
    break;

  case 121:
#line 665 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2465 "yacc_sql.tab.c"
    break;

  case 122:
#line 670 "yacc_sql.y"
                           {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2475 "yacc_sql.tab.c"
    break;

  case 123:
#line 675 "yacc_sql.y"
                                  {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2485 "yacc_sql.tab.c"
    break;

  case 124:
#line 680 "yacc_sql.y"
                                  {
        	RelAttr attr;
        	relation_agg_attr_init(&attr, NULL, "*",Count);
                selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
            }
#line 2495 "yacc_sql.tab.c"
    break;

  case 125:
#line 685 "yacc_sql.y"
                                       {
                    	RelAttr attr;
                    	relation_agg_attr_init(&attr, NULL, "*",Count);
                            selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
                        }
#line 2505 "yacc_sql.tab.c"
    break;

  case 126:
#line 690 "yacc_sql.y"
                                {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2515 "yacc_sql.tab.c"
    break;

  case 127:
#line 695 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2525 "yacc_sql.tab.c"
    break;

  case 134:
#line 712 "yacc_sql.y"
                                     {	
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 2533 "yacc_sql.tab.c"
    break;

  case 136:
#line 718 "yacc_sql.y"
                                   {
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 2541 "yacc_sql.tab.c"
    break;

  case 137:
#line 724 "yacc_sql.y"
                {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-2].string));
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
			// $$ = ( Condition *)malloc(sizeof( Condition));
			// $$->left_is_attr = 1;
			// $$->left_attr.relation_name = NULL;
			// $$->left_attr.attribute_name= $1;
			// $$->comp = CONTEXT->comp;
			// $$->right_is_attr = 0;
			// $$->right_attr.relation_name = NULL;
			// $$->right_attr.attribute_name = NULL;
			// $$->right_value = *$3;

		}
#line 2564 "yacc_sql.tab.c"
    break;

  case 138:
#line 743 "yacc_sql.y"
                {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 2];
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 0, NULL, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
			// $$ = ( Condition *)malloc(sizeof( Condition));
			// $$->left_is_attr = 0;
			// $$->left_attr.relation_name=NULL;
			// $$->left_attr.attribute_name=NULL;
			// $$->left_value = *$1;
			// $$->comp = CONTEXT->comp;
			// $$->right_is_attr = 0;
			// $$->right_attr.relation_name = NULL;
			// $$->right_attr.attribute_name = NULL;
			// $$->right_value = *$3;

		}
#line 2588 "yacc_sql.tab.c"
    break;

  case 139:
#line 763 "yacc_sql.y"
                {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-2].string));
			RelAttr right_attr;
			relation_attr_init(&right_attr, NULL, (yyvsp[0].string));

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
			// $$=( Condition *)malloc(sizeof( Condition));
			// $$->left_is_attr = 1;
			// $$->left_attr.relation_name=NULL;
			// $$->left_attr.attribute_name=$1;
			// $$->comp = CONTEXT->comp;
			// $$->right_is_attr = 1;
			// $$->right_attr.relation_name=NULL;
			// $$->right_attr.attribute_name=$3;

		}
#line 2612 "yacc_sql.tab.c"
    break;

  case 140:
#line 783 "yacc_sql.y"
                {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];
			RelAttr right_attr;
			relation_attr_init(&right_attr, NULL, (yyvsp[0].string));

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;

			// $$=( Condition *)malloc(sizeof( Condition));
			// $$->left_is_attr = 0;
			// $$->left_attr.relation_name=NULL;
			// $$->left_attr.attribute_name=NULL;
			// $$->left_value = *$1;
			// $$->comp=CONTEXT->comp;
			
			// $$->right_is_attr = 1;
			// $$->right_attr.relation_name=NULL;
			// $$->right_attr.attribute_name=$3;
		
		}
#line 2638 "yacc_sql.tab.c"
    break;

  case 141:
#line 805 "yacc_sql.y"
                {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-4].string), (yyvsp[-2].string));
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;

			// $$=( Condition *)malloc(sizeof( Condition));
			// $$->left_is_attr = 1;
			// $$->left_attr.relation_name=$1;
			// $$->left_attr.attribute_name=$3;
			// $$->comp=CONTEXT->comp;
			// $$->right_is_attr = 0;   //属性值
			// $$->right_attr.relation_name=NULL;
			// $$->right_attr.attribute_name=NULL;
			// $$->right_value =*$5;			
							
    }
#line 2663 "yacc_sql.tab.c"
    break;

  case 142:
#line 826 "yacc_sql.y"
                {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];

			RelAttr right_attr;
			relation_attr_init(&right_attr, (yyvsp[-2].string), (yyvsp[0].string));

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
			// $$=( Condition *)malloc(sizeof( Condition));
			// $$->left_is_attr = 0;//属性值
			// $$->left_attr.relation_name=NULL;
			// $$->left_attr.attribute_name=NULL;
			// $$->left_value = *$1;
			// $$->comp =CONTEXT->comp;
			// $$->right_is_attr = 1;//属性
			// $$->right_attr.relation_name = $3;
			// $$->right_attr.attribute_name = $5;
									
    }
#line 2688 "yacc_sql.tab.c"
    break;

  case 143:
#line 847 "yacc_sql.y"
                {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-6].string), (yyvsp[-4].string));
			RelAttr right_attr;
			relation_attr_init(&right_attr, (yyvsp[-2].string), (yyvsp[0].string));

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
			// $$=( Condition *)malloc(sizeof( Condition));
			// $$->left_is_attr = 1;		//属性
			// $$->left_attr.relation_name=$1;
			// $$->left_attr.attribute_name=$3;
			// $$->comp =CONTEXT->comp;
			// $$->right_is_attr = 1;		//属性
			// $$->right_attr.relation_name=$5;
			// $$->right_attr.attribute_name=$7;
        }
#line 2711 "yacc_sql.tab.c"
    break;

  case 144:
#line 865 "yacc_sql.y"
                { CONTEXT->comp = CONTAINED_BY; }
#line 2717 "yacc_sql.tab.c"
    break;

  case 145:
#line 866 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-5].string));

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

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2745 "yacc_sql.tab.c"
    break;

  case 146:
#line 889 "yacc_sql.y"
                       { CONTEXT->comp = CONTAINED_BY; }
#line 2751 "yacc_sql.tab.c"
    break;

  case 147:
#line 890 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-7].string), (yyvsp[-5].string));

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

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2779 "yacc_sql.tab.c"
    break;

  case 148:
#line 913 "yacc_sql.y"
                    { CONTEXT->comp = NOT_CONTAINED_BY; }
#line 2785 "yacc_sql.tab.c"
    break;

  case 149:
#line 914 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-6].string));

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

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2813 "yacc_sql.tab.c"
    break;

  case 150:
#line 937 "yacc_sql.y"
                           { CONTEXT->comp = NOT_CONTAINED_BY; }
#line 2819 "yacc_sql.tab.c"
    break;

  case 151:
#line 938 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-8].string), (yyvsp[-6].string));

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

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2847 "yacc_sql.tab.c"
    break;

  case 152:
#line 962 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-4].string));

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

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2875 "yacc_sql.tab.c"
    break;

  case 153:
#line 986 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-6].string), (yyvsp[-4].string));

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

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2903 "yacc_sql.tab.c"
    break;

  case 154:
#line 1010 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[0].string));

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

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, &rval, 1, &left_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2931 "yacc_sql.tab.c"
    break;

  case 155:
#line 1034 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-2].string), (yyvsp[0].string));

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

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, &rval, 1, &left_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2959 "yacc_sql.tab.c"
    break;

  case 156:
#line 1073 "yacc_sql.y"
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
#line 2978 "yacc_sql.tab.c"
    break;

  case 157:
#line 1088 "yacc_sql.y"
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
#line 3004 "yacc_sql.tab.c"
    break;

  case 158:
#line 1112 "yacc_sql.y"
             { CONTEXT->comp = EQUAL_TO; }
#line 3010 "yacc_sql.tab.c"
    break;

  case 159:
#line 1113 "yacc_sql.y"
         { CONTEXT->comp = LESS_THAN; }
#line 3016 "yacc_sql.tab.c"
    break;

  case 160:
#line 1114 "yacc_sql.y"
         { CONTEXT->comp = GREAT_THAN; }
#line 3022 "yacc_sql.tab.c"
    break;

  case 161:
#line 1115 "yacc_sql.y"
         { CONTEXT->comp = LESS_EQUAL; }
#line 3028 "yacc_sql.tab.c"
    break;

  case 162:
#line 1116 "yacc_sql.y"
         { CONTEXT->comp = GREAT_EQUAL; }
#line 3034 "yacc_sql.tab.c"
    break;

  case 163:
#line 1117 "yacc_sql.y"
         { CONTEXT->comp = NOT_EQUAL; }
#line 3040 "yacc_sql.tab.c"
    break;

  case 164:
#line 1118 "yacc_sql.y"
         { CONTEXT->comp = IS_CompOP; }
#line 3046 "yacc_sql.tab.c"
    break;

  case 165:
#line 1119 "yacc_sql.y"
             {CONTEXT->comp = IS_NOT_CompOP;}
#line 3052 "yacc_sql.tab.c"
    break;

  case 166:
#line 1124 "yacc_sql.y"
                {
		  CONTEXT->ssql->flag = SCF_LOAD_DATA;
			load_data_init(&CONTEXT->ssql->sstr.load_data, (yyvsp[0].string), (yyvsp[-3].string));
		}
#line 3061 "yacc_sql.tab.c"
    break;


#line 3065 "yacc_sql.tab.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

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
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (scanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
                  yystos[yystate], yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
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
                  yystos[+*yyssp], yyvsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1129 "yacc_sql.y"

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
