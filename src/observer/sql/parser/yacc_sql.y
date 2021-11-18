
%{

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

%}

%define api.pure full
%lex-param { yyscan_t scanner }
%parse-param { void *scanner }

//标识tokens
%token  SEMICOLON
        CREATE
        DROP
        TABLE
        TABLES
        INDEX
        SELECT
        DESC
        SHOW
        SYNC
        INSERT
        DELETE
        UPDATE
        LBRACE
        RBRACE
        COMMA
        TRX_BEGIN
        TRX_COMMIT
        TRX_ROLLBACK
        INT_T
        STRING_T
        FLOAT_T
        HELP
        EXIT
        DOT //QUOTE
        INTO
        VALUES
        FROM
        WHERE
        AND
        SET
        ON
        LOAD
        DATA
        DATE_T
        INFILE
        EQ
        LT
        GT
        LE
        GE
        NE
		IN
        MAX
        MIN
        COUNT
        AVG
        ORDER
        BY
        ASC
        INNER
        JOIN
        UNIQUE
	    GROUP
        NULLABLE
        NULL_K
        NOT
        IS

%union {
  struct _Attr *attr;
  struct _Condition *condition1;
  struct _Value *value1;
  char *string;
  int number;
  float floats;
	char *position;
}

%token <number> NUMBER
%token <floats> FLOAT 
%token <string> ID
%token <string> PATH
%token <string> SSS
%token <string> STAR
%token <string> STRING_V
//非终结符

%type <number> type;
%type <condition1> condition;
%type <value1> value;
%type <number> number;

%%

commands:		//commands or sqls. parser starts here.
    /* empty */
    | commands command SEMICOLON
    ;

command:
	  select  
	| insert
	| update
	| delete
	| create_table
	| drop_table
	| show_tables
	| desc_table
	| create_index	
	| drop_index
	| sync
	| begin
	| commit
	| rollback
	| load_data
	| help
	| exit
	| AGG
	| orders
	| order_list
	| relations
	| join_list
	| ATT
	| groups
	| group_list
    ;

exit:			
    EXIT {
        CONTEXT->ssql->flag=SCF_EXIT;//"exit";
    };

help:
    HELP {
        CONTEXT->ssql->flag=SCF_HELP;//"help";
    };

sync:
    SYNC {
      CONTEXT->ssql->flag = SCF_SYNC;
    }
    ;

begin:
    TRX_BEGIN {
      CONTEXT->ssql->flag = SCF_BEGIN;
    }
    ;

commit:
    TRX_COMMIT {
      CONTEXT->ssql->flag = SCF_COMMIT;
    }
    ;

rollback:
    TRX_ROLLBACK {
      CONTEXT->ssql->flag = SCF_ROLLBACK;
    }
    ;

drop_table:		/*drop table 语句的语法解析树*/
    DROP TABLE ID {
        CONTEXT->ssql->flag = SCF_DROP_TABLE;//"drop_table";
        drop_table_init(&CONTEXT->ssql->sstr.drop_table, $3);
    };

show_tables:
    SHOW TABLES {
      CONTEXT->ssql->flag = SCF_SHOW_TABLES;
    }
    ;

desc_table:
    DESC ID {
      CONTEXT->ssql->flag = SCF_DESC_TABLE;
      desc_table_init(&CONTEXT->ssql->sstr.desc_table, $2);
    }
    ;

create_index:		/*create index 语句的语法解析树*/
    CREATE INDEX ID ON ID LBRACE ID RBRACE
		{
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
			create_index_init(&CONTEXT->ssql->sstr.create_index, $3, $5, $7, false);
		}
		|
		CREATE UNIQUE INDEX ID ON ID LBRACE ID RBRACE
		{
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
                        create_index_init(&CONTEXT->ssql->sstr.create_index, $4, $6, $8, true);
		}
    ;

drop_index:			/*drop index 语句的语法解析树*/
    DROP INDEX ID
		{
			CONTEXT->ssql->flag=SCF_DROP_INDEX;//"drop_index";
			drop_index_init(&CONTEXT->ssql->sstr.drop_index, $3);
		}
    ;
create_table:		/*create table 语句的语法解析树*/
    CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE
		{
			CONTEXT->ssql->flag=SCF_CREATE_TABLE;//"create_table";
			// CONTEXT->ssql->sstr.create_table.attribute_count = CONTEXT->value_length;
			create_table_init_name(&CONTEXT->ssql->sstr.create_table, $3);
			//临时变量清零	
			CONTEXT->value_length = 0;
		}
    ;
attr_def_list:
    /* empty */
    | COMMA attr_def attr_def_list {    }
    ;
    
attr_def:
    ID_get type LBRACE number RBRACE 
		{
			AttrInfo attribute;
			attr_info_init(&attribute, CONTEXT->id, $2, $4, false);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name =(char*)malloc(sizeof(char));
			// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id); 
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type = $2;  
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length = $4;
			CONTEXT->value_length++;
		}
    |ID_get type
		{
			AttrInfo attribute;
			attr_info_init(&attribute, CONTEXT->id, $2, 4, false);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name=(char*)malloc(sizeof(char));
			// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id); 
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type=$2;  
			// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length=4; // default attribute length
			CONTEXT->value_length++;
		}
	|ID_get type NOT NULL_K
			{
				AttrInfo attribute;
				attr_info_init(&attribute, CONTEXT->id, $2, 4, false);
				create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name=(char*)malloc(sizeof(char));
				// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id);
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type=$2;
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length=4; // default attribute length
				CONTEXT->value_length++;
			}
	|ID_get type NULLABLE
			{
				AttrInfo attribute;
				attr_info_init(&attribute, CONTEXT->id, $2, 4, true);
				create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name=(char*)malloc(sizeof(char));
				// strcpy(CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].name, CONTEXT->id);
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].type=$2;
				// CONTEXT->ssql->sstr.create_table.attributes[CONTEXT->value_length].length=4; // default attribute length
				CONTEXT->value_length++;
			}
    ;
number:
		NUMBER {$$ = $1;}
		;
type:
	INT_T { $$=INTS; }
       | STRING_T { $$=CHARS; }
       | FLOAT_T { $$=FLOATS; }
       | DATE_T { $$=DATE; }
       ;
ID_get:
	ID 
	{
		char *temp=$1; 
		snprintf(CONTEXT->id, sizeof(CONTEXT->id), "%s", temp);
	}
	;

	
insert:				/*insert   语句的语法解析树*/
    INSERT INTO ID VALUES tuple { insert_tuple_add(CONTEXT); } tuple_list
		{
			// CONTEXT->values[CONTEXT->value_length++] = *$6;

			CONTEXT->ssql->flag=SCF_INSERT;//"insert";
			// CONTEXT->ssql->sstr.insertion.relation_name = $3;
			// CONTEXT->ssql->sstr.insertion.value_num = CONTEXT->value_length;
			// for(i = 0; i < CONTEXT->value_length; i++){
			// 	CONTEXT->ssql->sstr.insertion.values[i] = CONTEXT->values[i];
      // }
			inserts_init(&CONTEXT->ssql->sstr.insertion, $3, CONTEXT->insert_tuples, CONTEXT->insert_count);

      //临时变量清零
      CONTEXT->insert_count=0;
    }

tuple:
    LBRACE value value_list RBRACE;

tuple_list:
    | COMMA tuple {
		insert_tuple_add(CONTEXT);
	} tuple_list;

value_list:
    /* empty */
    | COMMA value value_list  {
  		// CONTEXT->values[CONTEXT->value_length++] = *$2;
	  }
    ;
value:
    NUMBER{	
  		value_init_integer(&CONTEXT->values[CONTEXT->value_length++], $1);
		}
    |FLOAT{
  		value_init_float(&CONTEXT->values[CONTEXT->value_length++], $1);
		}
    |SSS {
			$1 = substr($1,1,strlen($1)-2);
  		value_init_string(&CONTEXT->values[CONTEXT->value_length++], $1);
		}
		|NULL_K {
		value_init_null(&CONTEXT->values[CONTEXT->value_length++]);
		}
    ;
    
delete:		/*  delete 语句的语法解析树*/
    DELETE FROM ID where
		{
			CONTEXT->ssql->flag = SCF_DELETE;//"delete";
			deletes_init_relation(&CONTEXT->ssql->sstr.deletion, $3);
			deletes_set_conditions(&CONTEXT->ssql->sstr.deletion, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;	
    }
    ;
update:			/*  update 语句的语法解析树*/
    UPDATE ID SET ID EQ value where
		{
			CONTEXT->ssql->flag = SCF_UPDATE;//"update";
			Value *value = &CONTEXT->values[0];
			updates_init(&CONTEXT->ssql->sstr.update, $2, $4, value, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;
		}
    ;
select:				/*  select 语句的语法解析树*/
    SELECT select_attr FROM relations where orders groups
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
relations:
	ID join_list rel_list{
		selects_append_relation(&CONTEXT->ssql->sstr.selection, $1);
	};
join_list:
/* empty */
| INNER JOIN ID ON condition condition_list join_list{
		selects_append_relation(&CONTEXT->ssql->sstr.selection, $3);
}
rel_list:
    /* empty */
    | COMMA ID join_list rel_list {
		selects_append_relation(&CONTEXT->ssql->sstr.selection, $2);
	}
    ;
groups:
/* empty */
| GROUP BY ID group_list{
	RelAttr attr;
	relation_attr_init(&attr, NULL, $3);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
| GROUP BY ID DOT ID group_list{
	RelAttr attr;
	relation_attr_init(&attr, $3, $5);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
| GROUP BY ID ASC group_list{
	RelAttr attr;
	relation_attr_init(&attr, NULL, $3);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
| GROUP BY ID DOT ID ASC group_list{
	RelAttr attr;
	relation_attr_init(&attr, $3, $5);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
| GROUP BY ID DESC group_list{
	RelAttr attr;
	relation_attr_init(&attr, NULL, $3);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
| GROUP BY ID DOT ID DESC group_list{
	RelAttr attr;
	relation_attr_init(&attr, $3, $5);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
};
group_list:
 /* empty */
 | COMMA ID group_list{
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, $2);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
 | COMMA  ID DOT ID group_list{
 	RelAttr attr;
 	relation_attr_init(&attr, $2, $4);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
 | COMMA  ID ASC group_list{
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, $2);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
 | COMMA  ID DOT ID ASC group_list{
 	RelAttr attr;
 	relation_attr_init(&attr, $2, $4);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
 | COMMA  ID DESC group_list{
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, $2);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
 | COMMA  ID DOT ID DESC group_list{
 	RelAttr attr;
 	relation_attr_init(&attr, $2, $4);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 };
orders:
/* empty */
| ORDER BY ID order_list{
	RelAttr attr;
	relation_attr_init(&attr, NULL, $3);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
| ORDER BY ID DOT ID order_list{
	RelAttr attr;
	relation_attr_init(&attr, $3, $5);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
| ORDER BY ID ASC order_list{
	RelAttr attr;
	relation_attr_init(&attr, NULL, $3);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
| ORDER BY ID DOT ID ASC order_list{
	RelAttr attr;
	relation_attr_init(&attr, $3, $5);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
}
| ORDER BY ID DESC order_list{
	RelAttr attr;
	relation_attr_init(&attr, NULL, $3);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
}
| ORDER BY ID DOT ID DESC order_list{
	RelAttr attr;
	relation_attr_init(&attr, $3, $5);
        selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
};
order_list:
 /* empty */
 | COMMA ID order_list{
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, $2);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
 | COMMA  ID DOT ID order_list{
 	RelAttr attr;
 	relation_attr_init(&attr, $2, $4);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
 | COMMA  ID ASC order_list{
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, $2);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
 | COMMA  ID DOT ID ASC order_list{
 	RelAttr attr;
 	relation_attr_init(&attr, $2, $4);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,1);
 }
 | COMMA  ID DESC order_list{
 	RelAttr attr;
 	relation_attr_init(&attr, NULL, $2);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 }
 | COMMA  ID DOT ID DESC order_list{
 	RelAttr attr;
 	relation_attr_init(&attr, $2, $4);
         selects_append_orders(&CONTEXT->ssql->sstr.selection, &attr,0);
 };
ATT:
	STAR{
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
    	| ID {
			RelAttr attr;
			relation_attr_init(&attr, NULL, $1);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
  	| ID DOT ID {
			RelAttr attr;
			relation_attr_init(&attr, $1, $3);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
        | ID DOT STAR {
			RelAttr attr;
			relation_attr_init(&attr, $1, "*");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
            	}

    ;
AGG:
    MAX LBRACE ID DOT ID RBRACE{
	RelAttr attr;
	relation_agg_attr_init(&attr, $3, $5,Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
    |MAX LBRACE ID RBRACE{
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, $3,Max);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
    |MIN LBRACE ID DOT ID RBRACE{
	RelAttr attr;
	relation_agg_attr_init(&attr, $3, $5,Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
    |MIN LBRACE ID RBRACE{
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, $3,Min);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
    |COUNT LBRACE ID RBRACE{
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, $3,Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
    |COUNT LBRACE ID DOT ID RBRACE{
	RelAttr attr;
	relation_agg_attr_init(&attr, $3, $5,Count);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
         |COUNT LBRACE STAR RBRACE{
        	RelAttr attr;
        	relation_agg_attr_init(&attr, NULL, "*",Count);
                selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
            }
            |COUNT LBRACE NUMBER RBRACE{
                    	RelAttr attr;
                    	relation_agg_attr_init(&attr, NULL, "*",Count);
                            selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
                        }
    |AVG LBRACE ID DOT ID RBRACE{
	RelAttr attr;
	relation_agg_attr_init(&attr, $3, $5,Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
    |AVG LBRACE ID RBRACE{
	RelAttr attr;
	relation_agg_attr_init(&attr, NULL, $3,Avg);
        selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
    }
select_attr:
	AGG attr_list
	|ATT attr_list

    ;
attr_list:
    /* empty */
    | COMMA AGG attr_list
    | COMMA ATT attr_list

where:
    /* empty */ 
    | WHERE condition condition_list {	
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
    ;
condition_list:
    /* empty */
    | AND condition condition_list {
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
    ;
condition:
    ID comOp value 
		{
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $1);

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
		|value comOp value 
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
	|ID comOp ID
		{
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $1);
			RelAttr right_attr;
			relation_attr_init(&right_attr, NULL, $3);

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
    |value comOp ID
		{
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];
			RelAttr right_attr;
			relation_attr_init(&right_attr, NULL, $3);

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
    |ID DOT ID comOp value
		{
			RelAttr left_attr;
			relation_attr_init(&left_attr, $1, $3);
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
    |value comOp ID DOT ID
		{
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];

			RelAttr right_attr;
			relation_attr_init(&right_attr, $3, $5);

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
    |ID DOT ID comOp ID DOT ID
		{
			RelAttr left_attr;
			relation_attr_init(&left_attr, $1, $3);
			RelAttr right_attr;
			relation_attr_init(&right_attr, $5, $7);

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
	| ID IN { CONTEXT->comp = CONTAINED_BY; } LBRACE subQuery RBRACE // x IN (SELECT y FROM b)
		{
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $1);

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
	| ID DOT ID IN { CONTEXT->comp = CONTAINED_BY; } LBRACE subQuery RBRACE // t.x IN (SELECT y FROM b)
		{
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, $1, $3);

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
	| ID NOT IN { CONTEXT->comp = NOT_CONTAINED_BY; } LBRACE subQuery RBRACE // x NOT IN (SELECT y FROM b)
		{
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $1);

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
	| ID DOT ID NOT IN { CONTEXT->comp = NOT_CONTAINED_BY; } LBRACE subQuery RBRACE // t.x NOT IN (SELECT y FROM b)
		{
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, $1, $3);

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
	| ID comOp LBRACE subQuery RBRACE // x > (SELECT y FROM b)
		{
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $1);

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
	| ID DOT ID comOp LBRACE subQuery RBRACE // t.x > (SELECT y FROM b)
		{
			// 将子查询视作一个抽象的、惰性求值的值
			// 把子SQL语句包装成一个Unevaluated对象，数据库引擎按需求值
			RelAttr left_attr;
			relation_attr_init(&left_attr, $1, $3);

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
    ;

subQuery:
        // 为了减小代码变动范围，复用已有的SELECT解析器代码，做一个hack：
		// 1. 保存现有SELECT指令解析过程的上下文，到一个堆上新分配的临时区域
		// 2. 调用原有SELECT解析过程，覆盖当然SELECT指令（父指令）的上下文存储区域，解析子SELECT语句
		// 3. 子语句解析完毕后，将子语句的解析结果（即上下文）复制到堆上的另一个新区域，还原父SELECT语句的上下文
		// 4. 销毁两个临时空间
		//
		// 我们在此假设，SELECT指令的上下文包括：
		// 1. Selects结构体
		// 2. SQL WHERE条件（condition、condition_length）
		// 3. 父SELECT指令当前正在解析的条件语句的运算符
		// 我们只保存这些内容，其他部分保持不变。
		// 因此，**一旦未来SELECT指令的上下文扩展到上述内容之外，需要立刻对此段代码进行修改，以保证SELECT指令上下文的完整性**

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
		select
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
	;

comOp:
  	  EQ { CONTEXT->comp = EQUAL_TO; }
    | LT { CONTEXT->comp = LESS_THAN; }
    | GT { CONTEXT->comp = GREAT_THAN; }
    | LE { CONTEXT->comp = LESS_EQUAL; }
    | GE { CONTEXT->comp = GREAT_EQUAL; }
    | NE { CONTEXT->comp = NOT_EQUAL; }
    | IS { CONTEXT->comp = IS_CompOP; }
    | IS NOT {CONTEXT->comp = IS_NOT_CompOP;}
    ;

load_data:
		LOAD DATA INFILE SSS INTO TABLE ID
		{
		  CONTEXT->ssql->flag = SCF_LOAD_DATA;
			load_data_init(&CONTEXT->ssql->sstr.load_data, $7, $4);
		}
		;
%%
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
