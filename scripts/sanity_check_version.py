# Verify that the version string matches in all locations
# Run this script from the root level of your tfc project
# not inside the scripts folder
import json

# Read and parse the json file
def read_and_parse(filename: 'string'):
    with open(filename, 'r') as f:
        return json.loads(f.read())


# Return the vcpkg version string
def vcpkg_version_string():
    return read_and_parse('vcpkg.json')["version"]

# Return the cockpit IPC version string
def ipc_version_string():
    return read_and_parse('cockpit/IPC/package.json')["version"]

# Return the base cmake version
def cmake_project_version():
    json = read_and_parse('cmake/presets/cfg-build.json')['configurePresets']
    version_preset = [x for x in json if x['name'] == 'cfg-project-version']
    if (len(version_preset) != 1):
        print("Version preset not found in file!")
        exit(-1)
    version_preset = version_preset[0]
    return version_preset["cacheVariables"]["CMAKE_PROJECT_VERSION"]

if __name__ == "__main__":
    ipc_version   = ipc_version_string()
    vcpkg_version = vcpkg_version_string()
    cmake_version = cmake_project_version()
    print(f"Cockpit ipc version {ipc_version}" )
    print(f"vcpkg version {vcpkg_version}")
    print(f"Cmake version {cmake_version}")

    if ipc_version != vcpkg_version:
        print("ipc_version and vcpkg_version dont match!")
        exit(-1)

    if cmake_version != vcpkg_version:
        print("cmake_version and vcpkg_version dont match!")

    exit(0)