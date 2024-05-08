# ethtool-lua
The `ethtool-lua` library is a partial re-implementation of the [ethtool](https://cdn.kernel.org/pub/software/network/ethtool/).
The goal is to provide the CLI queries and configuration options as a Lua API.

## Motivation
The reason for staring this library, was the desire for a nice and efficient way to query DSA switch statistics in the [prometheus-node-exporter-lua](https://openwrt.org/packages/pkgdata/prometheus-node-exporter-lua) on [OpenWRT](https://openwrt.org/) devices.
Existing suggestions around the internet focussed mainly on calling the [ethtool](https://cdn.kernel.org/pub/software/network/ethtool/) CLI program and parsing the output.
This is neither elegant nor efficient, as the collection time for this implementation was 300% higher on my rtl838x based switch running [OpenWRT](https://openwrt.org/).

## Supported Lua versions
Currently, only Lua 5.1 is supported, which is the version used by default for [OpenWRT](https://openwrt.org/packages/pkgdata/lua).
On other devices with Lua 5.1, the library can also be used. For example, install the [lua51](https://archlinux.org/packages/?name=lua51) package for Arch Linux derived distributions.

## Building the library
Assuming a Linux environment, these instructions should get you started:
1. Clone the git repo:
```bash
git clone https://github.com/kevinjil/ethtool-lua.git
```
2. Create and change to build directory:
```bash
mkdir ethtool-lua-build
cd ethtool-lua-build
```
3. Start configuring:
```bash
ccmake ../ethtool-lua
```
4. Configure with `c`, `c`, and generate the Makefile with `g`
5. Build the project:
```bash
make
```
6. Copy the library file `ethtool.so` to your project or system-wide Lua library path.

## Using the library
To use the library, require the `ethtool` library, open a socket, and use the functions on the returned Lua object.
```lua
require "ethtool"

local eth = ethtool.open()

local stats,err = eth:statistics("eno1")
if stats then
    for k,v in pairs(stats) do
        print(k .. ": " .. v)
    end
else
    print("Error: " .. err)
end

eth:close()
```
Note that the socket is closed explicitly in this example. This is not strictly necessary, as an open socket will be closed when the `eth` object is garbage collected.

## Implementation status
Currently, the focus is on queries.

| ethtool CLI | Lua API | Status |
|-------------|---------|--------|
| `--driver devname` | `eth:driver(devname)` | :x: |
| `--phy-statistics devname` | `eth:statistics(devname)` | :x: |
| `--show-coalesce devname` | `eth:show_coalesce(devname)` | :x: |
| `--show-features devname` | `eth:show_features(devname)` | :x: |
| `--show-pause devname` | `eth:show_pause(devname)` | :x: |
| `--show-permaddr devname` | `eth:show_permaddr(devname)` | :x: |
| `--show-ring devname` | `eth:show_ring(devname)` | :x: |
| `--statistics devname` | `eth:statistics(devname)` | :heavy_check_mark: |

Feel free to request additional feature implementations, both listed and unlisted in the table, in GitHub issues for the project.
