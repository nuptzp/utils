package utils

import (
	"code.google.com/p/go.text/encoding/simplifiedchinese"
	"code.google.com/p/go.text/transform"
)

var decoder transform.Transformer

func init() {
	decoder = simplifiedchinese.GBK.NewDecoder()
}

func DecodeGBK(gbkbuff []byte) (string, error) {
	buff, err := ConvertGBK(gbkbuff)
	return string(buff), err
}

func ConvertGBK(gbkbuff []byte) (utf8Buff []byte, err error) {
	utf8Buff, _, err = transform.Bytes(decoder, gbkbuff)
	return
}
