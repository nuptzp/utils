#pragma once
#include <stdio.h>

#include "table.h"

namespace table_v3
{
class table_impl;
void clean_schema_pool(void);
class table_t
{
public:


public:
	static table_t * create(const char * schema, size_t rows, const char * name = "");
	table_t * clone() const;
	void addref();
	static void destory(table_t ** pt);
	static table_t * from_buff(const char * buff, size_t len);
	static void to_buff(const table_t * t, char * buff);
	static table_t * from_buff_extra(const char * buff, size_t len);
	static void to_buff_extra(const table_t * t, char * buff, const char * extra = NULL);
	size_t size_extra() const;
	static size_t to_csv(const table_t * t, char * buff, size_t len);
	void row_from_buff(size_t row, const char * buff, size_t len);
	void row_to_buff(size_t row, char * buff);
	size_t row_size(size_t row) const;

public:
	size_t rows() const;
	size_t cols() const;
	size_t size() const;
	size_t pod_size() const;
	size_t width() const;
	const char * schema() const;
	const char * name() const;

	size_t col_index(const char * name) const;
	size_t col_offset(size_t col) const;
	int col_type(size_t col)  const;
	size_t col_size(size_t col) const;
	size_t col_count(size_t col) const;
	size_t col_podsize(size_t col) const;
	const char * col_name(size_t col) const;
	size_t pkcol() const;
	KEYORDER pkorder() const;
	bool is_pod() const;

	size_t append();
	void tidy();
	bool is_tidy() const;
	void remove(size_t row, size_t col) ;
	void insert(size_t row, size_t col) ;

	int get_int(size_t row, size_t col) const;
	long long get_int64(size_t row, size_t col) const;
	unsigned int get_uint(size_t row, size_t col) const;
	unsigned long long get_uint64(size_t row, size_t col) const;
	float get_float(size_t row, size_t col) const;
	double get_double(size_t row, size_t col) const;
	const char * get_str(size_t row, size_t col) const;
	const table_t * get_table(size_t row, size_t col) const;
	const char * get_binary(size_t row, size_t col) const;
	const unsigned int get_binary_len(size_t row, size_t col) const;
	const void * get_buff(size_t row, size_t col) const;
	const size_t get_time_t(size_t row, size_t col) const;
	const long long get_time(size_t row, size_t col) const;

	void set_int(size_t row, size_t col, int val);
	void set_int64(size_t row, size_t col, long long val);
	void set_uint(size_t row, size_t col, unsigned int val);
	void set_uint64(size_t row, size_t col, unsigned long long val);
	void set_float(size_t row, size_t col, float val);
	void set_double(size_t row, size_t col, double val);
	void set_str_raw(size_t row, size_t col, char * val);
	void set_str(size_t row, size_t col, const char * val, bool delold = true);
	void set_table_raw(size_t row, size_t col, table_t * val);
	void set_table(size_t row, size_t col, const table_t * val, bool delod = true);
	//void set_binary_raw(size_t row, size_t col, size_t len, char * val);
	void set_binary(size_t row, size_t col, unsigned int len, const char * val, bool delold = true);
	void set_buff(size_t row, size_t col,
				  const void * buff, size_t len,size_t offset);
	void set_time(size_t row, size_t col, size_t sec, int msec = 0);
	bool col_isnull(size_t row, size_t col);
	void col_writenull(size_t row, size_t col);

	void fill(size_t drow, const table_t * other, size_t srow);
	void update(const table_t * other, const char * keycol);

	table_t * select(const char * fields, size_t from, size_t count)const;
	table_t * join(table_t * other, const char * keycol);
	table_t * sort(field_compare_func comparer, void * userdata);
	table_t * group(const char * field) const;
	table_t * filter(field_compare_func comparer, void * userdata);
	table_t * movecolsname(const char * srcColsName, const char * dstColsName);

	//custom attributes
	bool getkv_string(const char* key, char* val, size_t val_size);
	bool setkv_string(const char* key, const char* val);
	bool col_getkv_string(size_t col, const char* key, char* val, size_t val_size);
	bool col_setkv_string(size_t col, const char* key, const char* val);
	bool setkv_value(int key, int val);
	bool getkv_value(int key, int * val);
	bool col_setkv_value(size_t col, int key, int val);
	bool col_getkv_value(size_t col, int key, int * val);
	const size_t kv_string_len( const char* key);
	const size_t col_kv_string_len( size_t col, const char* key );


	//formater
	char * tojson();

private:
	table_t(const char * schema, size_t rows, const char * name);
	~table_t();
	void fill_attribute(const table_t * other);
	size_t attrib_size() const;
	size_t attrib_from_buff(const char * buff, size_t len);
	static size_t attrib_to_buff(const table_t * t, char * buff);
	static size_t nullflag_to_buff(const table_t * t, char * buff);
	static size_t nullflag_from_buff(const table_t * t, const char * buff);
	table_impl * impl;
};

typedef table_t * table_ptr_t;
};
