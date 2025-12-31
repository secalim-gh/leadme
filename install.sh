#!/bin/sh
set -e

# Build the project
./build.sh

echo "Successfully built!"

# Install to /usr/local/bin
echo "Installing to /usr/local/bin..."
sudo cp leadme /usr/local/bin/leadme
sudo chmod +x /usr/local/bin/leadme

echo "Done! You can now run 'leadme -s' to start the server, then run 'leadme' for the launcher."
echo "leadme requires the ~/.config/leadme/config file to load a custom configuration."
