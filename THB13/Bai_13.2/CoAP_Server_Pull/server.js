const coap = require('coap');
const server = coap.createServer();
let temperature = 30.0;
let observers = [];

server.on('request', (req, res) => {
    if (req.url === '/sensor/temp') {
        // Nếu client đăng ký Observe
        if (req.headers.Observe === 0) {
            console.log('Client registered Observe');
            observers.push(res);
            res.write(`temp=${temperature}`);
        }
        else {
            // GET thông thường (Polling)
            res.end(`temp=${temperature}`);
        }
    }
});

// Giả lập nhiệt độ thay đổi mỗi 5 giây
setInterval(() => {
    temperature += 0.5;
    console.log('Temperature updated:', temperature);

    observers.forEach(res => {
        res.write(`temp=${temperature}`);
    });
}, 5000);

server.listen(() => {
    console.log('CoAP server started on port 5683');
});