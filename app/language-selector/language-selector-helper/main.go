package main

import (
	"pkg.deepin.io/lib/dbus"
	"pkg.deepin.io/lib/log"
)

var logger = log.NewLogger("com.deepin.helper.LanguageSelector")

type LanguageSelector struct {
}

func (*LanguageSelector) Set(lang string) {
	var locale = lang + ".UTF-8"
	if !isSupportedLocale(locale) {
		if !isSupportedLocale(lang) {
			logger.Warning("Invalid locale:", locale)
			return
		} else {
			locale = lang
		}
	}

	err := doSetLocale(locale)
	if err != nil {
		logger.Warning("Set locale failed:", err)
		return
	}

	err = setTimezoneByLocale(locale)
	if err != nil {
		logger.Warning("Set timezone failed:", err)
		return
	}
}

func (*LanguageSelector) GetDBusInfo() dbus.DBusInfo {
	return dbus.DBusInfo{
		Dest:       "com.deepin.helper.LanguageSelector",
		ObjectPath: "/com/deepin/helper/LanguageSelector",
		Interface:  "com.deepin.helper.LanguageSelector",
	}
}

func main() {
	err := dbus.InstallOnSystem(&LanguageSelector{})
	if err != nil {
		print("Can't Init LanguageSelector DBus Servier: " + err.Error())
		return
	}
	dbus.DealWithUnhandledMessage()
	dbus.Wait()
}
