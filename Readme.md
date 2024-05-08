# Line Following Car 

Dự án này về việc tạo ra một chiếc xe tự đi theo đường sử dụng Arduino. Xe sử dụng cảm biến hồng ngoại (IR sensors) để phát hiện đường và điều chỉnh lộ trình của mình một cách phù hợp. Nó cũng bao gồm cảm biến khoảng cách để dừng xe lại nếu phát hiện vật cản trong phạm vi nhất định.

## Các Thành Phần

- Arduino Uno
- [Bộ điều khiển động cơ (Motor Driver, L298N hoặc tương tự)](http://arduino.vn/bai-viet/893-cach-dung-module-dieu-khien-dong-co-l298n-cau-h-de-dieu-khien-dong-co-dc)
- 2 Động cơ DC
- [5 Cảm biến IR](https://thuongtin.net/mo-dun-cam-bien-hong-ngoai-ir-voi-arduino/)
- [Cảm biến khoảng cách siêu âm (Ultrasonic Distance Sensor, HC-SR04)](http://arduino.vn/bai-viet/233-su-dung-cam-bien-khoang-cach-hc-sr04)
- Dây (Jumper Wires)
- Breadboard (tùy chọn)

## Kết Nối

### Động Cơ
- Kết nối động cơ trái vào chân 9 (IN3), 10 (IN4), và 11 (Enable) trên Arduino.
- Kết nối động cơ phải vào chân 7 (IN1), 8 (IN2), và 6 (Enable) trên Arduino.

### Cảm Biến IR
- Kết nối cảm biến IR vào các chân analog A0 đến A4 trên Arduino.

### Cảm Biến Khoảng Cách Siêu Âm
- Kết nối chân Trig vào chân 12 và chân Echo vào chân 13 trên Arduino.

## Tổng Quan Code

Code được cấu trúc thành một số hàm chính:

- `setup()`: Khởi tạo các chân động cơ và cảm biến.
- `loop()`: Chứa logic chính để điều khiển xe. Nó kiểm tra khoảng cách từ bất kỳ vật cản nào và quyết định liệu xe có di chuyển hay dừng lại. Nếu di chuyển, nó quét đường, cập nhật lỗi dựa trên vị trí của đường, tính toán sự điều chỉnh cần thiết, và điều chỉnh tốc độ động cơ cho phù hợp.
- `calDistance()`: Tính toán khoảng cách đến vật cản gần nhất sử dụng cảm biến siêu âm.
- `Scan()`: Đọc cảm biến IR và cập nhật biến `irSensors` với vị trí của đường.
- `UpdateError()`: Cập nhật biến `error` dựa trên vị trí của đường.
- `UpdateCorrection()`: Tính toán sự điều chỉnh cần thiết dựa trên `error`.
- `Drive()`: Điều chỉnh tốc độ động cơ dựa trên sự điều chỉnh đã tính toán.

## Cách Sử Dụng

1. Lắp ráp xe với các thành phần được liệt kê ở trên.
2. Tải code đã cung cấp lên Arduino của bạn.
3. Đặt xe trên một đường có nền trắng, line đen mà cảm biến IR có thể phát hiện.
4. Bật nguồn cho xe và quan sát nó đi theo đường. Nó sẽ dừng lại nếu phát hiện vật cản trong phạm vi 10cm.

## Tùy Chỉnh

Bạn có thể điều chỉnh biến `maxSpeed` để thay đổi tốc độ của xe. Độ nhạy của việc phát hiện đường và điều chỉnh có thể được tinh chỉnh bằng cách sửa đổi các hàm `UpdateError()` và `UpdateCorrection()`.

## Lưu Ý

Dự án này là một triển khai cơ bản của xe tự đi theo đường. Có thể cải thiện thêm bằng cách thêm nhiều cảm biến hơn, triển khai điều khiển PID cho các chuyển động mượt mà hơn, hoặc tích hợp điều khiển Bluetooth cho việc can thiệp thủ công.