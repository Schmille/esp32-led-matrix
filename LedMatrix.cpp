#include <SPI.h>
#include "LedMatrix.h"
#include "cp437font.h"

/**
 * Heavily influenced by the code and the blog posts from https://github.com/nickgammon/MAX7219_Dot_Matrix
 */
LedMatrix::LedMatrix(byte numberOfDevices, int8_t sck, int8_t miso, int8_t mosi, byte slaveSelectPin) {
    myNumberOfDevices = numberOfDevices;
    mySlaveSelectPin = slaveSelectPin;
    cols = new byte[numberOfDevices * 8];
	  _sck = sck;
    _miso = miso;
    _mosi = mosi;
}

/**
 *  numberOfDevices: how many modules are daisy changed togehter
 *  slaveSelectPin: which pin is controlling the CS/SS pin of the first module?
 */
void LedMatrix::init() {
    pinMode(mySlaveSelectPin, OUTPUT);

    SPI.begin ( _sck,  _miso,  _mosi,  mySlaveSelectPin);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV128);
    for(byte device = 0; device < myNumberOfDevices; device++) {
        sendByte (device, MAX7219_REG_SCANLIMIT, 7);   // show all 8 digits
        sendByte (device, MAX7219_REG_DECODEMODE, 0);  // using an led matrix (not digits)
        sendByte (device, MAX7219_REG_DISPLAYTEST, 0); // no display test
        sendByte (device, MAX7219_REG_INTENSITY, 0);   // character intensity: range: 0 to 15
        sendByte (device, MAX7219_REG_SHUTDOWN, 1);    // not in shutdown mode (ie. start it up)
    }
}

void LedMatrix::sendByte (const byte device, const byte reg, const byte data) {
    digitalWrite(mySlaveSelectPin,LOW);
    for (int i=0; i<myNumberOfDevices; i++)
    {
      SPI.transfer ((i==device)?reg:(byte)0);
      SPI.transfer ((i==device)?data:(byte)0);
    }
    digitalWrite (mySlaveSelectPin, HIGH);
}

void LedMatrix::sendByte (const byte reg, const byte data) {
    digitalWrite(mySlaveSelectPin,LOW);
    for (int i=0; i<myNumberOfDevices; i++)
    {
      SPI.transfer (reg);
      SPI.transfer (data);
    }
    digitalWrite (mySlaveSelectPin, HIGH);
}

void LedMatrix::setIntensity(const byte intensity) {
    intensityVal = intensity;
    sendByte(MAX7219_REG_INTENSITY, intensity);
}

byte LedMatrix::getIntensity() {
  return intensityVal;
}

void LedMatrix::setTextAlignment(byte textAlignment) {
    myTextAlignment = textAlignment;
    calculateTextAlignmentOffset();
}

void LedMatrix::calculateTextAlignmentOffset() {
    switch(myTextAlignment) {
        case TEXT_ALIGN_LEFT:
            myTextAlignmentOffset = 0;
            break;
        case TEXT_ALIGN_LEFT_END:
            myTextAlignmentOffset = myNumberOfDevices * 8;
            break;
        case TEXT_ALIGN_RIGHT:
            myTextAlignmentOffset = myTextLength - myNumberOfDevices * 8;
            break;
        case TEXT_ALIGN_RIGHT_END:
            myTextAlignmentOffset = - myTextLength;
            break;
        case TEXT_ALIGN_CENTER:
            myTextAlignmentOffset = (myNumberOfDevices * 8 - myTextLength) / 2;
            break;
    }

}

void LedMatrix::clear() {
    for (byte col = 0; col < myNumberOfDevices * 8; col++) {
        cols[col] = 0;
    }
}

void LedMatrix::commit() {
  if (myDisplayOrientation) {
    for (byte dcol=0; dcol < 8; dcol++) {
      digitalWrite(mySlaveSelectPin,LOW);
      for (int dev=0; dev < myNumberOfDevices; dev++) {
        byte b = 0;
        for (byte fcol=0; fcol < 8; fcol++)
          if (cols[dev*8+fcol]&(1<<dcol))
            b |= (128>>fcol);
        SPI.transfer(dcol+1);
        SPI.transfer(b);
      }
      digitalWrite(mySlaveSelectPin,HIGH);
    }
  }
  else {
    for (byte col=0; col < 8; col++) {
      digitalWrite(mySlaveSelectPin,LOW);
      for (int dev=0; dev < myNumberOfDevices; dev++) {
        SPI.transfer(col+1);
        SPI.transfer(cols[dev*8+col]);
      }
      digitalWrite(mySlaveSelectPin,HIGH);
    }
  }
}

void LedMatrix::setText(String text) {
    myText = text;
    myTextOffset = 0;
    myTextLength = 0;
    for (int i = 0; i < myText.length(); i++) myTextLength += cp437_width[(byte)myText.charAt(i)];
    calculateTextAlignmentOffset();
}

String LedMatrix::getText() {
  return myText;
}

int LedMatrix::getTextLength() {
    return myTextLength;
}

void LedMatrix::setNextText(String nextText) {
    myNextText = nextText;
}

String LedMatrix::getNextText() {
  return myNextText;
}

void LedMatrix::scrollTextRight() {
  if(myTextOffset >= 0) {
      myTextOffset = -myTextLength - myTextAlignmentOffset;
      if(hasNextText()) {
        updateNewText();
      }
  }
  
  calculateTextAlignmentOffset();
  myTextOffset += 1;
}

void LedMatrix::scrollTextLeft() {
    myTextOffset = (myTextOffset - 1) % (myTextLength + myNumberOfDevices * 8);
    if (myTextOffset == 0 && myNextText.length() > 0) {
        updateNewText();
        calculateTextAlignmentOffset();
    }
}

void LedMatrix::oscillateText() {
    if(myTextOffset - 1 == -(myTextLength + myTextAlignmentOffset) && !rightScrolling) {
      rightScrolling = true;
      if(hasNextText()) {
        updateNewText();
      }
    }

    if(myTextOffset == 0 && rightScrolling) { 
      rightScrolling = false;
      if(hasNextText()) {
        updateNewText();
      }
    }

    if(rightScrolling) {
      scrollTextRight();
    }
    else {
     scrollTextLeft(); 
    }  
}

void LedMatrix::breathe(const byte lower, const byte upper) {
  if(intensityRising) {
    if(intensityVal >= upper) {
      intensityRising = false;
      return;
    }
    setIntensity(intensityVal + 1);
  }
  else {
    if(intensityVal <= lower) {
      intensityRising = true;
      return;
    }
    setIntensity(intensityVal - 1);
  }
}

void LedMatrix::updateNewText() {
  myText = myNextText;
  myNextText = "";
  myTextLength = 0;
  for (int i = 0; i < myText.length(); i++) myTextLength += cp437_width[(byte)myText.charAt(i)];
}

bool LedMatrix::hasNextText() {
  return myNextText[0] != '\0';
}

void LedMatrix::setAlternateDisplayOrientation(byte x) {
    myDisplayOrientation = x;
}

void LedMatrix::drawText() {
  byte escape = 0;
  byte letter;
  uint32_t code;
  int  width;
  int  pos0 = myTextOffset + myTextAlignmentOffset;
  for (int i = 0; i < myText.length(); i++) {
    letter = (byte)myText.charAt(i);
    if (escape > 0)
    {
      code = (code << 8) + letter;
      escape--;
    }
    else if (letter == 226)
    {
      code = letter;
      escape = 2;
    }
    else if (letter == 194 || letter == 195)
    {
      code = letter;
      escape = 1;
    }
    else code = letter;
    if (escape > 0) continue;
    switch (code)
    {
      case 0xc280:
      case 0xe282ac: letter = 238; // Epsilon = Euro
                      break;
      case 0xc384: letter = 142; // Ä
                    break;
      case 0xc396: letter = 153; // Ö
                    break;
      case 0xc39c: letter = 154; // Ü
                    break;
      case 0xc39f: letter = 225; // ß
                    break;
      case 0xc2a0: letter = 32;  // Blank
                    break;
      case 0xc3a4: letter = 132; // ä
                    break;
      case 0xc2b0: letter = 248; // °
                    break;
      case 0xc3b6: letter = 148; // ö
                    break;
      case 0xc3bc: letter = 129; // ü
                    break;
      case 0xe28093: letter = 196; // long -
                     break;
      default: if (code < 256) letter = code;
                else letter = 32;
                break;
    }
    width = pgm_read_byte (&cp437_width[letter]);
    for (byte col = 0; col < width; col++) {
      int position = pos0 + col;
      if (position >= 0 && position < myNumberOfDevices * 8 && col < 8) {
        setColumn(position, pgm_read_byte (&cp437_font [letter] [col]));
      }
    }
    pos0 += width;
  }
}

void LedMatrix::setColumn(int column, byte value) {
    if (column < 0 || column >= myNumberOfDevices * 8) {
        return;
    }
    cols[column] = value;
}

void LedMatrix::setPixel(byte x, byte y) {
    bitWrite(cols[x], y, true);
}