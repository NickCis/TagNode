var tn = require('./build/Release/tagnode');

//for (key in tn) console.log(key);
var obj = new tn.TagNode('/home/nickcis/Music/OSI/Blood/01. The Escape Artist.mp3');
//for (key in obj) console.log(key);
	
obj.read(function() {
	console.log('--------------------');
	for (key in obj) {
		if (key != 'read' && typeof(obj[key]) == "function")
			console.log('%s -> %s', key, obj[key]());
	}
	console.log(obj.title);
	console.log(obj.artist);
	console.log('--------------------');
	//console.log(obj.title());
	//console.log(obj.track());
	//console.warn(obj);
	//console.warn(obj.path());
});

