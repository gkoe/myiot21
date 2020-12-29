#pragma once


class EspStationClass
{
    public:
        void init();
        bool isStationOnline();
        char* getIpAddressString();
        
    private:
};
extern EspStationClass EspStation;