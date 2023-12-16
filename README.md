# loco

Commandline program to handle the longest common strings at the beginning of each line.


## Background

I had the situation that I had many files where in each file each line started with the same string, but for each file
that string was different. Some network paths/dirs/files. There was no way cover all files with a single regex. In my
specific scenario I was able to handle it using `sed`, because I was the one who ran the command to create these files
in the first place. But if I wouldn't have been, for example if someone would have sended me the files then it would
have been an annoying task. So I checked if there is a tool that can handle common strings at beginning of lines in a
file, and there was none, so I created `loco` to handle the **lo**ngest **co**mmon strings at start of line.


## Building

    make


## Usage

    loco [-vhpds] [-r <char>] [-R <string>] [file]

You can't pass a file when text is piped into `loco`.


## Options

    -p          print longest common string
    -d          print without longest common string
    -s          skip first line
    -r <char>   replace each character of longest common string with <char>
    -R <string> replace longest common string with <string>
    -v          print version info
    -h          print help text


## Examples

### How the demo file looks like

    $ cat demo.txt 
    abcdfoobar
    abcdtrouper
    abcdsuper

### Print longest common string

    $ loco -p demo.txt 
    abcd

### Delete longest common string

    $ loco -d demo.txt 
    foobar
    trouper
    super

### Replace each character of longest common string with _

    $ loco -r _ demo.txt 
    ____foobar
    ____trouper
    ____super

### Replace longest common string with string

    $ loco -R prefix- demo.txt 
    prefix-foobar
    prefix-trouper
    prefix-super

### Replace longest common string with string but skip first line and read from pipe

    $ fd
    foo/
    foo/abc
    foo/def
    foo/ghi

    $ fd | loco -s -R '├─ '
    foo/
    ├─ abc
    ├─ def
    ├─ ghi

