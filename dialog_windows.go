//go:build windows

package main

import (
	"fmt"
	"os/exec"
	"syscall"
	"unsafe"
)

func showUpdateDialog(latestVersion string) {
	text := fmt.Sprintf("A new version (%s) is available!\nGo to GitHub?", latestVersion)
	caption := "Update Available"

	ret := messageBox(text, caption, 0x00000001)

	// ret will be 1 if the user clicked OK
	if ret == 1 {
		exec.Command("rundll32", "url.dll,FileProtocolHandler", "https://github.com/itgmania/itgmania/releases/latest").Start()
	}
}

func messageBox(text, caption string, flags uintptr) int {
	user32 := syscall.NewLazyDLL("user32.dll")
	msgBox := user32.NewProc("MessageBoxW")
	ret, _, _ := msgBox.Call(
		0,
		uintptr(unsafe.Pointer(syscall.StringToUTF16Ptr(text))),
		uintptr(unsafe.Pointer(syscall.StringToUTF16Ptr(caption))),
		flags,
	)
	return int(ret)
}
