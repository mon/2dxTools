# 2dxTools

A set of tools for working with 2dx audio containers.

[Download here](https://github.com/mon/2dxTools/releases)

### 2dxDump

`2dxDump infile.2dx`

Takes a 2dx and dumps all its wavs. Names them sequentially with no leading zeros.

### 2dxBuild

`2dxBuild outfile.2dx`

Builds a 2dx with default paramaters for loop point/volume etc.

### 2dxMerge

`2dxMerge infile.2dx outfile.2dx`

For each file in `infile.2dx`, if there is a `.wav` present in the directory,
it will replace it in the new file. Useful for extracting a single audio file,
editing it, then adding it back.

### 2dxWavConvert

`2dxWavConvert infile outfile.wav [preview]`

Takes any file that `sox` supports (mp3, flac, wav, ogg, etc) and converts it to
the specific format required for 2dx files (MS-ADPCM wav with a block size of 256).
If the third argument is "preview", the file is clipped to exactly 10 seconds
to comply with preview wav requirements.

## Tools to come:

2dxTransfer - will work like 2dxMerge, but will take two input files, an output,
and a list of tracks to transfer. Metadata (such as loop points) will also be
transferred.

Enhancements to 2dxDump and 2dxBuild to generate and load xml files so track
parameters can be modified.

If you have a burning need for one of these unfinished tools, please get in
contact. I appreciate motivation.