package utils

import (
	. "gw.com.cn/dzhyun/utils.git"
	"runtime"
	"testing"
)

func TestCRC16(t *testing.T) {
	key := "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456"
	crc16 := CRC16(key)
	("Key:%s crc16:%s", key, crc16)
	key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123457"
	crc16 = CRC16(key)
	t.Logf("Key:%s crc16:%s", key, crc16)
	key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123458"
	crc16 = CRC16(key)
	t.Logf("Key:%s crc16:%s", key, crc16)
	key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123459"
	crc16 = CRC16(key)
	t.Logf("Key:%s crc16:%s", key, crc16)
	key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123450"
	crc16 = CRC16(key)
	t.Logf("Key:%s crc16:%s", key, crc16)
}

func BenchmarkCRC16(b *testing.B) {
	b.SetParallelism(runtime.NumCPU())
	b.N = 100000000

	key := "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456"
	crc16test := func(pb *testing.PB) {
		for pb.Next() {
			CRC16(key)
		}
	}
	b.RunParallel(crc16test)
}
