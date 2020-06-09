#include "ConfigurableLedMatrix.h"

ConfigurableLedMatrix::ConfigurableLedMatrix(byte numberOfDevices, int8_t sck, int8_t miso, int8_t mosi, byte slaveSelectPin): LedMatrix(numberOfDevices, sck, miso, mosi, slaveSelectPin) {
}

void ConfigurableLedMatrix::setBreathing(bool breathing) {
    breathingEffect = breathing;
}

void ConfigurableLedMatrix::setBreathing(bool breathing, byte lower, byte upper) {
    breathingEffect = breathing;
    breathingEffectLower = lower;
    breathingEffectUpper = upper;
}

void ConfigurableLedMatrix::setBreathingLimit(byte lower, byte upper) {
    breathingEffectLower = lower;
    breathingEffectUpper = upper;
}

bool ConfigurableLedMatrix::isBreathing() {
    return breathingEffect;
}

void ConfigurableLedMatrix::setScroll(byte scroll) {
    scrollMode = scroll;
}

byte ConfigurableLedMatrix::getScroll() {
    return scrollMode;
}

void ConfigurableLedMatrix::update() {
    clear();

    switch (scrollMode) {
        default:
        case SCROLL_LEFT:
            scrollTextLeft();
            break;
        case SCROLL_RIGHT:
            scrollTextRight();
            break;
        case OSCILLATE:
            oscillateText();
            break;
    }

    if(breathingEffect) {
        breathe(breathingEffectLower, breathingEffectUpper);
    }

    drawText();
    commit();
}