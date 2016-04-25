package main

import "io/ioutil"
import "pkg.linuxdeepin.com/lib/dbus"
import log "pkg.linuxdeepin.com/lib/log"
import "os"
import "os/exec"

var logger = log.NewLogger("com.deepin.helper.LanguageSelector")

type LanguageSelector struct {
}

func run_shell_content(code string, argv ...string) {
	f, err := ioutil.TempFile(os.TempDir(), "shell_code")
	if err != nil {
		logger.Warning("can't create temp file: ", err)
		return
	}
	f.Close()
	os.Remove(f.Name())

	ioutil.WriteFile(f.Name(), ([]byte)(code), 0755)
	argv = append([]string{f.Name()}, argv...)
	cmd := exec.Command("/bin/sh", argv...)
	d, err := cmd.Output()
	logger.Info(string(d))
	if err != nil {
		logger.Error("Can't run shell code:", err)
	}
	defer os.Remove(f.Name())
}

func (*LanguageSelector) Set(lang string) {
	run_shell_content(shell_code, lang)
}

func (*LanguageSelector) GetDBusInfo() dbus.DBusInfo {
	return dbus.DBusInfo{
		"com.deepin.helper.LanguageSelector",
		"/com/deepin/helper/LanguageSelector",
		"com.deepin.helper.LanguageSelector",
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
