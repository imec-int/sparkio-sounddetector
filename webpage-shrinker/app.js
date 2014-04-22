#!/usr/bin/env node

var fs = require('fs');


var files = fs.readdir(__dirname + '/shrink', function (err, files) {
	if(err) return console.log(err);


	for(var i in files){
		var file = __dirname + '/shrink/' + files[i];
		fs.readFile(file, {encoding: 'utf8'}, function (err, data) {
			if(err) return console.log(err);

			// console.log(data);

			var shrinked = data.replace(/(\r\n|\n|\r|\t)/gm,"");
			var length = shrinked.length;

			shrinked = shrinked.replace(/\"/gm, "\\\"");

			var withHeader = "HTTP/1.1 200 OK\\r\\nServer: samserver/0.0.1\\r\\nContent-Type: text/html\\r\\nContent-Length: "+length+"\\r\\nConnection: close\\r\\n\\r\\n" + shrinked + "\\0";

			var newfile = __dirname + '/shrinked/' + files[i];

			fs.writeFile(newfile, withHeader, function (err, res) {
				if(err) return console.log(err);
				console.log("file written: " + newfile);
			});
		});
	}
});



