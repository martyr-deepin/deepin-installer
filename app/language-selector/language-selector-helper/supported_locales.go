/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

package main

import (
	"dbus/com/deepin/api/localehelper"
	"dbus/org/freedesktop/timedate1"
	"fmt"
	"io/ioutil"
	"strings"
)

const (
	defaultLocaleFile   = "/etc/default/locale"
	supportedLocaleFile = "/usr/share/i18n/SUPPORTED"
)

func isSupportedLocale(locale string) bool {
	locales, _ := getSupportedLocales(supportedLocaleFile)
	for _, l := range locales {
		if l == locale {
			return true
		}
	}

	return false
}

func doSetLocale(locale string) error {
	lhelper, err := localehelper.NewLocaleHelper(
		"com.deepin.api.LocaleHelper",
		"/com/deepin/api/LocaleHelper",
	)
	if err != nil {
		return err
	}
	defer localehelper.DestroyLocaleHelper(lhelper)

	err = lhelper.GenerateLocale(locale)
	if err != nil {
		return err
	}

	var content = fmt.Sprintf("LANG=%s\nLANGUAGE=%s", locale,
		strings.Split(locale, ".")[0])
	return ioutil.WriteFile(defaultLocaleFile, []byte(content), 0644)
}

func setTimezoneByLocale(locale string) error {
	tm, err := timedate1.NewTimedate1("org.freedesktop.timedate1",
		"/org/freedesktop/timedate1")
	if err != nil {
		return err
	}
	defer timedate1.DestroyTimedate1(tm)

	var zone string
	switch locale {
	case "zh_CN.UTF-8":
		zone = "Asia/Shanghai"
	case "zh_TW.UTF-8":
		zone = "Asia/Taipei"
	default:
		zone = "UTC"
	}

	return tm.SetTimezone(zone, false)
}

func getSupportedLocales(file string) ([]string, error) {
	content, err := ioutil.ReadFile(file)
	if err != nil {
		return nil, err
	}

	var locales []string
	lines := strings.Split(string(content), "\n")
	for _, line := range lines {
		if !strings.Contains(line, "UTF-8") {
			continue
		}

		locales = append(locales, strings.Split(line, " ")[0])
	}

	return locales, nil
}
