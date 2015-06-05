/**
 * Copyright (c) 2011 ~ 2015 Deepin, Inc.
 *               2013 ~ 2015 jouyouyun
 *
 * Author:      jouyouyun <jouyouwen717@gmail.com>
 * Maintainer:  jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 **/

package main

import (
	"dbus/com/deepin/api/localehelper"
	"fmt"
	"io/ioutil"
	dutils "pkg.linuxdeepin.com/lib/utils"
	"strings"
)

const (
	supportedLocaleFile = "/usr/share/i18n/SUPPORTED"
	defaultLocaleFile   = "/etc/default/locale"
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
	var file string
	switch locale {
	case "zh_CN.UTF-8":
		file = "/usr/share/zoneinfo/Asia/Shanghai"
	case "zh_TW.UTF-8":
		file = "/usr/share/zoneinfo/Asia/Taipei"
	default:
		file = "/usr/share/zoneinfo/UTC"
	}

	return dutils.SymlinkFile(file, "/etc/localtime")
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
