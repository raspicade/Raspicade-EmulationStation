/* 
 * File:   RaspicadeSystem.cpp
 * Author: matthieu
 * 
 * Created on 29 novembre 2014, 03:15
 */

#include "RaspicadeSystem.h"
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sstream>
#include "Settings.h"
#include <iostream>
#include <fstream>
#include "Log.h"
#include "HttpReq.h"

#include "AudioManager.h"
#include "VolumeControl.h"

RaspicadeSystem::RaspicadeSystem() {
}

RaspicadeSystem * RaspicadeSystem::instance = NULL;

RaspicadeSystem * RaspicadeSystem::getInstance() {
    if (RaspicadeSystem::instance == NULL) {
        RaspicadeSystem::instance = new RaspicadeSystem();
    }
    return RaspicadeSystem::instance;
}

unsigned long RaspicadeSystem::getFreeSpaceGB(std::string mountpoint) {
    struct statvfs fiData;
    const char * fnPath = mountpoint.c_str();
    int free = 0;
    if ((statvfs(fnPath, &fiData)) >= 0) {
        free = (fiData.f_bfree * fiData.f_bsize) / (1024 * 1024 * 1024);
    }
    return free;
}

std::string RaspicadeSystem::getFreeSpaceInfo() {
    struct statvfs fiData;
    std::string sharePart = Settings::getInstance()->getString("SharePartition");
    if(sharePart.size() > 0){
        const char * fnPath = sharePart.c_str();
        if ((statvfs(fnPath, &fiData)) < 0) {
            return "";
        } else {
            unsigned long total = (fiData.f_blocks * (fiData.f_bsize / 1024)) / (1024L * 1024L);
            unsigned long free = (fiData.f_bfree * (fiData.f_bsize / 1024)) / (1024L * 1024L);
            unsigned long used = total - free;
            unsigned long percent = used * 100 / total;

            std::ostringstream oss;
            oss << used << "GB/" << total << "GB (" << percent << "%)";
            return oss.str();
        }
    } else {
        return "ERROR";
    }
}

bool RaspicadeSystem::isFreeSpaceLimit() {
    std::string sharePart = Settings::getInstance()->getString("SharePartition");
    if(sharePart.size() > 0){
        return getFreeSpaceGB(sharePart) < 2;
    } else {
        return "ERROR";
    }
    
}

std::string RaspicadeSystem::getVersion() {
    std::string version = Settings::getInstance()->getString("VersionFile");
    if (version.size() > 0) {
        std::ifstream ifs(version);

        if (ifs.good()) {
            std::string contents;
            std::getline(ifs,contents);
            return contents;
        }
    }
    return "";
}

bool RaspicadeSystem::needToShowVersionMessage() {
    std::string versionFile = Settings::getInstance()->getString("LastVersionFile");
    if (versionFile.size() > 0) {
        std::ifstream lvifs(versionFile);
        if (lvifs.good()) {
            std::string lastVersion;
            std::getline(lvifs,lastVersion);
            std::string currentVersion = getVersion();
            if(lastVersion == currentVersion){
                return false;
            }
        }
    }
    return true;
}

bool RaspicadeSystem::versionMessageDisplayed() {
    std::string versionFile = Settings::getInstance()->getString("LastVersionFile");
    std::string currentVersion = getVersion();
    std::ostringstream oss;
    oss << "echo " << currentVersion << " > " << versionFile;
    if(system(oss.str().c_str())){
            LOG(LogWarning) << "Error executing " << oss.str().c_str();
        }else {
            LOG(LogInfo) << "Version message displayed ok";
        }
}

std::string RaspicadeSystem::getVersionMessage(){
    std::string versionMessageFile = Settings::getInstance()->getString("VersionMessage");
        if (versionMessageFile.size() > 0) {
        std::ifstream ifs(versionMessageFile);

        if (ifs.good()) {            
            std::string contents((std::istreambuf_iterator<char>(ifs)),
                 std::istreambuf_iterator<char>());
            return contents;
        }
    }
    return "";

}

bool RaspicadeSystem::setAudioOutputDevice(std::string device) {
    int commandValue = -1;
    int returnValue = false;
    
    if(device == "auto"){
        commandValue = 0;
    }else if(device == "jack"){
        commandValue = 1;
    }else if(device == "hdmi"){            
        commandValue = 2;
    }else {
        LOG(LogWarning) << "Unable to find audio output device to use !";
    }
    
    if(commandValue != -1){
        std::ostringstream oss;
        oss << "amixer cset numid=3 " << commandValue;
        std::string command = oss.str();
        LOG(LogInfo) << "Launching " << command;
        if(system(command.c_str())){
            LOG(LogWarning) << "Error executing " << command;
            returnValue = false;
        }else {
            LOG(LogInfo) << "Audio output device set to : " << device;
            returnValue = true;
        }
      }
    return returnValue;
}



bool RaspicadeSystem::setOverscan(bool enable){
   
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " << "overscan";
    if(enable){
        oss << " " << "enable";
    }else {
        oss << " " << "disable";
    }
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if(system(command.c_str())){
        LOG(LogWarning) << "Error executing " << command;
        return false;
    }else {
        LOG(LogInfo) << "Overscan set to : " << enable;
        return true;
    }
      
}
bool RaspicadeSystem::setOverclock(std::string mode){
    if(mode != ""){
        std::ostringstream oss;
        oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " 
                << "overclock" << " " << mode;
        std::string command = oss.str();
        LOG(LogInfo) << "Launching " << command;
        if(system(command.c_str())){
            LOG(LogWarning) << "Error executing " << command;
            return false;
        }else {
            LOG(LogInfo) << "Overclocking set to " << mode;
            return true;
        }
      }
}


bool RaspicadeSystem::setGPIOControllers(bool enable){
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " << "gpiocontrollers";
    if(enable){
        oss << " " << "enable";
    }else {
        oss << " " << "disable";
    }
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if(system(command.c_str())){
        LOG(LogWarning) << "Error executing " << command;
        return false;
    }else {
        LOG(LogInfo) << "Overscan set to : " << enable;
        return true;
    }
}

bool RaspicadeSystem::updateSystem(){
    std::string updatecommand = Settings::getInstance()->getString("UpdateCommand");
    if(updatecommand.size() > 0){
	int exitcode = system(updatecommand.c_str());
        return exitcode == 0;
    }
    return false;
}

bool RaspicadeSystem::ping(){
    std::string updateserver = Settings::getInstance()->getString("UpdateServer");
    std::string s("ping -c 1 " + updateserver);
    int exitcode = system(s.c_str());
    return exitcode == 0;
}

bool RaspicadeSystem::canUpdate(){
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " << "canupdate";
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if(system(command.c_str()) == 0){
        LOG(LogInfo) << "Can update ";
        return true;
    }else {
        LOG(LogInfo) << "Cannot update ";
        return false;
    }
}
    
bool RaspicadeSystem::launchKodi(Window * window){
    
    LOG(LogInfo) << "Attempting to launch kodi...";


    AudioManager::getInstance()->deinit();
    VolumeControl::getInstance()->deinit();

    window->deinit();

    std::string command = "/recalbox/scripts/kodilauncher.sh";
    int exitCode = system(command.c_str());

    window->init();
    VolumeControl::getInstance()->init();
    AudioManager::getInstance()->resumeMusic();
    window->normalizeNextUpdate();
    
    return exitCode == 0;

}

bool RaspicadeSystem::enableWifi(std::string ssid, std::string key){
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " 
            << "wifi" << " " 
            << "enable" << " " 
            << ssid << " " << key;
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if(system(command.c_str()) == 0){
        LOG(LogInfo) << "Wifi enabled ";
        return true;
    }else {
        LOG(LogInfo) << "Cannot enable wifi ";
        return false;
    }
}

bool RaspicadeSystem::disableWifi(){
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " 
            << "wifi" << " " 
            << "disable";
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if(system(command.c_str()) == 0){
        LOG(LogInfo) << "Wifi disabled ";
        return true;
    }else {
        LOG(LogInfo) << "Cannot disable wifi ";
        return false;
    }
}
