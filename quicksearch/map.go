package quicksearch

import "strings"

type qsMap map[string][]interface{}

// newMap 新建map快速查找结构
func newMap() IQuickSearch {
	return qsMap(make(map[string][]interface{}))
}

// Insert 插入数据
func (self qsMap) Insert(key string, value interface{}) {
	if len(key) == 0 {
		return
	}

	if array, ok := self[key]; ok {
		self[key] = append(array, value)
	} else {
		self[key] = []interface{}{value}
	}
}

// HasKey 查看是否存在名称为key的节点
func (self qsMap) HasKey(key string) bool {
	if len(key) == 0 {
		return false
	}
	_, exist := self[key]
	return exist
}

// HasPrefix 查看是否存在名称前缀为prefix的节点
func (self qsMap) HasPrefix(prefix string) bool {
	for key, _ := range self {
		if strings.HasPrefix(key, prefix) {
			return true
		}
	}
	return false
}

// HasSubstr 查看是否存在名称包含substr的节点
func (self qsMap) HasSubstr(substr string) bool {
	return false
}

// ValueForKey 获取节点名称为key的值
func (self qsMap) ValueForKey(key string, limit ...int64) []interface{} {
	if len(key) == 0 {
		return nil
	}
	if value, ok := self[key]; ok {
		if len(limit) != 0 && limit[0] < int64(len(value)) {
			value = value[:limit[0]]
		}
		return value
	}
	return nil
}

// ValueForPrefix 获取节点名称前缀为prefix的值
func (self qsMap) ValueForPrefix(prefix string, limit ...int64) []interface{} {
	result := make([]interface{}, 0)
	if len(limit) != 0 {
		max := limit[0]
		total := int64(0)
		for key, array := range self {
			if strings.HasPrefix(key, prefix) {
				if total+int64(len(array)) >= max {
					result = append(result, array[:max-total]...)
					break
				}
				result = append(result, array...)
				total += int64(len(array))
			}
		}
	} else {
		for key, array := range self {
			if strings.HasPrefix(key, prefix) {
				result = append(result, array)
			}
		}
	}

	if len(result) == 0 {
		return nil
	}
	return result
}

// ValueForSubstr 获取节点名称包含substr的值
func (self qsMap) ValueForSubstr(substr string, limit ...int64) []interface{} {
	return nil
}

// Destroy 销毁字典树
func (self qsMap) Destroy() {
}
