# `rustkill`
A highly accurate and efficient Linux tool for killing Rust programs for.

## Usage
```
Usage: rustkill [options] whitelist...
Options:
  -h [ --help ]         Help screen
  --whitelist arg       A list of Rust programs to spare
  -i [ --interval ] arg Kill Rust programs at this interval in milliseconds

```

## Build & Install
```
$ make
# make install

```
*Note that `rustkill` depends on the boost program options library*

*Note that `#` denotes a root shell, while `$` denotes a regular shell.*
