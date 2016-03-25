package utils

import (
	"testing"
)

func createHz2Py() (*Hz2Py, error) {
	h2p, err := NewHz2Py()
	if err != nil {
		return nil, err
	}

	err = h2p.Load("hz2py.txt")
	if err != nil {
		return nil, err
	}
	return h2p, nil
}

func TestLoad(t *testing.T) {
	h2p, err := createHz2Py()
	if err != nil {
		t.Error(err)
	}
	i := 0
	for k, v := range h2p.datas {
		t.Log(k, v)
		i++
		if i == 100 {
			break
		}
	}

}

func TestFull(t *testing.T) {

	h2p, err := createHz2Py()
	if err != nil {
		t.Error(err)
	}

	tests := map[string]string{
		"浦发银行":       "PuFaYinHang",
		"浦发银行123abc": "PuFaYinHang123abc",
		"收益率":        "ShouYiLv",
	}

	for t1, t2 := range tests {
		py := h2p.Full(t1)
		if py != t2 {
			t.Error(py, "!=", t2)
		}
	}
}

func TestShort(t *testing.T) {

	h2p, err := createHz2Py()
	if err != nil {
		t.Error(err)
	}

	tests := map[string]string{
		"浦发银行":       "pfyh",
		"浦发银行123abc": "pfyh123abc",
		"收益率":        "syl",
	}

	for t1, t2 := range tests {
		py := h2p.Short(t1)
		if py != t2 {
			t.Error(py, "!=", t2)
		}
	}
}

func BenchmarkFull(b *testing.B) {
	h2p, err := createHz2Py()
	if err != nil {
		b.Error(err)
	}

	var s string
	for i := 0; i < b.N; i++ {
		s = h2p.Full("浦发银行")
	}
	b.Log(s)
}

func BenchmarkShort(b *testing.B) {
	h2p, err := createHz2Py()
	if err != nil {
		b.Error(err)
	}

	var s string
	for i := 0; i < b.N; i++ {
		s = h2p.Short("浦发银行")
	}
	b.Log(s)
}

func BenchmarkShort2(b *testing.B) {
	h2p, err := createHz2Py()
	if err != nil {
		b.Error(err)
	}

	var s string
	for i := 0; i < b.N; i++ {
		s = h2p.short("浦发银行")
	}
	b.Log(s)
}
