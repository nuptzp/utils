package utils

import (
	"errors"
	"regexp"
	"time"
)

//解析请求串中的时间
//"20140822-154033-223-8"
func ParseYunTime(yuntime string) (time.Time, error) {
	match, _ := regexp.MatchString("^\\d{8}-\\d{6}(-\\d{3}(--?\\d{1,2})?)?$", yuntime)
	if !match {
		return time.Time{}, errors.New("ParseYunTime Error:bad format")
	}
	timelen := len(yuntime)
	newstr := yuntime[0:8] + "." + yuntime[9:15]
	if timelen < 16 {
		newstr = newstr + ".000"
	} else if timelen > 18 {
		newstr = newstr + "." + yuntime[16:19]
	} else {
		return time.Time{}, errors.New("ParseYunTime Error:" + yuntime)
	}
	if timelen < 20 {
		newstr = newstr + "+08:00"
	} else if timelen > 20 {
		var zone string
		if yuntime[20] == '-' {
			zone = yuntime[21:]
			if len(zone) == 2 {
				zone = "-" + zone + ":00"
			} else if len(zone) == 1 {
				zone = "-0" + zone + ":00"
			} else {
				return time.Time{}, errors.New("ParseYunTime zone Error:" + yuntime)
			}
		} else {
			zone = yuntime[20:]
			if len(zone) == 2 {
				zone = "+" + zone + ":00"
			} else if len(zone) == 1 {
				zone = "+0" + zone + ":00"
			} else {
				return time.Time{}, errors.New("ParseYunTime zone Error:" + yuntime)
			}
		}
		newstr = newstr + zone
	}
	return time.Parse("20060102.150405.000Z07:00", newstr)
}

func FormatYunTime(t time.Time) string {
	milli := t.Format(".000")
	timestr := t.Format("20060102-150405-") + milli[1:4] + "-"
	zone := t.Format("Z07:00")
	if zone[0] == '-' {
		timestr = timestr + "-"
	}
	timestr = timestr + zone[2:3]
	return timestr
}
