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
    NULLABLE = 312,
    NULL_K = 313,
    NOT = 314,
    IS = 315,
    ADD = 316,
    SUB = 317,
    DIV = 318,
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
#define YYLAST   362

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  71
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  56
/* YYNRULES -- Number of rules.  */
#define YYNRULES  163
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  329

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
     216,   217,   218,   222,   227,   232,   238,   244,   250,   256,
     262,   268,   275,   281,   289,   296,   305,   307,   311,   322,
     333,   344,   357,   360,   361,   362,   363,   366,   375,   375,
     392,   394,   395,   395,   399,   401,   406,   409,   412,   416,
     422,   432,   442,   456,   459,   461,   464,   466,   470,   472,
     477,   482,   487,   492,   497,   502,   504,   509,   514,   519,
     524,   529,   534,   536,   541,   546,   551,   556,   561,   566,
     568,   573,   578,   583,   588,   593,   599,   604,   609,   614,
     622,   626,   629,   632,   636,   639,   642,   646,   650,   653,
     656,   659,   664,   669,   674,   679,   684,   689,   694,   699,
     704,   709,   715,   716,   719,   721,   722,   724,   726,   730,
     732,   737,   887,   887,   916,   916,   942,   942,   968,   968,
     994,  1019,  1045,  1087,  1087,  1126,  1127,  1128,  1129,  1130,
    1131,  1132,  1133,  1137
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
  "GROUP", "NULLABLE", "NULL_K", "NOT", "IS", "ADD", "SUB", "DIV",
  "NUMBER", "FLOAT", "ID", "PATH", "SSS", "STAR", "STRING_V", "$accept",
  "commands", "command", "exit", "help", "sync", "begin", "commit",
  "rollback", "drop_table", "show_tables", "desc_table", "create_index",
  "drop_index", "create_table", "attr_def_list", "attr_def", "number",
  "type", "ID_get", "insert", "$@1", "tuple", "tuple_list", "$@2",
  "value_list", "value", "delete", "update", "select", "relations",
  "join_list", "rel_list", "groups", "group_list", "orders", "order_list",
  "ATT", "E_", "E", "T", "F", "AGG", "select_attr", "attr_list", "where",
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

#define YYPACT_NINF (-197)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-75)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -197,    11,  -197,    27,   103,    72,   -47,    42,  -197,    46,
       4,     0,    88,    24,  -197,  -197,  -197,  -197,  -197,    61,
      54,    94,    99,   101,    75,    77,    81,  -197,     2,  -197,
    -197,    36,  -197,  -197,   132,  -197,  -197,  -197,  -197,  -197,
    -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,
    -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,   140,  -197,
      62,     9,   141,  -197,  -197,    79,    92,   139,   104,   107,
     148,  -197,   142,    62,   -16,  -197,   142,   152,  -197,  -197,
     117,   118,   156,    34,   115,   162,   124,   128,   179,   137,
     147,   153,   154,    88,  -197,   -21,   196,  -197,    88,    88,
      88,    88,   202,   193,   163,  -197,  -197,    72,  -197,  -197,
     168,   206,   209,   175,  -197,   210,   178,   210,  -197,  -197,
     182,    23,    35,   230,    64,   238,    95,   159,   225,   169,
      38,  -197,  -197,   200,  -197,   -16,   -16,  -197,  -197,   218,
     219,   234,   142,   142,   233,   209,   271,    97,  -197,   250,
    -197,  -197,   158,  -197,  -197,   262,  -197,   229,  -197,   235,
    -197,  -197,   237,  -197,  -197,   240,   278,   241,   242,   278,
    -197,    97,   280,   245,   246,   280,  -197,  -197,   233,  -197,
     282,    19,   288,   247,  -197,  -197,   255,   174,  -197,    88,
     232,   283,   174,   210,   210,  -197,  -197,   308,   299,   300,
     301,   302,  -197,   197,   170,  -197,   283,  -197,   199,   194,
    -197,   196,   218,   305,  -197,  -197,  -197,  -197,    28,   257,
     309,   268,   310,   311,   313,   317,  -197,  -197,  -197,  -197,
    -197,  -197,  -197,   286,   273,   133,    97,  -197,   209,  -197,
    -197,  -197,  -197,   261,  -197,  -197,  -197,  -197,   278,   267,
     278,   278,   278,  -197,   233,   280,   269,   280,   280,   280,
    -197,  -197,   282,  -197,   270,  -197,   279,   319,   272,  -197,
     174,   322,   271,  -197,   239,  -197,   324,   325,  -197,  -197,
      88,    62,   283,  -197,  -197,   213,  -197,  -197,  -197,   215,
    -197,  -197,  -197,  -197,   326,  -197,  -197,   327,   310,  -197,
    -197,    88,  -197,  -197,   329,   330,   331,  -197,   278,   278,
     280,   280,  -197,  -197,  -197,   311,    62,   332,   333,  -197,
    -197,  -197,  -197,  -197,  -197,   334,   335,  -197,  -197
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,    74,     1,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,     0,    36,    37,    38,    34,    33,     0,
       0,     0,     0,     0,     0,     0,     0,    69,     0,    66,
      67,   107,    68,   106,     0,    20,    19,    14,    15,    16,
      17,     9,    10,    11,    12,    13,     8,     5,   121,     7,
       6,     4,    24,    25,    27,    28,    22,    23,   120,    29,
      30,   113,   116,    21,    18,     0,     0,     0,     0,     0,
     107,   120,   134,   110,   113,   116,   134,     0,    41,    40,
       0,     0,     0,     0,    85,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   119,     0,    76,     3,     0,     0,
       0,     0,     0,     0,     0,    39,    44,     0,   133,   132,
       0,     0,   137,     0,   117,    85,     0,    85,    86,   100,
       0,     0,     0,     0,     0,     0,     0,    99,     0,    85,
       0,   108,   109,     0,    73,   111,   112,   115,   114,     0,
       0,     0,   134,   134,    74,   137,     0,     0,    70,     0,
      90,   104,    85,    88,   102,     0,   123,     0,   125,     0,
     129,   126,     0,   128,   131,     0,    99,     0,     0,    99,
      93,     0,    85,     0,     0,    85,    79,   118,    74,    57,
      46,     0,     0,     0,   136,   135,    92,     0,    58,   153,
       0,   139,     0,    85,    85,    87,   101,     0,     0,     0,
       0,     0,    97,    99,    99,    95,   139,    83,    85,    85,
      81,    76,     0,     0,    53,    54,    55,    56,    49,     0,
       0,    78,    64,    61,     0,     0,   155,   156,   157,   158,
     159,   160,   142,     0,   161,     0,     0,   138,   137,    91,
     105,    89,   103,     0,   122,   124,   127,   130,    99,     0,
      99,    99,    99,    94,    74,    85,     0,    85,    85,    85,
      80,    77,    46,    45,     0,    51,     0,     0,     0,    72,
       0,     0,     0,    59,     0,   154,     0,     0,   146,   162,
     153,   141,   139,    71,   163,    99,    98,    96,    75,    85,
      84,    82,    47,    52,     0,    50,    42,     0,    64,    60,
      62,     0,   153,   153,     0,     0,     0,   140,    99,    99,
      85,    85,    48,    43,    65,    61,   151,     0,     0,   153,
     153,   150,    63,   143,   145,     0,     0,   147,   149
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,
    -197,  -197,  -197,  -197,  -197,    80,   135,  -197,  -197,  -197,
    -197,  -197,    82,    40,  -197,    55,  -178,  -197,  -197,   131,
     248,     7,   146,   138,    -1,   176,    12,    37,    -4,     1,
       3,     6,    -2,  -197,   -59,  -140,  -196,  -143,  -197,  -197,
    -197,  -197,   -50,  -197,    86,  -197
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,   213,   180,   294,   218,   181,
      47,   223,   188,   273,   315,   271,    48,    49,    50,    51,
      52,    96,   134,    54,   118,    56,   119,    71,    59,    83,
      74,    75,    63,    77,   108,   148,   237,   191,   276,   277,
     304,   305,   224,   225,   235,    64
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      55,    72,    60,    76,    61,   186,    73,    62,    53,   222,
     254,     2,   -31,    57,   238,     3,     4,   109,    93,    78,
       5,     6,     7,     8,     9,    10,    11,    12,   206,    13,
      14,    15,    16,    65,    81,    66,    17,    18,    58,   -74,
     156,   214,   215,   216,   264,   131,    19,   100,   132,    79,
     157,   114,   158,   101,   -74,   177,   217,    20,    21,    22,
      23,    24,   159,    95,    25,    94,    82,    26,    70,    27,
      86,    33,   100,    28,    80,    29,    30,    31,   101,    32,
      33,   161,    67,   184,   185,   265,   307,   266,    12,    25,
      84,   162,   298,   282,   130,    98,    99,    85,   283,    98,
      99,   135,   136,   142,    12,   143,   137,   138,    73,    68,
      87,    69,   164,   189,   150,    88,   153,    89,    20,    21,
      22,    23,   165,    98,    99,   115,    90,   151,   176,   154,
      27,    91,    92,    13,    28,    97,    29,    30,    70,   170,
      32,    33,   116,   -26,   -32,   102,    27,   104,   190,   280,
      28,   195,    29,    30,    70,    27,    32,    33,   103,    28,
     107,    29,    30,    70,   196,    32,    33,   117,   193,   166,
     105,   207,   190,   106,   210,    95,    13,   167,   202,   172,
     251,   205,   110,   111,   112,   211,   168,   173,   167,   113,
     121,    27,   239,   241,   122,    28,   174,    29,    30,    70,
     120,    32,    33,   126,   258,   240,   242,   248,   260,   255,
     194,   169,   173,   127,   133,   167,   253,   173,   139,   128,
     129,   175,   252,   308,   249,   310,   256,   140,    13,   141,
     306,   167,    27,   173,   144,   146,   281,   190,    29,    30,
     147,   149,    32,   123,   152,   124,   259,   160,   125,   250,
     155,   257,   317,   318,   150,   163,   153,   290,   291,   171,
     151,   288,   154,   286,   287,   309,   178,   311,   183,   325,
     326,   226,   227,   228,   229,   230,   231,   232,   226,   227,
     228,   229,   230,   231,   179,   182,    25,   187,   195,   192,
     197,   233,   234,    98,    99,   198,   167,   196,   173,   234,
     212,   199,   316,   200,   219,    24,   201,   203,   204,   239,
     241,   208,   209,   220,   243,   236,   244,   245,   246,   247,
     240,   242,   263,   267,    26,   268,     5,   284,   270,   272,
     274,   278,   279,   285,   293,   289,   296,   295,   297,   299,
     302,   303,   292,   312,   313,   319,   320,   262,   321,   323,
     324,   327,   328,   314,   300,   322,   275,   261,   145,   269,
     301,     0,   221
};

static const yytype_int16 yycheck[] =
{
       1,     5,     1,     5,     1,   145,     5,     1,     1,   187,
     206,     0,     3,     1,   192,     4,     5,    76,    16,    66,
       9,    10,    11,    12,    13,    14,    15,    16,   171,    18,
      19,    20,    21,     6,    30,     8,    25,    26,     1,     3,
      17,    22,    23,    24,    16,    66,    35,    63,    69,     7,
      27,    17,    17,    69,    18,    17,    37,    46,    47,    48,
      49,    50,    27,    27,    53,    28,    66,    56,    66,    58,
      16,    69,    63,    62,    28,    64,    65,    66,    69,    68,
      69,    17,    55,   142,   143,    57,   282,    59,    16,    53,
      66,    27,   270,   236,    93,    61,    62,    36,   238,    61,
      62,    98,    99,   107,    16,   107,   100,   101,   107,     6,
      16,     8,    17,    16,   115,    16,   117,    16,    46,    47,
      48,    49,    27,    61,    62,    10,    51,   115,   129,   117,
      58,    54,    51,    18,    62,     3,    64,    65,    66,   127,
      68,    69,    27,     3,     3,    66,    58,     8,   147,    16,
      62,   152,    64,    65,    66,    58,    68,    69,    66,    62,
      18,    64,    65,    66,   152,    68,    69,    52,    10,    10,
      66,   172,   171,    66,   175,    27,    18,    18,   166,    10,
      10,   169,    30,    66,    66,   178,    27,    18,    18,    33,
      66,    58,   193,   194,    66,    62,    27,    64,    65,    66,
      38,    68,    69,    66,    10,   193,   194,    10,   209,    10,
      52,    52,    18,    66,    18,    18,   204,    18,    16,    66,
      66,    52,    52,    10,    27,    10,    27,    34,    18,    66,
     280,    18,    58,    18,    66,    29,   235,   236,    64,    65,
      31,    66,    68,    64,    66,    66,    52,    17,    69,    52,
      68,    52,   302,   303,   255,    17,   257,   258,   259,    34,
     248,   254,   250,   251,   252,    52,    66,    52,    34,   319,
     320,    39,    40,    41,    42,    43,    44,    45,    39,    40,
      41,    42,    43,    44,    66,    66,    53,    16,   289,    39,
      28,    59,    60,    61,    62,    66,    18,   285,    18,    60,
      18,    66,   301,    66,    16,    50,    66,    66,    66,   310,
     311,    66,    66,    66,     6,    32,    17,    17,    17,    17,
     308,   309,    17,    66,    56,    16,     9,    66,    18,    18,
      17,    45,    59,    66,    64,    66,    17,    58,    66,    17,
      16,    16,   262,    17,    17,    16,    16,   212,    17,    17,
      17,    17,    17,   298,   272,   315,   225,   211,   110,   221,
     274,    -1,   186
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    72,     0,     4,     5,     9,    10,    11,    12,    13,
      14,    15,    16,    18,    19,    20,    21,    25,    26,    35,
      46,    47,    48,    49,    50,    53,    56,    58,    62,    64,
      65,    66,    68,    69,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    91,    97,    98,
      99,   100,   101,   102,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   126,     6,     8,    55,     6,     8,
      66,   108,   109,   110,   111,   112,   113,   114,    66,     7,
      28,    30,    66,   110,    66,    36,    16,    16,    16,    16,
      51,    54,    51,    16,   108,    27,   102,     3,    61,    62,
      63,    69,    66,    66,     8,    66,    66,    18,   115,   115,
      30,    66,    66,    33,    17,    10,    27,    52,   105,   107,
      38,    66,    66,    64,    66,    69,    66,    66,    66,    66,
     110,    66,    69,    18,   103,   111,   111,   112,   112,    16,
      34,    66,   109,   113,    66,   101,    29,    31,   116,    66,
     105,   107,    66,   105,   107,    68,    17,    27,    17,    27,
      17,    17,    27,    17,    17,    27,    10,    18,    27,    52,
     107,    34,    10,    18,    27,    52,   105,    17,    66,    66,
      87,    90,    66,    34,   115,   115,   116,    16,    93,    16,
     110,   118,    39,    10,    52,   105,   107,    28,    66,    66,
      66,    66,   107,    66,    66,   107,   118,   105,    66,    66,
     105,   102,    18,    86,    22,    23,    24,    37,    89,    16,
      66,   106,    97,    92,   123,   124,    39,    40,    41,    42,
      43,    44,    45,    59,    60,   125,    32,   117,    97,   105,
     107,   105,   107,     6,    17,    17,    17,    17,    10,    27,
      52,    10,    52,   107,   117,    10,    27,    52,    10,    52,
     105,   103,    87,    17,    16,    57,    59,    66,    16,   104,
      18,    96,    18,    94,    17,   100,   119,   120,    45,    59,
      16,   110,   118,   116,    66,    66,   107,   107,   102,    66,
     105,   105,    86,    64,    88,    58,    17,    66,    97,    17,
      93,   125,    16,    16,   121,   122,   123,   117,    10,    52,
      10,    52,    17,    17,    96,    95,   110,   123,   123,    16,
      16,    17,    94,    17,    17,   123,   123,    17,    17
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    71,    72,    72,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    83,    84,    85,    86,    86,    87,    87,
      87,    87,    88,    89,    89,    89,    89,    90,    92,    91,
      93,    94,    95,    94,    96,    96,    97,    97,    97,    97,
      98,    99,   100,   101,   102,   102,   103,   103,   104,   104,
     104,   104,   104,   104,   104,   105,   105,   105,   105,   105,
     105,   105,   106,   106,   106,   106,   106,   106,   106,   107,
     107,   107,   107,   107,   107,   107,   108,   108,   108,   108,
     109,   110,   110,   110,   111,   111,   111,   112,   112,   112,
     112,   112,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   114,   114,   115,   115,   115,   116,   116,   117,
     117,   118,   119,   118,   120,   118,   121,   118,   122,   118,
     118,   118,   118,   124,   123,   125,   125,   125,   125,   125,
     125,   125,   125,   126
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     2,     8,     9,     3,     7,     0,     3,     5,     2,
       4,     3,     1,     1,     1,     1,     1,     1,     0,     7,
       4,     0,     0,     4,     0,     3,     1,     1,     1,     1,
       4,     7,     7,     3,     0,     7,     0,     4,     0,     4,
       6,     5,     7,     5,     7,     0,     3,     5,     4,     6,
       4,     6,     0,     4,     6,     5,     7,     5,     7,     0,
       3,     5,     4,     6,     4,     6,     1,     1,     3,     3,
       1,     3,     3,     1,     3,     3,     1,     3,     4,     2,
       1,     1,     6,     4,     6,     4,     4,     6,     4,     4,
       6,     4,     2,     2,     0,     3,     3,     0,     3,     0,
       3,     3,     0,     6,     0,     6,     0,     7,     0,     7,
       5,     5,     5,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     2,     7
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
  case 33:
#line 222 "yacc_sql.y"
         {
        CONTEXT->ssql->flag=SCF_EXIT;//"exit";
    }
#line 1717 "yacc_sql.tab.c"
    break;

  case 34:
#line 227 "yacc_sql.y"
         {
        CONTEXT->ssql->flag=SCF_HELP;//"help";
    }
#line 1725 "yacc_sql.tab.c"
    break;

  case 35:
#line 232 "yacc_sql.y"
         {
      CONTEXT->ssql->flag = SCF_SYNC;
    }
#line 1733 "yacc_sql.tab.c"
    break;

  case 36:
#line 238 "yacc_sql.y"
              {
      CONTEXT->ssql->flag = SCF_BEGIN;
    }
#line 1741 "yacc_sql.tab.c"
    break;

  case 37:
#line 244 "yacc_sql.y"
               {
      CONTEXT->ssql->flag = SCF_COMMIT;
    }
#line 1749 "yacc_sql.tab.c"
    break;

  case 38:
#line 250 "yacc_sql.y"
                 {
      CONTEXT->ssql->flag = SCF_ROLLBACK;
    }
#line 1757 "yacc_sql.tab.c"
    break;

  case 39:
#line 256 "yacc_sql.y"
                  {
        CONTEXT->ssql->flag = SCF_DROP_TABLE;//"drop_table";
        drop_table_init(&CONTEXT->ssql->sstr.drop_table, (yyvsp[0].string));
    }
#line 1766 "yacc_sql.tab.c"
    break;

  case 40:
#line 262 "yacc_sql.y"
                {
      CONTEXT->ssql->flag = SCF_SHOW_TABLES;
    }
#line 1774 "yacc_sql.tab.c"
    break;

  case 41:
#line 268 "yacc_sql.y"
            {
      CONTEXT->ssql->flag = SCF_DESC_TABLE;
      desc_table_init(&CONTEXT->ssql->sstr.desc_table, (yyvsp[0].string));
    }
#line 1783 "yacc_sql.tab.c"
    break;

  case 42:
#line 276 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
			create_index_init(&CONTEXT->ssql->sstr.create_index, (yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string), false);
		}
#line 1792 "yacc_sql.tab.c"
    break;

  case 43:
#line 282 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
                        create_index_init(&CONTEXT->ssql->sstr.create_index, (yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string), true);
		}
#line 1801 "yacc_sql.tab.c"
    break;

  case 44:
#line 290 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_DROP_INDEX;//"drop_index";
			drop_index_init(&CONTEXT->ssql->sstr.drop_index, (yyvsp[0].string));
		}
#line 1810 "yacc_sql.tab.c"
    break;

  case 45:
#line 297 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_CREATE_TABLE;//"create_table";
			// CONTEXT->ssql->sstr.create_table.attribute_count = CONTEXT->value_length;
			create_table_init_name(&CONTEXT->ssql->sstr.create_table, (yyvsp[-4].string));
			//临时变量清零	
			CONTEXT->value_length = 0;
		}
#line 1822 "yacc_sql.tab.c"
    break;

  case 47:
#line 307 "yacc_sql.y"
                                   {    }
#line 1828 "yacc_sql.tab.c"
    break;

  case 48:
#line 312 "yacc_sql.y"
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
#line 1843 "yacc_sql.tab.c"
    break;

  case 49:
#line 323 "yacc_sql.y"
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
#line 1858 "yacc_sql.tab.c"
    break;

  case 50:
#line 334 "yacc_sql.y"
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
#line 1873 "yacc_sql.tab.c"
    break;

  case 51:
#line 345 "yacc_sql.y"
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
#line 1888 "yacc_sql.tab.c"
    break;

  case 52:
#line 357 "yacc_sql.y"
                       {(yyval.number) = (yyvsp[0].number);}
#line 1894 "yacc_sql.tab.c"
    break;

  case 53:
#line 360 "yacc_sql.y"
              { (yyval.number)=INTS; }
#line 1900 "yacc_sql.tab.c"
    break;

  case 54:
#line 361 "yacc_sql.y"
                  { (yyval.number)=CHARS; }
#line 1906 "yacc_sql.tab.c"
    break;

  case 55:
#line 362 "yacc_sql.y"
                 { (yyval.number)=FLOATS; }
#line 1912 "yacc_sql.tab.c"
    break;

  case 56:
#line 363 "yacc_sql.y"
                { (yyval.number)=DATE; }
#line 1918 "yacc_sql.tab.c"
    break;

  case 57:
#line 367 "yacc_sql.y"
        {
		char *temp=(yyvsp[0].string); 
		snprintf(CONTEXT->id, sizeof(CONTEXT->id), "%s", temp);
	}
#line 1927 "yacc_sql.tab.c"
    break;

  case 58:
#line 375 "yacc_sql.y"
                                { insert_tuple_add(CONTEXT); }
#line 1933 "yacc_sql.tab.c"
    break;

  case 59:
#line 376 "yacc_sql.y"
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
#line 1952 "yacc_sql.tab.c"
    break;

  case 62:
#line 395 "yacc_sql.y"
                  {
		insert_tuple_add(CONTEXT);
	}
#line 1960 "yacc_sql.tab.c"
    break;

  case 65:
#line 401 "yacc_sql.y"
                              {
  		// CONTEXT->values[CONTEXT->value_length++] = *$2;
	  }
#line 1968 "yacc_sql.tab.c"
    break;

  case 66:
#line 406 "yacc_sql.y"
          {	
  		value_init_integer(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].number));
		}
#line 1976 "yacc_sql.tab.c"
    break;

  case 67:
#line 409 "yacc_sql.y"
          {
  		value_init_float(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].floats));
		}
#line 1984 "yacc_sql.tab.c"
    break;

  case 68:
#line 412 "yacc_sql.y"
         {
			(yyvsp[0].string) = substr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
  		value_init_string(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].string));
		}
#line 1993 "yacc_sql.tab.c"
    break;

  case 69:
#line 416 "yacc_sql.y"
                        {
		value_init_null(&CONTEXT->values[CONTEXT->value_length++]);
		}
#line 2001 "yacc_sql.tab.c"
    break;

  case 70:
#line 423 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_DELETE;//"delete";
			deletes_init_relation(&CONTEXT->ssql->sstr.deletion, (yyvsp[-1].string));
			deletes_set_conditions(&CONTEXT->ssql->sstr.deletion, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;	
    }
#line 2013 "yacc_sql.tab.c"
    break;

  case 71:
#line 433 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_UPDATE;//"update";
			Value *value = &CONTEXT->values[0];
			updates_init(&CONTEXT->ssql->sstr.update, (yyvsp[-5].string), (yyvsp[-3].string), value, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;
		}
#line 2025 "yacc_sql.tab.c"
    break;

  case 72:
#line 443 "yacc_sql.y"
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
#line 2042 "yacc_sql.tab.c"
    break;

  case 73:
#line 456 "yacc_sql.y"
                             {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-2].string));
	}
#line 2050 "yacc_sql.tab.c"
    break;

  case 75:
#line 461 "yacc_sql.y"
                                                     {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-4].string));
}
#line 2058 "yacc_sql.tab.c"
    break;

  case 77:
#line 466 "yacc_sql.y"
                                  {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-2].string));
	}
#line 2066 "yacc_sql.tab.c"
    break;

  case 79:
#line 472 "yacc_sql.y"
                        {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2076 "yacc_sql.tab.c"
    break;

  case 80:
#line 477 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2086 "yacc_sql.tab.c"
    break;

  case 81:
#line 482 "yacc_sql.y"
                            {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2096 "yacc_sql.tab.c"
    break;

  case 82:
#line 487 "yacc_sql.y"
                                   {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2106 "yacc_sql.tab.c"
    break;

  case 83:
#line 492 "yacc_sql.y"
                             {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2116 "yacc_sql.tab.c"
    break;

  case 84:
#line 497 "yacc_sql.y"
                                    {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2126 "yacc_sql.tab.c"
    break;

  case 86:
#line 504 "yacc_sql.y"
                      {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2136 "yacc_sql.tab.c"
    break;

  case 87:
#line 509 "yacc_sql.y"
                              {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2146 "yacc_sql.tab.c"
    break;

  case 88:
#line 514 "yacc_sql.y"
                           {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2156 "yacc_sql.tab.c"
    break;

  case 89:
#line 519 "yacc_sql.y"
                                  {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2166 "yacc_sql.tab.c"
    break;

  case 90:
#line 524 "yacc_sql.y"
                            {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2176 "yacc_sql.tab.c"
    break;

  case 91:
#line 529 "yacc_sql.y"
                                   {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2186 "yacc_sql.tab.c"
    break;

  case 93:
#line 536 "yacc_sql.y"
                        {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2196 "yacc_sql.tab.c"
    break;

  case 94:
#line 541 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2206 "yacc_sql.tab.c"
    break;

  case 95:
#line 546 "yacc_sql.y"
                            {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2216 "yacc_sql.tab.c"
    break;

  case 96:
#line 551 "yacc_sql.y"
                                   {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2226 "yacc_sql.tab.c"
    break;

  case 97:
#line 556 "yacc_sql.y"
                             {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2236 "yacc_sql.tab.c"
    break;

  case 98:
#line 561 "yacc_sql.y"
                                    {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2246 "yacc_sql.tab.c"
    break;

  case 100:
#line 568 "yacc_sql.y"
                      {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2256 "yacc_sql.tab.c"
    break;

  case 101:
#line 573 "yacc_sql.y"
                              {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2266 "yacc_sql.tab.c"
    break;

  case 102:
#line 578 "yacc_sql.y"
                           {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2276 "yacc_sql.tab.c"
    break;

  case 103:
#line 583 "yacc_sql.y"
                                  {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2286 "yacc_sql.tab.c"
    break;

  case 104:
#line 588 "yacc_sql.y"
                            {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2296 "yacc_sql.tab.c"
    break;

  case 105:
#line 593 "yacc_sql.y"
                                   {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2306 "yacc_sql.tab.c"
    break;

  case 106:
#line 599 "yacc_sql.y"
            {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*");
			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
#line 2316 "yacc_sql.tab.c"
    break;

  case 107:
#line 604 "yacc_sql.y"
             {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[0].string));
			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
#line 2326 "yacc_sql.tab.c"
    break;

  case 108:
#line 609 "yacc_sql.y"
                    {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-2].string), (yyvsp[0].string));
			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
#line 2336 "yacc_sql.tab.c"
    break;

  case 109:
#line 614 "yacc_sql.y"
                      {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-2].string), "*");
			//selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
            	}
#line 2346 "yacc_sql.tab.c"
    break;

  case 110:
#line 622 "yacc_sql.y"
         {
		selects_append_attribute_plus(&CONTEXT->ssql->sstr.selection);
	}
#line 2354 "yacc_sql.tab.c"
    break;

  case 111:
#line 626 "yacc_sql.y"
               {
		e_e_t(Add);
	}
#line 2362 "yacc_sql.tab.c"
    break;

  case 112:
#line 629 "yacc_sql.y"
                 {
		e_e_t(Sub);
	}
#line 2370 "yacc_sql.tab.c"
    break;

  case 113:
#line 632 "yacc_sql.y"
           {
		e_t();
	}
#line 2378 "yacc_sql.tab.c"
    break;

  case 114:
#line 636 "yacc_sql.y"
                {
		t_t_f(Mul);
	}
#line 2386 "yacc_sql.tab.c"
    break;

  case 115:
#line 639 "yacc_sql.y"
                 {
		t_t_f(Div);
	}
#line 2394 "yacc_sql.tab.c"
    break;

  case 116:
#line 642 "yacc_sql.y"
           {
		t_f();
	}
#line 2402 "yacc_sql.tab.c"
    break;

  case 117:
#line 646 "yacc_sql.y"
                       {
		f_e();
	}
#line 2410 "yacc_sql.tab.c"
    break;

  case 118:
#line 650 "yacc_sql.y"
                           {
			set_sub();
	}
#line 2418 "yacc_sql.tab.c"
    break;

  case 119:
#line 653 "yacc_sql.y"
                {
        		set_sub();
        	}
#line 2426 "yacc_sql.tab.c"
    break;

  case 120:
#line 656 "yacc_sql.y"
            {
		//set_sub();
	}
#line 2434 "yacc_sql.tab.c"
    break;

  case 121:
#line 659 "yacc_sql.y"
              {
        	Value *value = &CONTEXT->values[CONTEXT->value_length - 1];
		relation_value_append(value);
        }
#line 2443 "yacc_sql.tab.c"
    break;

  case 122:
#line 664 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2453 "yacc_sql.tab.c"
    break;

  case 123:
#line 669 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2463 "yacc_sql.tab.c"
    break;

  case 124:
#line 674 "yacc_sql.y"
                                {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2473 "yacc_sql.tab.c"
    break;

  case 125:
#line 679 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2483 "yacc_sql.tab.c"
    break;

  case 126:
#line 684 "yacc_sql.y"
                           {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2493 "yacc_sql.tab.c"
    break;

  case 127:
#line 689 "yacc_sql.y"
                                  {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2503 "yacc_sql.tab.c"
    break;

  case 128:
#line 694 "yacc_sql.y"
                                  {
        	RelAttr attr;
        	relation_agg_attr_init(&attr, NULL, "*",Count);
                selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
            }
#line 2513 "yacc_sql.tab.c"
    break;

  case 129:
#line 699 "yacc_sql.y"
                                       {
                    	RelAttr attr;
                    	relation_agg_attr_init(&attr, NULL, "*",Count);
                            selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
                        }
#line 2523 "yacc_sql.tab.c"
    break;

  case 130:
#line 704 "yacc_sql.y"
                                {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2533 "yacc_sql.tab.c"
    break;

  case 131:
#line 709 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2543 "yacc_sql.tab.c"
    break;

  case 138:
#line 726 "yacc_sql.y"
                                     {	
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 2551 "yacc_sql.tab.c"
    break;

  case 140:
#line 732 "yacc_sql.y"
                                   {
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 2559 "yacc_sql.tab.c"
    break;

  case 141:
#line 737 "yacc_sql.y"
             {
    		Condition condition;
        	condition_init_ex(&condition, CONTEXT->comp);
        	CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
#line 2569 "yacc_sql.tab.c"
    break;

  case 142:
#line 887 "yacc_sql.y"
               { CONTEXT->comp = CONTAINED_BY; }
#line 2575 "yacc_sql.tab.c"
    break;

  case 143:
#line 888 "yacc_sql.y"
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
			condition_init(&condition, CONTEXT->comp, 1, left_attr, NULL, 0, &right_attr, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2608 "yacc_sql.tab.c"
    break;

  case 144:
#line 916 "yacc_sql.y"
               { CONTEXT->comp = CONTAINED_BY; }
#line 2614 "yacc_sql.tab.c"
    break;

  case 145:
#line 917 "yacc_sql.y"
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
			condition_init(&condition, CONTEXT->comp, 1, left_attr, NULL, 0, &right_attr, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2644 "yacc_sql.tab.c"
    break;

  case 146:
#line 942 "yacc_sql.y"
                   { CONTEXT->comp = NOT_CONTAINED_BY; }
#line 2650 "yacc_sql.tab.c"
    break;

  case 147:
#line 943 "yacc_sql.y"
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
			condition_init(&condition, CONTEXT->comp, 1, left_attr, NULL, 0, &right_attr, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2680 "yacc_sql.tab.c"
    break;

  case 148:
#line 968 "yacc_sql.y"
                   { CONTEXT->comp = NOT_CONTAINED_BY; }
#line 2686 "yacc_sql.tab.c"
    break;

  case 149:
#line 969 "yacc_sql.y"
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
			condition_init(&condition, CONTEXT->comp, 1, left_attr, NULL, 0, &right_attr, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2716 "yacc_sql.tab.c"
    break;

  case 150:
#line 995 "yacc_sql.y"
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
			condition_init(&condition, CONTEXT->comp, 1, left_attr, NULL, 0, &right_attr, &rval);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2745 "yacc_sql.tab.c"
    break;

  case 151:
#line 1020 "yacc_sql.y"
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
			condition_init(&condition, CONTEXT->comp, 0, &right_attr, &rval, 1, left_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2775 "yacc_sql.tab.c"
    break;

  case 152:
#line 1046 "yacc_sql.y"
                {
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr *left_attr;
                       // relation_attr_get(left_attr);

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
			condition_init(&condition, CONTEXT->comp, 0,&right_attr, &rval, 1, left_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2805 "yacc_sql.tab.c"
    break;

  case 153:
#line 1087 "yacc_sql.y"
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
#line 2824 "yacc_sql.tab.c"
    break;

  case 154:
#line 1102 "yacc_sql.y"
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
#line 2850 "yacc_sql.tab.c"
    break;

  case 155:
#line 1126 "yacc_sql.y"
             { CONTEXT->comp = EQUAL_TO; }
#line 2856 "yacc_sql.tab.c"
    break;

  case 156:
#line 1127 "yacc_sql.y"
         { CONTEXT->comp = LESS_THAN; }
#line 2862 "yacc_sql.tab.c"
    break;

  case 157:
#line 1128 "yacc_sql.y"
         { CONTEXT->comp = GREAT_THAN; }
#line 2868 "yacc_sql.tab.c"
    break;

  case 158:
#line 1129 "yacc_sql.y"
         { CONTEXT->comp = LESS_EQUAL; }
#line 2874 "yacc_sql.tab.c"
    break;

  case 159:
#line 1130 "yacc_sql.y"
         { CONTEXT->comp = GREAT_EQUAL; }
#line 2880 "yacc_sql.tab.c"
    break;

  case 160:
#line 1131 "yacc_sql.y"
         { CONTEXT->comp = NOT_EQUAL; }
#line 2886 "yacc_sql.tab.c"
    break;

  case 161:
#line 1132 "yacc_sql.y"
         { CONTEXT->comp = IS_CompOP; }
#line 2892 "yacc_sql.tab.c"
    break;

  case 162:
#line 1133 "yacc_sql.y"
             {CONTEXT->comp = IS_NOT_CompOP;}
#line 2898 "yacc_sql.tab.c"
    break;

  case 163:
#line 1138 "yacc_sql.y"
                {
		  CONTEXT->ssql->flag = SCF_LOAD_DATA;
			load_data_init(&CONTEXT->ssql->sstr.load_data, (yyvsp[0].string), (yyvsp[-3].string));
		}
#line 2907 "yacc_sql.tab.c"
    break;


#line 2911 "yacc_sql.tab.c"

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
#line 1143 "yacc_sql.y"

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
