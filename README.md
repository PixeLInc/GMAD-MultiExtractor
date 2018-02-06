# A GMOD GMA Converter that reads `.gma` files and extracts the files within to a specified directory.

### How do I use this?
Just clone the repo and compile the source file by doing:
```
gcc gmad.c -o gout
```

#### Running the compiled file

It's super easy to use. You just specify the directory in which the addon files are, and an ouput directory.
~~You can also, if you'd like, specify a file to extract by itself instead of a whole directory.~~ (will be fixed in the future)

A few example usages are below.
```
./gout addons/ output/

----------------

./gout ./ output/

----------------

./gout addons/ ./
```
