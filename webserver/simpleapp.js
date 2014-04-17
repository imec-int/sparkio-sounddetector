#!/usr/bin/env node

var serialport = require("serialport");
var SerialPort = serialport.SerialPort; // localize object constructor

var sp = new SerialPort("/dev/tty.usbmodemfa1311", {
  parser: serialport.parsers.readline("\n"),
  baudrate: 9600
});


sp.on("open", function () {
	console.log('serialport is open');
});

sp.on('data', function (data) {
	console.log(data);
});



	// serialPort.write("ls\n", function(err, results) {
	// 	console.log('err ' + err);
	// 	console.log('results ' + results);
	// });
