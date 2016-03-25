/*
汉字到拼音的转换器
*/
package utils

import (
	"bufio"
	"bytes"
	"os"
	"strings"
)

type recordSet [][][]byte

type Hz2Py struct {
	datas recordSet
}

// 创建一个转换器
func NewHz2Py() (*Hz2Py, error) {
	h2p := &Hz2Py{}
	h2p.datas = make(recordSet, 0xFFFF)
	return h2p, nil
}

/*
加载数据文件,如 hz2py.txt

㭱 hé|he2

㭲 jí|ji2

㭴 jiān|jian1

㭸 tú|tu2

*/
func (this *Hz2Py) Load(filename string) error {
	file, err := os.Open(filename)
	if err != nil {
		return err
	}

	reader := bufio.NewReader(file)
	for {
		line, err := reader.ReadBytes('\n')
		if err != nil {
			break
		}
		pos := bytes.Index(line, []byte(" "))
		if pos < 0 {
			continue
		}
		key := []rune(string(line[:pos]))[0]
		if key > 0xFFFF {
			continue
		}
		pos = bytes.LastIndex(line, []byte("|"))
		if pos < 0 {
			continue
		}
		val := bytes.Split(line[pos+1:len(line)-1], []byte(" "))
		this.datas[key] = val
	}
	return nil
}

func (this *Hz2Py) doMap(mapping func([]byte) []byte, s, sep string) string {
	var buff bytes.Buffer
	for _, c := range s {
		val := this.datas[c]
		if val != nil {
			buff.Write(mapping(val[0]))
		} else {
			buff.WriteRune(c)
		}
	}
	return buff.String()
}

// 返回汉字字符串的全格式 如 浦发银行 => PuFaYinHang
func (this *Hz2Py) Full(str string) string {

	return this.doMap(func(py []byte) []byte {
		buff := make([]byte, len(py)-1)
		copy(buff, py)
		buff[0] = buff[0] - 32
		return buff
	}, str, "")
}

// 返回汉字字符串的首字母 如 浦发银行 => pfyh
func (this *Hz2Py) Short(str string) string {
	return this.doMap(func(py []byte) []byte {
		return py[0:1]
	}, str, "")
}

func (this *Hz2Py) short(str string) string {
	return strings.Map(func(c rune) rune {
		val := this.datas[c]
		if val != nil {
			return rune(val[0][0])
		} else {
			return c
		}
	}, str)
}
