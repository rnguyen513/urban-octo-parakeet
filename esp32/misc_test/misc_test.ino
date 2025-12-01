void setup() {
	Serial.begin(115200);
	delay(2000);
	Serial.print("init test for LED_BUILTIN:");
	Serial.println(LED_BUILTIN);

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
	Serial.println("blinking high");
	digitalWrite(LED_BUILTIN, HIGH);
	delay(2000);
	digitalWrite(LED_BUILTIN, LOW);
	delay(2000);
}
