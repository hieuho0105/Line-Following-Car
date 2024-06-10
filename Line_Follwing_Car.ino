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
        
            int temp = 4 - i;
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
        case B10011: 
            error = -30;
            break;

        case B00100:
        case B01110: 
        case B11111: 
        case B10001:
        case B11011: 
            error = 0;
            break;

        case B00110: 
        case B11001:
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
    // Hệ số điều chỉnh cho các giá trị lỗi
    const int errorValues[] = {0, 30, 60, 90, 120, -30, -60, -90, -120};
    const int correctionValues[] = {0, 20, 75, 320, 345, -20, -75, -320, -345};
    const int numValues = sizeof(errorValues) / sizeof(errorValues[0]);

    // Các giá trị cực đại và cực tiểu cho việc điều chỉnh
    const int MinCorrection = 0;
    const int MaxCorrection = 320;
    const int ExtremeCorrectionPositive = 345;
    const int ExtremeCorrectionNegative = -345;
    const int MotorRSpeedAdjustmentNegative = -45;
    const int MotorLSpeedAdjustmentPositive = 20;
    const float CorrectionFactor = 0.5; // Hệ số điều chỉnh

    // Tìm giá trị điều chỉnh cho lỗi
    for (int i = 0; i < numValues; ++i) {
        if (error == errorValues[i]) {
            correction = correctionValues[i];
            break;
        }
    }

    // Điều chỉnh tốc độ động cơ
    correction = (int)(correction * maxSpeed / 255.0 + CorrectionFactor);

    if (correction > MinCorrection && correction < MaxCorrection) {
        motorRSpeed = maxSpeed - correction;
        motorLSpeed = maxSpeed;
    } else if (correction == MaxCorrection || correction == ExtremeCorrectionPositive) {
        motorRSpeed = MotorRSpeedAdjustmentNegative;
        motorLSpeed = MotorLSpeedAdjustmentPositive;
    } else if (correction < MinCorrection && correction > -MaxCorrection) {
        motorRSpeed = maxSpeed;
        motorLSpeed = maxSpeed + correction;
    } else if (correction == -MaxCorrection || correction == ExtremeCorrectionNegative) {
        motorRSpeed = MotorLSpeedAdjustmentPositive;
        motorLSpeed = MotorRSpeedAdjustmentNegative;
    } else if (correction == MinCorrection) {
        motorRSpeed = maxSpeed;
        motorLSpeed = maxSpeed;
    }
}

void Drive() {
    // Điều chỉnh tốc độ động cơ
    if (motorRSpeed > 255) {motorRSpeed = 255;}
    else if (motorRSpeed < -255) {motorRSpeed = -255;}
    if (motorLSpeed > 255) {motorLSpeed = 255;}
    else if (motorLSpeed < -255) {motorLSpeed = -255;}

    // Điều khiển động cơ bên phải
    if (motorRSpeed > 0) { // Động cơ phải tiến
        analogWrite(motorREnable, motorRSpeed);
        digitalWrite(motorRPin1, LOW);
        digitalWrite(motorRPin2, HIGH);
    }  else if (motorRSpeed < 0) {// Động cơ phải đi lùi
        analogWrite(motorREnable, abs(motorRSpeed));
        digitalWrite(motorRPin1, HIGH);
        digitalWrite(motorRPin2, LOW);
    } else if (motorRSpeed == 0) { // Động cơ bên phải dừng
        digitalWrite(motorREnable, HIGH);
        digitalWrite(motorRPin1, LOW);
        digitalWrite(motorRPin2, LOW);
    }
    // Điều khiển động cơ bên trái
    if (motorLSpeed > 0) { // Động cơ bên trái tiến
        analogWrite(motorLEnable, motorLSpeed);
        digitalWrite(motorLPin1, LOW);
        digitalWrite(motorLPin2, HIGH);
    } else if (motorLSpeed < 0) { // Động cơ bên trái lùi
        analogWrite(motorLEnable, abs(motorLSpeed));
        digitalWrite(motorLPin1, HIGH);
        digitalWrite(motorLPin2, LOW);
    } else if (motorLSpeed == 0) { // động cơ bên trái dừng
        digitalWrite(motorLEnable, HIGH);
        digitalWrite(motorLPin1, LOW);
        digitalWrite(motorLPin2, LOW);
    }
}