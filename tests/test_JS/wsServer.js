const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8080 });

wss.on('connection', function connection(ws) {
  ws.on('message', function incoming(message) {
    console.log('received: %s', message);
  });

  ws.send('something');
  setTimeout(doPing, 1000);

  function doClose() {
    console.log('close client connection');
    ws.close();
    }

  function doPing() {
    console.log('ping client connection');
    ws.ping("42");  
    }
});

