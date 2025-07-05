//go:build windows

package main

import (
	"os/exec"
	"syscall"
)

const targetExe = "ITGmania.exe"

func launchTarget() {
	cmd := exec.Command(targetExe)
	cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}
	cmd.Start()
}
