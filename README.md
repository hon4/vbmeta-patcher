# vbmeta-patcher
A simple, easy, tiny vbmeta patcher to enable or disable verification.

### How to get it on Windows
Just get the `vbmeta-patcher-<version>.exe` file from releases.

### How to get it on Linux/Android
Download the `vbmeta-patcher-<version>.tar.gz` file from releases and run the following commands.
```
./configure
make
make install
```

If you are using Termux you will need to use these commands:
```
./configure --prefix=$PREFIX
make
make install
```

### Example usage
#### To disable verification
You can use the following commands or just drag and drop the `.img` file on the executable.
```
vbmeta-patcher vbmeta-original.img
```

#### To enable verification
```
vbmeta-patcher vbmeta-original.img -e
```

After running these commmands you will see a file ending with `-patched` in the same dir as your source file.
For example if your source file is `vbmeta.img` the patched will be `vbmeta-patched.img`.

### More info
For detailed info you can use `-h` command.
```
vbmeta-patcher -h
```
