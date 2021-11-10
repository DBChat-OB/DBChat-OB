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


#line 149 "yacc_sql.tab.c"

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
    MAX = 300,
    MIN = 301,
    COUNT = 302,
    AVG = 303,
    ORDER = 304,
    BY = 305,
    ASC = 306,
    INNER = 307,
    JOIN = 308,
    UNIQUE = 309,
    NUMBER = 310,
    FLOAT = 311,
    ID = 312,
    PATH = 313,
    SSS = 314,
    STAR = 315,
    STRING_V = 316
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 140 "yacc_sql.y"

  struct _Attr *attr;
  struct _Condition *condition1;
  struct _Value *value1;
  char *string;
  int number;
  float floats;
	char *position;

#line 273 "yacc_sql.tab.c"

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
#define YYLAST   270

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  62
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  45
/* YYNRULES -- Number of rules.  */
#define YYNRULES  124
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  276

#define YYUNDEFTOK  2
#define YYMAXUTOK   316


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
      55,    56,    57,    58,    59,    60,    61
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   166,   166,   168,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   199,   204,
     209,   215,   221,   227,   233,   239,   245,   252,   258,   266,
     273,   282,   284,   288,   299,   312,   315,   316,   317,   318,
     321,   330,   330,   347,   349,   350,   350,   354,   356,   361,
     364,   367,   374,   384,   394,   409,   422,   425,   427,   430,
     432,   437,   440,   442,   445,   447,   452,   457,   462,   467,
     472,   477,   479,   484,   489,   494,   499,   504,   510,   515,
     520,   525,   530,   535,   540,   545,   550,   555,   561,   566,
     571,   576,   583,   585,   592,   599,   606,   616,   618,   622,
     624,   629,   650,   670,   690,   712,   733,   754,   776,   777,
     778,   779,   780,   781,   785
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
  "DATE_T", "INFILE", "EQ", "LT", "GT", "LE", "GE", "NE", "MAX", "MIN",
  "COUNT", "AVG", "ORDER", "BY", "ASC", "INNER", "JOIN", "UNIQUE",
  "NUMBER", "FLOAT", "ID", "PATH", "SSS", "STAR", "STRING_V", "$accept",
  "commands", "command", "exit", "help", "sync", "begin", "commit",
  "rollback", "drop_table", "show_tables", "desc_table", "create_index",
  "drop_index", "create_table", "attr_def_list", "attr_def", "number",
  "type", "ID_get", "insert", "$@1", "tuple", "tuple_list", "$@2",
  "value_list", "value", "delete", "update", "select", "relations",
  "join_list", "rel_list", "agg_attrs", "AGG_list", "orders", "order_list",
  "AGG", "select_attr", "attr_list", "where", "condition_list",
  "condition", "comOp", "load_data", YY_NULLPTR
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
     315,   316
};
# endif

#define YYPACT_NINF (-178)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -178,   119,  -178,     4,    47,    38,   -22,    41,    57,    45,
      35,    20,    43,    76,    94,   104,   112,   115,    58,   111,
     125,   135,   136,    70,    96,   101,  -178,  -178,  -178,  -178,
    -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,
    -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,   142,
    -178,   105,   106,   161,   113,   126,    54,   154,   152,   142,
     155,   181,   183,  -178,   130,   131,   156,     1,   142,  -178,
    -178,  -178,  -178,  -178,   153,   137,   138,   -24,   139,   140,
     141,   172,   133,  -178,   176,   159,   143,   198,   199,   -35,
      36,  -178,  -178,   146,   146,  -178,  -178,   175,   174,   149,
     189,   151,   157,   189,  -178,  -178,   150,     0,     3,   193,
      34,   194,    37,     5,   178,   158,  -178,   160,   162,   179,
    -178,  -178,    60,   154,   154,   154,   174,   174,   200,    55,
     215,   182,  -178,     6,  -178,   192,  -178,   165,  -178,   166,
    -178,  -178,   167,  -178,  -178,   168,   189,   169,   189,  -178,
      55,   101,  -178,   209,    22,   212,   173,    56,  -178,  -178,
    -178,  -178,   226,   184,    66,  -178,  -178,  -178,    -1,  -178,
      62,   202,  -178,    66,   189,   189,  -178,   225,   218,   219,
     220,   221,  -178,    11,  -178,   202,   172,   160,   222,  -178,
    -178,  -178,  -178,   216,   185,   224,   154,   154,  -178,   238,
     227,   228,   186,  -178,  -178,  -178,  -178,  -178,  -178,    91,
     102,    55,  -178,   174,  -178,  -178,   187,  -178,  -178,  -178,
    -178,   189,   189,  -178,   101,  -178,   209,   244,   195,   231,
     196,  -178,  -178,  -178,    66,   232,   200,   248,    62,  -178,
    -178,   229,  -178,   202,   249,   251,  -178,  -178,  -178,  -178,
    -178,  -178,   240,   252,   241,   227,  -178,  -178,  -178,   118,
     203,  -178,  -178,  -178,  -178,  -178,   256,  -178,   228,   234,
    -178,  -178,  -178,  -178,   205,  -178
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    67,     3,    20,    19,    14,
      15,    16,    17,     9,    10,    11,    12,    13,     8,     5,
       7,     6,     4,    26,    27,    21,    23,    24,    25,    22,
      18,     0,     0,     0,     0,     0,   102,   102,     0,    72,
       0,     0,     0,    30,     0,     0,     0,    81,    72,    31,
      32,    33,    29,    28,     0,     0,     0,     0,     0,     0,
       0,    69,     0,    71,     0,     0,     0,     0,     0,     0,
       0,    99,    98,     0,     0,    36,    35,     0,   107,     0,
      81,     0,     0,    81,    82,    73,     0,     0,     0,     0,
       0,     0,     0,    81,     0,     0,    66,     0,     0,     0,
      34,    39,   102,   102,   102,   102,   107,   107,     0,     0,
       0,     0,    86,    81,    84,     0,    89,     0,    91,     0,
      95,    92,     0,    94,    97,     0,    81,     0,    81,    75,
       0,    67,    50,    41,     0,     0,     0,     0,   103,   106,
     100,   101,     0,    74,     0,    51,    59,    60,     0,    61,
       0,   109,    62,     0,    81,    81,    83,     0,     0,     0,
       0,     0,    79,    81,    77,   109,    69,     0,     0,    46,
      47,    48,    49,    44,     0,     0,   102,   102,    65,     0,
      57,    54,     0,   118,   119,   120,   121,   122,   123,     0,
       0,     0,   108,   107,    87,    85,     0,    88,    90,    93,
      96,    81,    81,    76,    67,    70,    41,     0,     0,     0,
       0,   104,   105,    64,     0,     0,     0,     0,     0,   113,
     111,   114,   112,   109,     0,     0,    80,    78,    68,    42,
      40,    45,     0,     0,     0,    57,    53,    55,    52,     0,
       0,   110,    63,   124,    43,    37,     0,    58,    54,     0,
     115,   116,    38,    56,     0,   117
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,  -178,
    -178,  -178,  -178,  -178,  -178,    39,    77,  -178,  -178,  -178,
    -178,  -178,    27,    -2,  -178,    12,  -160,  -178,  -178,  -178,
      42,   -25,    82,   264,     8,   107,   -66,    13,  -178,   -54,
    -121,  -177,  -148,  -163,  -178
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,   188,   153,   252,   193,   154,
      39,   201,   165,   237,   268,   235,   170,    40,    41,    42,
      43,    44,   116,    45,    83,    47,    48,    68,    60,    91,
     130,   212,   171,   209,    50
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      81,   104,   185,    92,   200,   162,   163,   210,   224,    46,
      51,   100,    52,   213,    49,   146,   174,   136,    59,   101,
     138,   221,   122,   101,   101,   123,   202,   137,   102,   101,
     139,   109,   147,   110,   132,    61,   111,   134,   203,   204,
     205,   206,   207,   208,   189,   190,   191,   149,    62,   240,
     242,   141,   103,    54,   144,    55,   148,   175,    53,   192,
      63,   142,   222,   243,   145,    65,   261,   176,   158,   159,
     160,   161,    89,    64,   255,   259,   105,    66,    89,    69,
     182,    90,   184,    19,    20,    21,    22,   157,    19,    20,
      21,    22,   244,   124,    74,    56,   125,    70,    57,   270,
      67,   203,   204,   205,   206,   207,   208,    71,   214,   215,
     166,   167,   168,   196,   169,    72,   197,   223,    73,     2,
      79,   166,   167,     3,     4,   169,   186,    75,     5,     6,
       7,     8,     9,    10,    11,   126,   127,    12,    13,    14,
      15,    76,   231,   232,    16,    17,   166,   167,   239,    80,
     169,    77,    78,    24,    18,   246,   247,   166,   167,   241,
      82,   169,    84,    85,    19,    20,    21,    22,    23,    86,
      87,    24,    89,   166,   167,   269,    25,   169,    19,    20,
      21,    22,    93,    88,    95,    94,    96,    97,    98,    99,
     115,   106,   117,   118,   107,   108,   112,   113,   114,   248,
     119,   120,   121,    25,   128,   129,   131,   101,    67,   135,
     140,   143,   150,   156,   133,   151,   164,   152,   172,   155,
     177,   173,   178,   179,   180,   181,   183,   187,   194,   198,
     195,   216,   228,    23,   211,   217,   218,   219,   220,   227,
     230,   233,   229,   238,   245,   234,   236,   250,   253,   256,
     251,   258,   262,   254,   263,   265,   260,   264,   266,   272,
     271,   274,   275,   257,   226,   249,   273,   267,   225,    58,
     199
};

static const yytype_int16 yycheck[] =
{
      25,    67,   150,    57,   164,   126,   127,   170,   185,     1,
       6,    10,     8,   173,     1,    10,    10,    17,     5,    18,
      17,    10,    57,    18,    18,    60,    27,    27,    27,    18,
      27,    55,    27,    57,   100,    57,    60,   103,    39,    40,
      41,    42,    43,    44,    22,    23,    24,   113,     7,   209,
     210,    17,    51,     6,    17,     8,    51,    51,    54,    37,
       3,    27,    51,   211,    27,    30,   243,   133,   122,   123,
     124,   125,    18,    28,   234,   238,    68,    57,    18,     3,
     146,    27,   148,    45,    46,    47,    48,    27,    45,    46,
      47,    48,   213,    57,    36,    57,    60,     3,    60,   259,
      57,    39,    40,    41,    42,    43,    44,     3,   174,   175,
      55,    56,    57,    57,    59,     3,    60,   183,     3,     0,
      50,    55,    56,     4,     5,    59,   151,    16,     9,    10,
      11,    12,    13,    14,    15,    93,    94,    18,    19,    20,
      21,    16,   196,   197,    25,    26,    55,    56,    57,    53,
      59,    16,    16,    52,    35,   221,   222,    55,    56,    57,
      18,    59,    57,    57,    45,    46,    47,    48,    49,     8,
      57,    52,    18,    55,    56,    57,    57,    59,    45,    46,
      47,    48,    30,    57,     3,    30,     3,    57,    57,    33,
      18,    38,    16,    34,    57,    57,    57,    57,    57,   224,
      57,     3,     3,    57,    29,    31,    57,    18,    57,    59,
      17,    17,    34,    34,    57,    57,    16,    57,     3,    57,
      28,    39,    57,    57,    57,    57,    57,    18,    16,     3,
      57,     6,    16,    49,    32,    17,    17,    17,    17,    17,
      16,     3,    57,    57,    57,    18,    18,     3,    17,    17,
      55,     3,     3,    57,     3,     3,    27,    17,    17,     3,
      57,    27,    57,   236,   187,   226,   268,   255,   186,     5,
     163
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    63,     0,     4,     5,     9,    10,    11,    12,    13,
      14,    15,    18,    19,    20,    21,    25,    26,    35,    45,
      46,    47,    48,    49,    52,    57,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    82,
      89,    90,    91,    92,    93,    95,    96,    97,    98,    99,
     106,     6,     8,    54,     6,     8,    57,    60,    95,    99,
     100,    57,     7,     3,    28,    30,    57,    57,    99,     3,
       3,     3,     3,     3,    36,    16,    16,    16,    16,    50,
      53,    93,    18,    96,    57,    57,     8,    57,    57,    18,
      27,   101,   101,    30,    30,     3,     3,    57,    57,    33,
      10,    18,    27,    51,    98,    96,    38,    57,    57,    55,
      57,    60,    57,    57,    57,    18,    94,    16,    34,    57,
       3,     3,    57,    60,    57,    60,    92,    92,    29,    31,
     102,    57,    98,    57,    98,    59,    17,    27,    17,    27,
      17,    17,    27,    17,    17,    27,    10,    27,    51,    98,
      34,    57,    57,    78,    81,    57,    34,    27,   101,   101,
     101,   101,   102,   102,    16,    84,    55,    56,    57,    59,
      88,   104,     3,    39,    10,    51,    98,    28,    57,    57,
      57,    57,    98,    57,    98,   104,    93,    18,    77,    22,
      23,    24,    37,    80,    16,    57,    57,    60,     3,    97,
      88,    83,    27,    39,    40,    41,    42,    43,    44,   105,
     105,    32,   103,    88,    98,    98,     6,    17,    17,    17,
      17,    10,    51,    98,   103,    94,    78,    17,    16,    57,
      16,   101,   101,     3,    18,    87,    18,    85,    57,    57,
      88,    57,    88,   104,   102,    57,    98,    98,    93,    77,
       3,    55,    79,    17,    57,    88,    17,    84,     3,   105,
      27,   103,     3,     3,    17,     3,    17,    87,    86,    57,
      88,    57,     3,    85,    27,    57
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    62,    63,    63,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    74,    75,
      76,    77,    77,    78,    78,    79,    80,    80,    80,    80,
      81,    83,    82,    84,    85,    86,    85,    87,    87,    88,
      88,    88,    89,    90,    91,    91,    92,    93,    93,    94,
      94,    95,    96,    96,    97,    97,    97,    97,    97,    97,
      97,    98,    98,    98,    98,    98,    98,    98,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,   100,   100,
     100,   100,   101,   101,   101,   101,   101,   102,   102,   103,
     103,   104,   104,   104,   104,   104,   104,   104,   105,   105,
     105,   105,   105,   105,   106
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     4,     3,     3,     9,    10,     4,
       8,     0,     3,     5,     2,     1,     1,     1,     1,     1,
       1,     0,     8,     4,     0,     0,     4,     0,     3,     1,
       1,     1,     5,     8,     7,     6,     3,     0,     7,     0,
       4,     2,     0,     3,     0,     4,     6,     5,     7,     5,
       7,     0,     3,     5,     4,     6,     4,     6,     6,     4,
       6,     4,     4,     6,     4,     4,     6,     4,     2,     2,
       4,     4,     0,     3,     5,     5,     3,     0,     3,     0,
       3,     3,     3,     3,     3,     5,     5,     7,     1,     1,
       1,     1,     1,     1,     8
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
  case 28:
#line 199 "yacc_sql.y"
                   {
        CONTEXT->ssql->flag=SCF_EXIT;//"exit";
    }
#line 1647 "yacc_sql.tab.c"
    break;

  case 29:
#line 204 "yacc_sql.y"
                   {
        CONTEXT->ssql->flag=SCF_HELP;//"help";
    }
#line 1655 "yacc_sql.tab.c"
    break;

  case 30:
#line 209 "yacc_sql.y"
                   {
      CONTEXT->ssql->flag = SCF_SYNC;
    }
#line 1663 "yacc_sql.tab.c"
    break;

  case 31:
#line 215 "yacc_sql.y"
                        {
      CONTEXT->ssql->flag = SCF_BEGIN;
    }
#line 1671 "yacc_sql.tab.c"
    break;

  case 32:
#line 221 "yacc_sql.y"
                         {
      CONTEXT->ssql->flag = SCF_COMMIT;
    }
#line 1679 "yacc_sql.tab.c"
    break;

  case 33:
#line 227 "yacc_sql.y"
                           {
      CONTEXT->ssql->flag = SCF_ROLLBACK;
    }
#line 1687 "yacc_sql.tab.c"
    break;

  case 34:
#line 233 "yacc_sql.y"
                            {
        CONTEXT->ssql->flag = SCF_DROP_TABLE;//"drop_table";
        drop_table_init(&CONTEXT->ssql->sstr.drop_table, (yyvsp[-1].string));
    }
#line 1696 "yacc_sql.tab.c"
    break;

  case 35:
#line 239 "yacc_sql.y"
                          {
      CONTEXT->ssql->flag = SCF_SHOW_TABLES;
    }
#line 1704 "yacc_sql.tab.c"
    break;

  case 36:
#line 245 "yacc_sql.y"
                      {
      CONTEXT->ssql->flag = SCF_DESC_TABLE;
      desc_table_init(&CONTEXT->ssql->sstr.desc_table, (yyvsp[-1].string));
    }
#line 1713 "yacc_sql.tab.c"
    break;

  case 37:
#line 253 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
			create_index_init(&CONTEXT->ssql->sstr.create_index, (yyvsp[-6].string), (yyvsp[-4].string), (yyvsp[-2].string), false);
		}
#line 1722 "yacc_sql.tab.c"
    break;

  case 38:
#line 259 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
                        create_index_init(&CONTEXT->ssql->sstr.create_index, (yyvsp[-6].string), (yyvsp[-4].string), (yyvsp[-2].string), true);
		}
#line 1731 "yacc_sql.tab.c"
    break;

  case 39:
#line 267 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_DROP_INDEX;//"drop_index";
			drop_index_init(&CONTEXT->ssql->sstr.drop_index, (yyvsp[-1].string));
		}
#line 1740 "yacc_sql.tab.c"
    break;

  case 40:
#line 274 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_CREATE_TABLE;//"create_table";
			// CONTEXT->ssql->sstr.create_table.attribute_count = CONTEXT->value_length;
			create_table_init_name(&CONTEXT->ssql->sstr.create_table, (yyvsp[-5].string));
			//临时变量清零	
			CONTEXT->value_length = 0;
		}
#line 1752 "yacc_sql.tab.c"
    break;

  case 42:
#line 284 "yacc_sql.y"
                                   {    }
#line 1758 "yacc_sql.tab.c"
    break;

  case 43:
#line 289 "yacc_sql.y"
                {
			AttrInfo attribute;
			attr_info_init(&attribute, CONTEXT->id, (yyvsp[-3].number), (yyvsp[-1].number));
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name =(char*)malloc(sizeof(char));
			// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id); 
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type = $2;  
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length = $4;
			CONTEXT->value_length++;
		}
#line 1773 "yacc_sql.tab.c"
    break;

  case 44:
#line 300 "yacc_sql.y"
                {
			AttrInfo attribute;
			attr_info_init(&attribute, CONTEXT->id, (yyvsp[0].number), 4);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name=(char*)malloc(sizeof(char));
			// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id); 
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type=$2;  
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length=4; // default attribute length
			CONTEXT->value_length++;
		}
#line 1788 "yacc_sql.tab.c"
    break;

  case 45:
#line 312 "yacc_sql.y"
                       {(yyval.number) = (yyvsp[0].number);}
#line 1794 "yacc_sql.tab.c"
    break;

  case 46:
#line 315 "yacc_sql.y"
              { (yyval.number)=INTS; }
#line 1800 "yacc_sql.tab.c"
    break;

  case 47:
#line 316 "yacc_sql.y"
                  { (yyval.number)=CHARS; }
#line 1806 "yacc_sql.tab.c"
    break;

  case 48:
#line 317 "yacc_sql.y"
                 { (yyval.number)=FLOATS; }
#line 1812 "yacc_sql.tab.c"
    break;

  case 49:
#line 318 "yacc_sql.y"
                { (yyval.number)=DATE; }
#line 1818 "yacc_sql.tab.c"
    break;

  case 50:
#line 322 "yacc_sql.y"
        {
		char *temp=(yyvsp[0].string); 
		snprintf(CONTEXT->id, sizeof(CONTEXT->id), "%s", temp);
	}
#line 1827 "yacc_sql.tab.c"
    break;

  case 51:
#line 330 "yacc_sql.y"
                                { insert_tuple_add(CONTEXT); }
#line 1833 "yacc_sql.tab.c"
    break;

  case 52:
#line 331 "yacc_sql.y"
                {
			// CONTEXT->values[CONTEXT->value_length++] = *$6;

			CONTEXT->ssql->flag=SCF_INSERT;//"insert";
			// CONTEXT->ssql->sstr.insertion.relation_name = $3;
			// CONTEXT->ssql->sstr.insertion.value_num = CONTEXT->value_length;
			// for(i = 0; i < CONTEXT->value_length; i++){
			// 	CONTEXT->ssql->sstr.insertion.values[i] = CONTEXT->values[i];
      // }
			inserts_init(&CONTEXT->ssql->sstr.insertion, (yyvsp[-5].string), CONTEXT->insert_tuples, CONTEXT->insert_count);

      //临时变量清零
      CONTEXT->insert_count=0;
    }
#line 1852 "yacc_sql.tab.c"
    break;

  case 55:
#line 350 "yacc_sql.y"
                  {
		insert_tuple_add(CONTEXT);
	}
#line 1860 "yacc_sql.tab.c"
    break;

  case 58:
#line 356 "yacc_sql.y"
                              {
  		// CONTEXT->values[CONTEXT->value_length++] = *$2;
	  }
#line 1868 "yacc_sql.tab.c"
    break;

  case 59:
#line 361 "yacc_sql.y"
          {	
  		value_init_integer(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].number));
		}
#line 1876 "yacc_sql.tab.c"
    break;

  case 60:
#line 364 "yacc_sql.y"
          {
  		value_init_float(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].floats));
		}
#line 1884 "yacc_sql.tab.c"
    break;

  case 61:
#line 367 "yacc_sql.y"
         {
			(yyvsp[0].string) = substr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
  		value_init_string(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].string));
		}
#line 1893 "yacc_sql.tab.c"
    break;

  case 62:
#line 375 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_DELETE;//"delete";
			deletes_init_relation(&CONTEXT->ssql->sstr.deletion, (yyvsp[-2].string));
			deletes_set_conditions(&CONTEXT->ssql->sstr.deletion, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;	
    }
#line 1905 "yacc_sql.tab.c"
    break;

  case 63:
#line 385 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_UPDATE;//"update";
			Value *value = &CONTEXT->values[0];
			updates_init(&CONTEXT->ssql->sstr.update, (yyvsp[-6].string), (yyvsp[-4].string), value, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;
		}
#line 1917 "yacc_sql.tab.c"
    break;

  case 64:
#line 395 "yacc_sql.y"
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
#line 1934 "yacc_sql.tab.c"
    break;

  case 65:
#line 409 "yacc_sql.y"
                                                       {
                selects_append_conditions(&CONTEXT->ssql->sstr.selection, CONTEXT->conditions, CONTEXT->condition_length);

                			CONTEXT->ssql->flag=SCF_SELECT;//"select";
                			// CONTEXT->ssql->sstr.selection.attr_num = CONTEXT->select_length;

                			//临时变量清零
                			CONTEXT->condition_length=0;
                			CONTEXT->from_length=0;
                			CONTEXT->select_length=0;
                			CONTEXT->value_length = 0;
	}
#line 1951 "yacc_sql.tab.c"
    break;

  case 66:
#line 422 "yacc_sql.y"
                             {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-2].string));
	}
#line 1959 "yacc_sql.tab.c"
    break;

  case 68:
#line 427 "yacc_sql.y"
                                                     {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-4].string));
}
#line 1967 "yacc_sql.tab.c"
    break;

  case 70:
#line 432 "yacc_sql.y"
                                  {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-2].string));
	}
#line 1975 "yacc_sql.tab.c"
    break;

  case 71:
#line 437 "yacc_sql.y"
                {

    }
#line 1983 "yacc_sql.tab.c"
    break;

  case 73:
#line 442 "yacc_sql.y"
                        {

    }
#line 1991 "yacc_sql.tab.c"
    break;

  case 75:
#line 447 "yacc_sql.y"
                        {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2001 "yacc_sql.tab.c"
    break;

  case 76:
#line 452 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2011 "yacc_sql.tab.c"
    break;

  case 77:
#line 457 "yacc_sql.y"
                            {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2021 "yacc_sql.tab.c"
    break;

  case 78:
#line 462 "yacc_sql.y"
                                   {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
#line 2031 "yacc_sql.tab.c"
    break;

  case 79:
#line 467 "yacc_sql.y"
                             {
	RelAttr attr;
	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2041 "yacc_sql.tab.c"
    break;

  case 80:
#line 472 "yacc_sql.y"
                                    {
	RelAttr attr;
	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
#line 2051 "yacc_sql.tab.c"
    break;

  case 82:
#line 479 "yacc_sql.y"
                      {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2061 "yacc_sql.tab.c"
    break;

  case 83:
#line 484 "yacc_sql.y"
                              {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2071 "yacc_sql.tab.c"
    break;

  case 84:
#line 489 "yacc_sql.y"
                           {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2081 "yacc_sql.tab.c"
    break;

  case 85:
#line 494 "yacc_sql.y"
                                  {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
#line 2091 "yacc_sql.tab.c"
    break;

  case 86:
#line 499 "yacc_sql.y"
                            {
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2101 "yacc_sql.tab.c"
    break;

  case 87:
#line 504 "yacc_sql.y"
                                   {
 	RelAttr attr;
 	relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string));
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
#line 2111 "yacc_sql.tab.c"
    break;

  case 88:
#line 510 "yacc_sql.y"
                               {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2121 "yacc_sql.tab.c"
    break;

  case 89:
#line 515 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2131 "yacc_sql.tab.c"
    break;

  case 90:
#line 520 "yacc_sql.y"
                                {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2141 "yacc_sql.tab.c"
    break;

  case 91:
#line 525 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2151 "yacc_sql.tab.c"
    break;

  case 92:
#line 530 "yacc_sql.y"
                           {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2161 "yacc_sql.tab.c"
    break;

  case 93:
#line 535 "yacc_sql.y"
                                  {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2171 "yacc_sql.tab.c"
    break;

  case 94:
#line 540 "yacc_sql.y"
                                  {
        	RelAttr attr;
        	relation_agg_attr_init(&attr, NULL, "*",Count);
                selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
            }
#line 2181 "yacc_sql.tab.c"
    break;

  case 95:
#line 545 "yacc_sql.y"
                                       {
                    	RelAttr attr;
                    	relation_agg_attr_init(&attr, NULL, "*",Count);
                            selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
                        }
#line 2191 "yacc_sql.tab.c"
    break;

  case 96:
#line 550 "yacc_sql.y"
                                {
	RelAttr attr;
	relation_agg_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string),Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2201 "yacc_sql.tab.c"
    break;

  case 97:
#line 555 "yacc_sql.y"
                         {
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, (yyvsp[-1].string),Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
#line 2211 "yacc_sql.tab.c"
    break;

  case 98:
#line 561 "yacc_sql.y"
                  {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
#line 2221 "yacc_sql.tab.c"
    break;

  case 99:
#line 566 "yacc_sql.y"
                   {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[-1].string));
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
#line 2231 "yacc_sql.tab.c"
    break;

  case 100:
#line 571 "yacc_sql.y"
                              {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
#line 2241 "yacc_sql.tab.c"
    break;

  case 101:
#line 576 "yacc_sql.y"
                                   {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-3].string), "*");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
            	}
#line 2251 "yacc_sql.tab.c"
    break;

  case 103:
#line 585 "yacc_sql.y"
                         {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[-1].string));
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
     	  // CONTEXT->ssql->sstr.selection.attributes[CONTEXT->select_length].relation_name = NULL;
        // CONTEXT->ssql->sstr.selection.attributes[CONTEXT->select_length++].attribute_name=$2;
      }
#line 2263 "yacc_sql.tab.c"
    break;

  case 104:
#line 592 "yacc_sql.y"
                                {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string));
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
        // CONTEXT->ssql->sstr.selection.attributes[CONTEXT->select_length].attribute_name=$4;
        // CONTEXT->ssql->sstr.selection.attributes[CONTEXT->select_length++].relation_name=$2;
  	  }
#line 2275 "yacc_sql.tab.c"
    break;

  case 105:
#line 599 "yacc_sql.y"
                                  {
          			RelAttr attr;
          			relation_attr_init(&attr, (yyvsp[-3].string), "*");
          			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
                  // CONTEXT->ssql->sstr.selection.attributes[CONTEXT->select_length].attribute_name=$4;
                  // CONTEXT->ssql->sstr.selection.attributes[CONTEXT->select_length++].relation_name=$2;
            }
#line 2287 "yacc_sql.tab.c"
    break;

  case 106:
#line 606 "yacc_sql.y"
                                   {
            			RelAttr attr;
            			relation_attr_init(&attr, NULL, "*");
            			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
                 	  // CONTEXT->ssql->sstr.selection.attributes[CONTEXT->select_length].relation_name = NULL;
                    // CONTEXT->ssql->sstr.selection.attributes[CONTEXT->select_length++].attribute_name=$2;
                  }
#line 2299 "yacc_sql.tab.c"
    break;

  case 108:
#line 618 "yacc_sql.y"
                                     {	
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 2307 "yacc_sql.tab.c"
    break;

  case 110:
#line 624 "yacc_sql.y"
                                   {
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 2315 "yacc_sql.tab.c"
    break;

  case 111:
#line 630 "yacc_sql.y"
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
#line 2340 "yacc_sql.tab.c"
    break;

  case 112:
#line 651 "yacc_sql.y"
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
#line 2364 "yacc_sql.tab.c"
    break;

  case 113:
#line 671 "yacc_sql.y"
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
#line 2388 "yacc_sql.tab.c"
    break;

  case 114:
#line 691 "yacc_sql.y"
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
#line 2414 "yacc_sql.tab.c"
    break;

  case 115:
#line 713 "yacc_sql.y"
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
#line 2439 "yacc_sql.tab.c"
    break;

  case 116:
#line 734 "yacc_sql.y"
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
#line 2464 "yacc_sql.tab.c"
    break;

  case 117:
#line 755 "yacc_sql.y"
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
#line 2487 "yacc_sql.tab.c"
    break;

  case 118:
#line 776 "yacc_sql.y"
             { CONTEXT->comp = EQUAL_TO; }
#line 2493 "yacc_sql.tab.c"
    break;

  case 119:
#line 777 "yacc_sql.y"
         { CONTEXT->comp = LESS_THAN; }
#line 2499 "yacc_sql.tab.c"
    break;

  case 120:
#line 778 "yacc_sql.y"
         { CONTEXT->comp = GREAT_THAN; }
#line 2505 "yacc_sql.tab.c"
    break;

  case 121:
#line 779 "yacc_sql.y"
         { CONTEXT->comp = LESS_EQUAL; }
#line 2511 "yacc_sql.tab.c"
    break;

  case 122:
#line 780 "yacc_sql.y"
         { CONTEXT->comp = GREAT_EQUAL; }
#line 2517 "yacc_sql.tab.c"
    break;

  case 123:
#line 781 "yacc_sql.y"
         { CONTEXT->comp = NOT_EQUAL; }
#line 2523 "yacc_sql.tab.c"
    break;

  case 124:
#line 786 "yacc_sql.y"
                {
		  CONTEXT->ssql->flag = SCF_LOAD_DATA;
			load_data_init(&CONTEXT->ssql->sstr.load_data, (yyvsp[-1].string), (yyvsp[-4].string));
		}
#line 2532 "yacc_sql.tab.c"
    break;


#line 2536 "yacc_sql.tab.c"

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
#line 791 "yacc_sql.y"

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
