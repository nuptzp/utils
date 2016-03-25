## How to use it?
The first method used

	import (
		"gw.com.cn/dzhyun/utils/lrucache"
		"github.com/astaxie/beego/cache"
	)

	cache, err := cache.NewCache("lrucache", `{
		"lowCacheSize": 966367638,
		"highCacheSize": 1073741824
	}`)		

The second method used 

	import (
		lru "gw.com.cn/dzhyun/utils/lrucache"
	)

	cache := lru.NewLRUCache()
	err := cache.StartAndGC(`{
		"lowCacheSize": 966367638,
		"highCacheSize": 1073741824
	}`) error
	
After the second used method
	lru.FreeLRUCache(cache)

Use it like this:	
	type value struct {
		val string
	}
	func (this* value) Size(){ len(this.val)}
	cache.Put("token1", &value{"axlsdaaff00013ad"}, 10)
	cache.Get("token1")
	cache.IsExist("token1")
	cache.Delete("token1")
	
Note: Any value must implement ILRUValue interface, i.e. Size() method

## LRUCache config

LRUCache config like this:

	{
		"lowCacheSize": 966367638,
		"highCacheSize": 1073741824
	}

lowCacheSize: Minimum buffer size when cache shrinkage
highCacheSize: Maximum buffer size when cache growth

If config is not set, the default lowCacheSize is equal to 0.9GB, 
highCacheSize equal to 1GB

Note: the lowCacheSize must be less than highCacheSize, otherwise it would be panic

## LRUCache performance test

test config:

	cpu : one intel x64 2.7GHz
	memory: 4G x 2
	lowCacheSize : 140000000
	highCacheSize : 200000000
	key : 1-1000W string
	value : 1-1000W string
	
Execute 10000000 times Put

PUT : 55W/s

Execute 10000000 times Get

GET : 200W/s