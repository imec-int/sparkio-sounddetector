#!/usr/bin/env node

var express = require('express');
var http = require('http')
var path = require('path');
var utils = require('./utils');
var socketio = require('socket.io');
var serialport = require('serialport');
var SerialPort = serialport.SerialPort;
var net = require('net');

var app = express();

app.configure(function(){
	app.set('port', process.env.PORT || 3000);
	app.set('views', __dirname + '/views');
	app.set('view engine', 'jade');
	app.use(express.favicon());
	app.use(express.logger('dev'));
	app.use(express.bodyParser());
	app.use(express.methodOverride());
	app.use(app.router);
	app.use(require('stylus').middleware(__dirname + '/public'));
	app.use(express.static(path.join(__dirname, 'public')));
});

app.configure('development', function(){
	app.use(express.errorHandler());
});

var webserver = http.createServer(app).listen(app.get('port'), function(){
	console.log("Express server listening on port " + app.get('port'));
});
var io = socketio.listen(webserver);
io.set('log level', 0);

app.get('/', function (req, res){
	res.render('index', { title: 'Hello World' });
});


// http put:
app.put('/rest/soundstate', function (req, res){
	console.log(req.body);

	if(req.body.action)
		io.sockets.emit('action', req.body.action);

	res.send('received: ' + req.body.action + "\n\n");
});




// simple tcp server:
var tcpport = 8000;
var tcpserver = net.createServer(function (socket) {

	console.log('tcp socket connected: ' + socket.remoteAddress);

	socket.on('data', function (data) {
		data = data.toString();
		console.log('tcp data:', data);
		io.sockets.emit('action', data);
	});


	socket.on('end', function () {
		console.log('tcp socket disconnected');
	});

	socket.on('error', function (err) {
		console.log(err);
	});
});

tcpserver.listen(tcpport);
console.log('> tcp server listening on port ' + tcpport);




// var sp = new SerialPort("/dev/tty.usbmodemfd121", {
// 	parser: serialport.parsers.readline("\n"),
// 	baudrate: 9600
// });



// sp.on("open", function () {
// 	console.log('serialport is open');
// });

// sp.on('data', function (data) {
// 	console.log('serial data:', data);
// 	io.sockets.emit('button', data.replace(/\r/g, ''));
// });

// sp.on("error", function (err) {
// 	console.log(err);
// });



