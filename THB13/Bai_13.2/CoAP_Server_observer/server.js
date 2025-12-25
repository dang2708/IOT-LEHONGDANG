const coap = require('coap');
const server = coap.createServer();
let temperature = 30.0;
let observers = [];

server.on('request', (req, res) => {
    const clientAddr = `${req.rsinfo.address}:${req.rsinfo.port}`;
    
    // Endpoint để đăng ký Observer (dùng PUT từ ESP32)
    if (req.url === '/sensor/temp/observe') {
        console.log('✅ Client registered Observe:', clientAddr);
        
        // Lưu thông tin client để gửi notification
        const exists = observers.find(obs => 
            obs.ip === req.rsinfo.address && obs.port === req.rsinfo.port
        );
        
        if (!exists) {
            observers.push({
                ip: req.rsinfo.address,
                port: req.rsinfo.port,
                active: true
            });
        }
        
        // Gửi giá trị hiện tại
        res.end(`temp=${temperature}`);
    }
    // Endpoint để hủy Observer
    else if (req.url === '/sensor/temp/unobserve') {
        console.log('❌ Client unregistered:', clientAddr);
        observers = observers.filter(obs => 
            obs.ip !== req.rsinfo.address || obs.port !== req.rsinfo.port
        );
        res.end('Unregistered');
    }
    // GET thông thường (Polling)
    else if (req.url === '/sensor/temp') {
        console.log('📊 GET request from:', clientAddr);
        res.end(`temp=${temperature}`);
    }
    else {
        res.statusCode = '4.04';
        res.end('Not Found');
    }
});

// Giả lập nhiệt độ thay đổi mỗi 5 giây
setInterval(() => {
    temperature += 0.5;
    console.log('\n🌡️  Temperature updated:', temperature);

    if (observers.length > 0) {
        console.log(`📤 Sending notifications to ${observers.length} observer(s):`);
        
        // Gửi notification đến tất cả observers
        observers.forEach((obs, index) => {
            console.log(`   [${index + 1}] → ${obs.ip}:${obs.port}`);
            
            const req = coap.request({
                host: obs.ip,
                port: obs.port,
                pathname: '/sensor/temp/notify',
                method: 'PUT',
                confirmable: false
            });
            
            const payload = `temp=${temperature}`;
            req.write(payload);
            req.end();
            
            req.on('error', (err) => {
                console.log(`   ✗ Error: ${err.message}`);
            });
        });
    } else {
        console.log('   (No observers registered)');
    }
}, 5000);

server.listen(5683, () => {
    console.log('🚀 CoAP Observer Server started on port 5683');
    console.log('📡 Waiting for clients...\n');
});