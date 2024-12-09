# trim-ytdlp

Make sure to link against the math library when compiling!!

```bash
    gcc -o trim-ytdlp main.c -lm
```

### Usage
```bash
    trim-ytdlp <-d | -f> <Folders | Files>  [-n] [Name]
```
The ``` d ``` flag takes folders as arguments and bulk renames all valid files inside of them

```n``` is an optional flag, specifying a custom new name for the file/files.
If ```n``` is used with ```d```, files in the directory will have a number before the name, starting from 1
If it's used with ```f```, files will be enumerated as with ```d``` if there are more than one. If there's only
one file, it won't be enumerated.