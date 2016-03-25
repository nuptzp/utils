package quicksearch_test

import (
	"testing"
)

var Trie = quicksearch.New()

func TestTrieInsert(t *testing.T) {
	for i, str := range tests {
		Trie.Insert(str, i)
	}
	t.Log("Test Insert OK")
}

func TestTrieHasKey(t *testing.T) {
	for _, str := range tests {
		if !Trie.HasKey(str) {
			t.Error("Key:", str, " not found!")
			return
		}
	}
	t.Log("Test Has OK")
}

func TestTrieHasPrefix(t *testing.T) {
	for _, str := range tests {
		if !Trie.HasPrefix(str) {
			t.Error("Prefix", str, " not found!")
			return
		}
	}

	if !Trie.HasPrefix("a") ||
		!Trie.HasPrefix("b") ||
		!Trie.HasPrefix("c") ||
		!Trie.HasPrefix("ab") ||
		!Trie.HasPrefix("cd") ||
		!Trie.HasPrefix("cde") ||
		!Trie.HasPrefix("cdef") {
		t.Error("Item missing")
		return
	}
	t.Log("Test HasPrefix OK")
}

func TestTrieValueForKey(t *testing.T) {
	for _, str := range tests {
		v := Trie.ValueForKey(str)
		if nil == v {
			t.Error("Key:", str, " not found!")
			return
		}
	}

	t.Log("Test Value OK")
}

func TestTrieValueForPrefix(t *testing.T) {
	for _, str := range tests {
		v := Trie.ValueForPrefix(str)
		if nil == v {
			t.Error("Prefix:", str, " not found!")
			return
		}
	}

	var result []interface{}
	result = Trie.ValueForPrefix("a")
	if len(result) != 2 {
		t.Error("Wrong number results:", len(result))
		return
	}

	result = Trie.ValueForPrefix("cd")
	if len(result) != 1 {
		t.Error("Wrong number results:", len(result))
		return
	}

	result = Trie.ValueForPrefix("cde")
	if len(result) != 1 {
		t.Error("Wrong number results:", len(result))
		return
	}
}
