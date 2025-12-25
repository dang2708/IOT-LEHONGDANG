const coap = require('coap');
const server = coap.createServer();

// Giả lập firmware dung lượng 2KB (2048 bytes) toàn ký tự 'A'
const firmware = Buffer.alloc(2048, 'A'); 

server.on('request', (req, res) => {
    if (req.url === '/firmware') {
        console.log('Firmware requested');
        // Thư viện coap của Node.js tự động xử lý chia nhỏ gói (Blockwise) khi payload lớn
        res.end(firmware);
    }
});

server.listen(() => {
    console.log('CoAP Blockwise server started on port 5683');
});