var path = require('path');
var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);
app.use(express.static(path.join(__dirname)));
app.get('/', function(req, res){
  res.sendFile('index.html', {root: __dirname});
});

var available = [];

function isAvailable(socket) {
  return available.some(v => v === socket);
}

io.on('connection', function(socket) {
  console.log("Got a connection!");
  socket.on('available', function() {
    available.push(socket);
  });
  socket.on('unavailable', function() {
    available = available.filter(function(v) {
      return v !== socket;
    });
  });
  socket.on('offer', function(msg) {
    if (available.length === 0) {
      available.push(socket);
      console.log("First available dude...");
      socket.emit('cancel-connection');
      return;
    }
    console.log("Got offer, piping it through...");

    var peer = available.pop();
    console.log("Available length: ", available.length);
    socket.on('ice-candidate', function(v) {
      console.log('Got ice-candidate, piping it through...');
      peer.emit('ice-candidate', v);
    });
    peer.on('answer', function(msg) {
      console.log("Got answer, piping it through...");
      socket.emit('answer', msg);
    });
    peer.on('disconnect', function() {
      // available again when peer disconnects
      // something tells me that this is all hacky anyway... we'd need retry logic and stuff
      available.push(socket);
    });

    peer.emit('offer', msg);
  });

  socket.on('disconnect', function() {
    console.log("Disconnected!");
    available = available.filter(function(v) {
      return v !== socket;
    });
  });
});

http.listen(3000, function(){
  console.log('listening on *:3000');
});
