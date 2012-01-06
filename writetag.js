var TagNode = require('./build/Release/tagnode');

var TagNodeObj = new TagNode.TagNode('/home/nickcis/musica.mp3');
TagNodeObj.title = "Pepe";
TagNodeObj.year = 2010;

TagNodeObj.write(function(err) {
	if (! err)
	console.log('Writing successful');
});
