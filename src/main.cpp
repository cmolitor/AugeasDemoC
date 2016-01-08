#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <stdlib.h>
#include <unistd.h>
#include <augeas.h>
#include <boost/filesystem.hpp>

// set wifi client data; DHCP, IP, ...
int setWifiParameter(int DHCP, std::string sIP, std::string sSubnet, std::string sRouter){
    // Get execution path and construct relative path of example config file
    // for changing real interfaces config: std::string = "/etc/network/interfaces/"
    std::stringstream buffer;
    buffer << boost::filesystem::path(boost::filesystem::current_path()).string();
    buffer << "/network/interfaces/" << std::flush;
    std::string sPathExec = buffer.str();
    std::cout << "Execution path is: " << sPathExec << std::endl;

    // Augeas library for manipulating config files
    // read: https://github.com/hercules-team/augeas/wiki/Loading-specific-files
    // The comments contain the equivalent 
    // Terminal: augtool
    // Terminal: augtool> print /augeas/load/Interfaces
    /* Output:
    /augeas/load/Interfaces
    /augeas/load/Interfaces/lens = "@Interfaces"
    /augeas/load/Interfaces/incl[1] = "/etc/network/interfaces.d/"
    /augeas/load/Interfaces/incl[2] = "/etc/network/interfaces"
    */
    augeas *myAug = NULL;
    std::string sPath;
    int ret = 0;
    int iMatches = 0;
    unsigned int flags = AUG_NO_MODL_AUTOLOAD;  // AUG_NO_MODL_AUTOLOAD: really do not load // AUG_NO_LOAD: do not load/scan files // 0
    const char *root = NULL; //sRoot.c_str();
    const char *path = NULL;
    char **matches = NULL;
    const char *value;
    char **pathsErrors = NULL;

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
    path = sPathExec.c_str();
    ret = aug_set(myAug, "/augeas/load/Interfaces/incl", path);
    std::cout << "Incl: " << ret << std::endl;

    // Load/parse config files
    // Terminal: augtool> load
    ret = aug_load(myAug);
    std::cout << "Augeas load: " << ret << std::endl;

    // Reading and showing error messages
    ret = aug_match(myAug, "/augeas//error", &pathsErrors);
    std::cout << "Number of errors: " << std::to_string(ret) << std::endl;
    //printf("\n ERR: match = %d\n", n);
    for (int j = 0; j < ret; j++) {
        aug_get(myAug, pathsErrors[j], &value);
        std::cout << pathsErrors[j] << std::endl;
        //printf("\n ERR: %s\n", val);
    }

    // Check for matches
    sPath= "/files" + sPathExec + "iface";
    path = sPath.c_str();
    iMatches = aug_match(myAug, path, &matches);
    std::cout << "Number of matches: " << iMatches << std::endl;
    // Print matches
    for (int j = 0; j < iMatches; j++) {
        //aug_get(myAug, matches[j], &value);
        std::cout << matches[j] << std::endl;
        //printf("\n ERR: %s\n", val);
    }

    std::cout << "===========" << std::endl;


    // Check which network interface is the wifi interface (wlan0)
    int iWlanInterface = -1;

    for (int i=1; i<=iMatches; i++){
        sPath = "/files" + sPathExec + "iface[" + std::to_string(i) + "]";
        std::cout << sPath << std::endl;
        path = sPath.c_str();
        const char *value;
        ret = aug_get(myAug, path, &value);  // todo: ret has to be 1, otherwise value is NULL
        if (ret==1){
            //std::cout << ret << ": " << value << std::endl;
            buffer.str(std::string()); // clear stream "buffer"
            buffer << value << std::flush;
            //std::cout << buffer.str() << std::endl;
            if (buffer.str()=="wlan0"){
                iWlanInterface = i;
                std::cout << "wlan0: " << std::to_string(iWlanInterface) << std::endl;
            }
        }else{
            std::cout << "no value" << std::endl;
        }
    }

    if (DHCP==1){
        // augeas set /files/etc/network/interfaces/iface[3]/method = "dhcp"
        sPath= "/files" + sPathExec + "iface[" + std::to_string(iWlanInterface) + "]/method";
        path = sPath.c_str();
        ret = aug_set(myAug, path, "dhcp");
        std::cout << "Set dhcp: " << ret << std::endl;

        // augeas rm /files/etc/network/interfaces/iface[3]/address = "192.168.0.1" if available
        sPath= "/files" + sPathExec + "iface[" + std::to_string(iWlanInterface) + "]/address";
        path = sPath.c_str();
        ret = aug_rm(myAug, path);
        std::cout << "Address node removed: " << ret << std::endl;

        // augeas rm /files/etc/network/interfaces/iface[3]/netmask = "255.255.255.0" if available
        sPath= "/files" + sPathExec + "iface[" + std::to_string(iWlanInterface) + "]/netmask";
        path = sPath.c_str();
        ret = aug_rm(myAug, path);
        std::cout << "Netmask node removed: " << ret << std::endl;
    }else if (DHCP==0){
        // augeas set /files/etc/network/interfaces/iface[3]/method = "static"
        // augeas add and set /files/etc/network/interfaces/iface[3]/address = "192.168.0.1" if not available otherwise set
        // augeas add and set /files/etc/network/interfaces/iface[3]/netmask = "255.255.255.0" if not available otherwise set
    }else{
        std::cout << "No valid DHCP mode" << std::endl;
    }

    // Reading version of augeas
    ret = aug_get(myAug, "/augeas/version", &value);
    std::cout << "Version: " << value << std::endl;

    // Reading values; just for debugging
    sPath= "/files" + sPathExec + "iface[" + std::to_string(iWlanInterface) + "]/method";
    path = sPath.c_str();
    ret = aug_get(myAug, path, &value);
    std::cout << "Debug: " << value << std::endl;

    // Set the save mode: overwrite/backup/newfile/noop
    ret = aug_set(myAug, "/augeas/save", "newfile");
    std::cout << "Save mode set: " << ret << std::endl;

    // Save the changed config files
    ret = aug_save(myAug);
    std::cout << "File saved: " << ret << std::endl;

    // Check which files have been saved
    // ret = aug_get(myAug, "/augeas/events/saved", &value);
    // std::cout << "Number of saved files: " << ret << std::endl;
    // buffer.str(std::string()); // clear stream "buffer"
    // buffer << value << std::flush;
    // std::cout << "List of saved files: " << buffer.str() << std::endl;

    // Reading and showing saved files
    char **pathSaved= NULL;
    ret = aug_match(myAug, "/augeas/events/saved", &pathSaved);
    std::cout << "Number of files: " << std::to_string(ret) << std::endl;
    //const char *val;
    //printf("\n ERR: match = %d\n", n);
    for (int j = 0; j < ret; j++) {
        aug_get(myAug, pathSaved[j], &value);
        std::cout << pathSaved[j] << std::endl;
        //printf("\n ERR: %s\n", val);
    }

    // Reading and showing error messages
    // char **pathsErrors = NULL;
    ret = aug_match(myAug, "/augeas//error", &pathsErrors);
    std::cout << "Number of errors: " << std::to_string(ret) << std::endl;
    //printf("\n ERR: match = %d\n", n);
    for (int j = 0; j < ret; j++) {
        aug_get(myAug, pathsErrors[j], &value);
        std::cout << pathsErrors[j] << std::endl;
        //printf("\n ERR: %s\n", val);
    }

    aug_close(myAug);

    return 0;
}

/**
 * This is the main method which creates and sets consumer instance.
 * @Looping is essential for this MQTT library to work.
 * @Exceptions on connection and subscription error.
 */
int main(int argc, char *argv[]) {

    // write config files related to wifi
    int ret = setWifiParameter(1, "192.168.178.123", "255.255.255.0", "192.168.178.1");
    std::cout << "Return: " << ret << std::endl;

    return 0;
}