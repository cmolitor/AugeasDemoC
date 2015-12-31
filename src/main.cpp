#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <stdlib.h>
#include <unistd.h>
#include <augeas.h>

// set wifi client data; DHCP, IP, network, SSID, password
int setWifiParameter(int DHCP, int iWPA, std::string sPairwise, std::string sGroup, std::string sIP, std::string sSubnet, std::string sRouter, std::string sSSID, std::string sPassphrase){
    // Augeas library for manipulating config files
    // read: https://github.com/hercules-team/augeas/wiki/Loading-specific-files
    // The comments contain the equivalent 
    // Terminal: augtool
    // Terminal: augtool> print /augeas/load/Interfaces
    /* Output:
    /augeas/load/Interfaces
    /augeas/load/Interfaces/lens = "@Interfaces"
    /augeas/load/Interfaces/incl[1] = "/etc/network/interfaces.d/*"
    /augeas/load/Interfaces/incl[2] = "/etc/network/interfaces"
    */
    augeas *myAug = NULL;
    std::string sPath;
    int ret = 0;
    unsigned int flags = AUG_NO_MODL_AUTOLOAD;  // AUG_NO_MODL_AUTOLOAD: really do not load // AUG_NO_LOAD: do not load/scan files // 0
    const char *root = NULL; //sRoot.c_str();
    const char *path = NULL;
    char ***matches = NULL;

    // Terminal: augtool --noautoload
    // The flag is set to --noautoload in order to speed up the initialization. 
    // If the flag is not set, the standard config files are loaded which takes a while.
    // The commands after aug_init() then set the specific files which should be loaded.
    myAug = aug_init(root, "/usr/share/augeas/lenses", flags);

    // Set lens which should be loaded
    // Terminal: augtool> set /augeas/load/Interfaces/lens "Interfaces.lns"
    ret = aug_set(myAug, "/augeas/load/Interfaces/lens", "Interfaces.lns");
    std::cout << "Lens: " << ret << std::endl;

    // Set incl(ude) path which should be parsed
    // Terminal: augtool> set /augeas/load/Interfaces/incl "/etc/network/interfaces"
    //ret = aug_set(myAug, "/augeas/load/Interfaces/incl", "/etc/network/interfacesClient/");
    ret = aug_set(myAug, "/augeas/load/Interfaces/incl", "/home/pi/metec/metec_base/core_ui/network/interfaces/");
    std::cout << "Incl: " << ret << std::endl;

    // Load/parse config files
    // Terminal: augtool> load
    ret = aug_load(myAug);
    std::cout << "Augeas Load: " << ret << std::endl;

    // Check for matches
    //int nMatches = aug_match(myAug, "/files/etc/network/interfacesClient/iface", matches);
    int nMatches = aug_match(myAug, "/files/home/pi/metec/metec_base/core_ui/network/interfaces/iface", matches);
    std::cout << "Number of matches: " << nMatches << std::endl;


    // Check which network interface is the wifi interface (wlan0)
    int iWlanInterface = -1;

    for (int i=1; i<=nMatches; i++){
        //sPath = "/files/etc/network/interfacesClient/iface[";
        sPath = "/files/home/pi/metec/metec_base/core_ui/network/interfaces/iface[";
        sPath += std::to_string(i);
        sPath += "]";
        std::cout << sPath << std::endl;
        path = sPath.c_str();
        const char *value;
        int ret = aug_get(myAug, path, &value);  // todo: ret has to be 1, otherwise value is NULL
        if (ret==1){
            std::cout << ret << ": " << value << std::endl;
            std::stringstream buffer;
            buffer << value << std::flush;
            std::cout << buffer.str() << std::endl;
            if (buffer.str()=="wlan0"){
                iWlanInterface = i;
                std::cout << "here is the wifi" << std::to_string(iWlanInterface) << std::endl;
            }
        }else{
            std::cout << "no value" << std::endl;
        }
    }

    if (DHCP==1){
        // augeas set /files/etc/network/interfaces/iface[3]/method = "dhcp"
        //sPath= "/files/etc/network/interfacesClient/iface[" + std::to_string(iWlanInterface) + "]/method";
        sPath= "/files/home/pi/metec/metec_base/core_ui/network/interfaces/iface[" + std::to_string(iWlanInterface) + "]/method";
        path = sPath.c_str();
        ret = aug_set(myAug, path, "dhcp");
        std::cout << "Set dhcp: " << ret << std::endl;

        // augeas rm /files/etc/network/interfaces/iface[3]/address = "192.168.0.1" if available
        //sPath= "/files/etc/network/interfacesClient/iface[" + std::to_string(iWlanInterface) + "]/address";
        sPath= "/files/home/pi/metec/metec_base/core_ui/network/interfaces/iface[" + std::to_string(iWlanInterface) + "]/address";
        path = sPath.c_str();
        //ret = aug_rm(myAug, path);
        std::cout << "Address node removed: " << ret << std::endl;

        // augeas rm /files/etc/network/interfaces/iface[3]/netmask = "255.255.255.0" if available
        //sPath = "/files/etc/network/interfacesClient/iface[" + std::to_string(iWlanInterface) + "]/netmask";
        sPath = "/files/home/pi/metec/metec_base/core_ui/network/interfaces/iface[" + std::to_string(iWlanInterface) + "]/netmask";
        path = sPath.c_str();
        //ret = aug_rm(myAug, path);
        std::cout << "Address node removed: " << ret << std::endl;
    }else if (DHCP==0){
        // augeas set /files/etc/network/interfaces/iface[3]/method = "static"
        // augeas add and set /files/etc/network/interfaces/iface[3]/address = "192.168.0.1" if not available otherwise set
        // augeas add and set /files/etc/network/interfaces/iface[3]/netmask = "255.255.255.0" if not available otherwise set
    }else{
        std::cout << "No valid DHCP mode" << std::endl;
    }

    // Set the save mode: overwrite/backup/newfile/noop
    //ret = aug_set(myAug, "/augeas/save", "backup");
    std::cout << "Save mode set: " << ret << std::endl;

    // Save the changed config files
    ret = aug_save(myAug);
    std::cout << "File saved: " << ret << std::endl;

    aug_close(myAug);

    std::cout << "After augeas stuff\n";

    return 0;
}

/**
 * This is the main method which creates and sets consumer instance.
 * @Looping is essential for this MQTT library to work.
 * @Exceptions on connection and subscription error.
 */
int main(int argc, char *argv[]) {

    // write config files related to wifi
    int test = setWifiParameter(1, 1, "CCMP", "CCMP", "192.168.178.123", "255.255.255.0", "192.168.178.1", "MMM", "password");

    return 0;
}