#!/usr/bin/env python3
import subprocess
import os
import sys

def create_user(name, create_group=False, create_home=False, shell="/bin/bash"):
  if create_group:
    subprocess.run(["groupadd", name], check=True)
  if create_home:
    subprocess.run(["useradd", "-m", "-g", name, "-s", shell, name], check=True)
  else:
    subprocess.run(["useradd", "-g", name, "-s", shell, name], check=True)


if __name__ == "__main__":
  if os.getuid() != 0:
    print("Please run this script as root!", file=sys.stderr)
    exit -10
  # Service runner for tfc related services
  create_user("tfc", create_group=True, create_home=False, shell="/sbin/nologin")

  # Create directories for tfc, allow user & group read and write others read
  os.makedirs("/var/tfc", exist_ok=True, mode=0o775)
  os.makedirs("/run/tfc", exist_ok=True, mode=0o775)
  os.makedirs("/var/log/tfc", exist_ok=True, mode=0o775)
