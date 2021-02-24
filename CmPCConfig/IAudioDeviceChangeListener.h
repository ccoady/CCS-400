#pragma once

class IAudioDeviceChangeListener 
{
public:
    virtual void StateChanged() = 0;
};