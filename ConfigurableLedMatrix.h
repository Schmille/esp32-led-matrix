# include "LedMatrix.h"

#define SCROLL_LEFT 0
#define SCROLL_RIGHT 1
#define OSCILLATE 2

class ConfigurableLedMatrix: public LedMatrix {

public:

    /**
     * Constructor.
     * numberOfDisplays: number of connected devices
     * slaveSelectPin: CS (or SS) pin connected to your ESP8266
     */
    ConfigurableLedMatrix(byte numberOfDevices, int8_t sck, int8_t miso, int8_t mosi, byte slaveSelectPin);
    
    /**
     * Sets the configuration value for the breathing effect
     */
    void setBreathing(bool breathing);

    /**
     * Sets the configuration value for the breathing effect
     */
    void setBreathing(bool breathing, byte lower, byte upper);

    /**
     * Sets the two limits for the breathing effect 
     */
    void setBreathingLimit(byte lower, byte upper);

    /**
     *  True if the breathing effect is active
     */
    bool isBreathing();

    /**
     * Sets the prefered scroll mode
     */
    void setScroll(byte scroll);

    /**
     * Sets the prefered scroll mode
     */
    byte getScroll();

    /**
     * Updates the view
     */
    void update();

private:
    bool breathingEffect = false;
    byte breathingEffectLower = 0;
    byte breathingEffectUpper = 15;
    byte scrollMode = SCROLL_LEFT;
};
