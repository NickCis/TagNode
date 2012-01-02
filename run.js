var tn = require('./build/Release/tagnode');

//for (key in tn) console.log(key);
//var obj = new tn.TagNode('/home/nickcis/Music/OSI/Blood/01. The Escape Artist.mp3');
var obj = new tn.TagNode('/home/nickcis/asd.jpg');
//for (key in obj) console.log(key);
console.log('Pre Read');
obj.read(function(err, dat) {
	console.log('In read');
	console.log(err, dat);
	console.log('--------------------');
	for (key in obj) console.log('%s -> %s', key, obj[key]);
	console.log('--------------------');
});

