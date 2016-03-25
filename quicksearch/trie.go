package quicksearch

import "container/list"

const (
	_MAX_ARRAY_SIZE = 10
)

// trieNode 字典树节点
type trieNode struct {
	key    rune               // 节点代表的值
	values []interface{}      // 节点的值
	array  []*trieNode        // 子节点，当len等于_MAX_ARRAY_SIZE时，使用hash存储子节点
	hash   map[rune]*trieNode // 子节点
}

// pair 键值对
type pair struct {
	key  string
	node *trieNode
}

// newTrieNode 新建字典树节点
func newTrieNode() *trieNode {
	return &trieNode{
		values: make([]interface{}, 0),
		array:  make([]*trieNode, 0),
	}
}

// newTrie 新建字典树结构
func newTrie() IQuickSearch {
	return newTrieNode()
}

// Insert 插入数据
func (self *trieNode) Insert(key string, value interface{}) {
	if len(key) == 0 {
		return
	}

	node := self
	for _, r := range key {
		if child := node.get(r); nil == child {
			child = newTrieNode()
			child.key = r
			node.set(r, child)
			node = child
		} else {
			node = child
		}
	}
	node.values = append(node.values, value)
}

// HasKey 查看是否存在名称为key的节点
func (self *trieNode) HasKey(key string) bool {
	if len(key) == 0 {
		return false
	}

	node := self
	for _, r := range key {
		if node = node.get(r); nil == node {
			return false
		}
	}

	// 没有值的节点为空节点
	if len(node.values) == 0 {
		return false
	}
	return true
}

// HasPrefix 查看是否存在名称前缀为prefix的节点
func (self *trieNode) HasPrefix(prefix string) bool {
	if len(prefix) == 0 {
		return false
	}

	node := self
	for _, r := range prefix {
		if node = node.get(r); nil == node {
			return false
		}
	}
	return true
}

// HasSubstr 查看是否存在名称包含substr的节点
func (self *trieNode) HasSubstr(substr string) bool {
	return false
}

// ValueForKey 获取节点名称为key的值
func (self *trieNode) ValueForKey(key string, limit ...int64) []interface{} {
	if len(key) == 0 {
		return nil
	}

	node := self
	for _, r := range key {
		if node = node.get(r); nil == node {
			return nil
		}
	}

	if len(node.values) == 0 {
		return nil
	}
	if len(limit) != 0 && limit[0] < int64(len(node.values)) {
		node.values = node.values[limit[0]:]
	}
	return node.values
}

// ValueForPrefix 获取节点名称前缀为prefix的值
func (self *trieNode) ValueForPrefix(prefix string, limit ...int64) []interface{} {
	if len(prefix) == 0 {
		return nil
	}

	node := self
	result := make([]interface{}, 0)
	for _, r := range prefix {
		if node = node.get(r); nil == node {
			return nil
		}
	}

	// 遍历节点并加入结果
	if len(limit) != 0 {
		total := int64(0)
		max := limit[0]
		node.dfs(func(key string, n *trieNode) bool {
			if len(n.values) != 0 {
				if total+int64(len(n.values)) >= max {
					result = append(result, n.values[:max-total]...)
					return false
				} else {
					result = append(result, n.values...)
					total += int64(len(n.values))
				}
			}
			return true
		}, prefix)
	} else {
		node.dfs(func(key string, n *trieNode) bool {
			if len(n.values) != 0 {
				result = append(result, n.values...)
			}
			return true
		}, prefix)
	}

	if len(result) == 0 {
		return nil
	}

	return result
}

// ValueForSubstr 获取节点名称包含substr的值
func (self *trieNode) ValueForSubstr(substr string, limit ...int64) []interface{} {
	return nil
}

// Destroy 销毁字典树
func (self *trieNode) Destroy() {
}

// bfs 广度优先遍历
func (self *trieNode) bfs(f func(key string, node *trieNode) bool, key string) {
	queue := list.New()
	queue.PushBack(&pair{key: key, node: self})
	for e := queue.Front(); nil != e; e = e.Next() {
		v := e.Value.(*pair)
		if !f(v.key, v.node) {
			return
		}
		for _, n := range v.node.array {
			queue.PushBack(&pair{key: v.key + string([]rune{n.key}), node: n})
		}
	}
}

// dfs 深度遍历节点
func (self *trieNode) dfs(f func(key string, node *trieNode) bool, key string) bool {
	// 当前节点代表的字符串
	current := key + string([]rune{self.key})
	if !f(current, self) {
		return false
	}
	for _, child := range self.array {
		if !child.dfs(f, current) {
			return false
		}
	}
	return true
}

// get 获取子节点
func (self *trieNode) get(r rune) *trieNode {
	if nil != self.hash {
		return self.hash[r]
	} else {
		for _, node := range self.array {
			if node.key == r {
				return node
			}
		}
	}
	return nil
}

// set 设置子节点
func (self *trieNode) set(r rune, child *trieNode) {
	if nil == child {
		return
	}
	if nil != self.hash {
		self.hash[r] = child
	} else {
		if _MAX_ARRAY_SIZE == len(self.array) { // 转换为hash结构
			self.hash = make(map[rune]*trieNode)
			for _, node := range self.array {
				self.hash[node.key] = node
			}
			self.hash[r] = child
		}
	}
	self.array = append(self.array, child)
}
