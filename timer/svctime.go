package utils

import (
	"errors"
	"fmt"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"

	logger "github.com/alecthomas/log4go"
)

//日时间
type DayTime struct {
	Hour int
	Min  int
	Sec  int
}

func (this *DayTime) Valid() bool {
	return !(this.Hour < 0 || this.Hour > 23 ||
		this.Min < 0 || this.Min > 59 ||
		this.Sec < 0 || this.Sec > 59)
}

func (this *DayTime) Less(right *DayTime) bool {
	if this.Hour != right.Hour {
		return this.Hour < right.Hour
	} else if this.Min != right.Min {
		return this.Min < right.Min
	} else {
		return this.Sec < right.Sec
	}
}

func (this *DayTime) GetUnixSec() time.Duration {
	return time.Duration(this.Hour)*time.Hour +
		time.Duration(this.Min)*time.Minute +
		time.Duration(this.Sec)*time.Second
}

//间隔时间
type IntervalTime struct {
	BeginTime *DayTime
	EndTime   *DayTime
	Interval  int64
}

func (this *IntervalTime) Valid() bool {
	return this.BeginTime.Valid() &&
		this.EndTime.Valid() &&
		this.Interval > 0 &&
		this.BeginTime.Less(this.EndTime)
}

//日时间数据排序结构
type DayTimeOrder []*DayTime

func (this DayTimeOrder) Len() int {
	return len(this)
}

func (this DayTimeOrder) Less(i, j int) bool {
	return this[i].Less(this[j])
}

func (this DayTimeOrder) Swap(i, j int) {
	this[i], this[j] = this[j], this[i]
}

func parseDayTime(dayTimeStr string) (*DayTime, error) {
	curDayTime := new(DayTime)
	timeStrArr := strings.Split(dayTimeStr, ":")
	if len(timeStrArr) > 3 || len(timeStrArr) < 1 {
		return nil, fmt.Errorf("Split dayTimeStr err(timeStrArr:%v)", timeStrArr)
	}
	for idx, val := range timeStrArr {
		value, err := strconv.Atoi(val)
		if err != nil {
			return nil, fmt.Errorf("parseDayTime strconv.Atoi(%v) err:%s",
				value, err)
		}
		switch idx {
		case 0: //hour
			curDayTime.Hour = value
		case 1: //min
			curDayTime.Min = value
		case 2: //second
			curDayTime.Sec = value
		}
	}
	return curDayTime, nil
}

//解析时间字符串数组
func ParseDayTimeArray(array []string) ([]*DayTime, error) {
	logger.Debug("ParseDayTimeArray(%v)", array)
	if len(array) < 1 {
		return nil, errors.New("ParseDayTimeArray: timeArray is empty!")
	}
	dayTimeArr := make([]*DayTime, 0, len(array))
	for _, timeStr := range array {
		curDayTime, err := parseDayTime(timeStr)
		if err != nil {
			return nil, err
		}
		if !curDayTime.Valid() {
			return nil, fmt.Errorf("ParseDayTimeArray CurDayTime(%v) is invalid!", curDayTime)
		}
		dayTimeArr = append(dayTimeArr, curDayTime)
	}
	return dayTimeArr, nil

}

//解析等间隔触发 interval(单位为秒)
func ParseIntervalTime(beginTime, endTime string, interval int64) (*IntervalTime, error) {
	var err error
	tmBg, tmEnd := &DayTime{}, &DayTime{23, 59, 59}
	if len(beginTime) != 0 {
		tmBg, err = parseDayTime(beginTime)
		if err != nil {
			return nil, err
		}
	}
	if len(endTime) != 0 {
		tmEnd, err = parseDayTime(endTime)
		if err != nil {
			return nil, err
		}
	}
	interTime := &IntervalTime{tmBg, tmEnd, interval}
	if !interTime.Valid() {
		return nil, fmt.Errorf("ParseIntervalTime interTime(%v) is invalid!", *interTime)
	}
	return interTime, nil
}

//获取时间数组Duration
func GetDurByDayTime(curTime time.Time, dayTimeArr []*DayTime) (time.Duration, error) {
	if len(dayTimeArr) < 1 {
		return 0, errors.New("GetDurByDayTime:dayTimeArr is empty")
	}
	curTime = time.Unix(curTime.Unix(), 0) //时间按秒取整
	Year, mon, day := curTime.Date()
	curDate := time.Date(Year, mon, day, 0, 0, 0, 0, time.Local)
	OrderArray := DayTimeOrder(dayTimeArr)
	sort.Sort(OrderArray)
	for i, dayTime := range OrderArray {
		if dayTime.Hour < 0 || dayTime.Hour > 23 || dayTime.Min < 0 || dayTime.Min > 59 || dayTime.Sec < 0 || dayTime.Sec > 59 {
			return 0, errors.New("GetDurByDayTime:dayTime out of range")
		}
		pointTime := curDate.Add(dayTime.GetUnixSec())
		if pointTime.After(curTime) {
			return pointTime.Sub(curTime), nil
		}
		if i == len(OrderArray)-1 {
			dayTime = OrderArray[0]
			pointTime = curDate.Add(dayTime.GetUnixSec() + time.Hour*24)
			return pointTime.Sub(curTime), nil
		}
	}
	return 0, errors.New("GetDurByDayTime err")
}

//解析获取下一次触发时间
func GetDurByInterval(curTime time.Time, interTime *IntervalTime) (time.Duration, error) {
	if !interTime.Valid() {
		return 0, fmt.Errorf("GetDurByInterval:interTime(%v) invalid!", interTime)
	}
	curTime = time.Unix(curTime.Unix(), 0) //时间按秒取整
	Year, mon, day := curTime.Date()
	curDate := time.Date(Year, mon, day, 0, 0, 0, 0, time.Local)
	beginTime := curDate.Add(interTime.BeginTime.GetUnixSec())
	EndTime := curDate.Add(interTime.EndTime.GetUnixSec())
	if curTime.Before(beginTime) { //开始时间之前
		return beginTime.Sub(curTime), nil
	} else if curTime.After(EndTime) { //结束时间之后
		return beginTime.Add(24 * time.Hour).Sub(curTime), nil
	} else {
		interval := time.Duration(interTime.Interval) * time.Second
		index := int64(curTime.Sub(beginTime)) / int64(interval)
		nextTime := beginTime.Add(time.Duration(index+1) * interval)
		if nextTime.After(EndTime) {
			return beginTime.Add(24 * time.Hour).Sub(curTime), nil
		} else {
			return nextTime.Sub(curTime), nil
		}
	}
}

type SvcTimer struct {
	sync.WaitGroup
	exitCh chan bool
}

func NewSvcTimer() *SvcTimer {
	timer := new(SvcTimer)
	timer.exitCh = make(chan bool)
	return timer
}

//每次执行完serFunc后,调用durFunc获取定时持续时间
//durFunc获取的定时持续时间必须大于0否则panic
func (this *SvcTimer) Start(serFunc func(), durFunc func() time.Duration) {
	this.Add(1)
	go func() {
		logger.Info("SvcTimer::Start Begin...")
		defer logger.Info("SvcTimer::Start End...")
		defer this.Done()
		for {
			dur := durFunc()
			if dur <= 0 {
				panic("durFunc return value must be more than 0!")
			}
			logger.Info("SvcTimer Execute serFunc After Duration(%v)!", dur)
			select {
			case <-time.After(dur):
				serFunc()
			case <-this.exitCh:
				return
			}
		}
	}()
}

func (this *SvcTimer) Stop() {
	close(this.exitCh)
	this.Wait()
}

//定时器ticker结构
type SvcTicker struct {
	C <-chan time.Time
	sync.WaitGroup
	exitCh  chan bool
	durFunc func() time.Duration
}

func (this *SvcTicker) startTicker() {
	tickerCh := make(chan time.Time, 1)
	this.C = tickerCh
	this.Add(1)
	go func() {
		logger.Info("SvcTicker::Start Begin...")
		defer logger.Info("SvcTicker::Start End...")
		defer this.Done()
		for {
			dur := this.durFunc()
			if dur <= 0 {
				panic("SvcTicker::durFunc return value must be more than 0!")
			}
			logger.Info("SvcTicker execute After Duration(%v)!", dur)
			select {
			case <-time.After(dur):
				tickerCh <- time.Now()
			case <-this.exitCh:
				return
			}
		}
	}()
}

func (this *SvcTicker) Stop() {
	close(this.exitCh)
	this.Wait()
}

//重置按给定时间点执行
func (this *SvcTicker) ResetPoint(timePoints []string) error {
	timeArray, err := ParseDayTimeArray(timePoints)
	if err == nil {
		this.durFunc = func() time.Duration {
			dur, _ := GetDurByDayTime(time.Now(), timeArray)
			return dur
		}
	}
	return err
}

//重置按给定时间间隔执行
func (this *SvcTicker) ResetInterval(interval int64, bgtimes ...string) error {
	var beginTime, endTime string
	if len(bgtimes) > 0 {
		beginTime = bgtimes[0]
	}
	if len(bgtimes) > 1 {
		endTime = bgtimes[1]
	}
	//beginTime,endTime为空时默认起止时间为00:00:00~23:59:59
	timerInterval, err := ParseIntervalTime(beginTime, endTime, interval)
	if err == nil {
		this.durFunc = func() time.Duration {
			dur, _ := GetDurByInterval(time.Now(), timerInterval)
			return dur
		}
	}
	return err
}

//时间点定时器 timePoint 格式 HH:MM:SS
func NewPointTicker(timePoints []string) (*SvcTicker, error) {
	ticker := new(SvcTicker)
	ticker.exitCh = make(chan bool)
	err := ticker.ResetPoint(timePoints)
	if err == nil {
		ticker.startTicker()
	}
	return ticker, err
}

//时间间隔定时器 interval:时间间隔(单位秒)  bgtimes:开始结束时间(格式HH:MM:SS)
func NewIntervalTicker(interval int64, bgtimes ...string) (*SvcTicker, error) {
	ticker := new(SvcTicker)
	ticker.exitCh = make(chan bool)
	err := ticker.ResetInterval(interval, bgtimes...)
	if err == nil {
		ticker.startTicker()
	}
	return ticker, err
}
