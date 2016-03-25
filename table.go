// +build !windows

package utils

/*
#cgo windows CXXFLAGS: -fpermissive
#cgo windows LDFLAGS: -lstdc++
#include <stdlib.h>
#include <stdint.h>
#include "table.h"

#if defined(__MINGW32__)
#if __GNUC__ >= 4
unsigned
#endif
int _get_output_format(void) { return 1; }
#endif

*/
import "C"
import (
	"fmt"
	"reflect"
	"unsafe"
)

const (
	TtNull int = iota
	TtChar
	TtShort
	TtInt
	TtInt64
	TtFloat
	TtDouble
	TtStr
	TtTable
	TtUchar
	TtUshort
	TtUint
	TtUint64
	TtBinpt
	TtTimet
	TtPfloat
	TtVfloat
)

type Table struct {
	table C.table_t
}

// 根据制定的格式、行数、名称创建一个 Table
func NewTable(schema string, rows int, name string) Table {
	var table C.table_t
	g_schema := C.CString(schema)
	defer C.free(unsafe.Pointer(g_schema))
	g_rows := C.size_t(rows)
	g_name := C.CString(name)
	defer C.free(unsafe.Pointer(g_name))
	table = C.table_create(g_schema, g_rows, g_name)
	return Table{table}
}

// 从 Table 的二进制序列化数据中创建一个Table
func NewTableFromBuff(buff []byte) (table Table) {
	if(len(buff) == 0){
		return Table{}
	}
	cbuff := (*C.char)(unsafe.Pointer(&buff[0]))
	t := C.table_frombuff(cbuff, C.size_t(len(buff)))
	return Table{t}
}

// 从一个 C 指针创建一个 Table, C 指针须为合法的 table_t
func AttachTable(ptr unsafe.Pointer) Table {
	return Table{C.table_t(ptr)}
}

// 删除一个 Table，释放其所占用的资源
func (self Table) Destory() {
	C.table_destory(self.table)
	self.table = nil
}

// 复制一个 Table
func (self Table) Clone() Table {
	table := C.table_clone(self.table)
	return Table{table}
}

// 引用一个 Table
//
// 不要用 = 来复制一个 Table，这会引起内存泄漏，除非很明确的了解 Table 的内部机制
// Assign 进行一次浅复制，Clone 进行一次全复制
func (self Table) Assign() Table {
	table := C.table_assign(self.table)
	return Table{table}
}

// 判断是否为空Table，操作空的Table会导致Crash
func (self Table) IsNull() bool {
	if self.table == nil {
		return true
	} else {
		return false
	}
}

// 将 Table 序列化到缓存中
func (self Table) ToBuff() []byte {
	size := C.table_size(self.table)
	buff := make([]byte, int(size))
	cbuff := (*C.char)(unsafe.Pointer(&buff[0]))
	C.table_tobuff(self.table, cbuff)
	return buff
}

// Table 的行数
func (self Table) Rows() int {
	return int(C.table_rows(self.table))
}

// Table 的列数
func (self Table) Cols() int {
	return int(C.table_cols(self.table))
}

// Table 的列类型
func (self Table) Type(col int) int {
	return int(C.table_col_type(self.table, C.size_t(col)))
}

// Table 的列名称
func (self Table) ColName(col int) string {
	return C.GoString(C.table_col_name(self.table, C.size_t(col)))
}

// 获取 Table 的某一行列值
/******************************************
 TtChar -> int8 	TtUchar -> uint8
 TtShort -> int16	TtUshort -> uint16
 TtInt -> int 		TtUint -> uint
 TtInt64 -> int64 TtUint64 -> uint64
 TtFloat -> float32
 TtDouble -> float64
 TtTimet -> int64
 TtStr -> string
 TtBinpt -> []byte
 TtPfloat
 TtVfloat
******************************************/
func (self Table) Cell(row, col int) interface{} {
	crow := C.size_t(row)
	ccol := C.size_t(col)
	ft := C.table_col_type(self.table, ccol)
	ptr := C.table_get_buff(self.table, crow, ccol)
	size := C.table_col_size(self.table, ccol)
	if ptr == nil || int(ft) == TtNull || size == 0 {
		return nil
	}

	switch int(ft) {
	case TtChar:
		value := (*[0x0fffffff]C.char)(ptr)
		if size > 1 {
			buff := make([]int8, size)
			for i := 0; i < len(buff); i++ {
				buff[i] = int8(value[i])
			}
			return buff
		} else {
			return int8(value[0])
		}
	case TtUchar:
		value := (*[0x0fffffff]C.uchar)(ptr)
		if size > 1 {
			buff := make([]uint8, size)
			for i := 0; i < len(buff); i++ {
				buff[i] = uint8(value[i])
			}
			return buff
		} else {
			return uint8(value[0])
		}
	case TtShort:
		if int32(size)/2 > 1 {
			buff := make([]int16, int32(size)/2)
			value := (*[0x0fffffff]C.short)(ptr)
			for i := 0; i < len(buff); i++ {
				buff[i] = int16(value[i])
			}
			return buff
		} else {
			return int16(C.table_get_short(self.table, crow, ccol))
		}
	case TtUshort:
		value := (*[0x0fffffff]C.ushort)(ptr)
		if int32(size)/2 > 1 {
			buff := make([]uint16, int32(size)/2)
			for i := 0; i < len(buff); i++ {
				buff[i] = uint16(value[i])
			}
			return buff
		} else {
			return uint16(value[0])
		}
	case TtInt:
		if int32(size)/4 > 1 {
			buff := make([]int, int32(size)/4)
			value := (*[0x0fffffff]C.int)(ptr)
			for i := 0; i < len(buff); i++ {
				buff[i] = int(value[i])
			}
			return buff
		} else {
			return int(C.table_get_int(self.table, crow, ccol))
		}
	case TtUint:
		if int32(size)/4 > 1 {
			buff := make([]uint, int32(size)/4)
			value := (*[0x0fffffff]C.uint)(ptr)
			for i := 0; i < len(buff); i++ {
				buff[i] = uint(value[i])
			}
			return buff
		} else {
			return uint(C.table_get_uint(self.table, crow, ccol))
		}
	case TtInt64:
		if int32(size)/8 > 1 {
			buff := make([]int64, int32(size)/8)
			value := (*[0x0fffffff]C.longlong)(ptr)
			for i := 0; i < len(buff); i++ {
				buff[i] = int64(value[i])
			}
			return buff
		} else {
			return int64(C.table_get_int64(self.table, crow, ccol))
		}
	case TtUint64:
		if int32(size)/8 > 1 {
			buff := make([]uint64, int32(size)/8)
			value := (*[0x0fffffff]C.ulonglong)(ptr)
			for i := 0; i < len(buff); i++ {
				buff[i] = uint64(value[i])
			}
			return buff
		} else {
			return uint64(C.table_get_uint64(self.table, crow, ccol))
		}
	case TtFloat:
		if int32(size)/4 > 1 {
			buff := make([]float32, int32(size)/4)
			value := (*[0x0fffffff]C.float)(ptr)
			for i := 0; i < len(buff); i++ {
				buff[i] = float32(value[i])
			}
			return buff
		} else {
			return float32(C.table_get_float(self.table, crow, ccol))
		}
	case TtDouble:
		if int32(size)/8 > 1 {
			buff := make([]float64, int32(size)/8)
			value := (*[0x0fffffff]C.double)(ptr)
			for i := 0; i < len(buff); i++ {
				buff[i] = float64(value[i])
			}
			return buff
		} else {
			return float64(C.table_get_double(self.table, crow, ccol))
		}
	case TtTimet:
		if int32(size)/8 > 1 {
			buff := make([]int64, int32(size)/8)
			value := (*[0x0fffffff]C.longlong)(ptr)
			for i := 0; i < len(buff); i++ {
				buff[i] = int64(value[i])
			}
			return buff
		} else {
			return int64(C.table_get_int64(self.table, crow, ccol))
		}
	case TtStr:
		return C.GoString(C.table_get_str(self.table, crow, ccol))
	/*
		case TtBinpt:
			return C.GoBytes(ptr, C.int(size))
	*/
	case TtTable:
		table := C.table_get_table(self.table, crow, ccol)
		if table == nil {
			return &Table{}
		} else {
			return &Table{C.table_assign(table)}
		}
	}
	return nil
}

// 设置 Table 的某一行列值
func (self Table) SetCell(row, col int, v interface{}) {
	crow := C.size_t(row)
	ccol := C.size_t(col)
	ft := C.table_col_type(self.table, ccol)
	ptr := C.table_get_buff(self.table, crow, ccol)
	size := C.table_col_size(self.table, ccol)
	//fmt.Printf("(%d %d) type=%d size=%d value=%v\n", row, col, int(ft), size, v)
	if ptr == nil || int(ft) == TtNull || size == 0 {
		return
	}
	switch int(ft) {
	case TtChar:
		if size > 1 {
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&v.([]int8)[0]),
				size, C.size_t(0))
		} else {
			buff := v.(int8)
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&buff),
				size, C.size_t(0))
		}
	case TtUchar:
		if size > 1 {
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&v.([]uint8)[0]),
				size, C.size_t(0))
		} else {
			buff := v.(uint8)
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&buff),
				size, C.size_t(0))
		}
	case TtShort:
		if int32(size)/2 > 1 {
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&v.([]int16)[0]),
				size, C.size_t(0))
		} else {
			switch v.(type) {
			case int8:
				C.table_set_short(self.table, crow, ccol, C.short(v.(int8)))
			case int16:
				C.table_set_short(self.table, crow, ccol, C.short(v.(int16)))
			case int32:
				C.table_set_short(self.table, crow, ccol, C.short(v.(int32)))
			case int:
				C.table_set_short(self.table, crow, ccol, C.short(v.(int)))
			default:
				panic(fmt.Sprintf("Unsupport type %v Cell(%d,%d)", reflect.TypeOf(v), crow, ccol))
			}
		}
	case TtUshort:
		if int32(size)/2 > 1 {
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&v.([]uint16)[0]),
				size, C.size_t(0))
		} else {
			var buff uint16
			switch v.(type) {
			case uint8:
				buff = uint16(v.(uint8))
			case uint16:
				buff = uint16(v.(uint16))
			case uint32:
				buff = uint16(v.(uint32))
			case uint:
				buff = uint16(v.(uint))
			default:
				panic(fmt.Sprintf("Unsupport type %v Cell(%d,%d)", reflect.TypeOf(v), crow, ccol))
			}
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&buff),
				size, C.size_t(0))
		}
	case TtInt:
		if int32(size)/4 > 1 {
			var buf unsafe.Pointer
			switch v.(type) {
			case []int32:
				buf = unsafe.Pointer(&v.([]int32)[0])
			case []int:
				// buf = unsafe.Pointer(&v.([]int)[0]) // bug?
				vi := v.([]int)
				v2 := make([]int32, len(vi))
				for i := 0; i < len(vi); i += 1 {
					v2[i] = int32(vi[i])
				}
				buf = unsafe.Pointer(&v2[0])
			default:
				panic(fmt.Sprintf("Unsupport type %v Cell(%d,%d)", reflect.TypeOf(v), crow, ccol))
			}
			C.table_set_buff(self.table, crow, ccol, buf, size, C.size_t(0))
		} else {
			switch v.(type) {
			case int8:
				C.table_set_int(self.table, crow, ccol, C.int(v.(int8)))
			case int16:
				C.table_set_int(self.table, crow, ccol, C.int(v.(int16)))
			case int32:
				C.table_set_int(self.table, crow, ccol, C.int(v.(int32)))
			case int:
				C.table_set_int(self.table, crow, ccol, C.int(v.(int)))
			default:
				panic(fmt.Sprintf("Unsupport type %v Cell(%d,%d)", reflect.TypeOf(v), crow, ccol))
			}
		}
	case TtUint:
		if int32(size)/4 > 1 {
			var buf unsafe.Pointer
			switch v.(type) {
			case []uint32:
				buf = unsafe.Pointer(&v.([]uint32)[0])
			case []uint:
				// buf = unsafe.Pointer(&v.([]uint)[0]) // bug?
				vi := v.([]uint)
				v2 := make([]uint32, len(vi))
				for i := 0; i < len(vi); i += 1 {
					v2[i] = uint32(vi[i])
				}
				buf = unsafe.Pointer(&v2[0])
			default:
				panic(fmt.Sprintf("Unsupport type %v Cell(%d,%d)", reflect.TypeOf(v), crow, ccol))
			}
			C.table_set_buff(self.table, crow, ccol, buf, size, C.size_t(0))
		} else {
			switch v.(type) {
			case uint8:
				C.table_set_uint(self.table, crow, ccol, C.uint(v.(uint8)))
			case uint16:
				C.table_set_uint(self.table, crow, ccol, C.uint(v.(uint16)))
			case uint32:
				C.table_set_uint(self.table, crow, ccol, C.uint(v.(uint32)))
			case uint:
				C.table_set_uint(self.table, crow, ccol, C.uint(v.(uint)))
			default:
				panic(fmt.Sprintf("Unsupport type %v Cell(%d,%d)", reflect.TypeOf(v), crow, ccol))
			}
		}
	case TtInt64:
		if int32(size)/8 > 1 {
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&v.([]int64)[0]),
				size, C.size_t(0))
		} else {
			switch v.(type) {
			case int8:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int8)))
			case int16:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int16)))
			case int32:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int32)))
			case int:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int)))
			case int64:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int64)))
			default:
				panic(fmt.Sprintf("Unsupport type %v Cell(%d,%d)", reflect.TypeOf(v), crow, ccol))
			}
		}
	case TtUint64:
		if int32(size)/8 > 1 {
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&v.([]uint64)[0]),
				size, C.size_t(0))
		} else {
			switch v.(type) {
			case uint8:
				C.table_set_uint64(self.table, crow, ccol, C.ulonglong(v.(uint8)))
			case uint16:
				C.table_set_uint64(self.table, crow, ccol, C.ulonglong(v.(uint16)))
			case uint32:
				C.table_set_uint64(self.table, crow, ccol, C.ulonglong(v.(uint32)))
			case uint:
				C.table_set_uint64(self.table, crow, ccol, C.ulonglong(v.(uint)))
			case uint64:
				C.table_set_uint64(self.table, crow, ccol, C.ulonglong(v.(uint64)))
			default:
				panic(fmt.Sprintf("Unsupport type %v Cell(%d,%d)", reflect.TypeOf(v), crow, ccol))
			}
		}
	case TtFloat:
		if int32(size)/4 > 1 {
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&v.([]float32)[0]),
				size, C.size_t(0))
		} else {
			C.table_set_float(self.table, crow, ccol, C.float(v.(float32)))
		}
	case TtDouble:
		if int32(size)/8 > 1 {
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&v.([]float64)[0]),
				size, C.size_t(0))
		} else {
			C.table_set_double(self.table, crow, ccol, C.double(v.(float64)))
		}
	case TtTimet:
		if int32(size)/8 > 1 {
			C.table_set_buff(self.table, crow, ccol,
				unsafe.Pointer(&v.([]int64)[0]),
				size, C.size_t(0))
		} else {
			switch v.(type) {
			case int8:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int8)))
			case int16:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int16)))
			case int32:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int32)))
			case int:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int)))
			case int64:
				C.table_set_int64(self.table, crow, ccol, C.longlong(v.(int64)))
			default:
				panic(fmt.Sprintf("Unsupport type %v Cell(%d,%d)", reflect.TypeOf(v), crow, ccol))
			}
		}
	case TtStr:
		cstr := C.CString(v.(string))
		defer C.free(unsafe.Pointer(cstr))
		C.table_set_str(self.table, crow, ccol, cstr, C.int(1))
	/* crash
	case TtBinpt:
		C.table_set_buff(self.table, crow, ccol,
			unsafe.Pointer(&v.([]byte)[0]),
			C.size_t(len(v.([]byte))), C.size_t(0))
	*/
	case TtTable:
		t := v.(Table)
		C.table_set_table_raw(self.table, crow, ccol, C.table_assign(t.table))
	}
}

func (self Table) ToJson() (json string) {
	return ""
}
