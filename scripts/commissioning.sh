#!/bin/bash

# Check if the user is root
if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root. Bumping to root."
  # Use sudo to run the script with root permissions
  sudo "$0" "$@"
  # Exit the script once it's been run with root permissions
  exit $?
fi

set -e

echo "Adding group tfc"
groupadd tfc || echo ""

echo "Creating directory /var/tfc"
mkdir -p /var/tfc

echo "Creating directory /run/tfc"
mkdir -p /run/tfc

echo "Assigning directory /var/tfc to group tfc"
chgrp tfc /var/tfc

echo "Write permissions to /var/tfc and /run/tfc"
chmod 775 /var/tfc
chmod 775 /run/tfc

if [[ -n "$SUDO_USER" ]]; then
  echo "Adding user $SUDO_USER to tfc group"
  usermod -aG tfc "$SUDO_USER"
  su "$SUDO_USER"
else
  # todo try some other way
  echo "The script was run by the root user, unable to add user to tfc group"
fi
