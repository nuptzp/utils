package quicksearch

type QSType int

const (
	QSType_Trie QSType = iota
	QSType_Map
)

// IQuickSearch 快速搜索接口
type IQuickSearch interface {
	Insert(key string, value interface{})
	HasKey(key string) bool
	HasPrefix(prefix string) bool
	HasSubstr(substr string) bool
	ValueForKey(key string, limit ...int64) []interface{}
	ValueForPrefix(prefix string, limit ...int64) []interface{}
	ValueForSubstr(substr string, limit ...int64) []interface{}
	Destroy()
}

// New 产生typ类型的QuickSearch结构
func New(typ ...QSType) IQuickSearch {
	def := newTrie() // 默认使用Trie结构

	if len(typ) == 0 {
		return def
	} else {
		switch typ[0] {
		case QSType_Trie:
			return newTrie()
		case QSType_Map:
			return newMap()
		default:
			return def
		}
	}
}
