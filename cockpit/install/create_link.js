const fs = require('fs');
const os = require('os');
const path = require('path');

// Get the target symlink dir. Assumes that is /$HOME/.local/share/cockpit
const targetDir = path.join(os.homedir(), '.local', 'share', 'cockpit');

// Ensure the target directory exists
if (!fs.existsSync(targetDir)) {
  console.log(`Target directory ${targetDir} does not exist. Creating...`);
  fs.mkdirSync(targetDir, { recursive: true });
}

// Change directory, as symlink must be run in the dir in which the symlink should reside
process.chdir(targetDir);

// Get the root module dir. Assumes we are in /install
const moduleRootDir = path.dirname(__dirname);
// Gets the folder name for a symlink name
const moduleFolderName = path.basename(moduleRootDir);

// Check if symlink already exists
if (fs.existsSync(moduleFolderName)) {
  console.log(`A file or directory with the name ${moduleFolderName} already exists in ${targetDir}.`);
  process.exit(0);
}
console.log(`Linking ${targetDir} to ${moduleRootDir} as ${moduleFolderName}`);

// Make the link, display errors if present
fs.symlink(
  path.join(moduleRootDir, 'build'),
  moduleFolderName,
  'dir',
  (err) => {
    if (err) {
      console.error(`Failed to create symlink: ${err.message}`);
      process.exit(1);
    } else {
      console.log(`Successfully created symlink at ${path.join(targetDir, moduleFolderName)}`);
    }
  },
);
