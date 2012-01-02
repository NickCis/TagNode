var tn = require('./build/Release/tagnode');

var filepath = '/home/nickcis/musica.mp3';
var obj = new tn.TagNode(filepath);

console.log('Pre Read');
obj.read(function(err, dat) {
	console.log(err, dat);
	console.log('--------------------');
	for (key in obj) console.log('%s -> %s', key, obj[key]);
	console.log('Before Setting title %s', obj.title);
	obj.title = 'Pepe y los cosmo nautas';
	console.log('After Setting title %s', obj.title);
	obj.write(function(err) {
		var newObj = new tn.TagNode(filepath);
		newObj.read(function(err, dat) {
			console.log('New Object--------------------');
			for (key in obj) console.log('new Object: %s -> %s', key, obj[key]);
			console.log('--------------------');
		});
	});
});

