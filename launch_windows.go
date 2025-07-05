//go:build windows

package main

import (
	"os/exec"
	"syscall"
)

const targetExe = "ITGmania.exe"

func launchTarget() {
	cmd := exec.Command(targetExe)
	
	// HideWindow is used to make sure no console window appears
	cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}

	cmd.Start()
}
