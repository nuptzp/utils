// +build !windows

#include "table.h"
#include "table_v3.h"
#include <string.h>


#define TABLE_CAST(_table) ((table_v3::table_ptr_t)(_table))
#define TABLE_CAST_CONST(_table) ((const table_v3::table_ptr_t)(_table))

table_t table_create( const char * schema, size_t rows, const char * name )
{
	return (table_t)(table_v3::table_t::create(schema, rows, name));
}

void table_clean_schema_pool( void )
{
	return (void)(table_v3::clean_schema_pool());
}

table_t table_clone( table_t table )
{
	return TABLE_CAST(table)->clone();
}

table_t table_assign( table_t table )
{
	TABLE_CAST(table)->addref();
	return table;
}

void table_destory( table_t table )
{
	table_v3::table_ptr_t p = TABLE_CAST(table);
	table_v3::table_t::destory(&p);
}

void table_tobuff( table_t table, char * buff)
{
	table_v3::table_ptr_t p = TABLE_CAST(table);
	table_v3::table_t::to_buff(p, buff);
}

table_t table_frombuff(const char * buff, size_t len )
{
	return (table_t)table_v3::table_t::from_buff(buff, len);
}
void table_tobuff_extra( table_t table, char * buff, const char * extra )
{
	table_v3::table_ptr_t p = TABLE_CAST(table);
	table_v3::table_t::to_buff_extra(p, buff, extra);
}

table_t table_frombuff_extra(const char * buff, size_t len )
{
	return (table_t)table_v3::table_t::from_buff_extra(buff, len);
}
size_t table_size_extra(table_t table)
{
	return TABLE_CAST(table)->size_extra();
}

void table_row_tobuff(table_t table, size_t row, char * buff)
{
	return TABLE_CAST(table)->row_to_buff(row, buff);
}

void table_row_frombuff(table_t table, size_t row, const char * buff, size_t len)
{
	return TABLE_CAST(table)->row_from_buff(row, buff, len);
}

size_t table_row_size(table_t table, size_t row)
{
	return TABLE_CAST(table)->row_size(row);
}

size_t table_rows( table_t table )
{
	return TABLE_CAST(table)->rows();
}

size_t table_cols( table_t table )
{
	return TABLE_CAST(table)->cols();
}

size_t table_size( table_t table )
{
	return TABLE_CAST(table)->size();
}

size_t table_pod_size( table_t table )
{
	return TABLE_CAST(table)->pod_size();
}

size_t table_width( table_t table )
{
	return TABLE_CAST(table)->width();
}

const char * table_schema( table_t table )
{
	return TABLE_CAST(table)->schema();
}

const char * table_name( table_t table )
{
	return TABLE_CAST(table)->name();
}

bool table_is_pod( table_t table )
{
	return TABLE_CAST(table)->is_pod();
}

size_t table_append( table_t table )
{
	return TABLE_CAST(table)->append();
}

void table_tidy( table_t table )
{
	TABLE_CAST(table)->tidy();
}

bool table_is_tidy( table_t table )
{
	return TABLE_CAST(table)->is_tidy();
}

size_t table_col_index( table_t table, const char * name )
{
	return TABLE_CAST(table)->col_index(name);
}

size_t table_col_offset( table_t table, size_t col )
{
	return TABLE_CAST(table)->col_offset(col);
}

int table_col_type( table_t table, size_t col )
{
	return TABLE_CAST(table)->col_type(col);
}

size_t table_col_size( table_t table, size_t col )
{
	return TABLE_CAST(table)->col_size(col);
}

const char * table_col_name( table_t table, size_t col )
{
	return TABLE_CAST(table)->col_name(col);
}

const size_t table_pkcol( table_t table )
{
	return TABLE_CAST(table)->pkcol();
}

KEYORDER table_pkorder( table_t table )
{
	return TABLE_CAST(table)->pkorder();
}

int table_get_int( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_int(row, col);
}
short int table_get_short( table_t table, size_t row, size_t col )
{
	return (short)(TABLE_CAST(table)->get_int(row, col) & 0x0000ffff);
}

long long table_get_int64( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_int64(row, col);
}

unsigned int table_get_uint( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_uint(row, col);
}

unsigned long long table_get_uint64( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_uint64(row, col);
}

float table_get_float( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_float(row, col);
}

double table_get_double( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_double(row, col);
}

const char * table_get_str( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_str(row, col);
}

const table_t table_get_table( table_t table, size_t row, size_t col )
{
	return (table_t)(TABLE_CAST(table)->get_table(row, col));
}
const char * table_get_binary(table_t table, size_t row, size_t col)
{
	return TABLE_CAST(table)->get_binary(row, col);
}
const size_t table_get_binary_len(table_t table, size_t row, size_t col)
{
	return TABLE_CAST(table)->get_binary_len(row, col);
}

const void * table_get_buff( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_buff(row, col);
}

const size_t table_get_time_t( table_t table, size_t row, size_t col)
{
	return TABLE_CAST(table)->get_time_t(row, col);
}

const long long table_get_time( table_t table, size_t row, size_t col)
{
	return TABLE_CAST(table)->get_time(row, col);
}
const long long table_get_pfloat( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_int64(row, col);//
}
const long long table_get_vfloat( table_t table, size_t row, size_t col )
{
	return TABLE_CAST(table)->get_int64(row, col);//
}


void table_set_int( table_t table, size_t row, size_t col, int val )
{
	TABLE_CAST(table)->set_int(row, col, val);
}
void table_set_short( table_t table, size_t row, size_t col, short int val )
{
	TABLE_CAST(table)->set_int(row, col, val);
}


void table_set_int64( table_t table, size_t row, size_t col, long long val )
{
	TABLE_CAST(table)->set_int64(row, col, val);
}

void table_set_uint( table_t table, size_t row, size_t col, unsigned int val )
{
	TABLE_CAST(table)->set_uint(row, col, val);
}

void table_set_uint64( table_t table, size_t row, size_t col, unsigned long long val )
{
	TABLE_CAST(table)->set_uint64(row, col, val);
}

void table_set_float( table_t table, size_t row, size_t col, float val )
{
	TABLE_CAST(table)->set_float(row, col, val);
}

void table_set_double( table_t table, size_t row, size_t col, double val )
{
	TABLE_CAST(table)->set_double(row, col, val);
}

void table_set_str_raw( table_t table, size_t row, size_t col, char * val )
{
	TABLE_CAST(table)->set_str_raw(row, col, val);
}

void table_set_str( table_t table, size_t row, size_t col, const char * val, bool delold )
{
	TABLE_CAST(table)->set_str(row, col, val, delold);
}

void table_set_table_raw( table_t table, size_t row, size_t col, table_t val )
{
	TABLE_CAST(table)->set_table_raw(row, col, TABLE_CAST(val));
}

void table_set_table( table_t table, size_t row, size_t col, const table_t val, bool delold )
{
	TABLE_CAST(table)->set_table(row, col, TABLE_CAST_CONST(val), delold);
}

//void table_set_binary_raw(table_t table, size_t row, size_t col, size_t len, const char * val);
//{
//	TABLE_CAST(table)->set_binary_raw(row, col, len, val);
//}

void table_set_binary(table_t table, size_t row, size_t col, size_t len, const char * val, bool delold)
{
	TABLE_CAST(table)->set_binary(row, col, len, val, delold);
}

void table_set_buff( table_t table, size_t row, size_t col, const void * buff, size_t len,size_t offset )
{
	TABLE_CAST(table)->set_buff(row, col, buff, len, offset);
}

void table_set_time(table_t table, size_t row, size_t col, size_t sec, int msec)
{
	TABLE_CAST(table)->set_time(row, col, sec, msec);
}

bool table_col_isnull(table_t table, size_t row, size_t col)
{
	return TABLE_CAST(table)->col_isnull(row, col);
}

void table_col_writenull(table_t table, size_t row, size_t col)
{
	TABLE_CAST(table)->col_writenull(row, col);
}
void table_set_pfloat( table_t table, size_t row, size_t col, long long val )
{
	TABLE_CAST(table)->set_int64(row, col, val);//同 table_get/set_int64, 设置和返回 int64 即可
}
void table_set_vfloat( table_t table, size_t row, size_t col, long long val )
{
	TABLE_CAST(table)->set_int64(row, col, val);//同 table_get/set_int64, 设置和返回 int64 即可
}



void table_fill( table_t table, size_t drow, const table_t other, size_t srow )
{
	TABLE_CAST(table)->fill(drow, TABLE_CAST_CONST(other), srow);
}

void table_update( table_t table, const table_t other, const char * keycol )
{
	TABLE_CAST(table)->update(TABLE_CAST_CONST(other), keycol);
}

table_t table_select(table_t table, const char * fields, size_t from, size_t count)
{
	return (table_t)TABLE_CAST(table)->select(fields, from, count);
}

table_t table_join(table_t table, table_t other, const char * keycol)
{
	return (table_t)TABLE_CAST(table)->join( TABLE_CAST_CONST(other), keycol );
}

table_t table_sort(table_t table, field_compare_func comparer, void * userdata)
{
	return (table_t)TABLE_CAST(table)->sort( comparer, userdata );
}

table_t table_group(table_t table, const char * key)
{
	return (table_t)TABLE_CAST(table)->group( key );
}

table_t table_filter(table_t table, field_compare_func comparer, void * userdata)
{
	return (table_t)TABLE_CAST(table)->filter( comparer, userdata );
}

table_t table_move_colsname(table_t table, const char * srcColsName, const char * dstColsName)
{
	return (table_t)TABLE_CAST(table)->movecolsname( srcColsName, dstColsName );
}

table_malloc_func	gfpMalloc = NULL;
table_free_func		gfpFree = NULL;

void table_set_alloctor(table_malloc_func malloc_func, table_free_func free_func)
{
	gfpMalloc = malloc_func;
	gfpFree = free_func;
}

void table_delete( table_t table, size_t row, size_t num)
{
	return TABLE_CAST(table)->remove(row, num);
}
void table_insert( table_t table, size_t row, size_t num)
{
	return TABLE_CAST(table)->insert(row, num);
}
bool table_getkv_string(table_t table, const char* key, char* val, size_t val_size)
{
	return TABLE_CAST(table)->getkv_string(key, val, val_size);
}
bool table_setkv_string(table_t table, const char* key, const char* val)
{
	return TABLE_CAST(table)->setkv_string(key, val);
}
bool table_col_getkv_string(table_t table, size_t col, const char* key, char* val, size_t val_size)
{
	return TABLE_CAST(table)->col_getkv_string(col, key, val, val_size);
}
bool table_col_setkv_string(table_t table, size_t col, const char* key, const char* val)
{
	return TABLE_CAST(table)->col_setkv_string(col, key, val);
}
bool table_setkv_value(table_t table, int key, int val)
{
	return TABLE_CAST(table)->setkv_value(key, val);
}
bool table_getkv_value(table_t table, int key, int * val)
{
	return TABLE_CAST(table)->getkv_value(key, val);
}
bool table_col_setkv_value(table_t table, size_t col, int key, int val)
{
	return TABLE_CAST(table)->col_setkv_value(col, key, val);
}
bool table_col_getkv_value(table_t table, size_t col, int key, int * val)
{
	return TABLE_CAST(table)->col_getkv_value(col, key, val);
}
const size_t table_kv_string_len(table_t table, const char* key)
{
	return TABLE_CAST(table)->kv_string_len( key);
}
const size_t table_col_kv_string_len(table_t table, size_t col, const char* key)
{
	return TABLE_CAST(table)->col_kv_string_len(col, key);
}

char * table_strnew(size_t bytes){
	if(gfpMalloc)
		return gfpMalloc(bytes);
	else
		return new char[bytes];
}

void table_strdel(char * s){
	if(gfpFree)
		gfpFree(s);
	else
	{
		delete [] s;
	}
}

char * table_strdup(const char * s){
	if(NULL == s)  return NULL;
	size_t len = strlen(s);
	char * r = table_strnew(len + 1);
	memcpy(r, s, len + 1);
	return r;
}

char * table_tojson(table_t table)
{
	return TABLE_CAST(table)->tojson();
}
