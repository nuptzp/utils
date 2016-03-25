package lrucache

import (
	"bytes"
	"encoding/binary"
	"fmt"
	ci "github.com/astaxie/beego/cache"
	"runtime"
	"testing"
	"time"
)

func (this *LRUCache) size() int {
	this.Lock()
	defer this.Unlock()
	return this.curCacheSize
}

type TestType struct {
	token string
}

func (this *TestType) GetToken() string { return this.token }

func (this *TestType) Size() int { return len(this.token) }

//第一种使用方式测试
func TestCache(t *testing.T) {
	runtime.GOMAXPROCS(4)
	cache := NewLRUCache()
	defer FreeLRUCache(cache)
	cache.StartAndGC(`{"lowCacheSize": 140,"highCacheSize": 200}`)

	cache.Incr("")
	cache.Decr("")
	cache.Put("test", &TestType{"test"}, 2)
	testval := cache.Get("test")
	if testval == nil {
		t.Error("testval should not nil!")
	}
	if !cache.IsExist("test") {
		t.Error("key->test should exist!")
	}
	time.Sleep(3 * time.Second)
	testval = cache.Get("test")
	if testval != nil {
		t.Error("testval should nil!")
	}
	if cache.IsExist("test") {
		t.Error("key->test should not exist!")
	}

	testval = cache.Get("testtttt")

	cache.Put("test", &TestType{"testtt"}, 1)

	err := cache.Put("test1", "", 0)
	if err == nil {
		t.Error("put should return error!")
	}

	err = cache.Delete("test2")
	if err == nil {
		t.Error("delete should return error!")
	}

	cache.Put("test3", &TestType{"test3"}, 0)
	err = cache.Delete("test3")
	if err != nil {
		t.Error("delete should return right!")
	}
	cache.ClearAll()
	if cache.size() != 0 {
		t.Error("cache.size() should zero")
	}

	cache.Put("test4", &TestType{"test4"}, 0)
	cache.Put("test5", &TestType{"test5"}, 0)
	cache.Put("test6", &TestType{"test6"}, 0)

	for e := cache.valueList.Back(); e != nil; e = e.Prev() {
		fmt.Println("First = ", e.Value.(*MemoryItem).key)
	}

	cache.Get("test5")

	for e := cache.valueList.Back(); e != nil; e = e.Prev() {
		fmt.Println("Second = ", e.Value.(*MemoryItem).key)
	}

	cache.ClearAll()
	for i := 1; i < 15; i++ {
		key := fmt.Sprintf("kkkkkkkkkkkkkkkk->%d", i)
		value := &TestType{key}
		cache.Put(key, value, 0)
		info := cache.Get(key)
		fmt.Println("Put Key = ", info.(*TestType).GetToken())
		fmt.Println("Current CacheNum = ", cache.size())
		fmt.Println("\n*******************\n")
		time.Sleep(time.Second)
	}

	for k := range cache.keyIndex {
		fmt.Println("LastKey = ", k)
	}
}

//第二种使用方式测试
func TestCache2(t *testing.T) {
	runtime.GOMAXPROCS(4)
	cache, err := ci.NewCache("lrucache", `{"lowCacheSize": 140,"highCacheSize": 200}`)
	if err != nil {
		fmt.Println("newCache err: ", err)
		return
	}

	cache.Incr("")
	cache.Decr("")
	cache.Put("test", &TestType{"test"}, 2)
	testval := cache.Get("test")
	if testval == nil {
		t.Error("testval should not nil!")
	}
	if !cache.IsExist("test") {
		t.Error("key->test should exist!")
	}
	time.Sleep(3 * time.Second)
	testval = cache.Get("test")
	if testval != nil {
		t.Error("testval should nil!")
	}
	if cache.IsExist("test") {
		t.Error("key->test should not exist!")
	}

	testval = cache.Get("testtttt")

	cache.Put("test", &TestType{"testtt"}, 1)

	err = cache.Put("test1", "", 0)
	if err == nil {
		t.Error("put should return error!")
	}

	err = cache.Delete("test2")
	if err == nil {
		t.Error("delete should return error!")
	}

	cache.Put("test3", &TestType{"test3"}, 0)
	err = cache.Delete("test3")
	if err != nil {
		t.Error("delete should return right!")
	}
	cache.ClearAll()

	for i := 1; i < 15; i++ {
		key := fmt.Sprintf("kkkkkkkkkkkkkkkk->%d", i)
		value := &TestType{key}
		cache.Put(key, value, 0)
		info := cache.Get(key)
		fmt.Println("Put Key = ", info.(*TestType).GetToken())
		fmt.Println("\n*******************\n")
		time.Sleep(time.Second)
	}
}

func GenerateTopic(fund_account int32) []byte {
	buf := new(bytes.Buffer)
	binary.Write(buf, binary.BigEndian, fund_account)
	return buf.Bytes()
}

func BenchmarkPut(b *testing.B) {
	cache, err := ci.NewCache("lrucache", `{"lowCacheSize": 140000000,"highCacheSize": 200000000}`)
	if err != nil {
		fmt.Println("newCache err: ", err)
		return
	}
	b.N = 10000000
	keys := make([]string, b.N)
	for i := 0; i < b.N; i++ {
		keys[i] = string(GenerateTopic(int32(i)))
	}
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		cache.Put(keys[i], &TestType{keys[i]}, -1)
	}
}

func BenchmarkGet(b *testing.B) {
	cache, err := ci.NewCache("lrucache", `{"lowCacheSize": 140000000,"highCacheSize": 200000000}`)
	if err != nil {
		fmt.Println("newCache err: ", err)
		return
	}
	b.N = 10000000
	keys := make([]string, b.N)
	for i := 0; i < b.N; i++ {
		keys[i] = string(GenerateTopic(int32(i)))
	}
	for i := 0; i < b.N; i++ {
		cache.Put(keys[i], &TestType{keys[i]}, -1)
	}
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		cache.Get(keys[i])
	}
}
