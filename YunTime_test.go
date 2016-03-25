package utils_test

import (
	"testing"
	"time"

	utils "gw.com.cn/dzhyun/utils.git"
)

func TestParseYunTime(t *testing.T) {
	RFC3339Nano2 := "20060102.150405.000Z0700"
	tm := time.Now()
	t.Log(tm.Format(RFC3339Nano2))

	testString := "20140822-154033"
	tm, err := utils.ParseYunTime(testString)
	if nil != err {
		t.Error("Parse "+testString+" error:", err)
	} else {
		t.Log("Parse " + testString + " OK")
	}

	testString = "20140822-154033-223"
	tm, err = utils.ParseYunTime(testString)
	if nil != err {
		t.Error("Parse "+testString+" error:", err)
	} else {
		t.Log("Parse " + testString + " OK")
	}

	testString = "20140822-154033-223-10"
	tm, err = utils.ParseYunTime(testString)
	if nil != err {
		t.Error("Parse "+testString+" error:", err)
	} else {
		t.Log("Parse " + testString + " OK")
	}

	testString = "20140822-154033-223--9"
	tm, err = utils.ParseYunTime(testString)
	if nil != err {
		t.Error("Parse "+testString+" error:", err)
	} else {
		t.Log("Parse " + testString + " OK")
	}

	testString = "111"
	tm, err = utils.ParseYunTime(testString)
	if nil == err {
		t.Error("Bad format passed", testString)
	}
}
