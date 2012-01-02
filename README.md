# TabNode #
Async node.js implementation of the taglib c++ library.
Dependencies: taglib (c++ library)
These node wrapper is capable of:
* Reading tags
* Writting tags

To compile, run

    node-waf configure
    node-waf build

## Documentation ##
    new [TagNodeModule].TagNode([Path]);
Where [Path] is the path of the file. Retrives a new TagNode object

    [TagNodeObj].read([function(err, dat)]);
Where [function] is a function to be executed after getting the tags. Two arguments are passed to the function, err and dat. Err is the status error (null if everything is OK), dat is a dictiory with the tag information. Dat has the following format:

    {
         tag: [true if there is tag info],
         title: 'Pepe y los cosmo nautas',
         path: '/home/nickcis/musica.mp3',
         artist: 'Nice Artist',
         album: 'Nice Album',
         year: 2009,
         comment: 'This is a nice comment =D',
         track: 1,
         genre: 'Rock',
         audioProperties: [true if there is tag info],
         bitrate: 256,
         sample_rate: 48000,
         channels: 2,
         length: 349
    }

    [TagNodeObj].write([function(err)]);
Where [function] is a function to be executed after writing the tags to the file. One argument is passed, err. Err is the status error (null if everything is OK);

## Version
Version 1.0, tested with:
    - node 0.6.6
    - taglib 1.7-3
    - Arch Linux

## Example ##

### Reading Tags ###
    var TagNode = require('./build/Release/tagnode);

    var TagNodeObj = new TagNode.TagNode('/home/user/nicemp3.mp3');
    TagNodeObj.read(function(err, dat) {
         if (dat.tag) {
            var tags = ['title', 'artist', 'album', 'year', 'comment', 'track', 'genre'];
            for (key in tags) console.log('%s -> %s', tags[key], dat[tags[key]]);
         }
    });

### Writing Tags ###
    var TagNode = require('./build/Release/tagnode);

    var TagNodeObj = new TagNode.TagNode('/home/user/nicemp3.mp3');
    TagNodeObj.title = "Pepe";
    TagNodeObj.year = 2010;

    TagNodeObj.write(function(err) {
        if (! err)
            console.log('Writing successful');
    });

## License

(The Gpl v2 License)

Copyright (c) 2012 NickCis

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
