/* Khai báo chân động cơ */
const int motorREnable = 6; // Chân Enable cho động cơ phải
const int motorRPin1 = 7; // chân tín hiệu 1 cho động cơ bên phải, kết nối với IN1               
const int motorRPin2 = 8; // chân tín hiệu 2 cho động cơ bên phải, kết nối với IN2

const int motorLPin1 = 9; // chân tín hiệu 1 cho động cơ bên trái, kết nối với IN3               
const int motorLPin2 = 10; // chân tín hiệu 2 cho động cơ bên trái, kết nối với IN4
const int motorLEnable = 11; // Chân Enable cho động cơ trái


/* Khai báo chân cảm biến hồng ngoại */
#define NUM_IR 5 // Số lượng cảm biến hồng ngoại
const int irPins[NUM_IR] = {A0, A1, A2, A3, A4};
// khai báo mảng để giữ các giá trị (1/0) cho các cảm biến ir ( Quy ước: 0 == trắng; 1 == đen) (**)
int irSensorDigital[NUM_IR] = {0,0,0,0,0};


/* Khai báo chân cảm biến khoảng cách */
unsigned long duration; // biến đo thời gian
int distance;           // biến lưu khoảng cách đọc được
const int trigPin = 12; // kết nối chân trigPin với chân 11 arduino
const int echoPin = 13; // kết nối chân echoPin với chân 12 arduino

// Biểu diễn nhị phân các giá trị cảm biến đọc được
// Từ Trái sang phải 
int irSensors = B00000; 

int count = 0; // Số lượng cảm biến nằm trên line (đen)

// Xác định độ lệch của xe. Error thuộc : [-180; 180] 
// Số âm là xe ở bên trái line, Số dương là xe ở bên phải line
int error = 0;  
int errorLast = 0;  // Lưu giá trị lỗi trước đó (dùng để nếu xe đi theo quán tính vượt ra khỏi line thì sẽ có thể về lại line)

// Giá trị hiệu chỉnh, dựa trên error
// Nó được sử dụng để thay đổi tốc độ động cơ tương đối với PWM.
int correction = 0; 



/* Thiết lập tốc độ và tốc độ quay tối đa */
int maxSpeed = 85; 

/* Các biến để theo dõi tốc độ hiện tại của động cơ */
int motorLSpeed = 0;
int motorRSpeed = 0;

void setup() {

    /*Thiết lập chân động cơ là OUTPUT */
    pinMode(motorLPin1,OUTPUT);        
    pinMode(motorLPin2,OUTPUT);
    pinMode(motorLEnable,OUTPUT);
    
    pinMode(motorRPin1,OUTPUT);        
    pinMode(motorRPin2,OUTPUT);
    pinMode(motorREnable,OUTPUT);
    
    /* Thiết lập chân cảm biến hồng ngoại là INPUT */
    for (int i = 0; i < NUM_IR; i++) {
        pinMode(irPins[i], INPUT);
    }

    /*Thiết lập chân cảm biến khoảng cách */
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

void loop() {
    // tinh khoang cach  
    distance = calDistance();

    if(distance > 10){ // nếu khoảng cách lớn hơn 10cm thì xe sẽ di chuyển
        Scan();
        UpdateError();
        UpdateCorrection();
        Drive();
    }
    else{ // nếu khoảng cách nhỏ hơn 10cm thì xe sẽ dừng lại
        digitalWrite(motorLEnable, HIGH);
        digitalWrite(motorLPin1, LOW);
        digitalWrite(motorLPin2, LOW);

        digitalWrite(motorREnable, HIGH);
        digitalWrite(motorRPin1, LOW);
        digitalWrite(motorRPin2, LOW);
    }
}

float calDistance(){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH);
    int distance = duration * 0.034 / 2;
    return distance;
}

void Scan() {
    irSensors = B00000;
    
    for (int i = 0; i < NUM_IR; i++) {
            irSensorDigital[i] = !digitalRead(irPins[i]);
        
            int temp = 4-i;
            irSensors = irSensors + (irSensorDigital[i] << temp);
        }    
}

void UpdateError() {
  
    errorLast = error;  
    
    switch (irSensors) {
    
        case B00000:
            if (errorLast < 0) { error = -120;}
            else if (errorLast > 0) {error = 120;}
            break;
        
        case B10000: 
        case B11100:
        case B11110:
        case B10110:
            error = -90;
            break;

        case B11000:
            error = -60;
            break;

        case B01100: 
        case B10011: // Đi theo line trắng(*)
            error = -30;
            break;

        case B00100:
        case B01110: 
        case B11111: 
// Đi  theo line trắng(*)
        case B10001:
        case B11011: 
            error = 0;
            break;

        case B00110: 
        case B11001: // Đi theo line trắng(*)
            error = 30;
            break;
        
        case B00011:
            error = 60;
            break;

        case B00001: 
        case B00111:
        case B01101:
        case B01111:
            error = 90;
            break;
            
        default:  
            error = errorLast;
    }
}

void UpdateCorrection() {

    if (error == 0) {
        correction = 0;
    }
    
    else if (error == 30) {
        correction = 20;
    }
    
    else if (error == 60) {
        correction = 75;
    }
    
    else if (error == 90) {
        correction = 320;
    }  
    
    else if (error ==120) {
        correction = 345;
    } 
    
    
    else if (error == -30 ) {
        correction = -20;
    }
    
    else if (error == -60) {
        correction = -75;
    }
    
    else if (error == -90 ) {
        correction = -320;
    }  
    
    else if (error == -120 ) {
        correction = -345;
    } 
    
    
    /* Điều chỉnh correction nếu maxSpeed nhỏ hơn 255 */
    correction = (int) (correction * maxSpeed / 255 + 0.5);
    
    if (correction > 0 && correction < 320 ) {
        motorRSpeed = maxSpeed - correction;
        motorLSpeed = maxSpeed;
    }
    if (correction == 320 || correction == 345) {
        motorRSpeed = -45;
        motorLSpeed = 20;
    }
    
    else if (correction < 0 && correction > -320) {
        motorRSpeed = maxSpeed;
        motorLSpeed = maxSpeed + correction;
    }
    if (correction == -320 || correction == -345) {
        motorRSpeed = 20;
        motorLSpeed = -45;
    }
        else if (correction == 0) {
        motorRSpeed = maxSpeed ;
        motorLSpeed = maxSpeed ;
    }
}

void Drive() {
    if (motorRSpeed > 255) {motorRSpeed = 255;}
    else if (motorRSpeed < -255) {motorRSpeed = -255;}
    
    if (motorLSpeed > 255) {motorLSpeed = 255;}
    else if (motorLSpeed < -255) {motorLSpeed = -255;}
    
    if (motorRSpeed > 0) { // Động cơ phải tiến
        analogWrite(motorREnable, motorRSpeed);
        digitalWrite(motorRPin1, LOW);
        digitalWrite(motorRPin2, HIGH);
    } 
    
    else if (motorRSpeed < 0) {// Động cơ phải đi lùi
        analogWrite(motorREnable, abs(motorRSpeed));
        digitalWrite(motorRPin1, HIGH);
        digitalWrite(motorRPin2, LOW);
    } 
    
    else if (motorRSpeed == 0) { // Động cơ bên phải dừng
        digitalWrite(motorREnable, HIGH);
        digitalWrite(motorRPin1, LOW);
        digitalWrite(motorRPin2, LOW);
    }
    
    if (motorLSpeed > 0) { // Động cơ bên trái tiến
        analogWrite(motorLEnable, motorLSpeed);
        digitalWrite(motorLPin1, LOW);
        digitalWrite(motorLPin2, HIGH);
    } 
    
    else if (motorLSpeed < 0) { // Động cơ bên trái lùi
        analogWrite(motorLEnable, abs(motorLSpeed));
        digitalWrite(motorLPin1, HIGH);
        digitalWrite(motorLPin2, LOW);
    } 
    
        else if (motorLSpeed == 0) { // động cơ bên trái dừng
        digitalWrite(motorLEnable, HIGH);
        digitalWrite(motorLPin1, LOW);
        digitalWrite(motorLPin2, LOW);
    }
}