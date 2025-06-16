const dgram = require('dgram');
const os = require('os');

// Create an UDP socket
const server = dgram.createSocket('udp4');

// Receive message event
server.on('message', (msg, rinfo) => {
  console.log(`Mesage received from ${rinfo.address}:${rinfo.port}:\n ${msg}\n`);
});

server.on('listening', () => {
  const address = server.address();
  console.log(`UDP server listening on ${address.address}:${address.port}`);

  // Show local IP adresses 
  const interfaces = os.networkInterfaces();
  console.log('Addresses available on the local network:');
  for (const iface of Object.values(interfaces)) {
    for (const config of iface) {
      if (config.family === 'IPv4' && !config.internal) {
        console.log(` → ${config.address}`);
      }
    }
  }
});

// Start server
server.bind(41234);
