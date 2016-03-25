package utils_test

import (
	"strconv"
	"testing"
	"reflect"

	. "gw.com.cn/dzhyun/utils.git"
)

func TestTableCreate(t *testing.T) {
	table := NewTable("id:i;data:s", 10, "")
	if table.Rows() != 10 {
		t.Error("Rows mismatch")
	}
	if table.Cols() != 2 {
		t.Error("Cols mismatch")
	}
	if table.ColName(0) != "id" {
		t.Error("ColName(0) mismatch")
	}
	if table.ColName(1) != "data" {
		t.Error("ColName(1) mismatch")
	}
	table.Destory()
}

func TestTableSetGet(t *testing.T) {
	table := NewTable("id:i;data:s", 10, "")
	defer table.Destory()

	for row := 0; row < table.Rows(); row++ {
		table.SetCell(row, 0, row*table.Cols()+0)
		table.SetCell(row, 1, strconv.Itoa(row*table.Cols()+1))
	}

	for row := 0; row < table.Rows(); row++ {
		if table.Cell(row, 0) != row*table.Cols()+0 {
			t.Errorf("Cell(%d, %d) mismatch", row, 0)
		}
		if cell := table.Cell(row, 1); cell != strconv.Itoa(row*table.Cols()+1) {
			t.Errorf("Cell(%d, %d) mismatch, %v", row, 1, cell)
		}
	}
}

func TestTableMashall(t *testing.T) {
	t1 := NewTable("id:i;data:s", 10, "")
	defer t1.Destory()

	for row := 0; row < t1.Rows(); row++ {
		t1.SetCell(row, 0, row*t1.Cols()+0)
		t1.SetCell(row, 1, strconv.Itoa(row*t1.Cols()+1))
	}

	buff := t1.ToBuff()

	t2 := NewTableFromBuff(buff)
	defer t2.Destory()

	for row := 0; row < t1.Rows(); row++ {
		for col := 0; col < t1.Cols(); col++ {
			v1 := t1.Cell(row, col)
			v2 := t2.Cell(row, col)
			if v1 != v2 {
				t.Errorf("Cell(%d, %d) mismatch, %v != %v", row, col, v1, v2)
			}
		}
	}
}

func TestTableType_char(t *testing.T) {
	val,vals := int8(10),[]int8{0,1,2,3,4}
	uval,uvals := uint8(11),[]uint8{5,6,7,8,9}
	uvals2 := []byte{10,11,12,13,14}

	table := NewTable("c1:c;c2:5c;c3:C;c4:5C;c5:5C", 1, "")
	defer table.Destory()

	table.SetCell(0, 0, val)
	table.SetCell(0, 1, vals)
	table.SetCell(0, 2, uval)
	table.SetCell(0, 3, uvals)
	table.SetCell(0, 4, uvals2)

	if table.Cell(0,0).(int8) != val {
		t.Errorf("Cell(0,0) mismatch, %v", table.Cell(0,0))
	}
	if !reflect.DeepEqual(table.Cell(0,1).([]int8),vals) {
		t.Errorf("Cell(0,1) mismatch, %v", table.Cell(0,1))
	}
	if table.Cell(0,2).(uint8) != uval {
		t.Errorf("Cell(0,2) mismatch, %v", table.Cell(0,2))
	}
	if !reflect.DeepEqual(table.Cell(0,3).([]uint8),uvals) {
		t.Errorf("Cell(0,3) mismatch, %v", table.Cell(0,3))
	}
	if !reflect.DeepEqual(table.Cell(0,4).([]uint8),uvals2) {
		t.Errorf("Cell(0,4) mismatch, %v", table.Cell(0,4))
	}
}

func TestTableType_short(t *testing.T) {
	val,vals := int16(10),[]int16{0,1,2,3,4}
	uval,uvals := uint16(11),[]uint16{5,6,7,8,9}

	table := NewTable("c1:h;c2:5h;c3:H;c4:5H", 1, "")
	defer table.Destory()

	table.SetCell(0, 0, val)
	table.SetCell(0, 1, vals)
	table.SetCell(0, 2, uval)
	table.SetCell(0, 3, uvals)

	if table.Cell(0,0).(int16) != val {
		t.Errorf("Cell(0,0) mismatch, %v", table.Cell(0,0))
	}
	if !reflect.DeepEqual(table.Cell(0,1).([]int16),vals) {
		t.Errorf("Cell(0,1) mismatch, %v", table.Cell(0,1))
	}
	if table.Cell(0,2).(uint16) != uval {
		t.Errorf("Cell(0,2) mismatch, %v", table.Cell(0,2))
	}
	if !reflect.DeepEqual(table.Cell(0,3).([]uint16),uvals) {
		t.Errorf("Cell(0,3) mismatch, %v", table.Cell(0,3))
	}
}

func TestTableType_int32(t *testing.T) {
	val,vals := int(10),[]int{0,1,2,3,4}
	uval,uvals := uint(11),[]uint{5,6,7,8,9}

	table := NewTable("c1:i;c2:5i;c3:I;c4:5I", 1, "")
	defer table.Destory()

	table.SetCell(0, 0, val)
	table.SetCell(0, 1, vals)
	table.SetCell(0, 2, uval)
	table.SetCell(0, 3, uvals)

	if table.Cell(0,0).(int) != val {
		t.Errorf("Cell(0,0) mismatch, %v", table.Cell(0,0))
	}
	if !reflect.DeepEqual(table.Cell(0,1).([]int),vals) {
		t.Errorf("Cell(0,1) mismatch, %v", table.Cell(0,1))
	}
	if table.Cell(0,2).(uint) != uval {
		t.Errorf("Cell(0,2) mismatch, %v", table.Cell(0,2))
	}
	if !reflect.DeepEqual(table.Cell(0,3).([]uint),uvals) {
		t.Errorf("Cell(0,3) mismatch, %v", table.Cell(0,3))
	}
}

func TestTableType_int64(t *testing.T) {
	val,vals := int64(10),[]int64{0,1,2,3,4}
	uval,uvals := uint64(11),[]uint64{5,6,7,8,9}

	table := NewTable("c1:q;c2:5q;c3:Q;c4:5Q", 1, "")
	defer table.Destory()

	table.SetCell(0, 0, val)
	table.SetCell(0, 1, vals)
	table.SetCell(0, 2, uval)
	table.SetCell(0, 3, uvals)

	if table.Cell(0,0).(int64) != val {
		t.Errorf("Cell(0,0) mismatch, %v", table.Cell(0,0))
	}
	if !reflect.DeepEqual(table.Cell(0,1).([]int64),vals) {
		t.Errorf("Cell(0,1) mismatch, %v", table.Cell(0,1))
	}
	if table.Cell(0,2).(uint64) != uval {
		t.Errorf("Cell(0,2) mismatch, %v", table.Cell(0,2))
	}
	if !reflect.DeepEqual(table.Cell(0,3).([]uint64),uvals) {
		t.Errorf("Cell(0,3) mismatch, %v", table.Cell(0,3))
	}
}

func TestTableType_TimeT(t *testing.T) {
	val,vals := int64(10),[]int64{0,1,2,3,4}

	table := NewTable("c1:T;c2:5T", 1, "")
	defer table.Destory()

	table.SetCell(0, 0, val)
	table.SetCell(0, 1, vals)

	if table.Cell(0,0).(int64) != val {
		t.Errorf("Cell(0,0) mismatch, %v", table.Cell(0,0))
	}
	if !reflect.DeepEqual(table.Cell(0,1).([]int64),vals) {
		t.Errorf("Cell(0,1) mismatch, %v", table.Cell(0,1))
	}
}

func TestTableType_Float(t *testing.T) {
	val,vals := float32(10),[]float32{0,1,2,3,4}
	uval,uvals := float64(11),[]float64{5,6,7,8,9}

	table := NewTable("c1:f;c2:5f;c3:d;c4:5d", 1, "")
	defer table.Destory()

	table.SetCell(0, 0, val)
	table.SetCell(0, 1, vals)
	table.SetCell(0, 2, uval)
	table.SetCell(0, 3, uvals)

	if table.Cell(0,0).(float32) != val {
		t.Errorf("Cell(0,0) mismatch, %v", table.Cell(0,0))
	}
	if !reflect.DeepEqual(table.Cell(0,1).([]float32),vals) {
		t.Errorf("Cell(0,1) mismatch, %v", table.Cell(0,1))
	}
	if table.Cell(0,2).(float64) != uval {
		t.Errorf("Cell(0,2) mismatch, %v", table.Cell(0,2))
	}
	if !reflect.DeepEqual(table.Cell(0,3).([]float64),uvals) {
		t.Errorf("Cell(0,3) mismatch, %v", table.Cell(0,3))
	}
}

func TestTableType_String(t *testing.T) {
	val := "Test";

	table := NewTable("c1:s", 1, "")
	defer table.Destory()

	table.SetCell(0, 0, val)

	if table.Cell(0,0).(string) != val {
		t.Errorf("Cell(0,0) mismatch, %v", table.Cell(0,0))
	}
}

func TestTableType_Binary(t *testing.T) {
	/*
	val := []byte{0,1,2,3,4};

	table := NewTable("c1:b", 1, "")
	defer table.Destory()

	table.SetCell(0, 0, val)

	if !reflect.DeepEqual(table.Cell(0,0).([]byte),val) {
		t.Errorf("Cell(0,0) mismatch, %v", table.Cell(0,0))
	}*/
	t.Error("Not implemention")
}

func TestTableType_pvFloat(t *testing.T) {
	t.Error("Not implemention")
}