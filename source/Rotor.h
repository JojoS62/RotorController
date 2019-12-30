#ifndef __Rotor_h__
#define __Rotor_h__

#include "mbed.h"

class Rotor {
public:
    enum MovingPath { direct, always_cw, always_ccw};
    
    Rotor(PinName ainPosition, PinName outRelaisCW, PinName outRelaisCCW, bool invertCW=false, bool invertCCW=false);

    void process();
    void setDeadZone(float deadZone);
    float getDeadZone() {return _deadZone;};
    float getPosition() {return _positionActual;};
    void moveTo(float positionSetpoint, MovingPath movingPath = MovingPath::direct);

    bool isMovingCW();
    void moveCW(bool on);

    bool isMovingCCW();
    void moveCCW(bool on);

private:
    AnalogIn _ainPosition;
    DigitalOut _outRelaisCW;
    DigitalOut _outRelaisCCW;

    float _positionActual;
    float _positionActualOld;
    float _positionSetpoint;
    float _deadZone;
    bool _invertCW;
    bool _invertCCW;
};

#endif