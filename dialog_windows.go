//go:build windows

// NOTE: Be sure to compile the Windows binary with the `-ldflags "-H windowsgui"` flag
// to avoid a console window from appearing at launch.

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
	url := fmt.Sprintf("https://github.com/%s/%s/releases/latest", owner, repo)

	// 0x00000001 is the MB_OKCANCEL flag, (message box with OK and Cancel buttons)
	ret := messageBox(text, caption, 0x00000001)

	// ret will be 1 if the user clicked OK
	if ret == 1 {
		exec.Command("rundll32", "url.dll,FileProtocolHandler", url).Start()
	}
}

func messageBox(text, caption string, flags uintptr) int {
	user32 := syscall.NewLazyDLL("user32.dll")
	msgBox := user32.NewProc("MessageBoxW")

	// StringToUTF16Ptr converts a Go string to a pointer to a UTF-16 encoded string
	// so that it can be used with the Windows API.
	// msgBox.call returns 3 values, but we only care about the first one (the return value).
	// The second and third values are unused, so they are ignored by using underscores.
	ret, _, _ := msgBox.Call(
		0,
		uintptr(unsafe.Pointer(syscall.StringToUTF16Ptr(text))),
		uintptr(unsafe.Pointer(syscall.StringToUTF16Ptr(caption))),
		flags,
	)
	return int(ret)
}
