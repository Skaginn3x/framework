//Imports filesystem tools
const fs = require('fs');
const os = require('os');

//Get the target symlink dir. Assumes that is /$HOME/.local/share/cockpit
const targetDir = os.homedir() + "/.local/share/cockpit";
//Change directory, as symlink must be run in the dir in which the symlink should reside
process.chdir(targetDir);

//Get the root module dir. Assumes we are in /install
const moduleRootDir = __dirname.substring(0, __dirname.lastIndexOf('/'));
//Gets the folder name for a symlink name
const moduleFolderName = moduleRootDir.substring( moduleRootDir.lastIndexOf('/') + 1, moduleRootDir.length );
console.log("Linking " + targetDir + " to " + moduleRootDir + " as " + moduleFolderName);

//Make the link, display errors if present
fs.symlink(
    moduleRootDir + "/build",
    moduleFolderName,
    "dir",
    (err) => {
        if (err !== null) {
            console.log(err);
        }
    }
);