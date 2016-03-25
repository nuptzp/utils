package quicksearch_test

import (
	"testing"
)

var Map = quicksearch.New()

func TestMapInsert(t *testing.T) {
	for i, str := range tests {
		Map.Insert(str, i)
	}
	t.Log("Test Insert OK")
}

func TestMapHasKey(t *testing.T) {
	for _, str := range tests {
		if !Map.HasKey(str) {
			t.Error("Key:", str, " not found!")
			return
		}
	}
	t.Log("Test Has OK")
}

func TestMapHasPrefix(t *testing.T) {
	for _, str := range tests {
		if !Map.HasPrefix(str) {
			t.Error("Prefix", str, " not found!")
			return
		}
	}

	if !Map.HasPrefix("a") ||
		!Map.HasPrefix("b") ||
		!Map.HasPrefix("c") ||
		!Map.HasPrefix("ab") ||
		!Map.HasPrefix("cd") ||
		!Map.HasPrefix("cde") ||
		!Map.HasPrefix("cdef") {
		t.Error("Item missing")
		return
	}
	t.Log("Test HasPrefix OK")
}

func TestMapValueForKey(t *testing.T) {
	for _, str := range tests {
		v := Map.ValueForKey(str)
		if nil == v {
			t.Error("Key:", str, " not found!")
			return
		}
	}

	t.Log("Test Value OK")
}

func TestMapValueForPrefix(t *testing.T) {
	for _, str := range tests {
		v := Map.ValueForPrefix(str)
		if nil == v {
			t.Error("Prefix:", str, " not found!")
			return
		}
	}

	var result []interface{}
	result = Map.ValueForPrefix("a")
	if len(result) != 2 {
		t.Error("Wrong number results:", len(result))
		return
	}

	result = Map.ValueForPrefix("cd")
	if len(result) != 1 {
		t.Error("Wrong number results:", len(result))
		return
	}

	result = Map.ValueForPrefix("cde")
	if len(result) != 1 {
		t.Error("Wrong number results:", len(result))
		return
	}
}
