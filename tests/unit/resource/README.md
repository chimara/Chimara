To recreate the Blorb file, run inblorb on the Blurb manifest file.
e.g.,

```sh
/path/to/inform/inblorb/Tangled/inblorb unittest.blurb unittest.blorb
```
Then, hex-edit bytes 0x24-0x27 from `Snd ` to `Data` (0x44617461), because I'm
not sure it's possible with the current inblorb to embed a `FORM` chunk directly
without a `BINA` chunk in front of it; however, that's the situation we need to
test.

The sound file `silence.aiff` is a hand-crafted minimal AIFF file
consisting of these bytes:
```
00000000  46 4f 52 4d 00 00 00 38  41 49 46 46 43 4f 4d 4d  |FORM...8AIFFCOMM|
00000010  00 00 00 12 00 01 00 00  00 05 00 10 40 0e ac 44  |............@..D|
00000020  00 00 00 00 00 00 53 53  4e 44 00 00 00 12 00 00  |......SSND......|
00000030  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
```
