package utils_test

import (
	"fmt"
	"gw.com.cn/dzhyun/utils.git"
	"testing"
	"time"
)

var count int = 0

func TestTicker(t *testing.T) {
	tk := utils.NewTicker(time.Second * 2)
	ch := make(chan bool, 1)
	go Wait(ch)
	for count < 20 {
		select {
		case val := <-ch:
			if val {
				tk.Restart()
			} else {
				tk.Stop()
			}
		case <-tk.C:
			DoSomething()
		}
	}
	close(ch)
	tk.Stop()
}

func DoSomething() {
	count++
	fmt.Println("DoSomething: ", time.Now().Format("2006-01-02 15:04:05"))
}

func Wait(ch chan bool) {
	time.Sleep(time.Second * 10)
	ch <- false
	time.Sleep(time.Second * 10)
	ch <- true
}
