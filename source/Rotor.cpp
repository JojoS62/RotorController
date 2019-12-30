#include "Rotor.h"

Rotor::Rotor(PinName ainPosition, PinName outRelaisCW, PinName outRelaisCCW, bool invertCW, bool invertCCW) :
    _ainPosition(ainPosition),
    _outRelaisCW(outRelaisCW),
    _outRelaisCCW(outRelaisCCW),
    _invertCW(invertCW),
    _invertCCW(invertCCW)
{
    moveCCW(false);
    moveCW(false);
}

void Rotor::process()
{
    // simulation
    if(isMovingCW()) {
        float tempPos = _positionActual  + 0.3f;
        if (tempPos < 360.0f)
            _positionActual = tempPos;
        else
            _positionActual = tempPos - 360.0f;
    }

    if(isMovingCCW()) {
        float tempPos = _positionActual  - 0.3f;
        if (tempPos > 0.0f)
            _positionActual = tempPos;
        else
            _positionActual = tempPos + 360.0f;
    }

    // setpoint correction on wrap around
    if (_positionActual != _positionActualOld) {
        // wrap around?
        if (abs(_positionActual - _positionActualOld) > 180.0f) {
            if (_positionSetpoint > 360.0f) {
                _positionSetpoint -= 360.0f;
            }
            else if (_positionSetpoint < 0.0f) {
                _positionSetpoint += 360.0f;
            }
        }
        
        _positionActualOld = _positionActual;
    }

    // if moving, stop at setpoint
    if (isMovingCW()) {
        if(_positionActual > _positionSetpoint) {
            moveCW(false);
        }
    }

    if (isMovingCCW()) {
        if(_positionActual < _positionSetpoint) {
            moveCCW(false);
        }
    }
}

void Rotor::setDeadZone(float deadZone)
{
    _deadZone = deadZone;
}

void Rotor::moveTo(float positionSetpoint, MovingPath movingPath)
{
    _positionSetpoint = positionSetpoint;
    if (abs(_positionActual - _positionSetpoint) <= _deadZone)
        return;

    switch (movingPath) {
        case direct:
            if (_positionActual > positionSetpoint) {
                if (abs(_positionActual - _positionSetpoint) < 180.0f)
                    moveCCW(true);
                else {
                    _positionSetpoint = positionSetpoint + 360.0f;
                    moveCW(true);
                }
            } else {
                if (abs(_positionActual - _positionSetpoint) < 180.0f)
                    moveCW(true);
                else {
                    _positionSetpoint = positionSetpoint - 360.0f;
                    moveCCW(true);
                }
            }
            break;
        case always_cw:
            if (_positionActual < positionSetpoint)
                _positionSetpoint = positionSetpoint;
            else
                _positionSetpoint = positionSetpoint + 360.0f;
            moveCW(true);
            break;
        case always_ccw:
            if (_positionActual > positionSetpoint)
                _positionSetpoint = positionSetpoint;
            else
                _positionSetpoint = positionSetpoint - 360.0f;
            moveCCW(true);
            break;
    }
}

bool Rotor::isMovingCW()
{
    return (_invertCW ? (_outRelaisCW == 0) : (_outRelaisCW == 1));
}

void Rotor::moveCW(bool on)
{
    if (on) {
        moveCCW(false);
    }

    if (!_invertCW) {
        _outRelaisCW = on;
    }
    else
        _outRelaisCW = !on;
}


bool Rotor::isMovingCCW()
{
    return (_invertCCW ? (_outRelaisCCW == 0) : (_outRelaisCCW == 1));
}

void Rotor::moveCCW(bool on)
{
    if (on) {
        moveCW(false);
    }

    if (!_invertCCW)
        _outRelaisCCW = on;
    else
        _outRelaisCCW = !on;
}

