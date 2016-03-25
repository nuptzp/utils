// Usage:
// import(
//   "gw.com.cn/dzhyun/utils/lrucache"
//   "github.com/astaxie/beego/cache"
// )
//
// init a LRUCache
// The first method
//
//	cache, err := cache.NewCache("lrucache", `{"lowCacheSize": 966367638,"highCacheSize": 1073741824}`)
//
// The second method
//
//	cache := NewLRUCache()
//	err := cache.StartAndGC(`{"lowCacheSize": 966367638,"highCacheSize": 1073741824}`)
//
// Use it like this:
//
//	type value struct {
//		val string
//	}
//	func (this* value) Size(){ len(this.val)}
//	cache.Put("token1", &value{"axlsdaaff00013ad"}, 10)
//	cache.Get("token1")
//	cache.IsExist("token1")
//	cache.Delete("token1")
//
//  more docs github.com/astaxie/beego/cache/README.md
//  and gw.com.cn/dzhyun/utils/lrucache/Readme.md

package lrucache

import (
	"container/list"
	"encoding/json"
	"errors"
	logger "github.com/alecthomas/log4go"
	"github.com/astaxie/beego/cache"
	"sync"
	"time"
)

var (
	// The default minimum and maximum cache value
	DefaultLowCacheSize  int = (1 << 30) / 10 * 9 // 0.9 GB
	DefaultHighCacheSize int = 1 << 30            // 1 GB
)

//LRUCache users must implement this interface
type ILRUValue interface {
	Size() int
}

// Memory cache item.
type MemoryItem struct {
	key        string
	val        ILRUValue
	Lastaccess time.Time
	expired    int64
}

type LRUCache struct {
	sync.Mutex
	lowCacheSize  int
	highCacheSize int
	curCacheSize  int
	chCacheExit   chan bool
	chShrinkCache chan bool
	waitGroup     sync.WaitGroup
	valueList     *list.List
	keyIndex      map[string]*list.Element
}

//lowCacheSize--Minimum buffer size when cache shrinkage
//highCacheSize--Maximum buffer size when cache growth
func NewLRUCache() *LRUCache {
	cache := new(LRUCache)
	cache.lowCacheSize = DefaultLowCacheSize
	cache.highCacheSize = DefaultHighCacheSize
	cache.valueList = list.New()
	cache.keyIndex = make(map[string]*list.Element)
	return cache
}

//The FreeLRUCache to ensure the release of internal goroutine
func FreeLRUCache(cache *LRUCache) {
	if cache != nil {
		logger.Debug("Free LRU Cache")
		cache.Lock()
		if cache.chCacheExit != nil {
			close(cache.chCacheExit)
			cache.chCacheExit = nil
		}
		cache.Unlock()
		cache.waitGroup.Wait()
	}
}

func (this *LRUCache) Get(key string) interface{} {
	this.Lock()
	defer this.Unlock()
	if e, exist := this.keyIndex[key]; exist {
		itm := e.Value.(*MemoryItem)
		if (time.Now().Unix() - itm.Lastaccess.Unix()) > itm.expired {
			return nil
		}
		this.valueList.MoveToBack(e)
		return itm.val
	}
	return nil
}

// Put cache to memory.
// if expired is less than 0, It will be modified to 24*3600*365 ( The units are seconds).
func (this *LRUCache) Put(key string, value interface{}, expired int64) error {
	val, ok := value.(ILRUValue)
	if !ok {
		return errors.New("Value must implement ILRUValue interface")
	}
	if expired < 0 {
		expired = 24 * 3600 * 365
	}
	this.Lock()
	defer this.Unlock()
	if e, exist := this.keyIndex[key]; exist {
		this.valueList.MoveToBack(e)
		itm := e.Value.(*MemoryItem)
		this.curCacheSize -= itm.val.Size()
		this.curCacheSize += val.Size()
		itm.val = val
		itm.expired = expired
		itm.Lastaccess = time.Now()
	} else {
		this.curCacheSize += val.Size()
		itm := &MemoryItem{key, val, time.Now(), expired}
		this.keyIndex[key] = this.valueList.PushBack(itm)
	}
	this.cacheCheck()
	return nil
}

// Cache check Judge whether need to shrink
func (this *LRUCache) cacheCheck() {
	if this.curCacheSize >= this.highCacheSize {
		if this.chShrinkCache == nil {
			this.chShrinkCache = make(chan bool, 1)
			this.chCacheExit = make(chan bool)
			this.waitGroup.Add(1)
			go this.shrinkCache()
		}
		select {
		case this.chShrinkCache <- true:
		default:
		}
	}
}

// shrink cache
func (this *LRUCache) shrinkCache() {
	defer this.waitGroup.Done()
	logger.Debug("Start LRUCache Shrink Proccess goroutine;")
	defer logger.Debug("End LRUCache Shrink Proccess goroutine;")
	for {
		select {
		case <-this.chShrinkCache:
			for {
				this.Lock()
				if this.curCacheSize > this.lowCacheSize {
					Value := this.valueList.Remove(this.valueList.Front())
					itm := Value.(*MemoryItem)
					this.curCacheSize -= itm.val.Size()
					delete(this.keyIndex, itm.key)
					this.Unlock()
				} else {
					this.Unlock()
					break
				}
			}
		case <-this.chCacheExit:
			return
		}
	}
}

// Delete cache in memory.
func (this *LRUCache) Delete(key string) error {
	this.Lock()
	defer this.Unlock()
	if e, exist := this.keyIndex[key]; exist {
		this.curCacheSize -= e.Value.(*MemoryItem).val.Size()
		delete(this.keyIndex, key)
		this.valueList.Remove(e)
	} else {
		return errors.New("key not exist")
	}
	return nil
}

// Increase cache counter in memory.
func (this *LRUCache) Incr(key string) error {
	return errors.New("LRUCache is not support!")
}

// Decrease counter in memory.
func (this *LRUCache) Decr(key string) error {
	return errors.New("LRUCache is not support!")
}

// check cache exist in memory.
func (this *LRUCache) IsExist(key string) bool {
	this.Lock()
	defer this.Unlock()
	e, exist := this.keyIndex[key]
	if exist {
		itm := e.Value.(*MemoryItem)
		exist = (time.Now().Unix() - itm.Lastaccess.Unix()) <= itm.expired
	}
	return exist
}

// delete all cache in memory.
func (this *LRUCache) ClearAll() error {
	this.Lock()
	defer this.Unlock()
	this.curCacheSize = 0
	this.valueList = list.New()
	this.keyIndex = make(map[string]*list.Element)
	return nil
}

// start memory cache. it will set lowCacheSize and highCacheSize.
func (this *LRUCache) StartAndGC(config string) error {
	logger.Info("LRUCache config: %s", config)
	var cf map[string]int
	json.Unmarshal([]byte(config), &cf)
	if lowCacheSize, exist := cf["lowCacheSize"]; exist {
		this.lowCacheSize = lowCacheSize
	}

	if highCacheSize, exist := cf["highCacheSize"]; exist {
		this.highCacheSize = highCacheSize
	}
	if this.highCacheSize < this.lowCacheSize {
		panic("the lowCacheSize must be less than highCacheSize...")
	}
	logger.Info("StartAndGC LRU Cache lowCacheSize(%d), highCacheSize(%d)",
		this.lowCacheSize, this.highCacheSize)
	return nil
}

func init() {
	cache.Register("lrucache", NewLRUCache())
}
