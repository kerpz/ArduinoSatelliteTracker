/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

/* crc 16 */
uint16_t crc16(uint8_t *buffer, uint8_t length)
{
    uint8_t i, j;
    uint16_t crc = 0xFFFF;
    uint16_t tmp;

    // Calculate the CRC.
    for (i = 0; i < length; i++) {
        crc = crc ^ buffer[i];
        for (j = 0; j < 8; j++) {
            tmp = crc & 0x0001;
            crc = crc >> 1;
            if (tmp) {
                crc = crc ^ 0xA001;
            }
        }
    }
    return crc;
}

int getRSSIasQuality(int RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}
