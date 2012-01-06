var TagNode = require('./build/Release/tagnode');

var TagNodeObj = new TagNode.TagNode('/home/nickcis/musica.mp3');
TagNodeObj.read(function(err, dat) {
	if (dat.tag) {
		var tags = ['title', 'artist', 'album', 'year', 'comment', 'track', 'genre'];
		for (key in tags) console.log('%s -> %s', tags[key], dat[tags[key]]);
		setTimeout(function() {console.log(TagNodeObj.title);}, 10000);
	}
});
