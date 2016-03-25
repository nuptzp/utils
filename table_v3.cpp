// +build !windows

#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <cfloat>
#include <cmath>
#include "table_v3.h"
#include "table.h"


#if defined(_WIN32) && !defined(__MINGW32__)
	#define snprintf _snprintf
	typedef unsigned __int32 uint32_t;
#else
	#include <stdint.h>
	#include <stdarg.h>
	inline void strncpy_s(char * dst, size_t dstlen, const char * src, size_t srclen)
	{
		strncpy(dst, src, srclen);
	}
	inline char * strcpy_s(char * dst, size_t dstlen, const char * src)
	{
		return strncpy(dst, src, dstlen);
	}
	inline int _snprintf_s(char * dst, size_t sizeofBuffer, size_t count, const char * format, ...)
	{
		va_list arg;
		va_start(arg, format);
		int ret = vsnprintf(dst, count, format, arg);
		va_end(arg);
		return ret;
	}
	#define sprintf_s snprintf
#endif

using namespace std;

#define _VERSION_ 101					//版本号1.01

inline int intcmp(int a, int b){
	int r = a - b;
	if (r < 0)
		return -1;
	else if (r > 0)
		return 1;
	else
		return 0;
}

inline int int64cmp(long long a, long long b){
	long long r = a - b;
	if (r < 0)
		return -1;
	else if (r > 0)
		return 1;
	else
		return 0;
}

inline int floatcmp(float a, float b){
	float r = a - b;
	if (fabs(r) < FLT_EPSILON)
		return 0;
	else if (r > 0)
		return 1;
	else
		return -1;
}

inline int doublecmp(double a, double b){
	double r = a - b;
	if (fabs(r) < DBL_EPSILON)
		return 0;
	else if (r > 0)
		return 1;
	else
		return -1;
}

int str_field_comparer(table_t table, size_t r1, size_t r2, void * userdata)
{
    size_t col = (size_t)userdata;
	return strcmp(table_get_str(table, r1, col), table_get_str(table, r2, col));
}
int int_field_comparer(table_t table, size_t r1, size_t r2, void * userdata)
{
    size_t col = (size_t)userdata;
	return intcmp(table_get_int(table, r1, col), table_get_int(table, r2, col));
}
int int64_field_comparer(table_t table, size_t r1, size_t r2, void * userdata)
{
    size_t col = (size_t)userdata;
	return int64cmp(table_get_int64(table, r1, col), table_get_int64(table, r2, col));
}
int float_field_comparer(table_t table, size_t r1, size_t r2, void * userdata)
{
    size_t col = (size_t)userdata;
	return floatcmp(table_get_float(table, r1, col), table_get_float(table, r2, col));
}
int double_field_comparer(table_t table, size_t r1, size_t r2, void * userdata)
{
    size_t col = (size_t)userdata;
	return doublecmp(table_get_double(table, r1, col), table_get_double(table, r2, col));
}

int str_value_comparer(table_t table, size_t row, size_t col, void * userdata)
{
    char * val = (char *)userdata;
    return strcmp(table_get_str(table, row, col), val);
}
int int_value_comparer(table_t table, size_t row, size_t col, void * userdata)
{
    int * val = (int *)userdata;
    return intcmp(table_get_int(table, row, col), *val);
}

int int64_value_comparer(table_t table, size_t row, size_t col, void * userdata)
{
    long long * val = (long long *)userdata;
    return int64cmp(table_get_int64(table, row, col), *val);
}
int float_value_comparer(table_t table, size_t row, size_t col, void * userdata)
{
    float * val = (float*)userdata;
    return floatcmp(table_get_float(table, row, col), *val);
}
int double_value_comparer(table_t table, size_t row, size_t col, void * userdata)
{
    double * val = (double*)userdata;
    return doublecmp(table_get_double(table, row, col), *val);
}
value_compare_func valuefunc[] = 
{
	str_value_comparer,
	int_value_comparer,
	int64_value_comparer,
	float_value_comparer,
	double_value_comparer
};

field_compare_func fieldfunc[] = 
{
	str_field_comparer,
	int_field_comparer,
	int64_field_comparer,
	float_field_comparer,
	double_field_comparer
};

namespace table_v3
{
class filed_t
{
public:
	filed_t(const string & schema, size_t offset) : offset_(offset) {
		parse(schema);
	}

	bool parse(const string & schema) {
		size_t s = 0;
		size_t e = 0;
		size_t l = 0;
		e = schema.rfind(':');
		if (e == string::npos) {
			return false;
		}
		name_ = schema.substr(s, e);l = name_.length();
		if ( (name_[0] == '\'') && (name_[l-1] == '\'') )
			name_ = name_.substr(1,l-2);

		s = ++e;	l = schema.length() + 1;
		while(schema[e] >= '0' && schema[e] <= '9' && e < l) {
			e++;
		}
		if (s == e) {
			count_ = 1;
		} else {
			count_ = atoi(schema.c_str() + s);
		}

		parse_type(schema[e]);
		width_ = count_ * podsize_;
		return true;
	}

	void parse_type(char t) {
		static const char * types = " chiqfdstCHIQbTpv";
		static const size_t podsize [] = {0, 1, 2, 4, 8, 4, 8, 8, 8, 1, 2, 4, 8, 8, 8, 8, 8};
		const char * found = strchr(types, t);
		if (found != NULL) {
			type_ = (table_filed_type)(found - types);
			podsize_ = podsize[type_];
		} else {
			type_ = tft_null;
			podsize_ = 0;
		}
	}

	inline const string & name() const {
		return name_;
	}

	inline table_filed_type type() const {
		return type_;
	}

	inline size_t podsize() const {
		return podsize_;
	}

	inline size_t count() const {
		return count_;
	}

	inline size_t width() const {
		return width_;
	}

	inline size_t offset() const {
		return offset_;
	}

	inline bool empty() const {
		return type_ == tft_null;
	}

protected:
	string name_;
	table_filed_type type_;
	size_t podsize_;
	size_t count_;
	size_t width_;
	size_t offset_;
};

class schema_t
{
public:
	schema_t(const char * schema) {
		parse(schema);
	}

	void parse(const char * schema) {
		size_t s = 0;
		size_t e = 0;
		size_t i = 0;
		schema_ = schema;
		e = schema_.find('=', s);
		if (e != string::npos) {
			name_ = schema_.substr(s, e);
		}
		s = ++e;
		width_ = 0;
		is_pod_ = true;
		keycol_ = (size_t)(-1);
		keycolorder_ = ASC;
		fileds_.clear();	filed_name_map_.clear();
		vector<filed_t> & fileds = fileds_;
		bool notincolname = true;
		while(e < schema_.length()) {
			if (schema_[e] == ';' && notincolname) {
				if(schema_[s] == '<'){s++;	keycol_ = i;		keycolorder_ = ASC;}
				else if(schema_[s] == '>'){s++; keycol_ = i;	keycolorder_ = DESC;}
				filed_t f(schema_.substr(s, e-s), width_);
				fileds.push_back(f);
				filed_name_map_[f.name()] = i++;
				width_ += f.width();
				if (f.type() == tft_str || f.type() == tft_table || f.type() == tft_binpt) {
					is_pod_ = false;
				}
				s = ++e;
			}
			if (e < schema_.length())			//有可能 ; 就在结尾，此时e 等于 length, 再取schema_[e]就越界了。
			{
				if (schema_[e] == '\''){
					notincolname = !notincolname;
				}
			}
			e++;
		}
		if( (s < e) && (s < schema_.length()) ){
			if(schema_[s] == '<'){s++;	keycol_ = i;		keycolorder_ = ASC;}
			else if(schema_[s] == '>'){s++;	keycol_ = i;	keycolorder_ = DESC;}
			filed_t f(schema_.substr(s, e), width_);
			fileds.push_back(f);
			filed_name_map_[f.name()] = i++;
			width_ += f.width();
			if (f.type() == tft_str || f.type() == tft_table || f.type() == tft_binpt) {
				is_pod_ = false;
			}
		}
	}

	inline size_t width() const {
		return width_;
	}

	inline size_t fileds_count() const {
		//return fileds_count_;
		return fileds_.size();
	}

	inline size_t fileds_index(const string & name) const {
		map<string, size_t>::const_iterator found = filed_name_map_.find(name);
		if (found != filed_name_map_.end()) {
			return found->second;
		}
		return (size_t)(-1);
	}

	inline const filed_t & filed(size_t i) const {
		if (i < fileds_count()) {
			return fileds_[i];
		}
		static filed_t empty_filed("", 0);
		return empty_filed;
	}

	inline const string & name() const {
		return name_;
	}

	inline bool is_pod() const {
		return is_pod_;
	}

	inline const string & schema() const {
		return schema_;
	}

	inline size_t keycol() const {
		return keycol_;
	}

	inline KEYORDER keycolorder() const {
		return keycolorder_;
	}

protected:
	string schema_;
	string name_;
	vector<filed_t> fileds_;
	map<string, size_t> filed_name_map_;
	size_t width_;
	bool is_pod_;
	size_t keycol_;
	KEYORDER keycolorder_;
};

class row_t
{
public:

	row_t() : buff_(NULL), hasmemory_(false){} 

	row_t(char * buff) : buff_(buff), hasmemory_(false) {}

	row_t(size_t width) : hasmemory_(true) {
		buff_ = table_strnew(width);
		memset(buff_, 0, width);
	}

	~row_t() {
		//if (hasmemory_) {
		//	table_strdel(buff_);
		//}
	}

	inline char * buff(size_t offset) {
		return buff_ + offset;
	}

	inline const char * buff(size_t offset) const {
		return buff_ + offset;
	}

	char * buff_;
	bool hasmemory_;
};

////////////////////////////////////////////////////////////////////////////////
//custon attrib data type
typedef struct _custom_attrib_string{
	char * skey;
	char * sval;
}_String_Attrib;
typedef struct _custom_attrib_int{
	int nkey;
	int nval;
}_Int_Attrib;
typedef struct _custom_attrib{
	_String_Attrib spair[8];
	_Int_Attrib    npair[8];
}_Custom_Attrib;

////////////////////////////////////////////////////////////////////////////////

class table_impl
{
	friend class table_t;
public:
	table_impl(const char * schema, size_t rows, const char * name);

	~table_impl();

	char * index(size_t row, size_t col);
	const char * index(size_t row, size_t col) const;

	size_t add_row();
	void tidy();
	void remove_row(size_t row, size_t num);
	void insert_row(size_t row, size_t num);

	template<class T>
	inline T get(int row, int col) const {
		const T * p = (const T*)index(row, col);
		if (p) {
			return *p;
		}
		return 0;
	}

	template<class T, class DelT>
	inline void set(int row, int col,  T obj, DelT del) {
		T * p = (T*)index(row, col);
		if (p) {
			del(p);
		}
		*p = obj;
	}

	schema_t * create_table_schema(const char * schema);

public:
	char * buff_;
	size_t init_size_;
	vector<row_t> datas_;
	//schema_t schema_;
	schema_t * schema_p;
	string name_;
	size_t ref_;
	vector<bool> nullflag_;
	vector <_Custom_Attrib> attrib_;
};

////////////////////////////////////////////////////////////////////////////////
typedef struct _schema_buff_{
	char * schema_string;
	schema_t * schema_struct;
}SCHEMABUFF;

//vector <SCHEMABUFF> g_vecSchemaBuff;
map<string,schema_t *> g_mapSchemaBuff;
#ifdef WIN32
#include <Windows.h>
CRITICAL_SECTION cs_;
#else
#endif
bool initCs = false;

////////////////////////////////////////////////////////////////////////////////

template<class T, class F>
inline size_t for_each_filed (table_impl * impl, size_t filed, F func)
{
	size_t sum = 0;
	for(size_t i = 0; i < impl->datas_.size(); i++) {
		char * base = impl->index(i, filed);
		T * obj = (T*)(base);
		sum += func(obj);
	}
	return sum;
};
template<class T, class F>
inline size_t for_each_filed_section (table_impl * impl, size_t filed, F func, size_t from, size_t to)
{
	size_t sum = 0;
	for(size_t i = from; i < to; i++) {
		char * base = impl->index(i, filed);
		T * obj = (T*)(base);
		sum += func(obj);
	}
	return sum;
};

inline size_t str_size(const char ** s)
{
	if (*s) {
		return strlen(*s) + sizeof(uint32_t);
	} else {
		return 0+ sizeof(uint32_t);
	}
}

inline size_t str_delete(char ** s)
{
	if (*s) {
		table_strdel(*s);
		*s = NULL;
	}
	return 0;
}


struct str_to_buff {
	str_to_buff(char * buff) : buff_(buff) {}

	inline size_t operator()(const char **s) {
		uint32_t l = 0;
		if (s && *s) {
			l = strlen(*s) ;
			memcpy(buff_, &l, sizeof(uint32_t));
			memcpy(buff_+sizeof(uint32_t), *s, l);
		}else
			memcpy(buff_, &l,sizeof(uint32_t));
		buff_ += (l+sizeof(uint32_t));
		return (l+sizeof(uint32_t));
	}
	
protected:
	char * buff_;
};

struct str_from_buff {
	str_from_buff(const char * buff) : buff_(buff) {}

	inline size_t operator()(char **s) {
		size_t l = *(const uint32_t *)buff_;
		char * str_ = new char[l+1];
		strncpy(str_, (buff_+sizeof(uint32_t)), l); str_[l]=0;
		*s = table_strdup(str_);
		buff_ += (l+sizeof(uint32_t));
		delete [] str_;
		return l+sizeof(uint32_t);
	}
protected:
	const char * buff_;
};

inline size_t binpt_size(const char ** s)
{
	uint32_t l = 0;
	if (s && *s) {
		memcpy((char *)&l, *s, 4);
		l = l+4;
		return l;
	} else {
		return 4;
	}
}

struct bin_to_buff {
	bin_to_buff(char * buff) : buff_(buff) {}

	inline size_t operator()(const char **s) {
		uint32_t l = 0;
		if (s && *s) {
			memcpy((char *)&l, *s, 4);
		}
		l = l+4;
		memcpy(buff_, *s, l);
		buff_ += l;
		return l;
	}
	
protected:
	char * buff_;
};

struct bin_from_buff {
	bin_from_buff(const char * buff) : buff_(buff) {}

	inline size_t operator()(char **s) {
		size_t l = 0;
		memcpy((char *)&l, buff_, 4);
		l = l+4;
		*s = table_strnew(l + 1);
		memcpy(*s, buff_, l);
		buff_ += l;
		return l;
	}
protected:
	const char * buff_;
};

inline size_t table_size(const table_t ** t)
{
	if (*t) {
		return (*t)->size(); // + sizeof(uint32_t);
	} else {
		return 0;
	}
}
inline size_t table_size_extra(const table_t ** t)
{
	if (*t) {
		return (*t)->size_extra(); 
	} else {
		return 0;
	}
}

inline size_t table_delete(table_t ** t)
{
	table_t::destory(t);
	return 0;
}

struct table_to_buff {
	table_to_buff(char * buff) : buff_(buff){}

	inline size_t operator()(const table_t **t) {
		uint32_t l = 0;
		if (t && *t) {
			l = table_size(t);
		}
		*(uint32_t*)buff_ = l;
		l += sizeof(uint32_t);
		table_t::to_buff(*t, buff_ + sizeof(uint32_t));
		buff_ += l;
		return l;
	}
protected:
	char * buff_;
	const char * extra_;
};

struct table_from_buff {
	table_from_buff(const char * buff) : buff_(buff){}

	inline size_t operator()(table_t **t) {
		uint32_t l = *(const uint32_t*)buff_;
		*t = table_t::from_buff(buff_ + sizeof(uint32_t), l);
		l += sizeof(uint32_t);
		buff_ += l;
		return l;
	}
protected:
	const char * buff_;
};
struct table_to_buff_extra {
	table_to_buff_extra(char * buff, const char * extra = NULL) : buff_(buff),extra_(extra) {}

	inline size_t operator()(const table_t **t) {
		uint32_t l = 0;
		if (t && *t) {
			l = table_size_extra(t);
		}
		*(uint32_t*)buff_ = l;
		l += sizeof(uint32_t);
		table_t::to_buff_extra(*t, buff_ + sizeof(uint32_t),extra_);
		buff_ += l;
		return l;
	}
protected:
	char * buff_;
	const char * extra_;
};

struct table_from_buff_extra {
	table_from_buff_extra(const char * buff) : buff_(buff){}

	inline size_t operator()(table_t **t) {
		uint32_t l = *(const uint32_t*)buff_;
		*t = table_t::from_buff_extra(buff_ + sizeof(uint32_t), l);
		l += sizeof(uint32_t);
		buff_ += l;
		return l;
	}
protected:
	const char * buff_;
};

template<class T>
inline void nan_delete(T* )
{
}

////////////////////////////////////////////////////////////////////////////////

table_impl::table_impl(const char * schema, size_t rows, const char * name) :
	init_size_(rows), /*schema_(schema), */name_(name)
{
	if(!initCs){
#ifdef WIN32
		InitializeCriticalSection(&cs_);
#else
#endif
		initCs = true;
	}
	schema_p = create_table_schema(schema);

	if (rows != 0) {
		buff_ = table_strnew(schema_p->width() * rows);
		memset(buff_, 0, schema_p->width() * rows);
		for(size_t i = 0; i < rows; i++) {
			datas_.push_back(row_t(buff_ + i * schema_p->width()));
		}
	} else {
		buff_ = NULL;
	}
	ref_ = 1;
	nullflag_.clear();

	//custom attrib alloc space
	size_t cols = schema_p->fileds_count();
	_Custom_Attrib cell;
	for(size_t i=0;i<8;i++){
		cell.npair[i].nkey=0;cell.npair[i].nval=0;cell.spair[i].skey=NULL;cell.spair[i].sval=NULL;
	}
	attrib_.push_back(cell);
	for(size_t i=0; i < cols;i++) attrib_.push_back(cell);
}

table_impl::~table_impl()
{
	for(size_t i = 0; i < schema_p->fileds_count(); i++) {
		const filed_t & f = schema_p->filed(i);
		if ((f.type() == tft_str) || (f.type() == tft_binpt)) {
			for_each_filed<char*>(this, i, str_delete);
		} else if (f.type() == tft_table) {
			for_each_filed<table_t*>(this, i, table_delete);
		}
	}
	table_strdel((char*)buff_);
	for(size_t i = 0; i < datas_.size(); i++) {		//考虑到插入，所以从0开始，加判断hasmemory_条件
		row_t & r = datas_[i];
		if(r.hasmemory_)	table_strdel(r.buff_);
	}
	nullflag_.clear();
	//delete attrib string
	size_t cols = schema_p->fileds_count();
	for(size_t i=0;i<=cols;i++){
		_Custom_Attrib &cell = attrib_[i];
		for(size_t j=0;j<8;j++){
			char * skey = cell.spair[j].skey;
			char * sval = cell.spair[j].sval;
			if(skey != NULL) delete skey;
			if(sval != NULL) delete sval;
		}
	}
}

inline char * table_impl::index(size_t row, size_t col)
{
	if (row < datas_.size() && col < schema_p->fileds_count()) {
		const filed_t & f = schema_p->filed(col);
		return datas_[row].buff(f.offset());
	}
	return NULL;
}

inline const char * table_impl::index(size_t row, size_t col) const
{
	if (row < datas_.size() && col < schema_p->fileds_count()) {
		const filed_t & f = schema_p->filed(col);
		return datas_[row].buff(f.offset());
	}
	return NULL;
}
size_t table_impl::add_row()
{
	row_t r(schema_p->width());
	datas_.push_back(r);
	if(0<nullflag_.size()){
		for(size_t i = 0; i < schema_p->fileds_count(); i++){
			nullflag_.push_back(false);						//初始化空值标记表新的一行
		}
	}
	return datas_.size() - 1;
}

void table_impl::tidy()
{
	size_t width = schema_p->width();
	size_t rows = datas_.size();
	char * newbuff = table_strnew(rows * width);
	//memcpy(newbuff, buff_, init_size_ * width);

	//char * buff = newbuff + init_size_ * width;
	char * buff = newbuff;
	for(size_t i = 0; i < rows; i++){
		memcpy(buff, datas_[i].buff_, width);
		if(datas_[i].hasmemory_)	table_strdel(datas_[i].buff_);		//从0开始，判断条件，释放append、insert时申请的内存，因为不再使用了
		buff += width;	
	}

	init_size_ = rows;
	datas_.clear();

	for(size_t i = 0; i < rows; i++) {
		datas_.push_back(row_t(newbuff + i * width));
	}

	table_strdel(buff_);
	buff_ = newbuff;
}
void table_impl::remove_row(size_t row, size_t num)
{
	size_t from = row;
	size_t to = row+num;

	for(size_t i = 0; i < schema_p->fileds_count(); i++) {
		const filed_t & f = schema_p->filed(i);
		if ((f.type() == tft_str) || (f.type() == tft_binpt)) {
			for_each_filed_section<char*>(this, i, str_delete, from, to);
		} else if (f.type() == tft_table) {
			for_each_filed_section<table_t*>(this, i, table_delete, from, to);
		}
	}

	for(size_t i = from; i < to ; i++)
	{
		if(datas_[i].hasmemory_)
			table_strdel(datas_[i].buff_);			//释放了单独申请的内存，
	}
	datas_.erase( datas_.begin() + from, datas_.begin() + to ); // erase methon : contain fromIx and not contain toIx

	if(0<nullflag_.size()){
		int fromIx = row * schema_p->fileds_count();
		int toIx = fromIx + num * schema_p->fileds_count() ;
		nullflag_.erase(nullflag_.begin() + fromIx, nullflag_.begin() +  toIx);	//空值标记表减少num行		, 
	}

	return ;
}
void table_impl::insert_row(size_t row, size_t num)
{
	vector<row_t>::iterator from = datas_.begin() + row;	
	for(size_t i=0;i < num; i++)	
	{
		row_t r(schema_p->width());
		datas_.insert(from + i, r);
	}
	if(0<nullflag_.size()){
		int fromIx = row * schema_p->fileds_count();
		int count = num * schema_p->fileds_count() ;
		nullflag_.insert(nullflag_.begin() + fromIx, count, false);	//初始化空值标记表对应的行
	}
	return ;
}

////////////////////////////////////////////////////////////////////////////////
//得到字符串（len+string），返回该字符串长度值
size_t get_string_from_len_string(const char *src, char ** dst)
{
	uint32_t len  = *((uint32_t*)src);
	*dst = new char[len+1];
	strncpy_s(*dst, len+1, src+sizeof(uint32_t), len);
	return len+sizeof(uint32_t);
}

////////////////////////////////////////////////////////////////////////////////

table_t::table_t(const char * schema, size_t rows, const char * name)
{
	impl = new table_impl(schema, rows, name);
}

table_t::~table_t()
{
	delete impl;
}

size_t table_t::rows() const
{
	return impl->datas_.size();
}

size_t table_t::cols() const
{
	return impl->schema_p->fileds_count();
}

size_t table_t::size() const
{
	size_t s = rows() * width();
	size_t n = 0;
	size_t ptrsize = 0;
	for(size_t i = 0; i < cols(); i++) {
		const filed_t & f = impl->schema_p->filed(i);
		if (f.type() == tft_str) {
			s += for_each_filed<const char*>(impl, i, str_size);
			ptrsize += 8 * rows();
		}
		if (f.type() == tft_table) {
			s += for_each_filed<const table_t*>(impl, i, table_size);
			s += (sizeof(uint32_t)) * rows();    //attachment a length field 
			ptrsize += 8 * rows();
		}
		if (f.type() == tft_binpt) {
			s += for_each_filed<const char*>(impl, i, binpt_size);
			ptrsize += 8 * rows();
		}
		//上面几种类型，tobuff用数据，而table一行中原放的是1个指针。所以长度+数据实际长度，并准备-指针长度。
	}
	size_t attribsize = attrib_size();
	size_t nullflagbitsize = impl->nullflag_.size();
	size_t nullflagsize = (nullflagbitsize % 8 ) ? (nullflagbitsize/8+1) : nullflagbitsize/8 ;	
	// （ver+size+offset+3length）schema+name+rows-累计的指针长度+空值表长度+属性长度
	return s + 6* sizeof(uint32_t) + strlen(schema()) + strlen(name()) + sizeof(uint32_t) - ptrsize + nullflagsize + attribsize;
}
size_t table_t::size_extra() const
{
	return size();		//此时不可知附加串的长度，应该由上层得到当前table序列化后的长度，然后上层自己+附加串的长度。
}

size_t table_t::pod_size() const
{
	return rows() * width();
}

size_t table_t::width() const
{
	return impl->schema_p->width();
}

size_t table_t::col_index(const char * name) const
{
	return impl->schema_p->fileds_index(name);
}

size_t table_t::col_offset(size_t col) const
{
	const filed_t & f = impl->schema_p->filed(col);
	return f.offset();
}

int table_t::col_type(size_t col) const
{
	const filed_t & f = impl->schema_p->filed(col);
	return f.type();
}

size_t table_t::col_size(size_t col) const
{
	const filed_t & f = impl->schema_p->filed(col);
	return f.width();
}
size_t table_t::col_podsize(size_t col) const
{
	const filed_t & f = impl->schema_p->filed(col);
	return f.podsize();
}

size_t table_t::col_count(size_t col) const
{
	const filed_t & f = impl->schema_p->filed(col);
	return f.count();
}

const char* table_t::col_name(size_t col) const
{
	const filed_t & f = impl->schema_p->filed(col);

	return f.name().c_str();
}
size_t table_t::pkcol() const
{
	return impl->schema_p->keycol();
}
KEYORDER table_t::pkorder() const
{
	return impl->schema_p->keycolorder();
}

bool table_t::is_pod() const
{
	return impl->schema_p->is_pod();
}


size_t table_t::append(){
	return impl->add_row();
}

void table_t::tidy(){
	if (!is_tidy())
		impl->tidy();
}

bool table_t::is_tidy() const
{
	//return impl->init_size_ == impl->datas_.size();
	for(size_t r = 0 ; r < rows(); r++){
		if(impl->datas_[r].hasmemory_)	{
			return false;	//有一个是单独申请的内存，它就不是整齐的
		}
	}
	return true;			//所有行都不是单独申请的内存，认为它是整齐的。
}

int table_t::get_int(size_t row, size_t col) const
{
	int ret = 0;
	switch(col_type(col))
	{
	case tft_char:
		ret = impl->get<char>(row, col);
		break;
	case tft_short:
		ret = impl->get<short>(row, col);
		break;
	default:
		ret = impl->get<int>(row, col);
	}
	return ret;
}

long long table_t::get_int64(size_t row, size_t col) const
{
	return impl->get<long long>(row, col);
}

unsigned int table_t::get_uint(size_t row, size_t col) const
{
	unsigned int ret = 0;
	switch(col_type(col))
	{
	case tft_uchar:
		ret = impl->get<unsigned char>(row, col);
		break;
	case tft_ushort:
		ret = impl->get<unsigned short>(row, col);
		break;
	default:
		ret = impl->get<unsigned int>(row, col);
	}
	return ret;
}

unsigned long long table_t::get_uint64(size_t row, size_t col) const
{
	return impl->get<unsigned long long>(row, col);
}

float table_t::get_float(size_t row, size_t col) const
{
	return impl->get<float>(row, col);
}

double table_t::get_double(size_t row, size_t col) const
{
	return impl->get<double>(row, col);
}

const char * table_t::get_str(size_t row, size_t col) const
{
	return impl->get<const char *>(row, col);
}

const table_t * table_t::get_table(size_t row, size_t col) const
{
	return impl->get<const table_t *>(row, col);
}

const void * table_t::get_buff(size_t row, size_t col) const
{
	return impl->index(row, col);
}
const char * table_t::get_binary(size_t row, size_t col) const
{
	const char * r = impl->get<const char *>(row, col);
	return (NULL == r) ? NULL : (r+4);
	/*如果(NULL == r)成立说明该列并未写入数据，所以所得到应为NULL；如果不成立，则有数据，数据向后偏移一个uint_32类型的长度*/
}
const uint32_t table_t::get_binary_len(size_t row, size_t col) const
{
	const char * r = impl->get<const char *>(row, col);
	if(NULL == r)  return (uint32_t)0;

	union {
		uint32_t i;
		unsigned char c[4];
	}v;
	memcpy(v.c, r, 4);
	return v.i;
}
const size_t table_t::get_time_t(size_t row, size_t col) const
{
	//T 是一个 64 bit 的整数 其高32bit为 32bit 的 time_t (sec), 低 32bit 为 32bit 整数，表示毫秒(msec)

	size_t r = 0;
	long long v = impl->get<long long>(row, col);
	r = v >> 32;
	return r;
}
const long long table_t::get_time(size_t row, size_t col) const
{
	//T 是一个 64 bit 的整数 其高32bit为 32bit 的 time_t (sec), 低 32bit 为 32bit 整数，表示毫秒(msec)

	uint32_t sec = 0;
	int msec = 0;
	long long v = impl->get<long long>(row, col);
	sec = v >> 32;
	msec = v & 0x00000000ffffffff;
	return (long long)sec*1000+msec;
}

void table_t::set_int(size_t row, size_t col, int val)
{
	switch(col_type(col))
	{
	case tft_char:
		impl->set(row, col, (char)val, nan_delete<char> );	
		break;
	case tft_short:
		impl->set(row, col, (short)val, nan_delete<short> );	
		break;
	default:
		impl->set(row, col, val, nan_delete<int> );	
	}
}

void table_t::set_int64(size_t row, size_t col, long long val)
{
	impl->set(row, col, val, nan_delete<long long>);
}

void table_t::set_uint(size_t row, size_t col, unsigned int val)
{
	switch(col_type(col))
	{
	case tft_uchar:
		impl->set(row, col, (unsigned char)val, nan_delete<unsigned char> );	
		break;
	case tft_ushort:
		impl->set(row, col, (unsigned short)val, nan_delete<unsigned short> );	
		break;
	default:
		impl->set(row, col, val, nan_delete<unsigned int> );	 
	}
}

void table_t::set_uint64(size_t row, size_t col, unsigned long long val)
{
	impl->set(row, col, val, nan_delete<unsigned long long>);
}

void table_t::set_float(size_t row, size_t col, float val)
{
	impl->set(row, col, val, nan_delete<float>);
}

void table_t::set_double(size_t row, size_t col, double val)
{
	impl->set(row, col, val, nan_delete<double>);
}

void table_t::set_str_raw(size_t row, size_t col, char * val)
{
	impl->set(row, col, val, str_delete);
}

void table_t::set_str(size_t row, size_t col, const char * val, bool delold)
{
	char *s = table_strdup(val);

	if (delold) {
		impl->set(row, col, s, str_delete);
	} else {
		impl->set(row, col, s, nan_delete<char *>);
	}
}

void table_t::set_table_raw(size_t row, size_t col, table_t * val)
{
	impl->set(row, col, val, table_delete);
}

void table_t::set_table(size_t row, size_t col, const table_t * val, bool delold)
{
	table_t * t = NULL;
	if (val)
		t = val->clone();

	if (delold) {
		impl->set(row, col, t, table_delete);
	} else {
		impl->set(row, col, t, nan_delete<table_t*>);
	}
}
//void table_t::set_binary_raw(size_t row, size_t col, size_t len, const char * val)
//{
//	impl->set(row, col, val, str_delete);
//}

void table_t::set_binary(size_t row, size_t col, uint32_t len, const char * val, bool delold) 
{
	char * s = table_strnew(len + 4+1);
	memcpy(s, (char *)&len, 4);
	memcpy(s+4, val, len);

	if (delold) {
		impl->set(row, col, s, str_delete);
	} else {
		impl->set(row, col, s, nan_delete<char *>);
	}
}

void table_t::set_buff(size_t row, size_t col, const void * buff, size_t len, size_t offset)
{
	size_t w = col_size(col);

	if (len == (size_t)(-1)) {
		len = w;
	}

	if (offset + len > w) {
		return;
	}
	char * dest = impl->index(row, col);
	if (dest == NULL) {
		return;
	}
	dest += offset;
	memcpy(dest, buff, len);
}

void table_t::set_time(size_t row, size_t col, size_t sec, int msec)
{
	uint32_t sec1 = (uint32_t)sec;
	long long val = sec1;
	long long val1 = msec & 0x00000000ffffffff;
	val = val << 32;
	val = val | val1;
	impl->set(row, col, val, nan_delete<long long>);
}

const char * table_t::name() const
{
	return impl->name_.c_str();
}

const char * table_t::schema() const
{
	return impl->schema_p->schema().c_str();
}


table_t * table_t::create(const char * schema, size_t rows, const char * name)
{
	return new table_t(schema, rows, name);
}

void table_t::addref() {
	impl->ref_++;
}

void table_t::destory(table_t ** pt)
{
	if (pt && *pt) {
		(*pt)->impl->ref_ -= 1;
		if ((*pt)->impl->ref_ == 0)
			delete (*pt);
	}
}


table_t * table_t::from_buff(const char * buff, size_t bytes )
{
	if (bytes == 0)
		return NULL;

	char * schema;
	char * name;
	size_t rows;
	const char * begin = buff;
	const char * cur = begin;
	int len = 0;
	char *p = NULL;
	bool bExistAttach = false;
	const char *pAttach = NULL;

	int nVersion = *(const uint32_t *)cur;		//version
	cur += sizeof(uint32_t);

	int nsize = *(const uint32_t *)cur;				//size
	cur += sizeof(uint32_t);

	int nOffset = *(const uint32_t *)cur;			//data offset
	cur += sizeof(uint32_t);

	len = *(const uint32_t *)cur;						//schema: length+string
	cur += sizeof(uint32_t);
	schema = new char[len+1];
	begin = cur; p = schema;
	while(len--) {
		*p++ = *cur++;
	}
	*p = 0;

	len = *(const uint32_t *)cur;						//name: length+string
	cur += sizeof(uint32_t);
	name = new char[len+1];
	begin = cur; p = name;
	while(len--) {
		*p++ = *cur++;
	}
	*p = 0;

	len = *(const uint32_t *)cur;						//extra: length+string, int the function default len=0.
	cur += sizeof(uint32_t);
	char *extra = new char[len+1];
	begin = cur; p = extra;
	while(len--) {
		*p++ = *cur++;
	}
	*p = 0;

	if ((buff+nOffset) == cur)
	{
		bExistAttach = false; pAttach=NULL;
	}
	else
	{
		bExistAttach = true;
		pAttach = cur;
		cur = buff+nOffset;
	}

	rows = *(const uint32_t *)cur;				//rows
	cur += sizeof(uint32_t);

	table_t * t = table_t::create(schema, rows, name);

	delete [] schema;
	delete[] name;
	delete[] extra;

	for(size_t i = 0; i < t->cols(); i++) {
		const filed_t & f = t->impl->schema_p->filed(i);
		size_t off = 0;
		if (f.type() == tft_str) {
			off = for_each_filed<char*>(t->impl, i, str_from_buff(cur));
		} else if (f.type() == tft_table) {
			off = for_each_filed<table_t*>(t->impl, i, table_from_buff(cur));
		} else if (f.type() == tft_binpt) {
			off = for_each_filed<char*>(t->impl, i,bin_from_buff(cur));
		}
		else{
			size_t width = t->width();
			off = 0;
			for(size_t j = 0; j < t->rows(); j++){
				memcpy((char*)t->get_buff(j, i), cur, f.width());
				cur += f.width();
			}
		}
		cur += off;
	}
	if (bExistAttach)
	{
		cur  = pAttach;
		size_t attrib_len = t->attrib_from_buff(cur, 0);	
		cur += attrib_len;
		if( (buff + nOffset) > cur)
			size_t nullflag_len = nullflag_from_buff(t,cur);
	}
	return t;
}

void table_t::to_buff(const table_t * t, char * buff)
{
	if (t == NULL)
		return;

	char * cur = buff;
	size_t len = 0;

	*((uint32_t*)cur) = 1;							//version = 1
	cur += sizeof(uint32_t);

	len = t->size();
	*((uint32_t*)cur) = len;							//size
	cur += sizeof(uint32_t);

	size_t attribsize = t->attrib_size();
	size_t nullflagbitsize = t->impl->nullflag_.size();
	size_t nullflagsize = (nullflagbitsize % 8 ) ? (nullflagbitsize/8+1) : nullflagbitsize/8 ;	
	len = attribsize+nullflagsize+6*sizeof(uint32_t)+strlen(t->schema())+strlen(t->name());
	*((uint32_t*)cur) = len;							//data offset
	cur += sizeof(uint32_t);

	len = strlen(t->schema());						//schema len
	*((uint32_t*)cur) = len;							
	cur += sizeof(uint32_t);
	memcpy(cur, t->schema(), len + 1);
	cur += len;

	len = strlen(t->name());					//name len
	*((uint32_t*)cur) = len;							
	cur += sizeof(uint32_t);
	memcpy(cur, t->name(), len + 1);
	cur += len;

	*((uint32_t*)cur) = 0;					//extra is 0 length		
	cur += sizeof(uint32_t);

	size_t attrib_len = attrib_to_buff(t, cur);		//attrib_to_buff(cur);
	cur += attrib_len;
	size_t nullflag_len = nullflag_to_buff(t,cur);//nullflag_to_buff;
	cur += nullflag_len;

	*((uint32_t*)cur) = t->rows();
	cur += sizeof(uint32_t);

	for(size_t i = 0; i < t->cols(); i++) {
		const filed_t & f = t->impl->schema_p->filed(i);
		size_t off = 0;
		if (f.type() == tft_str) {
			off = for_each_filed<const char*>(t->impl, i, str_to_buff(cur));
		} else if (f.type() == tft_table) {
			off = for_each_filed<const table_t*>(t->impl, i, table_to_buff(cur));
		} else if (f.type() == tft_binpt) {
			off = for_each_filed<const char*>(t->impl, i, bin_to_buff(cur));
		}
		else{
			size_t width = t->width();
			off = 0;
			for(size_t j = 0; j < t->rows(); j++){
				memcpy(cur, t->get_buff(j, i), f.width());
				cur += f.width();
			}
		}
		cur += off;
	}
}
table_t * table_t::from_buff_extra(const char * buff, size_t bytes )
{
	return from_buff(buff, bytes);
}

void table_t::to_buff_extra(const table_t * t, char * buff, const char * extra_)
{
	if (t == NULL)
		return;

	char * cur = buff;
	size_t len = 0;

	*((uint32_t*)cur) = 1;							//version = 1
	cur += sizeof(uint32_t);

	len = t->size();
	*((uint32_t*)cur) = len;							//size
	cur += sizeof(uint32_t);

	size_t attribsize = t->attrib_size();
	size_t nullflagbitsize = t->impl->nullflag_.size();
	size_t nullflagsize = (nullflagbitsize % 8 ) ? (nullflagbitsize/8+1) : nullflagbitsize/8 ;	
	len = attribsize+nullflagsize+6*sizeof(uint32_t)+strlen(t->schema())+strlen(t->name());
	*((uint32_t*)cur) = len;							//data offset
	cur += sizeof(uint32_t);

	len = strlen(t->schema());						//schema len
	*((uint32_t*)cur) = len;							
	cur += sizeof(uint32_t);
	memcpy(cur, t->schema(), len + 1);
	cur += len;

	len = strlen(t->name());					//name len
	*((uint32_t*)cur) = len;							
	cur += sizeof(uint32_t);
	memcpy(cur, t->name(), len + 1);
	cur += len;

	len = strlen(extra_);							//extra is 0 length		
	*((uint32_t*)cur) = len;							
	cur += sizeof(uint32_t);
	memcpy(cur, extra_, len + 1);
	cur += len;

	size_t attrib_len = attrib_to_buff(t, cur);		//attrib_to_buff(cur);
	cur += attrib_len;
	size_t nullflag_len = nullflag_to_buff(t,cur);//nullflag_to_buff;
	cur += nullflag_len;

	*((uint32_t*)cur) = t->rows();
	cur += sizeof(uint32_t);

	for(size_t i = 0; i < t->cols(); i++) {
		const filed_t & f = t->impl->schema_p->filed(i);
		size_t off = 0;
		if (f.type() == tft_str) {
			off = for_each_filed<const char*>(t->impl, i, str_to_buff(cur));
		} else if (f.type() == tft_table) {
			off = for_each_filed<const table_t*>(t->impl, i, table_to_buff(cur));
		} else if (f.type() == tft_binpt) {
			off = for_each_filed<const char*>(t->impl, i, bin_to_buff(cur));
		}
		else{
			size_t width = t->width();
			off = 0;
			for(size_t j = 0; j < t->rows(); j++){
				memcpy(cur, t->get_buff(j, i), f.width());
				cur += f.width();
			}
		}
		cur += off;
	}
}

void table_t::row_from_buff(size_t row, const char * buff, size_t bytes )
{
	//一处重要的不同在于，row_from_buff是针对一个既有table来说的，即本身
	if (bytes == 0)
		return ;

	const char * begin = buff;
	const char * cur = begin;

	for(size_t i = 0; i < cols(); i++) {
		const filed_t & f = impl->schema_p->filed(i);
		size_t off = 0;
		if (f.type() == tft_str) {
			str_from_buff func(cur);
			char * base = impl->index(row, i);
			char ** obj = (char **)(base);
			if(*obj != NULL) table_strdel( *obj );
			off = func(obj);	
		} else if (f.type() == tft_table) {
			table_from_buff func(cur);
			char * base = impl->index(row, i);
			table_t ** obj = (table_t **)(base);
			if(*obj != NULL) table_destory( (table_t *)*obj );
			off = func(obj);	
		} else if (f.type() == tft_binpt) {
			bin_from_buff func(cur);
			char * base = impl->index(row, i);
			char ** obj = (char **)(base);
			if(*obj != NULL) table_strdel( *obj );
			off = func(obj);	
		}
		else{
			memcpy((char*)get_buff(row, i), cur, f.width());
			off =  f.width();
		}
		cur += off;
	}
	return ;
}

void table_t::row_to_buff( size_t row, char * buff)
{

	char * cur = buff;
	size_t len = 0;

	for(size_t i = 0; i < cols(); i++) {
		const filed_t & f =impl->schema_p->filed(i);
		size_t off = 0;
		if (f.type() == tft_str) {
			str_to_buff func(cur);
			char * base = impl->index(row, i);
			const char ** obj = (const char **)(base);
			off = func(obj);
		} else if (f.type() == tft_table) {
			table_to_buff func(cur);
			char * base = impl->index(row, i);
			const table_t ** obj = (const table_t **)(base);
			off = func(obj);	
		} else if (f.type() == tft_binpt) {
			bin_to_buff func(cur);
			char * base = impl->index(row, i);
			const char ** obj = (const char **)(base);
			off = func(obj);	
		}
		else{
			memcpy(cur, get_buff(row, i), f.width());
			off =  f.width();			
		}
		cur += off;
	}
}
size_t table_t::row_size(size_t row) const
{
	size_t s = width();
	size_t n = 0;
	size_t ptrsize = 0;
	for(size_t i = 0; i < cols(); i++) {
		const filed_t & f = impl->schema_p->filed(i);
		if (f.type() == tft_str) {
			char * base = impl->index(row, i);
			const char ** obj = (const char **)(base);
			s += str_size(obj);
			ptrsize += 8;
		} else if (f.type() == tft_table) {
			char * base = impl->index(row, i);
			const table_t ** obj = (const table_t **)(base);
			s += table_size(obj);	
			s += 4;  //contain length own
			ptrsize += 8;
		} else if (f.type() == tft_binpt) {
			char * base = impl->index(row, i);
			const char ** obj = (const char **)(base);
			s +=  binpt_size(obj);	
			ptrsize += 8;
		}
	}
	//return s + strlen(schema()) + strlen(name()) + sizeof(uint32_t) + 2 - ptrsize;
	return s - ptrsize;  //not contain schema,name, rowNo, and 2 '\0'
}

size_t table_t::to_csv(const table_t * t, char * buff, size_t len)
{
	if (len < 1) {
		return 0;
	}

	size_t used = 0;
	size_t remain = len;
	const char * format =  NULL;
	int n = 0;

#define CSV_SEP "\t"

	for(size_t col = 0; col < t->cols(); col++) {
		const char * colname = t->col_name(col);
		format = col == 0 ? "%s" : CSV_SEP"%s";
		int n = _snprintf_s(buff + used, len - used, remain, format, colname);
		if (n >= 0) {
			used += n;
			remain -= n;
		} else {
			return used;
		}
		if (used >= len) {
			return used;
		}
	}

	if (remain > 0) {
		*(buff + used) = '\n';
		used += 1;
		remain -=1;
	} else {
		return used;
	}

	for(size_t row = 0; row < t->rows(); row++) {
		for(size_t col = 0; col < t->cols(); col++) {
			const filed_t & f = t->impl->schema_p->filed(col);
			switch(f.type()) {
			case tft_null:
				format = col == 0 ? "(empty)%s" : CSV_SEP"(empty)%s";
				n = _snprintf_s(buff + used, len - used, remain, format, "");
				break;
			case tft_char: case tft_uchar:
				format = col == 0 ? "%s" : CSV_SEP"%s";
				n = _snprintf_s(buff + used, len - used, min(f.width() + 1, remain), format,
							  (const char*)t->get_buff(row, col));
				break;
			case tft_int: case tft_short:
				format = col == 0 ? "%d" : CSV_SEP"%d";
				n = _snprintf_s(buff + used, len - used, remain, format, t->get_int(row, col));
				break;
			case tft_uint: case tft_ushort:
				format = col == 0 ? "%d" : CSV_SEP"%d";
				n = _snprintf_s(buff + used, len - used, remain, format, t->get_uint(row, col));
				break;
			case tft_int64:
				format = col == 0 ? "%lld" : CSV_SEP"%lld";
				n = _snprintf_s(buff + used, len - used, remain, format, t->get_int64(row, col));
				break;
			case tft_uint64:
				format = col == 0 ? "%lld" : CSV_SEP"%lld";
				n = _snprintf_s(buff + used, len - used, remain, format, t->get_uint64(row, col));
				break;
			case tft_float:
				format = col == 0 ? "%f" : CSV_SEP"%f";
				n = _snprintf_s(buff + used, len - used, remain, format, t->get_float(row, col));
				break;
			case tft_double:
				format = col == 0 ? "%f" : CSV_SEP"%f";
				n = _snprintf_s(buff + used, len - used, remain, format, t->get_double(row, col));
				break;
			case tft_str:
				format = col == 0 ? "%s" : CSV_SEP"%s";
				n = _snprintf_s(buff + used, len - used, remain, format, t->get_str(row, col));
				break;
			case tft_table:
				format = col == 0 ? "(table)%s" : CSV_SEP"(table)%s";
				n = _snprintf_s(buff + used, len - used, remain, format, "");
				break;
			case tft_binpt:
				format = col == 0 ? "(binary)%s" : CSV_SEP"(binary)%s";
				n = _snprintf_s(buff + used, len - used, remain, format, "");
				break;
			case tft_timet:{
				format = col == 0 ? "%lld" : CSV_SEP"%lld";
				long long val = t->get_uint64(row, col);
				uint32_t sec = val >> 32;
				int msec = val & 0x00000000ffffffff;
				val = sec * 1000 + msec;
				n = _snprintf_s(buff + used, len - used, remain, format, val);
				break;}
			case tft_pfloat:		//同 table_getint64, 设置和返回 int64 即可
			case tft_vfloat:
				format = col == 0 ? "%lld" : CSV_SEP"%lld";
				n = _snprintf_s(buff + used, len - used, remain, format, t->get_int64(row, col));
				break;
			}
			if (n >= 0) {
				used += n;
				remain -= n;
			} else {
				return used;
			}
			if (used >= len) {
				return used;
			}
		}
		if (remain > 0) {
			*(buff + used) = '\n';
			used += 1;
			remain -=1;
		} else {
			return used;
		}
	}
	return used;
}

void table_t::fill( size_t drow, const table_t * other, size_t srow )
{
	uint32_t len = 0;
	size_t scols = other->cols();
	for(size_t scol = 0; scol < scols; scol++) {						//一个循环填充一行
		const char * key = other->col_name(scol);
		size_t dcol = this->col_index(key);								//以列名称为关键字，列序号可能不同
		if (dcol == (size_t)-1) {
			continue;													//如源Table中具有目的Table中没有的列，则忽略
		}
		if( (0==impl->nullflag_.size()) && (0<other->impl->nullflag_.size()) ){		//如果目的table没有空值标志表，源table有空值标志表
			size_t cellcount = rows() * cols();
			for(size_t i = 0; i < cellcount; i++) impl->nullflag_.push_back(false);
			impl->nullflag_[drow*cols()+dcol] = other->impl->nullflag_[srow*scols+scol];
		}else if( (0<impl->nullflag_.size()) && (0<other->impl->nullflag_.size()) ){	//如果目的table有空值标志表，  源table有空值标志表
			impl->nullflag_[drow*cols()+dcol] = other->impl->nullflag_[srow*scols+scol];
		}else if( (0<impl->nullflag_.size()) && (0==other->impl->nullflag_.size()) ){//如果目的table有空值标志表，  源table没有空值标志表
			impl->nullflag_[drow*cols()+dcol] = false;
		}																			//如果都没有，就不用干什么了。注意，这里的目的是本身，源是other
		size_t dcoltype = col_type(dcol);
		switch(dcoltype) {
		case tft_int: case tft_short:
			this->set_int(drow, dcol, other->get_int(srow, scol));
			break;
		case tft_int64:
			this->set_int64(drow, dcol, other->get_int64(srow, scol));
			break;
		case tft_uint: case tft_ushort:
			this->set_uint(drow, dcol, other->get_uint(srow, scol));
			break;
		case tft_uint64:
			this->set_uint64(drow, dcol, other->get_uint64(srow, scol));
			break;
		case tft_float:
			this->set_float(drow, dcol, other->get_float(srow, scol));
			break;
		case tft_double:
			this->set_double(drow, dcol, other->get_double(srow, scol));
			break;
		case tft_str:
			this->set_str(drow, dcol, other->get_str(srow, scol));
			break;
		case tft_table:
			this->set_table(drow, dcol, other->get_table(srow, scol));
			break;
		case tft_char: case tft_uchar:
			len = other->col_size(scol);
			this->set_buff(drow, dcol, other->get_buff(srow, scol), len, 0);
			break;
		case tft_binpt:
			len = other->get_binary_len(srow, scol);
			this->set_binary(drow, dcol, len, other->get_binary(srow, scol), false);
			break;
		case tft_timet:
			this->set_int64(drow, dcol, other->get_int64(srow, scol));
			break;
		case tft_pfloat:
		case tft_vfloat:
			this->set_int64(drow, dcol, other->get_int64(srow, scol));  //同 table_getint64, 设置和返回 int64 即可
			break;
		default:
			;//assert(false);
		}
	}
}


struct coldata_t {

	coldata_t() {}

	coldata_t(const void * data): data(data) {}

	coldata_t(const coldata_t & other): data(other.data) {}

	const void * data;
};


struct coldata_lesser{
	coldata_lesser(size_t width, size_t type): width(width), type(type) {}

	bool operator()(const coldata_t & d1, const coldata_t & d2) const{
		if (type == tft_str){
			const char * s1 = *(const char **)d1.data;
			const char * s2 = *(const char **)d2.data;
			return strcmp(s1, s2) < 0;
		}
		
		else{
			return memcmp(d1.data, d2.data, width) < 0;
		}
		
	}
	
	size_t width;
	size_t type;
};

struct coldata_equal{
	coldata_equal(size_t width, size_t type): width(width), type(type) {}

	bool operator()(const coldata_t & d1, const coldata_t & d2) const{
		if (type == tft_str){
			const char * s1 = *(const char **)d1.data;
			const char * s2 = *(const char **)d2.data;
			return strcmp(s1, s2) == 0;
		}
		else if(type == tft_binpt)
		{
			const char * s1 = *(const char **)d1.data;
			const char * s2 = *(const char **)d2.data;
			int *l = (int *)s1;
			int len = *l;
			return memcmp(s1, s2, len) == 0;
		}
		else{
			return memcmp(d1.data, d2.data, width) == 0;
		}
		
	}
	
	size_t width;
	size_t type;
};

typedef std::map<coldata_t, int, coldata_lesser> colindex_t;


void build_colindex(const table_t * t, size_t col, colindex_t & index)
{
	for(size_t row = 0; row < t->rows(); row++){
		index[coldata_t(t->get_buff(row, col))] = row;
	}	
}

size_t find_col(colindex_t & index, const void * data, coldata_equal & compare_func){
	colindex_t::iterator where = index.lower_bound(data);
	if (where != index.end()){
		if (compare_func(data, where->first))
			return where->second;
	}
	return (size_t)(-1);
}


void table_t::update(const table_t * other, const char * keycol)
{
	size_t dcol = this->col_index(keycol);
	size_t scol = other->col_index(keycol);
	if (dcol == (size_t)(-1) || scol == (size_t)(-1)){
		return;
	}

	size_t colsize = this->col_size(dcol);
	size_t coltype = this->col_type(dcol);
	coldata_lesser lesser(colsize, coltype);
	coldata_equal  equal (colsize, coltype);


	colindex_t index(lesser);
	build_colindex(this, dcol, index);

	for(size_t srow = 0; srow < other->rows(); srow++){
		size_t drow = find_col(index, other->get_buff(srow, scol), equal);			//找到对应行
		if (drow != (size_t)(-1)){
			this->fill(drow, other, srow);
		}
	}
}

table_t * table_t::clone() const
{
	table_t * dest = table_t::create(schema(), rows(), name());
	size_t width = this->width();

	if( is_tidy() && (impl->init_size_ == impl->datas_.size()) )  //既没插入（增加）也没减少过，
	{
		memcpy(dest->impl->buff_, this->impl->buff_, this->impl->init_size_ * width);
	}
	else
	{
		for(size_t r = 0; r < rows(); r++){						//从0开始，经过插入删除后，全部遍历一遍更合适
			char * d = (char*)dest->get_buff(r, 0);
			const char * s = (const char*)get_buff(r, 0);
			memcpy(d, s, width);
		}
	}

	if (!dest->is_pod()) {
		for(size_t j = 0; j < dest->rows(); j++) {
			for(size_t i = 0; i < dest->cols(); i++) {
				const filed_t & f = dest->impl->schema_p->filed(i);
				if (f.type() == tft_str) {
					dest->set_str(j, i, get_str(j, i), false);
				} else if (f.type() == tft_table) {
					dest->set_table(j, i, get_table(j, i), false);
				} else if(f.type() == tft_binpt)  {
					int len = get_binary_len(j, i);
					dest->set_binary(j, i, len, get_binary(j, i), false);
				}
			}
		}
	}
	if(0 < impl->nullflag_.size()){		//克隆 空值标志表,而且dest是新建出来的，dest->impl->nullflag_肯定是空的，不用再判断
		size_t count = rows() * cols();
		for(size_t i=0; i < count; i++){
			dest->impl->nullflag_.push_back(impl->nullflag_[i]);
		}
	}
	//克隆属性域。在create过程中，空间已经分配
	for(size_t i=0; i < cols()+1; i++){
		_Custom_Attrib &cell_src = impl->attrib_[i];
		_Custom_Attrib &cell_dst = dest->impl->attrib_[i];
		for(size_t j=0;j < 8;j++){
			char * key = cell_src.spair[j].skey;	char * val = cell_src.spair[j].sval;
			if(NULL != key){
				size_t len = strlen(key);
				char * skey = new char[len+1];		strcpy_s(skey, len+1, key);
				len = strlen(val);
				char * sval = new char[len+1];		strcpy_s(sval, len+1, val);
				cell_dst.spair[j].skey = skey;		cell_dst.spair[j].sval = sval;
			}
			cell_dst.npair[j].nkey = cell_src.npair[j].nkey;
			cell_dst.npair[j].nval = cell_src.npair[j].nval;
		}
	}
	return dest;
}

void  get_select_schema( const table_t * t, const char * fields, char ** selectschema)
{
	string fields_ = fields;
	string subschema_;
	string field;
	string schema_ =  t->schema();

	//loop paser
	size_t s = 0;
	size_t e = 0;
	size_t i = 0;

	while(e < fields_.length()) {
		if (fields_[e] == ',') {			//发现分割符，开始进一步处理
			field = fields_.substr(s, e-s);	//取一个子串作为列名
			field +=":";
			size_t s1 = 0;
			size_t e1 = 0;
			s1 = schema_.find(field);
			if(s1 == string::npos)
				;
			else{
				e1 = schema_.find(';',s1);
				if(e1 == string::npos)
					e1 = schema_.length();
				subschema_ = subschema_ + schema_.substr(s1,(e1-s1));
				subschema_ = subschema_ + ";";
			}
			s = ++e;	//指向下一个字段首
		}
		e++;		//逐字节循环
	}
	if ( (s < e) && (s < fields_.length()) ) {	//最后一个字段的处理，后面直接是结尾，无分号
			field = fields_.substr(s, e-s);	
			size_t s1 = 0;
			size_t e1 = 0;
			s1 = schema_.find(field);
			if(s1 == string::npos)
				;
			else{
				e1 = schema_.find(';',s1);
				if(e1 == string::npos)
					e1 = schema_.length();
				subschema_ = subschema_ + schema_.substr(s1,(e1-s1));
			}
	}


	//get buff
	*selectschema = table_strnew(subschema_.length()+1);
	strcpy_s(*selectschema,subschema_.length()+1,subschema_.c_str());

	return;
}

table_t * table_t::select(const char * fields, size_t from, size_t count) const
{
	//反相求出所选子集schema
	char * selectschema = NULL;
	get_select_schema(this, fields, &selectschema);

	//创建新的table
	size_t end = from + count;
	if(end > rows())	{
		end = rows();
		count = end - from;
	}
	table_t * t = create(selectschema, count, "");

	//逐行逐列填充
	size_t i;
	for(i = from;i < end; i++)
		t->fill(i-from, this, i);

	//复制属性信息
	t->fill_attribute(this);

	table_strdel(selectschema);
	return t;
}

//列为两个源table列的并集去除重复
void get_join_schema(table_t * t, table_t * other, char ** joinschema)
{
	static const char * types = " chiqfdstCHIQbTpv";

	string schema_ =  t->schema();										//以table1的schema为基础
	size_t scols = other->cols();
	string subschema;
	basic_string <char>::reverse_iterator str_rIter = schema_.rbegin();
	if( *str_rIter != ';') subschema = ";";							//如table1的schema末尾不是分隔符，则新增第一个列字段前加一个分隔符

	bool colextend = false;
	for(size_t scol = 0; scol < scols; scol++) {						//循环,取table2的每一个列
		const char * key = other->col_name(scol);
		size_t dcol = t->col_index(key);								//以列名称为关键字，到table1中查找，
		if (dcol != (size_t)-1) {
			continue;													//如源Table中具有目的Table中相同的列，则跳过，不做添加
		}
		if(colextend)		subschema += ";";							//新增第二个及以后列字段前追加一个分隔符
		colextend = true;
		size_t scoltype = other->col_type(scol);								//并集添加
		size_t scolcount = other->col_count(scol);

		char *buff = new char [strlen(key)+10];							//包含':'等10个字节足够了
		if(scolcount > 1)
			sprintf_s(buff, (strlen(key)+10), "%s:%d%c",key,(int)scolcount,types[scoltype]);
		else
			sprintf_s(buff, (strlen(key)+10), "%s:%c",key,types[scoltype]);
		subschema = subschema + buff;
		delete buff;
	}
	if(colextend)
		schema_ += subschema;
		
	//get buff
	*joinschema = table_strnew(schema_.length()+1);
	strcpy_s(*joinschema, schema_.length()+1, schema_.c_str());

}

table_t * table_t::join(table_t * other, const char * keycol)	//left join
{
	if(other == NULL ) return NULL;

	//判断keycol类型是否一致
	if( this ->col_type(this->col_index(keycol)) != other->col_type(other->col_index(keycol)) )	return NULL;

	//可以有非同名列，但两个table的同名列，数据类型必须相同,如有不同为非法，返回空指针
	size_t othercols  = other->cols();
	for(size_t col =0 ; col < othercols ; col++)
	{
		size_t othertype = other->col_type(col);
		const char * name = other->col_name(col);
		size_t thiscol = this->col_index(name);
		if (thiscol != (size_t)(-1)){
			size_t thistype = this->col_type(thiscol);
			if(thistype != othertype) return NULL;
		}
	}

	//对left table建立一个有序序列对象index，供后面程序使用以提高效率
	size_t dcol = this->col_index(keycol);
	size_t colsize = this->col_size(dcol);		
	size_t coltype = this->col_type(dcol);
	coldata_lesser lesser(colsize, coltype);		
	colindex_t index(lesser);
	build_colindex(this, dcol, index);

	//获取join后的schema
	size_t drows = this->rows();
	char * joinschema = NULL;
	get_join_schema(this, other, &joinschema);
	table_t * dest = table_t::create(joinschema, drows, "");
	table_strdel(joinschema);

	//循环向目标table中写入左侧table的数据。
	for(size_t srow = 0; srow < this->rows(); srow++){
		dest->fill(srow, this, srow);
	}

	//循环遍历右侧table的所有行，关键列相同行合并到目标table中的指定行中
	size_t scol = other->col_index(keycol);	
	coldata_equal  equal (colsize, coltype);		
	for(size_t srow = 0; srow < other->rows(); srow++){
		size_t drow = find_col(index, other->get_buff(srow, scol), equal);			//找到对应行
		if (drow != (size_t)(-1)){
			dest->fill(drow, other, srow);
		}
	}

	//分别从左右table复制属性信息,先右后左，table属性取左table的属性
	dest->fill_attribute(other);dest->fill_attribute(this);

	return dest;
}
#if 0
table_t * table_t::join(table_t * other, const char * keycol)
{
	if(other == NULL ) return NULL;

	//判断keycol类型是否一致
	if( this ->col_type(this->col_index(keycol)) != other->col_type(other->col_index(keycol)) )	return NULL;

	size_t col1 = this ->col_index(keycol);
	size_t col2 = other->col_index(keycol);
	size_t colsize = this->col_size(col1);		//
	size_t coltype = this->col_type(col1);

	coldata_lesser lesser(colsize, coltype);		//
	coldata_equal  equal (colsize, coltype);		//

	//判断有两个table中有多少个相同的，然后确定新建table的行数和合并的列数
	colindex_t index(lesser);
	build_colindex(this, col1, index);												//index绑定到table1变量上
	size_t repeatrow = 0;
	for(size_t srow = 0; srow < other->rows(); srow++){
		size_t drow = find_col(index, other->get_buff(srow, col2), equal);			//在table2中找到和table1中keycol数据相同的行
		if (drow != (size_t)(-1)){
			repeatrow++;
		}
	}
	size_t drows = this ->rows()+other->rows()-repeatrow;

	char * joinschema = NULL;
	get_join_schema(this, other, &joinschema);
	table_t * dest = table_t::create(joinschema, drows, "");
	table_strdel(joinschema);

	//向新table中，以keycol内容升序，从两个table中取数据写入
	colindex_t index2(lesser);
	build_colindex(other, col2, index2);												//index2绑定到table2变量上
	colindex_t::iterator it1 = index.begin();
	colindex_t::iterator it2 = index2.begin();
	for(size_t drow = 0;drow < drows;drow++)
	{
		if(it1 == index.end() && it2 != index2.end()){
			dest->fill(drow, other, it2->second);		//如果table1已取完而table2尚未取完，取用table2的数据，
			it2++;continue;
		}else if(it1 != index.end()&& it2 == index2.end()){
			dest->fill(drow, this, it1->second);		//如果table2已取完而table1尚未取完，取用table1的数据，
			it1++;continue;
		}
		//
		if( equal(it1->first, it2->first) ){
			dest->fill(drow, this, it1->second);		//如果相等，写入数据，默认相同列取table2的数据
			dest->fill(drow, other, it2->second);		//
			it1++;it2++;
		}else if( lesser(it1->first,it2->first) ){
			dest->fill(drow, this, it1->second);		//如果table1数据值小，取用table1的数据，
			it1++;
		}else{
			dest->fill(drow, other, it2->second);		//如果table1数据值大，取用table2的数据，
			it2++;
		}

	}
	return dest;
}
#endif
table_t * table_t::sort(field_compare_func comparer, void * userdata)
{
	if((long)comparer >= TABLE_MAX_COMPARER) return NULL;

	field_compare_func comparefunc = fieldfunc[(long)comparer];

	size_t count = this->rows();
	
	typedef struct {
		size_t rowNo;
		size_t lesscount;
	}record;

	record rec;
	record * result = new record[count];

	//统计每一行和其他所有行相比各有多少行比它小，可能有相等行；不改变table行序，得到一个结构数组
	int lesscount = 0;
	for(size_t i = 0; i< count;i++)
	{
		lesscount = 0;
		for(size_t j = 0;j < count;j++){
			if(1 == comparefunc(this, i, j,userdata))	//
				lesscount++;
		}
		rec.rowNo = i; rec.lesscount = lesscount;	//比i行小的总个数是lesscount,
		result[i]=rec;	
	}
	//使用冒泡法，两层循环，对结构数组按排序
	for(size_t i = 0; i< count;i++)
	{
		for(size_t j = i+1;j < count;j++){
			if( result[i].lesscount > result[j].lesscount ){	//j行比i行小，返回非0值
				rec = result[i];
				result[i] = result[j];
				result[j] = rec;
			}
		}
	}

	//新建table，将结果数据写入，并返回
	table_t * dest = table_t::create("sn:i", count, "");
	for(size_t i = 0; i< count;i++)
	{
		int nVal = result[i].rowNo;
		dest->set_int(i, 0, nVal);
	}

	delete [] result;
	return dest;
}

typedef struct _group {
	const void * data;
	std::vector <size_t> lineNo;
	struct _group  * next;
} GroupNode;

table_t * table_t::group(const char * field) const
{
	size_t col = this ->col_index(field);
	size_t colsize = this->col_size(col);		
	size_t coltype = this->col_type(col);
	coldata_equal  equal (colsize, coltype);		//比较对象，通过它来分组

	GroupNode * head, * tail;
	GroupNode *p;

	const void * data, * datanext;

	//遍历整个table 得到一个链表，动态分配内存，每个节点保存列内容和具有相同列内容的行的行号
	data = this->get_buff(0, col);
	p = new GroupNode;
	p->data = data;p->lineNo.push_back(0);p->next = NULL;
	head = p; tail = p;
	size_t newcount = 1;

	for(size_t i = 1;i < rows();i++)
	{
		datanext = get_buff(i, col);
		p = head; 
		while(p)
		{
			data = p->data;
			if(equal(data,datanext)){
				p->lineNo.push_back(i);
				break;	//在链表中找到相同的, 则在该节点增加一个行号。退出while循环，仍在for循环中
			}
			p = p->next;
		}
		if(p == NULL)//在链表中没有找到相同的，则在tail后面增加一个节点
		{
			p = new GroupNode;
			p->data = datanext; p->lineNo.push_back(i);p->next = NULL;
			tail->next = p;	tail = p;
			newcount++;
		}
	}

	//分析schema,得到新的schema，然后创建新table.
	string newschema;
	static const char * types = " chiqfdstCHIQbTpv";
	string fieldcount = "";
	for(size_t i = 0; i < col;i++)
	{
		string newfield;

		newfield = col_name(i);
		newfield += ":5";
		newfield += "d";	//统一为double型，比如平均数可能有小数。
		newfield += ";";
		newschema += newfield;
	}

	{
		int bufflen = strlen(col_name(col))+10;
		char *buff = new char [bufflen];							//包含':'等10个字节足够了
		size_t scolcount = col_count(col);
		if(scolcount > 1)
			sprintf_s(buff, bufflen, "%s:%d%c;",col_name(col),(int)scolcount,types[col_type(col)]);
		else
			sprintf_s(buff, bufflen,"%s:%c;",col_name(col),types[col_type(col)]);
		newschema = newschema + buff;
		delete buff;
	}
	for(size_t i = col+1; i < cols();i++)
	{
		string newfield;
		newfield = col_name(i);
		newfield += ":5";
		newfield += "d";
		newfield += ";";
		newschema += newfield;
	}
	newschema += "_ROWS_:t";
	table_t * dest = table_t::create(newschema.c_str(), newcount, "");

	//遍历链表，逐个处理计算(count, sum, avg, max, min) 
	p = head;size_t row = 0;
	while(p){
		//写keycol列
		const void * data = p->data;
		size_t dcol = dest->col_index(field);
		if(tft_str == coltype)
			dest->set_str(row, col, *(const char **)data, false);	//如果是字符串，内存中缓存的是地址，要按地址在取一次，与equal同理
		else if( (tft_table == coltype) || (tft_binpt == coltype) )
			return NULL;		//这两种类型，不能分组。
		else
			dest->set_buff(row, dcol, data, colsize, 0);	//注意colsize 是特别针对keycol列的。
		table_t * rowtable = table_t::create("sn:i",p->lineNo.size(),"");

		//之前列和之后列,顺序已定，这里可以合并处理
		for(size_t i = 0; i < cols();i++)
		{
			if( i == col ) continue;	//keycol
			const char * name = col_name(i);
			dcol = dest->col_index(name);
			double count=0, sum=0, avg=0, max=0, min;
			for(size_t j = 0;j < p->lineNo.size();j++)
			{
				switch(col_type(i))
				{
				case tft_int:
					{
					int val = get_int(p->lineNo[j],i);
					if( 0 == count)
						max = min = val;
					else
						{	max = (max > val)?max:val;	min = (min < val)?min:val;	}
					sum += val;
					count++;
					avg = (double)sum / count; 
					}
					break;
				case tft_int64:
					{
					long long val = get_int64(p->lineNo[j],i);
					if( 0 == count)
						max = min = (double)val;
					else
						{	max = (max > val)?max:val;	min = (min < val)?min:val;	}
					sum += val;
					count++;
					avg = (double)sum / count; 
					}
					break;
				case tft_float:
					{
					float val = (float)get_float(p->lineNo[j],i);
					if( 0 == count)
						max = min = val;
					else
						{	max = (max > val)?max:val;	min = (min < val)?min:val;	}
					sum += val;
					count++;
					avg = (double)sum / count; 
					}
					break;
				case tft_double:
					{
					double val = get_double(p->lineNo[j],i);
					if( 0 == count)
						max = min = val;
					else
						{	max = (max > val)?max:val;	min = (min < val)?min:val;	}
					sum += val;
					count++;
					avg = (double)sum / count; 
					}
					break;
				default:		//其他类型，tft_char,tft_string,tft_table
					{
						max = min = sum = avg = count = (double)0;
					}
				}//end switch
				
			}//end for j p->lineNo
			dest->set_buff(row, dcol, (void *)&count, sizeof(double), 0);
			dest->set_buff(row, dcol, (void *)&sum,   sizeof(double), sizeof(double));
			dest->set_buff(row, dcol, (void *)&avg,   sizeof(double), 2*sizeof(double));
			dest->set_buff(row, dcol, (void *)&max,   sizeof(double), 3*sizeof(double));
			dest->set_buff(row, dcol, (void *)&min,   sizeof(double), 4*sizeof(double));
		}//end for i , col
		
		for(size_t j = 0;j < p->lineNo.size();j++)
		{
			rowtable->set_int(j,0,p->lineNo[j]);
		}
		dcol = dest->col_index("_ROWS_");
		dest->set_table(row, dcol, rowtable);
		destory(&rowtable);		//对于字符串，子table，变长二进制串，在写入table时，复制了一个原内容，并保存指向复制品的指针，所以原应该被释放或销毁
		p = p->next;row++;	//这里的一个节点，对应结果table的一行。
	}

	//删除链表，释放内存，其中data指向的是table数据的内存地址，不需要释放，lineNo是vector，需要clear.
	p = head;
	while(p){
		GroupNode *q = p->next;
		p->lineNo.clear();
		delete p;
		p = q;
	}

	return dest;
}

typedef struct CompareType{
	const char * colname;
	void * value;
}USERCOMPARETYPE, * PUSERCOMPARETYPE;

table_t * table_t::filter(field_compare_func comparer, void * userdata)
{
	size_t count = this->rows();
	vector <size_t> result;

	if((long)comparer >= TABLE_MAX_COMPARER) return NULL;

	value_compare_func comparefunc = valuefunc[(long)comparer];
	USERCOMPARETYPE * data = (PUSERCOMPARETYPE)userdata;

	size_t col = col_index(data->colname);
	//扫描每一行，如满足条件则将该行行号写入新table
	for(size_t i = 0; i< count;i++)
	{
		if(0 == comparefunc(this, i, col, data->value))	//comparer函数功能是：该行的特定列
			result.push_back(i);	
	}

	//新建table，准备将数据写入，并返回
	table_t * dest = table_t::create("sn:i", result.size(), "");
	for(size_t i = 0; i< result.size();i++)
	{
		dest->set_int(i, 0,result[i]);
	}
	return dest;
}

table_t * table_t::movecolsname(const char * srcColsName, const char * dstColsName)
{
	//基本思路：扫描修改schema_字符串，然后对schema_再进行parse

	//将原和目的列名串拆解到各自的容器中，如数量不同则报错返回NULL
	string src = srcColsName;
	string dst = dstColsName;
	vector<string>	vecSrcCol;
	vector<string>  vecDstCol;

	//loop paser
	size_t s = 0;
	size_t e = 0;
	size_t i = 0;
	string field;

	while(e < src.length()) {
		if (src[e] == ',') {			
			field = src.substr(s, e-s);	
			vecSrcCol.push_back(field);
			s = ++e;	
		}
		e++;		
	}
	if ( (s < e) && (s < src.length()) ) {	//
			field = src.substr(s, e-s);	
			vecSrcCol.push_back(field);
	}

	s = 0;e = 0;
	while(e < dst.length()) {
		if (dst[e] == ',') {			
			field = dst.substr(s, e-s);	
			vecDstCol.push_back(field);
			s = ++e;	
		}
		e++;		
	}
	if ( (s < e) && (s < dst.length()) ) {	//
			field = dst.substr(s, e-s);	
			vecDstCol.push_back(field);
	}
	if(vecSrcCol.size() != vecDstCol.size())	return NULL;

	//调整顺序,匹配schema_的顺序
	for(int i=vecSrcCol.size(); i>0; i--)
	{
		for(int j = 0; j<i-1;j++){
			s = col_index(vecSrcCol[j].c_str());
			e = col_index(vecSrcCol[j+1].c_str());
			if(s > e){
				field = vecSrcCol[j];
				vecSrcCol[j] = vecSrcCol[j+1];
				vecSrcCol[j+1] = field;
				field = vecDstCol[j];
				vecDstCol[j] = vecDstCol[j+1];
				vecDstCol[j+1] = field;
			}
		}
	}

	//循环，形成新的schema_字符串
	string schema_ =  schema();
	string newschema;		newschema.clear();
	s = 0; e = 0;
	for(unsigned int i=0; i<vecSrcCol.size(); i++)
	{
		field = vecSrcCol[i];
		field += ":";
		e = schema_.find(field);
		if(e == string::npos)
			return NULL;
		else{
			newschema += schema_.substr(s,e-s);
			newschema += vecDstCol[i];
			s = e+vecSrcCol[i].length();
		}
	}
	e = schema_.length();
	newschema += schema_.substr(s,e-s);

	//清空原数据，重新解析schema_字符串
	//impl->schema_.parse(newschema.c_str());
	impl->schema_p = impl->create_table_schema(newschema.c_str());

	return this; //
}


schema_t * table_impl::create_table_schema(const char * schema)
{
	//SCHEMABUFF temp;
#ifdef WIN32
	EnterCriticalSection(&cs_);
#else
#endif
	//第一步：查找是否有相同的串，如有返回解析结果指针
	//bool bInbuff = false;
	//for(int i = 0; i < g_vecSchemaBuff.size();i++)
	//{
	//	temp = g_vecSchemaBuff[i];
	//	bInbuff = !strcmp(schema, temp.schema_string);
	//	if(bInbuff)
	//		return temp.schema_struct;
	//}

	string sch_ = schema;
	map<string,schema_t *>::iterator iter=g_mapSchemaBuff.find(sch_);
	if(iter != g_mapSchemaBuff.end())
	{
		schema_t * temp = iter->second;
	#ifdef WIN32
		LeaveCriticalSection(&cs_);
	#else
	#endif
		return temp;
	}

	//第二步：(新的schema),new 出schema_t对象，并在构造中解析
	schema_t * sc = new schema_t(schema);

	//第三步：schema串指针和解析结果指针，保存到结构vector中
	//int iLen = strlen(schema)+1;
	//char * str = new char[iLen];
	//memset(str, 0, iLen);	strcpy(str, schema);
	//temp.schema_string = str;temp.schema_struct = sc;
	//g_vecSchemaBuff.push_back(temp);
	g_mapSchemaBuff.insert(make_pair(sch_, sc));

	//返回：解析结果指针。
#ifdef WIN32
	LeaveCriticalSection(&cs_);
#else
#endif
	return sc;
}

void clean_schema_pool(void)
{
	//for(int i = 0; i < g_vecSchemaBuff.size();i++)
	//{
	//	SCHEMABUFF temp = g_vecSchemaBuff[i];
	//	delete [] temp.schema_string;
	//	//delete temp.schema_struct;
	//}
	//g_vecSchemaBuff.clear();

	map<string,schema_t *>::iterator iter;
	for ( iter = g_mapSchemaBuff.begin( ); iter != g_mapSchemaBuff.end( ); iter++ )
		delete iter->second;
	g_mapSchemaBuff.clear();

#ifdef WIN32 
	DeleteCriticalSection(&cs_);
#else
#endif
}

//写入或判断无效数据，比如1日1价商品，的高开低收。
#if 0
bool table_t::col_isnull(size_t row, size_t col)
{
	bool ret = false;
	switch(col_type(col))
	{
	case tft_char:
		ret = (impl->get<char>(row, col)== 0x80);
		break;
	case tft_short:
		ret = (impl->get<short>(row, col)== 0x8000);
		break;
	case tft_int:
		ret = (impl->get<long int>(row, col) == 0x80000000);
		break;
	case tft_int64:
		ret = (impl->get<long long>(row, col) == 0x8000000000000000);
		break;
	case tft_float:
		ret = (impl->get<float>(row, col) == 0xff800000);//最左边一位（bit 31）表示符号（正负），接下来8位(bit 30 - 23)表示指数，剩下23位(bit 22 - 0)表示数值
		break;
	case tft_double:
		ret = (impl->get<double>(row, col) == 0xfff0000000000000);//double 符号1 指数 11 尾数 52
		break;
	case tft_uchar:
		ret = (impl->get<unsigned char>(row, col) == 0xff);
		break;
	case tft_ushort:
		ret = (impl->get<unsigned short>(row, col) == 0xffff);
		break;
	case tft_uint:
		ret = (impl->get<unsigned int>(row, col) == 0xffffffff);
		break;
	case tft_uint64:
		ret = (impl->get<unsigned long long>(row, col) == 0xffffffffffffffff);
		break;
	case tft_timet:
		ret = (impl->get<long long>(row, col) == 0x8000000000000000);
		break;
	default:
		ret = (impl->get<char *>(row, col) == NULL);//address of point is NULL
	}
	return ret;
}

void table_t::col_writenull(size_t row, size_t col)
{
	switch(col_type(col))
	{
	case tft_char:
		impl->set(row, col, (char)0x80, nan_delete<char>);
		break;
	case tft_short:
		impl->set(row, col, (short)0x8000, nan_delete<short>);
		break;
	case tft_int:
		impl->set(row, col, (int)0x80000000, nan_delete<int>);
		break;
	case tft_int64:
		impl->set(row, col, (long long)0x8000000000000000, nan_delete<long long>);
		break;
	case tft_float:
		impl->set(row, col, (float)0xff800000, nan_delete<float>);//float 符号1 指数 8 尾数 23
		break;
	case tft_double:
		impl->set(row, col, (double)0xfff0000000000000, nan_delete<double>);//double 符号1 指数 11 尾数 52
		break;
	case tft_uchar:
		impl->set(row, col, (unsigned char)0xff, nan_delete<unsigned char>);
		break;
	case tft_ushort:
		impl->set(row, col, (unsigned short)0xffff, nan_delete<unsigned short>);
		break;
	case tft_uint:
		impl->set(row, col, (unsigned int)0xffffffff, nan_delete<unsigned int>);
		break;
	case tft_uint64:
		impl->set(row, col, (unsigned long long)0xffffffffffffffff, nan_delete<unsigned long long>);
		break;
	case tft_timet:
		impl->set(row, col, (long long)0x8000000000000000, nan_delete<long long>);
		break;
	case tft_table:
		{
		table_t * valt = NULL;
		impl->set(row, col, valt, table_delete);//address of point is NULL
		break;
		}
	case tft_str:
	case tft_binpt:
		{
		char * vals = NULL;
		impl->set(row, col, vals, str_delete);//address of point is NULL
		break;
		}
	default:
		/*assert(0)*/;
	}
}
#endif
bool table_t::col_isnull(size_t row, size_t col)
{
	if(0 == impl->nullflag_.size())
		return false;
	else{
		size_t index =  (cols() * row + col);
		return impl->nullflag_[index];
	}
}
void table_t::col_writenull(size_t row, size_t col)
{
	if(0 == impl->nullflag_.size())
	{
		size_t count = rows() * cols();
		for(size_t i=0; i<count; i++){
			impl->nullflag_.push_back(false);
		}
	}
	size_t index =  (cols() * row + col);
	impl->nullflag_[index] = true;	
}
void table_t::remove(size_t row, size_t num)
{
	impl->remove_row(row, num);
}
void table_t::insert(size_t row, size_t num)
{
	impl->insert_row(row, num);
}

	//custom attributes
bool table_t::getkv_string(const char* key, char* val, size_t val_size)
{
	_Custom_Attrib &tab_attrib = impl->attrib_[0];	//
	for(size_t i=0;i<8;i++){
		if(NULL == tab_attrib.spair[i].skey) break;
		if(0 == strcmp(key, tab_attrib.spair[i].skey)){
			strcpy_s(val, val_size, tab_attrib.spair[i].sval);
			return true;
		}
	}
    return false;
}
bool table_t::setkv_string(const char* key, const char* val)
{
	_Custom_Attrib &tab_attrib = impl->attrib_[0];	//
	for(size_t i=0;i<8;i++){
		if(NULL == tab_attrib.spair[i].skey) {						//new attribute
			size_t len = strlen(key);
			char * skey = new char[len+1];
			strcpy_s(skey, len+1, key);
			len = strlen(val);
			char * sval = new char[len+1];
			strcpy_s(sval, len+1, val);
			tab_attrib.spair[i].skey = skey;
			tab_attrib.spair[i].sval = sval;
			return true;
		}
		else if(0 == strcmp(key, tab_attrib.spair[i].skey)){		//replace
			char * sval = tab_attrib.spair[i].sval;
			delete sval;	sval=NULL;
			size_t len = strlen(val);
			sval = new char[len+1];
			strcpy_s(sval, len+1, val);
			tab_attrib.spair[i].sval = sval;
			return true;
		}
	}
	return false;
}
bool table_t::col_getkv_string(size_t col, const char* key, char* val, size_t val_size)
{
	_Custom_Attrib &cell_attrib = impl->attrib_[col+1];	//
	for(size_t i=0;i<8;i++){
		if(NULL == cell_attrib.spair[i].skey) break;
		if(0 == strcmp(key, cell_attrib.spair[i].skey)){
			strcpy_s(val, val_size, cell_attrib.spair[i].sval);
			return true;
		}
	}
    return false;
}
bool table_t::col_setkv_string(size_t col, const char* key, const char* val)
{
	_Custom_Attrib &cell_attrib = impl->attrib_[col+1];	//
	for(size_t i=0;i<8;i++){
		if(NULL == cell_attrib.spair[i].skey) {						//new attribute
			size_t len = strlen(key);
			char * skey = new char[len+1];
			strcpy_s(skey, len+1, key);
			len = strlen(val);
			char * sval = new char[len+1];
			strcpy_s(sval, len+1, val);
			cell_attrib.spair[i].skey = skey;
			cell_attrib.spair[i].sval = sval;
			return true;
		}
		else if(0 == strcmp(key, cell_attrib.spair[i].skey)){		//replace
			char * sval = cell_attrib.spair[i].sval;
			delete sval;	sval=NULL;
			size_t len = strlen(val);
			sval = new char[len+1];
			strcpy_s(sval, len+1, val);
			cell_attrib.spair[i].sval = sval;
			return true;
		}
	}
	return false;
}
bool table_t::setkv_value(int key, int val)
{
	_Custom_Attrib &tab_attrib = impl->attrib_[0];	//
	for(size_t i=0;i<8;i++){
		if(0 == tab_attrib.npair[i].nkey){
			tab_attrib.npair[i].nkey = key;
			tab_attrib.npair[i].nval = val;
			return true;
		}else if(key == tab_attrib.npair[i].nkey){
			tab_attrib.npair[i].nval = val;
			return true;
		}
	}
	return false;
}
bool table_t::getkv_value(int key, int * val)
{
	_Custom_Attrib &tab_attrib = impl->attrib_[0];	//
	for(size_t i=0;i<8;i++){
		if(0 == tab_attrib.npair[i].nkey) break;
		if(key == tab_attrib.npair[i].nkey){
			*val = tab_attrib.npair[i].nval;
			return true;
		}
	}
	return false;
}
bool table_t::col_setkv_value(size_t col, int key, int val)
{
	_Custom_Attrib &cell_attrib = impl->attrib_[col+1];	//
	for(size_t i=0;i<8;i++){
		if(0 == cell_attrib.npair[i].nkey){
			cell_attrib.npair[i].nkey = key;
			cell_attrib.npair[i].nval = val;
			return true;
		}else if(key == cell_attrib.npair[i].nkey){
			cell_attrib.npair[i].nval = val;
			return true;
		}
	}
	return false;
}
bool table_t::col_getkv_value(size_t col, int key, int * val)
{
	_Custom_Attrib &cell_attrib = impl->attrib_[col+1];	//
	for(size_t i=0;i<8;i++){
		if(0 == cell_attrib.npair[i].nkey) break;
		if(key == cell_attrib.npair[i].nkey){
			*val = cell_attrib.npair[i].nval;
			return true;
		}
	}
	return false;
}
const size_t table_t::kv_string_len( const char* key) 
{
	size_t len = 0;
	_Custom_Attrib &tab_attrib = impl->attrib_[0];	//
	for(size_t i=0;i<8;i++){
		if(NULL == tab_attrib.spair[i].skey) break;
		if(0 == strcmp(key, tab_attrib.spair[i].skey)){
			len = strlen( tab_attrib.spair[i].sval );
			return (len+1);		//contain 0 in string tail
		}
	}
	return len;
}
const size_t table_t::col_kv_string_len( size_t col, const char* key )
{
	size_t len = 0;
	_Custom_Attrib &cell_attrib = impl->attrib_[col+1];	//
	for(size_t i=0;i<8;i++){
		if(NULL == cell_attrib.spair[i].skey) break;
		if(0 == strcmp(key, cell_attrib.spair[i].skey)){
			len = strlen( cell_attrib.spair[i].sval );
			return (len+1);		//contain 0 in string tail
		}
	}
	return len;
}
void table_t::fill_attribute(const table_t * src)
{
	//内存空间在Crete时已准备好，在这里按原列目标列对应复制就可以了。

	//table attrib
	_Custom_Attrib &tab_attrib = src->impl->attrib_[0];	//
	for(size_t i=0; i<8; i++){
		const char * key = tab_attrib.spair[i].skey;
		const char * val = tab_attrib.spair[i].sval;
		if(NULL == key) break;
		else		setkv_string(key, val);
	}
	for(size_t i=0;i<8;i++){
		int key = tab_attrib.npair[i].nkey;
		int val = tab_attrib.npair[i].nval;
		if(0 == key) break;
		else		setkv_value(key, val);
	}

	//table col attrib
	uint32_t len = 0;
	size_t scols = src ->cols();
	for(size_t scol = 0; scol < scols; scol++) {		//按源table的列号为序，循环
		const char * key = src ->col_name(scol);
		size_t dcol = this->col_index(key);				//以列名称为关键字，列序号可能不同
		if (dcol == (size_t)-1) {
			continue;									//如源Table中具有目的Table中没有的列，则忽略
		}
		size_t dcoltype = col_type(dcol);

		_Custom_Attrib &tab_attrib = src->impl->attrib_[1+scol];	//
		for(size_t i=0; i<8; i++){
			const char * key = tab_attrib.spair[i].skey;
			const char * val = tab_attrib.spair[i].sval;
			if(NULL == key) break;
			else		col_setkv_string(dcol, key, val);
		}
		for(size_t i=0;i<8;i++){
			int key = tab_attrib.npair[i].nkey;
			int val = tab_attrib.npair[i].nval;
			if(0 == key) break;
			else		col_setkv_value(dcol, key, val);
		}
		
	}
}
size_t table_t::attrib_size() const
{
	size_t ncols = cols();
	size_t result = 2 * sizeof(uint32_t) * (ncols+1);

	size_t len_temp = 0;

	//0 is table attrib, 1 to ncols is col attribute
	for(size_t col=0; col<=ncols; col++){
		_Custom_Attrib &tab_attrib = impl->attrib_[col];
		len_temp = 0;
		for(size_t i=0; i<8; i++){
			const char * key = tab_attrib.spair[i].skey;		const char * val = tab_attrib.spair[i].sval;
			if(NULL == key) break;
			else{
				len_temp += strlen(key);	len_temp+=sizeof(uint32_t);
				len_temp += strlen(val);	len_temp+=sizeof(uint32_t);
			}
		}
		for(size_t i=0;i<8;i++){
			int key = tab_attrib.npair[i].nkey;		int val = tab_attrib.npair[i].nval;
			if(0 == key) break;
			else		len_temp += 2 * sizeof(uint32_t);
		}
		result += len_temp;
	}
	return result;
}
//buff struct
//(table)str_count(4byte)+key_str+val_str+key_str+val_str...int_count(4byte)+key_int(4byte)+val_int(4byte)...
//(col0) str_count(4byte)+key_str+val_str+key_str+val_str...int_count(4byte)+key_int(4byte)+val_int(4byte)...
//  .
//(cols-1)str_count(4byte)+key_str+val_str+key_str+val_str...int_count(4byte)+key_int(4byte)+val_int(4byte)...
// NOTE : key_str and val_str format is  len+string , not contain tail zero.
size_t table_t::attrib_to_buff(const table_t * t, char * buff)
{
	size_t ncols = t->cols();
	size_t i = 0;
	char * p = buff;
	char * q = buff;
	//0 is table attrib, 1 to ncols is col attribute
	for(size_t col=0; col<=ncols; col++){
		_Custom_Attrib &tab_attrib = t->impl->attrib_[col];
		q = p+sizeof(uint32_t);
		for(i=0; i<8; i++){
			const char * key = tab_attrib.spair[i].skey;		const char * val = tab_attrib.spair[i].sval;
			if(NULL == key) break;
			else{
				uint32_t len = strlen(key);
				*((uint32_t*)q) = len; q+=sizeof(uint32_t);
				strcpy_s(q, len+1, key); q+=len;	//*q=0; q++;old
				len = strlen(val);
				*((uint32_t*)q) = len; q+=sizeof(uint32_t);
				strcpy_s(q, len+1, val); q+=len;		//*q=0; q++;
			}
		}
		*((uint32_t*)p) = i;
		p = q;	q = p+sizeof(uint32_t);
		for(i=0;i<8;i++){
			int key = tab_attrib.npair[i].nkey;		int val = tab_attrib.npair[i].nval;
			if(0 == key) break;
			else{
				*((uint32_t*)q) = key; q+=sizeof(uint32_t);*((uint32_t*)q) = val; q+=sizeof(uint32_t);
			}
		}
		*((uint32_t*)p) = i;
		p = q;
	}
	return (p-buff);
}
size_t table_t::attrib_from_buff(const char * buff, size_t len)
{
	size_t ncols = cols();
	size_t count = 0;
	const char * p = buff;
	const char * q = buff;

	//0 is table attrib, 
		count = *((uint32_t*)p) ; q = p+sizeof(uint32_t);
		for(size_t i=0; i<count; i++){
			//const char * key = q;	q += strlen(q); q++; const char * val = q;  q += strlen(q); q++;
			uint32_t len = 0;	char *key = NULL;	char *val = NULL;
			len = get_string_from_len_string(q, &key);	q+=len;
			len = get_string_from_len_string(q, &val);		q+=len;
			setkv_string(key, val);
			delete [] key;
			delete [] val;
		}
		p = q;	count = *((uint32_t*)p) ;	q = p+sizeof(uint32_t);
		for(size_t i=0; i<count; i++){
			int key = *((uint32_t*)q);	q += sizeof(uint32_t); int val = *((uint32_t*)q); q += sizeof(uint32_t);
			setkv_value(key, val);
		}
		
	//	1 to ncols is col attribute
	for(size_t col=0; col<ncols; col++){
		p = q;	count = *((uint32_t*)p) ;	q = p+sizeof(uint32_t);
		for(size_t i=0; i<count; i++){
			//const char * key = q;	q += strlen(q); q++; const char * val = q;  q += strlen(q); q++;
			uint32_t len = 0;	char *key = NULL;	char *val = NULL;
			len = get_string_from_len_string(q, &key);	q+=len;
			len = get_string_from_len_string(q, &val);		q+=len;
			col_setkv_string(col, key, val);
			delete [] key;
			delete [] val;
		}
		
		p = q;	count = *((uint32_t*)p) ;	q = p+sizeof(uint32_t);		
		for(size_t i=0;i<count;i++){
			int key = *((uint32_t*)q);	q += sizeof(uint32_t); int val = *((uint32_t*)q); q += sizeof(uint32_t);
			col_setkv_value(col, key, val);
		}
		p = q;
	}
	len = p-buff;
	return len;
}

size_t table_t::nullflag_to_buff(const table_t * t, char * buff)
{
	char * cur = buff;
	size_t bitsize_ = t->impl->nullflag_.size();
	size_t bytesize_ = (bitsize_ % 8) ? (bitsize_ / 8)+1 : (bitsize_ / 8) ;
	bitsize_ = bytesize_ * 8;
	if(0 < t->impl->nullflag_.size()){		//
		memset(buff, 0x0, bytesize_);
		for(size_t i = 0;i < t->impl->nullflag_.size();i++){
			*cur<<=1;
			*cur += (t->impl->nullflag_[i] != 0);
			if(!(--bitsize_ % 8))
				cur++;
		}
		while(bitsize_--)
			*cur<<=1;
		return bytesize_;
	}
	else
		return 0;
}
size_t table_t::nullflag_from_buff(const table_t * t, const char * buff)	//t is establish from schema in buff,保持风格一致
{
	const char * cur = buff;
	size_t bitsize_ = t->rows() * t->cols();
	size_t bytesize_ = (bitsize_ % 8) ? (bitsize_ / 8)+1 : (bitsize_ / 8) ;
	int bitindex=0;
	int bitmask=0x80;
	while(bitsize_--)
	{
		t->impl->nullflag_.push_back( ((*cur) & bitmask) != 0 );
		bitmask >>= 1;
		if( 0 == ++bitindex % 8 )  {
			bitmask = 0x80;
			cur++;
		}
	}
	return bytesize_;

}


char * table_t::tojson() {
	return NULL;
}



};


