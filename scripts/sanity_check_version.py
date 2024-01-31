# Verify that the version string matches in all locations
# Run this script from the root level of your tfc project
# not inside the scripts folder
import json
import re

# Read and parse the json file
def read_and_parse(filename: 'string'):
    with open(filename, 'r') as f:
        return json.loads(f.read())


# Return the vcpkg version string
def vcpkg_version_string():
    return read_and_parse('vcpkg.json')["version"]

# Return the cockpit version string
def cockpit_version_string():
    return read_and_parse('cockpit/package.json')["version"]

# Return the base cmake version
def cmake_project_version():
    cmake_contents = ""
    with open("CMakeLists.txt", "r") as f:
        cmake_contents = f.read()

    ## Use regex and try and find the version of the format "2023.10.2"
    p = re.compile(r'\d{4}\.\d{1,2}.\d*')
    version_strings = p.findall(cmake_contents)
    if len(version_strings) != 1:
        print("Multiple possible versions found {version_strings}")
        exit(-1)
    return version_strings[0]

if __name__ == "__main__":
    cockpit_version   = cockpit_version_string()
    vcpkg_version = vcpkg_version_string()
    cmake_version = cmake_project_version()
    print(f"Cockpit ipc version {cockpit_version}")
    print(f"vcpkg version {vcpkg_version}")
    print(f"Cmake version {cmake_version}")

    if cockpit_version != vcpkg_version:
        print("ipc_version and vcpkg_version dont match!")
        exit(-1)

    if cmake_version != vcpkg_version:
        print("cmake_version and vcpkg_version dont match!")
        exit(-1)

    exit(0)