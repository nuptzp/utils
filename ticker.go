package utils

import (
	"time"
)

type Ticker struct {
	ticker   *time.Ticker
	duration time.Duration
	C        <-chan time.Time
}

func NewTicker(d time.Duration) *Ticker {
	tk := new(Ticker)
	tk.ticker = time.NewTicker(d)
	tk.duration = d
	tk.C = tk.ticker.C
	return tk
}

func (self *Ticker) Stop() {
	self.ticker.Stop()
}

func (self *Ticker) Restart() {
	self.ticker.Stop()
	newTicker := time.NewTicker(self.duration)
	self.ticker = newTicker
	self.C = newTicker.C
}
