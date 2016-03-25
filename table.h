/** 
 * @file table.h
 * @brief 动态数据结构table的使用
 * @author Gliu
 * @date 09/10/2012
 * 主要使用table进行定义、创建、使用等操作。
 *
 */

#ifndef TABLE_H_V1_0
#define TABLE_H_V1_0

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define bool int

#ifndef TABLE_API
	#define TABLE_API 
#endif

/** \mainpage
 *
 * C语言中，结构是对一些基本数据类型的组合，通过 struct 关键字，定义了组合的
 * 逻辑结构，在编译时，编译器产生定位字段的偏移，以此，使得我们可以编写访问
 * 某一字段的代码。在另一些情况下，对结构的创建访问，需要在运行时才能决定，
 * 这时，需要一种动态的结构。
 *
 *
 * table是这样的一种动态结构，与C结构类似，使用table分为定义、创建、使用等步骤。
 * \section sec_create 创建
 * 以一个日K线的数据类型为例：
 * \code
 * struct Day{
 *     long long   date;
 *     float open;
 *     float high;
 *     float low;
 *     float close;
 *     char  extend[64];	// 扩展内容
 * };
 * \endcode
 * 通过类型定义字符串
 * \code
 * const char * day_schema = "date:q;open:f;high:f;low:f;close:f;extend:64c";
 * \endcode
 * 可以定义一个相同的日线数据结构。
 *
 * 在C语言中，通过 new/delete 关键字或 malloc/free 来创建结构
 * \code
 * Day * days = new Day[100];
 * ...
 * delete [] days;
 * \endcode
 * 创建table则通过 table_create/table_destory 函数
 * \code
 * table_t days = table_create(day_schema, 100, "days");
 * ...
 * table_destory(days);
 * \endcode
 * \section sec_schema 定义语言
 * 定义语言的描述如下:
 * \code
 * table_schema    = name:[count]type[{;name:[count]type}]
 * name            = 字符串
 * count           = 整数
 * type            = c|h|i|q|f|d|s|t|C|H|I|Q|b|T|p
 * 其中在主键列前可选设排序标志，>代表降序， <代表升序。
 * \endcode
 * \section sec_example 一个完整的例子
 * \code
 * #include "table.h"
 * int main(){
 *     const char * day_schema = "date:q;open:f;high:f;low:f;close:f;extend:64c";
 *     table_t days = table_create(day_schema, 100, "");
 *     size_t rows = table_rows(days);
 *     size_t date_col = table_col_index("date");
 *
 *     // set date with recent 100 days
 *     time_t begin = 1337576660;
 *     for(size_t row = 0; row < rows; row++){
 *         table_set_int64(row, date_col, begin - 86400 * row);
 *         ...
 *     }
 *
 *     // print days
 *     for(size_t row = 0; row < rows; row++){
 *         printf("%I64d\n", table_get_int64(row, date_col);
 *         ...
 *     }
 *     table_destory(days);
 * }
 * \endcode
 * \section sec_thread 线程安全
 * table \em 不是 线程安全的，这一点类似于一个通常的指针，如果希望在线程间共享，
 * 那么，调用者需要来进行锁的操作。
 * \section sec_memory 内存管理和字符串
 * table实现中，所有内部内存均通过 table_strnew 来申请，通过 table_strdel 来释放，所以，
 * 当需要将一个字符串传递到 table 内部时，也同样需要 table_strnew 来进行分配内存，例如，\ref table_set_str_raw
 */



 /**
  * @file table.h
  * @brief table 的定义头文件
  *
  */

/**
 * @brief table_t 表示一个 table 数据类型
 *
 * 通过 table_* 系列API来对table进行创建、销毁、访问等操作。
 */
typedef void * table_t;

 /**
  * @brief table 目前支持的字段数据类型
  */
enum table_filed_type {
	/**
	 * @brief 空，无意义，占位符
	 */
	tft_null,
	/**
	 * @brief 1 字节整数，类型为 c
	 */
	tft_char,
	/**
	 * @brief 2 字节整数，类型为 h
	 */
	tft_short,
	/**
	 * @brief 4 字节整数，类型为 i    
	 */
	tft_int,
	/**
	 * @brief 8 字节整数，类型为 q
	 */
	tft_int64,
	/**
	 * @brief 4 字节浮点，类型为 f
	 */
	tft_float,
	/**
	 * @brief 8 字节浮点，类型为 d
	 */
	tft_double,
	/**
	 * @brief 8 字节指向字符串的指针，类型为 s 
	 */
	tft_str,
	/**
	 * @brief 8 字节指向table_t的指针，类型为 t 
	 */
	tft_table,
	/**
	 * @brief 1 字节整数，类型为 C
	 */
	tft_uchar,
	/**
	 * @brief 2 字节整数，类型为 H
	 */
	tft_ushort,
	/**
	 * @brief 4 字节整数，类型为 I    
	 */
	tft_uint,
	/**
	 * @brief 8 字节整数，类型为 Q
	 */
	tft_uint64,
	/**
	 * @brief 8 字节指向变长二进制的指针，类型为 b
	 */
	tft_binpt,
	/**
	 * @brief 8 字节时间类型数，类型为 T
	 */
	tft_timet,
	/**
	 * @brief 8 字节定点浮点数，类型为 p
	 */
	tft_pfloat,
	/**
	 * @brief 8 字节定点浮点数，类型为 v
	 */
	tft_vfloat
};

enum key_col_order {
	ASC,
	DESC
};
typedef enum key_col_order KEYORDER;

/**
 * @name 创建、删除、序列化相关
 * @{ */

/**
 * @brief table_create 用于创建一个 table
 *
 * @param schema table的格式定义字符串，参考 \ref sec_schema
 * @param rows 行数，创建后，通过 \ref table_rows 返回行数
 * @param name table的名称 \ref table_name
 *
 * @return 返回创建成功的 table，通过 \ref table_destory 销毁，若为 NULL 则表示创建失败
 */
TABLE_API table_t table_create(const char * schema, size_t rows, const char * name);
/**
 * @brief table_clone 对给定的 table 进行一次深拷贝(Deep Copy)
 *
 * 一般来说，table 可以通过 table_assign 来增加引用计数，使其可以安全的传递到其他的模块中。
 * 但由于 table 并不是线程安全的，通常，在线程间共享 table 时，应该对 table 进行一次 clone。
 *
 * clone 后的 table 与源table具有完全一致的结构，但其引用计数会置为 1
 *
 * @param table 给定的 table
 *
 * @return 新生成的 table
 */
TABLE_API table_t table_clone(table_t table);
/**
 * @brief table_assign 为给定的table增加引用计数
 *
 * 考虑如下的场景，在同一线程中
 * \code
 *
 *
 * void func(){
 *    table_t t = table_create(...);
 *    SendMessage(hWnd, WM_DATACOMING 0, table_assign(t));
 *    SendMessage(hWnd, WM_DATACOMING 0, table_clone(t));
 *    table_destory(t);
 * }
 *
 * void OnDataComing(table_t t){
 *    // ...
 *    table_destory(t);
 * }
 * \endcode
 *
 * 此时，使用 table_assign 将比 table_clone 更加有效率
 *
 * @param table 给定的table
 *
 *
 * @return 经过增加引用计数后的 table
 */
TABLE_API table_t table_assign(table_t table);
/**
 * @brief table_destory 用于销毁一个 table
 *
 *
 * @param table 由返回 table_t 的相应API的返回值
 *
 * @sa table_create table_clone table_assign table_frombuff
 */
TABLE_API void table_destory(table_t table);
/**
 * @brief table_tobuff 将一个给定的 table 序列化到一块缓存中
 *
 * 缓存由 buff 指出，其大小应该至少为 \ref table_size, 序列化会恰好
 * 使用 table_size 大小的缓存
 *
 * @param table 给定的 table
 * @param buff 给定的缓存
 * 序列化后的内存结构：
 * schema string
 * name string
 * rows int32
 * col1 rows * cell
 * col2 rows * cell
 * ...
 * colN rows * cell
 * attribute
 * nullflag rows * cols * 1byte

 * cell = char|short|int32|...|double|string|binary|table
 * string = length*1byte '\0'
 * binary = length length*1byte
 * table = length table_size()*1byte
 *
 * attribute 排列顺序为 table_attrib col1_atrrib ... colN_attrib.
 * 一个属性结构是：str_num(int32),str_key1+'\0',str_val1+'\0'....int_num(int32),int_key1,int_val1....
 */
TABLE_API void table_tobuff(table_t table, char * buff);
/**
 * @brief table_frombuff 从一块缓存中反序列化出一个 table
 *
 * @param buff 给定的 buff，通常，这块buff的内容由 table_tobuff 来填充
 * @param len buff 的大小
 *
 * @return 反序列化出的 table
 */
TABLE_API table_t table_frombuff(const char * buff, size_t len);
/**
 * @brief table_tobuff_extra 将一个给定的 table 序列化到一块缓存中
 *
 * 缓存由 buff 指出，其大小应该至少为 \ref table_size_extra, 序列化会恰好
 * 使用 table_size_extra 大小的缓存
 *
 * @param table 给定的 table
 * @param buff 给定的缓存
 * @param extra 给定的加密串
 * 序列化后的内存结构
 * version int32
 * schema string
 * name string
 * extra string
 * rows int32
 * col1 rows * cell
 * col2 rows * cell
 * ...
 * colN rows * cell
 * nullflag rows * cols * 1byte

 * cell = char|short|int32|...|double|string|binary|table
 * string = length*1byte '\0'
 * binary = length length*1byte
 * table = length table_size_extra()*1byte 
 */
TABLE_API void table_tobuff_extra(table_t table, char * buff, const char * extra);
/**
 * @brief table_frombuff_extra 从一块缓存中反序列化出一个 table
 *
 * @param buff 给定的 buff，通常，这块buff的内容由 table_tobuff 来填充
 * @param len buff 的大小
 *
 * @return 反序列化出的 table
 */
TABLE_API table_t table_frombuff_extra(const char * buff, size_t len);

/**
 * @brief table_row_tobuff 将一个给定的 table的一行序列化到一块缓存中
 *
 * 缓存由 buff 指出，其大小应该至少为 \ref table_row_size, 序列化会恰好
 * 使用 table_row_size 大小的缓存
 *
 * @param table 给定的 table
 * @param buff 给定的缓存
 *
 */
TABLE_API void table_row_tobuff(table_t table, size_t row, char * buff);
/**
 * @brief table_row_frombuff 从一块缓存中反序列化出一个 table
 *
 * @param buff 给定的 buff，通常，这块buff的内容由 table_row_tobuff 来填充
 * @param len buff 的大小
 *
 * @return 反序列化出的 table
 */
TABLE_API void table_row_frombuff(table_t table, size_t row, const char * buff, size_t len);
/**
 * @brief table_move_colsname 按照给定的列名字替换成新名字
 *
 * 参考 \ref table_sort
 *
 * @param table 给定的 table
 * @param srcColsName 列名字
 * @param dstColsName 列新名字
 *
 * @return 匹配到的行号列表
 */
TABLE_API table_t table_move_colsname(table_t table, const char * srcColsName, const char * dstColsName);
/**
 * @brief table_clean_schema_pool 清空schema缓存池
 *
 */
TABLE_API void table_clean_schema_pool(void);

/**  @} */

/**
 * @name 增加和整理
 * @{ */

/**
 * @brief table_append 在给定的 table 中增加一行
 *
 * 在 table 中增加将使 table 内部的内存不连续，这样，一些将 table 对应到 C 结构的操作将
 * 失效，可以调用 table_tidy 来进行整理使得内存连续，但频繁的整理将带来性能问题。
 *
 * @param table 给定的 table
 *
 * @return 增加后得到的行号
 */
TABLE_API size_t table_append(table_t table);


/**
 * @brief table_tidy 整理内部的内存，使其成为连续的一块
 *
 * table 在创建之后，其内部的内存是连续的，所以可以通过 table_get_buff(t, 0, 0) 来将其
 * 当成一个C结构使用，但当对 table_append 之后，这种连续将被打断，若仍要进行类似的操作，
 * 可以通过调用 table_tidy 来进行整理。
 *
 * @param table 给定的 table
 *
 */
TABLE_API void table_tidy(table_t table);


/**
 * @brief table_is_tidy 查询内部的内存是否为连续的一块
 *
 *
 * @param table 给定的 table
 *
 * @return 是否连续
 */
TABLE_API bool table_is_tidy(table_t table);


/**  @} */

/**
 * @name 属性
 * @{ */

/**
 * @brief table_rows 返回 table 的行数
 *
 * @param table 给定的table
 *
 * @return 行数
 */
TABLE_API size_t table_rows(table_t table);
/**
 * @brief table_cols 返回 table 的列数
 *
 * @param table 给定的 table
 *
 * @return 列数
 */
TABLE_API size_t table_cols(table_t table);
/**
 * @brief table_size 返回table序列化后的大小
 *
 * @param table 给定的table
 *
 * @return 序列化后的大小（）
 */
TABLE_API size_t table_size(table_t table);
/**
 * @brief table_size_extra 返回table序列化后的大小
 *
 * @param table 给定的table
 *
 * @return 序列化后的大小（不含序列化加密配信息,为域保留结束符）
 */
TABLE_API size_t table_size_extra(table_t table);
/**
 * @brief table_pod_size 返回 table POD 部分的大小
 *
 * POD是指那些编译时可预知的类型，如整数、浮点数，字符串和table则不属于POD
 * 如果一个 table 只有 POD 类型，那么，它的序列化和 clone 将很有效率，同时
 * 它也可以通过定义一个C结构，来进行快速的访问。
 *
 * 语义同 sizeof(T) * rows
 *
 * @param table 给定的table
 *
 * @return POD部分的大小
 */
TABLE_API size_t table_pod_size(table_t table);
/**
 * @brief table_width 返回 table 的宽
 *
 * 语义同 sizeof(T)
 *
 * @param table 给定的table
 *
 * @return sizeof 的结果
 */
TABLE_API size_t table_width(table_t table);
/**
 * @brief table_schema 返回 table 的定义字符串
 *
 * @param table 给定的table
 *
 * @return 定义字符串
 */
TABLE_API const char * table_schema(table_t table);
/**
 * @brief table_name 返回 table 的名称
 *
 * 在创建table时，可以对其命名 \ref table_create
 *
 * @param table 给定的名称
 *
 * @return table 的名称
 */
TABLE_API const char * table_name(table_t table);
/**
 * @brief table_is_pod 测试给定的table是否为 POD 类型
 *
 * 不包含字符串(s)和table(t)字段的table，为 POD类型
 *
 * @param table 给定的 table
 *
 * @return true 表示为POD类型
 */
TABLE_API bool table_is_pod(table_t table);

/**
 * @brief table_col_index 根据名称返回列的序号
 *
 * 不应该在一个循环中反复的调用此函数，而应该在循环外缓存此函数的返回值，
 * 然后进行其他的操作
 *
 * @param table 给定的table
 * @param name 列名称
 *
 * @return -1 表示该列不存在
 */
TABLE_API size_t table_col_index(table_t table, const char * name);
/**
 * @brief table_col_offset 返回某一列距开始的偏移
 *
 * 以结构 Day 为例，open 字段的偏移为 8，low 字段的偏移为 16
 *
 * @param table 给定的table
 * @param col 列ID
 *
 * @return 
 */
TABLE_API size_t table_col_offset(table_t table, size_t col);
/**
 * @brief table_col_type 返回某一列的类型
 *
 * @param table 给定的table
 * @param col 列ID
 *
 * @return 参看 \ref table_filed_type
 */
TABLE_API int table_col_type(table_t table, size_t col) ;
/**
 * @brief table_col_size 返回某一列的大小
 *
 * 以结构 Day 为例，date 字段的大小为 8，low 字段的大小为 4
 *
 * @param table 给定的 table
 * @param col 列ID
 *
 * @return 列大小
 */
TABLE_API size_t table_col_size(table_t table, size_t col);
/**
 * @brief table_col_name 返回给定列的名称
 *
 * @param table 给定的 table
 * @param col 列ID
 *
 * @return 列名称  参看 \ref table_col_index
 */
TABLE_API const char * table_col_name(table_t table, size_t col);
/**
 * @brief table_pkcol 返回主键列
 *
 * @param table 给定的 table
 * 
 * @return 主键列ID,
 */
TABLE_API const size_t table_pkcol(table_t table);
/**
 * @brief table_pkorder 返回主键排序方式
 *
 * @param table 给定的 table
 * 
 * @return 主键排序方式
 */
TABLE_API KEYORDER table_pkorder(table_t table);
/**
 * @brief table_row_size 返回一行的大小
 * 
 * @param table 给定的 table
 * @param row 行号
 *
 * @return 行大小
 */
TABLE_API size_t table_row_size(table_t table, size_t row);
/**
 * @brief table_col_isnull 返回字段值是否为NULL
 * 
 * @param table 给定的 table
 * @param row 行号
 * @param col 列ID
 *
 * @return true or false
 */
TABLE_API bool table_col_isnull(table_t table, size_t row, size_t col);

/**  @} */

/**
 * @name 取值相关
 * @{ */

TABLE_API int table_get_int(table_t table, size_t row, size_t col);
TABLE_API long long table_get_int64(table_t table, size_t row, size_t col);
TABLE_API unsigned int table_get_uint(table_t table, size_t row, size_t col);
TABLE_API unsigned long long table_get_uint64(table_t table, size_t row, size_t col);
TABLE_API float table_get_float(table_t table, size_t row, size_t col);
TABLE_API double table_get_double(table_t table, size_t row, size_t col);
TABLE_API const char * table_get_str(table_t table, size_t row, size_t col);
TABLE_API const table_t table_get_table(table_t table, size_t row, size_t col);
TABLE_API const char * table_get_binary(table_t table, size_t row, size_t col);
TABLE_API const size_t table_get_binary_len(table_t table, size_t row, size_t col);
TABLE_API const size_t table_get_time_t(table_t table, size_t row, size_t col);
TABLE_API const long long table_get_time(table_t table, size_t row, size_t col);
TABLE_API const long long table_get_pfloat(table_t table, size_t row, size_t col);
TABLE_API const long long table_get_vfloat(table_t table, size_t row, size_t col);
TABLE_API short int table_get_short(table_t table, size_t row, size_t col);
/**
 * @brief table_get_buff 返回某一列的指针
 *
 * 当列的格式不为1时，这通常表示该列为一个已知大小但自定义的数据，如一块固定大小的缓存等。
 * 此时可以，通过本函数来获取到在table中内存的指针，指针指向的内存的安全大小是 table_col_size 。
 *
 * 有时候，需要将 table 转换为C结构之间访问，也可以调用此函数
 * \code
 * const Day * day = (const Day *)table_get_buff(t, 0, 0);
 * for(int row = 0; row < table_rows(t); row++){
 *     printf("%lld\n", day[row].date);
 * }
 * \endcode
 *
 * @param table 给定的 table
 * @param row 行
 * @param col 列
 *
 * @return 数据的内存指针
 */
TABLE_API const void * table_get_buff(table_t table, size_t row, size_t col);

/**  @} */

/**
 * @name 设置相关
 * @{ */

TABLE_API void table_set_int(table_t table, size_t row, size_t col, int val);
TABLE_API void table_set_int64(table_t table, size_t row, size_t col, long long val);
TABLE_API void table_set_uint(table_t table, size_t row, size_t col, unsigned int val);
TABLE_API void table_set_uint64(table_t table, size_t row, size_t col, unsigned long long val);
TABLE_API void table_set_float(table_t table, size_t row, size_t col, float val);
TABLE_API void table_set_double(table_t table, size_t row, size_t col, double val);
TABLE_API void table_set_pfloat(table_t table, size_t row, size_t col, long long val);
TABLE_API void table_set_vfloat(table_t table, size_t row, size_t col, long long val);
TABLE_API void table_set_short(table_t table, size_t row, size_t col, short int val);

/**
 * @brief table_set_str_raw 设置字符串，并将其内存管理交于 table
 *
 * val 必须是一个使用 table_strnew 或者 table_strdup 产生的字符串，
 * 并且也不应该再通过 table_strdel 释放，使用此接口，
 * 在某些情况下可以减少一次字符串复制。table 内部原先的字符串将会被释放。
 *
 * @param table 给定的 table
 * @param row 行号
 * @param col 列号
 * @param val 字符串
 *
 * @return 
 */
TABLE_API void table_set_str_raw(table_t table, size_t row, size_t col, char * val);
TABLE_API void table_set_table_raw(table_t table, size_t row, size_t col, table_t val);
//TABLE_API void table_set_binary_raw(table_t table, size_t row, size_t col, size_t len,const char * val);
/**
 * @brief table_set_str 设置字符串
 *
 * 给定的字符串将会在内部通过 table_strdup 创建一份拷贝，然后通过
 * table_set_str_raw 进行设置
 *
 * @param table 给定的 table
 * @param row 行号
 * @param col 列号
 * @param val 字符串
 * @param delold 是否删除给定位置原有的字符串，一般来说，应该传 ture
 *
 */
TABLE_API void table_set_str(table_t table, size_t row, size_t col, const char * val, bool delold);
TABLE_API void table_set_table(table_t table, size_t row, size_t col, const table_t val, bool delod);
TABLE_API void table_set_binary(table_t table, size_t row, size_t col, size_t len,const char * val, bool delod);
/**
 * @brief table_set_buff 为一个类型为 buffer 的列设置内容
 *
 * 将从 (const char*)buff + offset 复制 len 字节到 table 内部
 *
 * @param table
 * @param table 给定的 table
 * @param row 行号
 * @param col 列号
 * @param buff 源内容
 * @param len 将要复制的长度
 * @param offset 将从源的 offest 开始复制
 *
 * @return 
 */
TABLE_API void table_set_buff(table_t table, size_t row, size_t col, const void * buff, size_t len,size_t offset);

/**
 * @brief table_set_time 为一个类型为 time 的列设置内容
 *
 * @param table
 * @param table 给定的 table
 * @param row 行号
 * @param col 列号
 * @param sec time_t 时间
 * @param msec 毫秒数
 *
 * @return 
 */
TABLE_API void table_set_time(table_t table, size_t row, size_t col, size_t sec, int msec);

/**
 * @brief table_col_writenull 写入NULL值
 *
 * @param table 给定的 table
 * @param row 行号
 * @param col 列号
 *
 */
TABLE_API void table_col_writenull(table_t table, size_t row, size_t col);

/**  @} */

/**
 * @name 高级函数
 * @{ */
/**
 * @brief table_fill 将一个 table 的指定行复制到另一个 table 的指定行中
 *
 * 在复制时，只有列名匹配的字段会进行复制，若有字符串或table类型的字段，
 * 也会进行相应的 clone。
 *
 * 例如，当 table 是一个行情列表的全集，当其中只有某些商品的某些数据推送到时，
 * 可以通过此方法来更新全集。
 *
 * @param table 目标table
 * @param drow  目标行
 * @param other 源table
 * @param srow  源行
 *
 */
TABLE_API void table_fill(table_t table, size_t drow, const table_t other, size_t srow);
/**
 * @brief table_update 将一个源 table 更新到一个目标table中
 *
 * 源table与目标table应该至少具有一个相同名称的列，这个列将会作为key，在更新时，
 * 对匹配key的源行和目标行做 table_fill
 *
 * @param table 目标table
 * @param other 源table
 * @param keycol 主键列
 *
 */
TABLE_API void table_update(table_t table, const table_t other, const char * keycol);
/**
 * @brief table_select 从一个table中获取一个子集
 *
 * 新产生的table，列为 fields 里与母table匹配到的列，行数为 min(count, 母行数 - from)
 *
 * @param table 母table
 * @param fields 字段的列表，使用,分割的字符串
 * @param from 母table的起始行号
 * @param count 最多复制的个数
 *
 * @return 子集的顺序将可能的按照 fields 里的顺序，但若给定的字段不存在或重复，
 * 则会被舍弃
 */
TABLE_API table_t table_select(table_t table, const char * fields, size_t from, size_t count);
/**
 * @brief table_join 按照给定列将两个 table 做列合并
 *
 * 合并后的table的列为两个源table列的并集去除重复，如
 * \code
 * table_join("f1;f2;f3", "f1;f3;f4") = "f1;f2;f3;f4"
 * \endcode
 * 行数为 源1行数, 与 left join 法保持一致
 *
 * @param table 源1
 * @param other 源2
 * @param keycol 索引列
 *
 * @return 
 */
TABLE_API table_t table_join(table_t table, table_t other, const char * keycol);

enum{
	TABLE_STR_COMPARER = 0,
	TABLE_INT_COMPARER,
	TABLE_INT64_COMPARER,
	TABLE_FLOAT_COMPARER,
	TABLE_DOUBLE_COMPARER,
	TABLE_MAX_COMPARER
};

typedef int (*field_compare_func)(void * table, size_t r1, size_t r2, void * userdata);
typedef int (*value_compare_func)(void * table, size_t row, size_t col, void * userdata);
typedef TABLE_API char * (*table_malloc_func)(size_t bytes);
typedef TABLE_API void (*table_free_func)(char * ptr);

/**
 * @brief table_sort 按照给定的函数对table进行排序
 *
 * table_sort 总是返回一个一列的 table，其定义为 "sn:i"，行数与待排序的table
 * 相同，每一行中的内容为排序后的行号。
 * 例如
 * \code
 * table_t table = table_create("f1:i;f2:i", 3, "");
 * table_set_int(table, 0, 0, 2);
 * table_set_int(table, 0, 0, 1);
 * table_set_int(table, 0, 0, 3);
 *
 * // 以第一列按照整数排序规则排序
 * table_t sorted = table_sort(table, INT_FIELD_COMPARER, 0);
 *
 * for(size_t i = 0; i < table_rows(sorted); i++)
 *     printf("%d ", table_get_int(sorted, i, 0));
 * table_destory(sorted);
 * table_destory(table);
 *
 * // 将输出
 * // 1 0 2
 * \endcode
 *
 * 预定义的排序函数可以用于简单规则的排序
 *
 * @param table 给定的 table
 * @param comparer 比较函数
 * @param userdata 在比较时传递的参数
 *
 * @return 排序后的行号列表
 */
TABLE_API table_t table_sort(table_t table, field_compare_func comparer, void * userdata);

/**
 * @brief table_filter 按照给定的匹配函数挑选符合条件的行
 *
 * 参考 \ref table_sort
 *
 * @param table 给定的 table
 * @param comparer 匹配函数
 * @param userdata 匹配的自定义参数
 *
 * @return 匹配到的行号列表
 */
TABLE_API table_t table_filter(table_t table, field_compare_func comparer, void * userdata);

/**
 * @brief table_group 将给定的table按照某一列做分组
 
 * table_group 将会产生一个分组结果table，其定义为
 * \code
 * // 若源 table 定义为
 * const char * table_schema = "f1:s;f2:d;f3:i"
 * // 则以 f1 列 group 后产生的table的定义为
 * const char * group_schema = "f1:s;f2:5d;f2:5d;_ROWS_:t";
 * // 其中，_ROWS_ 的定义为
 * const char * group_rows_schema = "sn:i";
 * // 表示当前分组的行号的列表
 *
 * // 每一个数值类型的字段，会进行基本的统计，产生 (count, sum, avg, max, min) 的元组
 *
 * table_t table = table_create(table_schema, 5, "");
 * // 以如下的表格设置 table 的内容
 * // a 1 4
 * // a 2 3
 * // b 3 2
 * // b 4 1
 * // c 1 1
 * table_t grouped = table_group(table, "f1");
 * // 则 grouped 的内容为
 * // f1        f2              f3           _ROWS_
 * // a (2, 3, 1.5, 2, 1) (2, 7, 3.5, 4, 3) [0, 1]
 * // b (2, 7, 3.5, 4, 3) (2, 1, 1.5, 2, 1) [2, 3]
 * // c (1, 1, 1  , 1, 1) (1, 1, 1  , 1, 1) [5]
 * 
 * \endcode
 * 对于字符串 table 类型的列，统计值将置为 0
 *
 * @param table
 * @param keycol
 *
 * @return 
 */
TABLE_API table_t table_group(table_t table, const char * keycol);

/**
 * @brief table_delete 删除table中从第row行开始的num行（包含第row行）
 *
 * @param table 给定的 table
 * @param row   被删除的起始行
 * @param num   删除的总行数
 *
 * @return 匹配到的行号列表
 */
TABLE_API void table_delete(table_t table, size_t row, size_t num);

/**
 * @brief table_insert 在第row行之前插入num行
 *
 * @param table 给定的 table
 * @param row   插入位置，原table该行之前
 * @param num   插入的总行数
 *
 * @return 匹配到的行号列表
 */
TABLE_API void table_insert(table_t table, size_t row, size_t num);


/**  @} */



/**
 * @name 字符串和内存
 * @{ */
TABLE_API void table_set_alloctor(table_malloc_func malloc_func, table_free_func free_func);
TABLE_API char * table_strnew(size_t bytes);
TABLE_API void table_strdel(char * s);
TABLE_API char * table_strdup(const char * s);

/**  @} */

/**
 * @name 设置和读取自定义属性
 * @{ */
TABLE_API bool table_getkv_string(table_t table, const char* key, char* val, size_t val_size);
TABLE_API bool table_setkv_string(table_t table, const char* key, const char* val);
TABLE_API bool table_col_getkv_string(table_t table, size_t col, const char* key, char* val, size_t val_size);
TABLE_API bool table_col_setkv_string(table_t table, size_t col, const char* key, const char* val);
TABLE_API bool table_setkv_value(table_t table, int key, int val);
TABLE_API bool table_getkv_value(table_t table, int key, int * val);
TABLE_API bool table_col_setkv_value(table_t table, size_t col, int key, int val);
TABLE_API bool table_col_getkv_value(table_t table, size_t col, int key, int * val);
TABLE_API const size_t table_kv_string_len(table_t table, const char* key);
TABLE_API const size_t table_col_kv_string_len(table_t table, size_t col, const char* key);
/**  @} */


/**
 * @name 输出格式
 * @{ */

TABLE_API char * table_tojson(table_t table);

/**  @} */



#ifdef TABLE_WITH_SQLITE3

#include <sqlite.h>

TABLE_API table_t table_from_sqlite3(sqlite3 * sqlite, const char * sql);
TABLE_API table_t table_from_sqlite3_stmt(sqlite3 * sqlite, sqlite3_stmt * stmt);

#endif // TABLE_WITH_SQLITE3

#ifdef TABLE_WITH_MYSQL

TABLE_API table_t table_from_mysql(MYSQL * mysql, MYSQL_RES * result);

#endif // TABLE_WITH_MYSQL

#ifdef __cplusplus
}	// extern "C"
#endif

#endif
