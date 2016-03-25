package utils

import (
	"testing"
	"code.google.com/p/go.text/encoding/simplifiedchinese"
	"code.google.com/p/go.text/transform"
)


func TestGBK(t *testing.T) {
	encoder := simplifiedchinese.GBK.NewEncoder()
	decoder := simplifiedchinese.GBK.NewDecoder()

	input := "中国"

	utfbuff1 := []byte(input)
	gbkbuff, _, _  := transform.Bytes(encoder, utfbuff1)
	utfbuff2, _, _ := transform.Bytes(decoder, gbkbuff)
	output := string(utfbuff2)

	if input != output {
		t.Error("faield")
	}
}


func BenchmarkGBK(b *testing.B) {

	buff := []byte{0xd6, 0xd0, 0xb9, 0xfa}
	//decoder := simplifiedchinese.GBK.NewDecoder()
	for i := 0; i < b.N; i++ {
		//transform.Bytes(decoder, buff)
		DecodeGBK(buff)
	}
}
