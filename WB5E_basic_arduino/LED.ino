#define PIN_LED 16 //定义LED灯：PIN_LED所对应的引脚为16号
void setup() {
  pinMode(PIN_LED, OUTPUT); //初始化PIN_LED引脚模式为输出
  digitalWrite(PIN_LED,HIGH);//初始LED灯置为低电平，表示点亮
  delay(1000); //延时1秒
}
void loop() {
  digitalWrite(PIN_LED, LOW);  
  delay(500);                      
  digitalWrite(PIN_LED,HIGH); 
  delay(500);                    
}