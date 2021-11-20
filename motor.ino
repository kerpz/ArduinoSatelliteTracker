//Motor pins
const int azFwdPin  = D5;
const int azRevPin  = D6;
const int elFwdPin  = D7;
const int elRevPin  = D8;

void motorStopAz() {
  digitalWrite(azFwdPin, LOW);
  digitalWrite(azRevPin, LOW);
}

void motorStopEl() {
  digitalWrite(elFwdPin, LOW);
  digitalWrite(elRevPin, LOW);
}

void motorFwdAz() {
  digitalWrite(azFwdPin, LOW);
  digitalWrite(azRevPin, HIGH);
}

void motorRevAz() {
  digitalWrite(azFwdPin, HIGH);
  digitalWrite(azRevPin, LOW);
}

void motorFwdEl() {
  digitalWrite(elFwdPin, LOW);
  digitalWrite(elRevPin, HIGH);
}

void motorRevEl() {
  digitalWrite(elFwdPin, HIGH);
  digitalWrite(elRevPin, LOW);
}


void motorSetup() {
  pinMode(azFwdPin, OUTPUT);
  pinMode(azRevPin, OUTPUT);
  pinMode(elFwdPin, OUTPUT);
  pinMode(elRevPin, OUTPUT);

  motorStopAz();
  motorStopEl();
}


void motorLoop() {
  switch (motor_mode) {
    case 0: // stop
      motorStopAz();
      motorStopEl();
      break;
    case 1: // demo fwd az
      motorFwdAz();
      break;
    case 2: // demo rev az
      motorRevAz();
      break;
    case 3: // demo fwd el
      motorFwdEl();
      break;
    case 4: // demo rev el
      motorRevEl();
      break;
    case 5: // demo fwd both
      motorFwdAz();
      motorFwdEl();
      break;
    case 6: // demo rev both
      motorRevAz();
      motorRevEl();
      break;
    default:
      break;
  }
}
