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
	flags := uintptr(0x00000001) // MB_OKCANCEL

	user32 := syscall.NewLazyDLL("user32.dll")
	msgBox := user32.NewProc("MessageBoxW")
	ret, _, _ := msgBox.Call(0,
		uintptr(unsafe.Pointer(syscall.StringToUTF16Ptr(text))),
		uintptr(unsafe.Pointer(syscall.StringToUTF16Ptr(caption))),
		flags)
	if int(ret) == 1 { // OK pressed
		exec.Command("rundll32", "url.dll,FileProtocolHandler", "https://github.com/itgmania/itgmania/releases/latest").Start()
	}
}
