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
	app.set('port', process.env.PORT || 8090);
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
app.put('/message/putit', function (req, res){

	console.log(req.body);

	// if(req.body.action)
	// 	io.sockets.emit('action', req.body.action);

	res.send('received: ' + req.body + "\n\n");
});


