#include "Module.h"
#include "Modulo.h"

#ifdef SPARK
#include <spark_wiring.h>
#else
#include "Arduino.h"

#endif

uint8_t Module::_lastAssignedAddress = 9;
Module* Module::_firstModule = NULL;

Module::Module(const char *deviceType) :
    _deviceType(deviceType), _deviceID(0xFFFF), _address(0xFF), _disconnected(true) {
    
    _nextModule = _firstModule;
    _firstModule = this;
}

Module::Module(const char *deviceType, uint16_t deviceID) : 
    _deviceType(deviceType), _deviceID(deviceID), _address(0xFF), _disconnected(true) {

    _nextModule = _firstModule;
    _firstModule = this;
}

Module::~Module() {

    // Remove this module from the linked list
    Module *prev = NULL;
    for (Module *m = _firstModule; m ; m = m->_nextModule) {
        if (m == this) {
            if (prev) {
                prev->_nextModule = _nextModule;
            } else {
                _firstModule = _nextModule;
            }
        }
    }
}

Module *Module::findByDeviceID(uint16_t deviceID) {
    for (Module *m = _firstModule; m ; m = m->_nextModule) {
        if (m->getDeviceID() == deviceID) {
            return m;
        }
    }
    return NULL;
}

void Module::_reset() {
    _address = 0xFF;
}

void Module::_globalReset() {
    for (Module *m = _firstModule; m ; m = m->_nextModule) {
        m->_reset();
    }
}

void Module::_processEvent(uint8_t eventCode, uint16_t eventData) {
}

uint16_t Module::getDeviceID() {
    _init();
    return _deviceID;
}

uint8_t Module::getAddress() {
    _init();
    return _address;
}

void Module::loop() {
    for (Module *m = _firstModule; m ; m = m->_nextModule) {
        m->_loop();
    }
}

void Module::_loop() {
    if (_disconnected) {
        if (getAddress() != 0xFF) {
            _disconnected = false;
        }
    }
}

bool Module::_init() {
    if (_address != 0xFF) {
        return false;
    }

    // Ensure that a global reset has been performed
    static bool _moduloInitialized = false;
    if (!_moduloInitialized) {

        // Wait until at least 100ms after startup
        // so that connected devices can initialize
        unsigned long t = millis();
        if (t < 100) {
            delay(100-t);
        }

        ModuloSetup();
        ModuloGlobalReset();
        _moduloInitialized = true;
    }
    
    if (_deviceID == 0xFFFF) {
        // Find the first device with the specified type and no assigned address
        uint16_t deviceID = ModuloGetNextDeviceID(0);
        while (deviceID != 0xFFFF) {

            // First look for a Module that already has this deviceID
            Module *m = _firstModule;
            for (; m && m->_deviceID != deviceID; m = m->_nextModule) {
            }

            if (m == NULL) {
                char deviceType[32] = {0};
                ModuloGetDeviceType(deviceID, deviceType, 31);
                if (strcmp(deviceType,_deviceType) == 0) {
                    _deviceID = deviceID;
                    break;
                }
            }

            deviceID = ModuloGetNextDeviceID(deviceID+1);
        }
    }

    if (_deviceID == 0xFFFF) {
        // Couldn't find a device
        return false;
    }
    
    _address = ModuloGetAddress(_deviceID);
    if (_address == 0) {
        _address = ++_lastAssignedAddress;
        ModuloSetAddress(_deviceID, _address);
    }
    return true;
}

bool Module::_transfer(uint8_t command, uint8_t *sendData, uint8_t sendLen,
        uint8_t *receiveData, uint8_t receiveLen, uint8_t retries)
{
    if (_disconnected) {
        return false;
    }

    for (int i=0; i < retries; i++) {
        // Unable to assign an address. Return false.
        uint8_t address = getAddress();
        if (address == 0xFF) {
            continue;
        }

        // We have a valid address, attempt the transfer.
        if (moduloTransfer(address, command, sendData, sendLen,
            receiveData, receiveLen)) {
          
            return true;
        }

        // If the transfer failed, try to re-assign an address. The device may
        // have been removed and re-connected
        ModuloSetAddress(_deviceID, _address);
    }


    _disconnected = true;

    return false;
}
