package utils

import (
	"flag"
	"os"
	"strings"
)

//空输出
type NullWriter struct {
}

func (this *NullWriter) Write(p []byte) (n int, err error) {
	return 0, nil
}

type ErrorHandling flag.ErrorHandling

//LaxFlagSet扩展FlagSet
type LaxFlagSet struct {
	flag.FlagSet
	flagmap map[string]*flag.Flag
}

//构造LaxFlagSet
func NewLaxFlagSet(name string, errorHandling ErrorHandling) *LaxFlagSet {
	newLFS := new(LaxFlagSet)
	newLFS.Init(name, flag.ErrorHandling(errorHandling))
	newLFS.flagmap = make(map[string]*flag.Flag)
	newLFS.SetOutput(new(NullWriter))
	return newLFS
}

//内部枚举数据
func (self *LaxFlagSet) enumFlag(one *flag.Flag) {
	self.flagmap[one.Name] = one
}

//解析参数列表
func (self *LaxFlagSet) LaxParse(arguments []string) {
	//获取Flag
	self.VisitAll(self.enumFlag)
	//遍历补齐
	for idx, arg := range arguments {
		if strings.HasPrefix(arg, "-") {
			arg = strings.TrimLeft(arg, "-")
			nIndex := strings.Index(arg, "=")
			if nIndex < 0 {
				_, ok := self.flagmap[arg]
				if !ok {
					if idx+1 == len(arguments) || strings.HasPrefix(arguments[idx+1], "-") {
						self.Bool(arg, false, "auto Bool insert")
					} else {
						self.String(arg, "default", "auto String insert")
					}
				}
			}
		}
	}
	self.Parse(arguments)
}

//无参默认Flag
type LaxFlagDefault struct {
	LaxFlagSet
}

func NewLaxFlagDefault() *LaxFlagDefault {
	newLF := new(LaxFlagDefault)
	newLF.Init(os.Args[0], flag.ErrorHandling(-1))
	newLF.flagmap = make(map[string]*flag.Flag)
	newLF.SetOutput(new(NullWriter))
	return newLF
}

//解析
func (self *LaxFlagDefault) LaxParseDefault() {
	self.LaxParse(os.Args[1:])
}
