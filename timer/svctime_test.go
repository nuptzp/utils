package utils

import (
	"fmt"
	"testing"
	"time"
)

/*
func TestParseDayTimeArray(t *testing.T) {
	array := []string{"17:20:15", "12:59:50", "00:05:31", "5", "9:31", "14:8"}
	Array, err := ParseDayTimeArray(array)
	if err != nil {
		t.Error(err)
	} else {
		for _, core := range Array {
			fmt.Printf("TestParseDayTimeArray:%v %v %v\n", core.Hour, core.Min, core.Sec)
		}
	}
}

func TestParseIntervalTime(t *testing.T) {
	StarTime, EndTime := "23:2:4", "23"
	interval := int64(98)
	tmCfg, err := ParseIntervalTime(StarTime, EndTime, interval)
	if err != nil {
		t.Error(err)
	} else {
		fmt.Printf("TestParseIntervalTime:%v %v %v\n", *tmCfg.BeginTime, *tmCfg.EndTime, tmCfg.Interval)
	}
}

func TestGetDurByDayTime(t *testing.T) {
	CurTime := time.Now()
	timeConfig := []*DayTime{{11, 11, 11}, {14, 0, 0}}
	dur, err := GetDurByDayTime(CurTime, timeConfig)
	if err != nil {
		t.Error(err)
	} else {
		fmt.Printf("TestGetDurByDayTime CurTime:%v,dur:%v\n", CurTime, dur)
	}
}

func TestGetDurByInterval(t *testing.T) {
	CurTime := time.Now()
	tm_bg := &DayTime{Hour: 17, Min: 0, Sec: 0}
	tm_end := &DayTime{Hour: 17, Min: 2, Sec: 0}
	pointtmcfg := &IntervalTime{BeginTime: tm_bg, EndTime: tm_end, Interval: 1800}
	dur, err := GetDurByInterval(CurTime, pointtmcfg)
	if err != nil {
		t.Error(err)
	} else {
		fmt.Printf("TestGetDurByInterval CurTime:%v,dur:%v\n", CurTime, dur)
	}
}
*/
func TestSvcTimer(t *testing.T) {
	SvcTimer := NewSvcTimer()
	surFunc := func() {
		fmt.Printf("TestSvcTimer :serFunc\n")
	}
	durFunc := func() time.Duration {
		CurTime := time.Now()
		timeConfig := []*DayTime{{Hour: 1, Min: 38, Sec: 54}}
		dur, err := GetDurByDayTime(CurTime, timeConfig)
		if err != nil {
			t.Error(err)
			return 0
		}
		fmt.Printf("TestGetDurByDayTime CurTime:%v,dur:%v\n", CurTime, dur)
		return dur
	}
	surFunc()
	SvcTimer.Start(surFunc, durFunc)
	time.Sleep(5 * time.Second)
	SvcTimer.Stop()
}

func TestTimerArray(t *testing.T) {
	fmt.Printf("TestTimerArray START================\n")

	array := []string{"17:20:15", "12:59:50", "08:08:31", "5", "9:31", "14:8"}
	timer, _ := NewPointTicker(array)
	select {
	case <-timer.C:
		timer.Stop()
		fmt.Printf("TestTimerArray NewTimerArray timer\n")
	case <-timer.exitCh:
		fmt.Printf("TestTimerArray NewTimerArray exit\n")
		return
	}
	fmt.Printf("TestTimerArray END ================\n")
}

func TestTimerInterval(t *testing.T) {
	fmt.Printf("TestTimerInterval START================\n")
	StarTime, EndTime := "", ""
	interval := int64(10)
	timer, _ := NewIntervalTicker(interval, StarTime, EndTime)
	for {
		select {
		case <-timer.C:
			timer.Stop()
			fmt.Printf("TestTimerArray NewTimerArray timer\n")
		case <-timer.exitCh:
			fmt.Printf("TestTimerArray NewTimerArray exit\n")
			return
		}
	}
	fmt.Printf("TestTimerInterval END ================\n")
}

func TestTickerPoint(t *testing.T) {
	fmt.Printf("TestTimerInterval START================\n")
	point := []string{"16:20:30", "16:30:10"}
	//point1 := []string{"16:04:00", "16:04:30"}
	ticker, err := NewPointTicker(point)
	if err != nil {
		fmt.Println("New error:", err)
		return
	}
	defer ticker.Stop()
	time.Sleep(time.Second)
	//ticker.ResetPoint(point1)
	//ticker.ResetInterval(5)
	go func() {
		for {
			select {
			case <-ticker.C:
				fmt.Println("exeTime:", time.Now())

			case <-ticker.exitCh:
				fmt.Printf("TestTickerPoint Point  exit\n")
				return
			}
		}
	}()
	time.Sleep(time.Minute * 5)
}
