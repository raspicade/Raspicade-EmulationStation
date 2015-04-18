#include "EmulationStation.h"
#include "guis/GuiMenu.h"
#include "Window.h"
#include "Sound.h"
#include "Log.h"
#include "Settings.h"
#include "RaspicadeSystem.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiDetectDevice.h"
#include "views/ViewController.h"
#include "AudioManager.h"

#include "components/ButtonComponent.h"
#include "components/SwitchComponent.h"
#include "components/SliderComponent.h"
#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/MenuComponent.h"
#include "VolumeControl.h"
#include "scrapers/GamesDBScraper.h"
#include "scrapers/TheArchiveScraper.h"

#include "guis/GuiTextEditPopup.h"

//get ip adress
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

void GuiMenu::createInputTextRow(GuiSettings * gui, const char*  title, const char*  settingsID, bool password){
    // LABEL
        Window * window = mWindow;
        ComponentListRow row;

        auto lbl = std::make_shared<TextComponent>(window, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
        row.addElement(lbl, true); // label

        std::shared_ptr<GuiComponent> ed;

        ed = std::make_shared<TextComponent>(window, ((password && Settings::getInstance()->getString(settingsID) != "") ? "*********" : Settings::getInstance()->getString(settingsID) ), Font::get(FONT_SIZE_MEDIUM, FONT_PATH_LIGHT), 0x777777FF, ALIGN_RIGHT);
        row.addElement(ed, true);

        auto spacer = std::make_shared<GuiComponent>(mWindow);
        spacer->setSize(Renderer::getScreenWidth() * 0.005f, 0);
        row.addElement(spacer, false);

        auto bracket = std::make_shared<ImageComponent>(mWindow);
        bracket->setImage(":/arrow.svg");
        bracket->setResize(Eigen::Vector2f(0, lbl->getFont()->getLetterHeight()));
        row.addElement(bracket, false);

        auto updateVal = [ed,settingsID, password](const std::string& newVal) { 
            if(!password)
                ed->setValue(newVal); 
            else {
                ed->setValue("*********"); 
            }
            Settings::getInstance()->setString(settingsID, newVal);
        }; // ok callback (apply new value to ed)
        row.makeAcceptInputHandler([this, title, updateVal,settingsID] {
                mWindow->pushGui(new GuiTextEditPopup(mWindow, title, Settings::getInstance()->getString(settingsID), updateVal, false));
        });
        gui->addRow(row);
}

GuiMenu::GuiMenu(Window* window) : GuiComponent(window), mMenu(window, "MAIN MENU"), mVersion(window)
{
	// MAIN MENU

	// SCRAPER >
	// SOUND SETTINGS >
	// UI SETTINGS >
	// CONFIGURE INPUT >
	// GET IP ADRESS
	// QUIT >

	// [version]


        addEntry("VIEW IP ADDRESS", 0x777777FF, true,   
                [this] {
                        auto s = new GuiSettings(mWindow, "VIEW IP ADDRESS");
                         Window* window = mWindow;

                        ComponentListRow row;
/*                        row.makeAcceptInputHandler([window]{
                        window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES",
                                [] {
                                        if(runRestartCommand() != 0)
                                                LOG(LogWarning) << "Restart terminated with non-zero result!";
                                }, "NO", nullptr));
                         });
*/


                //get ip
                struct ifaddrs *ifaddr, *ifa;
                int family, st, n, n_ip;
                char host[NI_MAXHOST]="";
                char tabhost[10][NI_MAXHOST]={"","","","","","","","","",""};

           if (getifaddrs(&ifaddr) == -1) {
                LOG(LogError) << "getifaddrs error\n";
           }

          /* Walk through linked list, maintaining head pointer so we
              can free list later */
        n_ip=0;
           for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
              {
               if (ifa->ifa_addr == NULL)
                   continue;

               family = ifa->ifa_addr->sa_family;

               /* For an AF_INET* interface address, display the address */

               if (family == AF_INET || family == AF_INET6)
                   {
                   st = getnameinfo(ifa->ifa_addr,
                           (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                 sizeof(struct sockaddr_in6),
                           host, NI_MAXHOST,
                           NULL, 0, NI_NUMERICHOST);
                   if (st != 0)
                       {
                       LOG(LogError) << "getnameinfo() failed: \n";
                       }

/*
               } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
                   struct rtnl_link_stats *stats = ifa->ifa_data;
                   printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                          "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                          stats->tx_packets, stats->rx_packets,
                          stats->tx_bytes, stats->rx_bytes);
*/
                  }
                if (strcmp(host,"")!=0)
                        {
                        //printf("%s\n", host);
                        strncpy(tabhost[n_ip],host, NI_MAXHOST);
                        //printf("%s\n", tabhost[n_ip]);
                        row.addElement(std::make_shared<TextComponent>(window, tabhost[n_ip], Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
                        //printf("%s\n", tabhost[n_ip]);
                        s->addRow(row);
                        row.elements.clear();

                        n_ip++;
                        }
           }

           freeifaddrs(ifaddr);
/*
        printf("%s\n", host);
        row.addElement(std::make_shared<TextComponent>(window, host, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
        s->addRow(row);
*/
        mWindow->pushGui(s);

//                      mWindow->pushGui(new GuiGetIpAddress(mWindow, false, nullptr));


        });

        addEntry("SYSTEM SETTINGS", 0x777777FF, true, 
		[this] { 
                        Window* window = mWindow;

			auto s = new GuiSettings(mWindow, "SYSTEM SETTINGS");
                        
//                        auto version = std::make_shared<TextComponent>(mWindow, RaspicadeSystem::getInstance()->getVersion(), Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
//                        s->addWithLabel("VERSION", version);
//                        bool warning = RaspicadeSystem::getInstance()->isFreeSpaceLimit();
//                        auto space = std::make_shared<TextComponent>(mWindow, RaspicadeSystem::getInstance()->getFreeSpaceInfo(), Font::get(FONT_SIZE_MEDIUM), warning ? 0xFF0000FF : 0x777777FF);
//                        s->addWithLabel("STORAGE", space);
                        
                        // Overclock choice 
			auto overclock_choice = std::make_shared< OptionListComponent<std::string> >(window, "OVERCLOCK", false);
                        std::string currentOverclock = Settings::getInstance()->getString("Overclock");
                        overclock_choice->add("EXTREM", "extrem", currentOverclock == "extrem");
                        overclock_choice->add("TURBO", "turbo", currentOverclock == "turbo");
                        overclock_choice->add("HIGH", "high", currentOverclock == "high");
                        overclock_choice->add("NONE", "none", currentOverclock == "none");
                        s->addWithLabel("OVERCLOCK", overclock_choice);
                        
                        // overscan
			auto overscan_enabled = std::make_shared<SwitchComponent>(mWindow);
			overscan_enabled->setState(Settings::getInstance()->getBool("Overscan"));
			s->addWithLabel("OVERSCAN", overscan_enabled);
                        
                        // Screen ratio choice 
			auto ratio_choice = std::make_shared< OptionListComponent<std::string> >(window, "GAME RATIO", false);
                        std::string currentRatio = Settings::getInstance()->getString("GameRatio");
                        ratio_choice->add("AUTO", "auto", currentRatio == "auto");
                        ratio_choice->add("4/3", "4/3", currentRatio == "4/3");
                        ratio_choice->add("16/9", "16/9", currentRatio == "16/9");
                        ratio_choice->add("CUSTOM", "custom", currentRatio == "custom");

                        s->addWithLabel("GAME RATIO", ratio_choice);
                        
                        // smoothing
			auto smoothing_enabled = std::make_shared<SwitchComponent>(mWindow);
			smoothing_enabled->setState(Settings::getInstance()->getBool("Smooth"));
			s->addWithLabel("SMOOTH GAMES", smoothing_enabled);
			
                        
                        s->addSaveFunc([overscan_enabled,smoothing_enabled ,overclock_choice, ratio_choice, window] { 
                            bool reboot = false;
                            Settings::getInstance()->setBool("Smooth", smoothing_enabled->getState()); 
                            Settings::getInstance()->setString("GameRatio", ratio_choice->getSelected());
                            
                            if(Settings::getInstance()->getBool("Overscan") != overscan_enabled->getState()){
                                Settings::getInstance()->setBool("Overscan", overscan_enabled->getState()); 
                                RaspicadeSystem::getInstance()->setOverscan(overscan_enabled->getState());
                                reboot = true;
                            }
                            
                            if(Settings::getInstance()->getString("Overclock") != overclock_choice->getSelected()){
                                Settings::getInstance()->setString("Overclock", overclock_choice->getSelected()); 
                                RaspicadeSystem::getInstance()->setOverclock(overclock_choice->getSelected());
                                reboot = true;
                            }
                            if(reboot){
                                window->pushGui(
                                   new GuiMsgBox(window, "THE SYSTEM WILL NOW REBOOT", "OK", 
                                   [] {
                                       if(runRestartCommand() != 0)
                                           LOG(LogWarning) << "Reboot terminated with non-zero result!";
                                   })
                                );
                            }
                            
                        });
                        mWindow->pushGui(s);

	});
        addEntry("NETWORK SETTINGS", 0x777777FF, true, 
		[this] { 
                        Window* window = mWindow;

			auto s = new GuiSettings(mWindow, "NETWORK SETTINGS");

	                // scrape now
                        ComponentListRow row;

                        auto scrape_now = std::make_shared<TextComponent>(mWindow, "IP", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                        auto bracket = makeArrow(mWindow);
                        row.addElement(scrape_now, true);
                        row.addElement(bracket, false);
                        s->addRow(row);


                       	// scrape ratings
			auto enable_wifi = std::make_shared<SwitchComponent>(mWindow);
			enable_wifi->setState(Settings::getInstance()->getBool("EnableWifi"));
			s->addWithLabel("ENABLE WIFI", enable_wifi);
                        
                        
                        // TODO PUT THIS IN A FUNCTION
                        // window, title, settingstring, 
                        const std::string baseSSID =  Settings::getInstance()->getString("WifiSSID");
                        const std::string baseKEY = Settings::getInstance()->getString("WifiKey");
                        bool baseEnabled = Settings::getInstance()->getBool("EnableWifi");
                        createInputTextRow(s, "WIFI SSID", "WifiSSID", false);
                        createInputTextRow(s, "WIFI KEY", "WifiKey",true);

                        s->addSaveFunc([baseEnabled, baseSSID, baseKEY, enable_wifi, window] {
                            bool wifienabled = enable_wifi->getState();
                            Settings::getInstance()->setBool("EnableWifi", wifienabled); 
                            std::string newSSID = Settings::getInstance()->getString("WifiSSID");
                            std::string newKey = Settings::getInstance()->getString("WifiKey");
                            if(wifienabled){
                                if(baseSSID != newSSID
                                    || baseKEY!= newKey
                                        || ! baseEnabled ){
                                    if(RaspicadeSystem::getInstance()->enableWifi(newSSID,newKey)){
                                         window->pushGui(
                                            new GuiMsgBox(window, "WIFI ENABLED")
                                         );
                                    }else {
                                        window->pushGui(
                                            new GuiMsgBox(window, "WIFI CONFIGURATION ERROR")
                                         );
                                    }
                                }
                            }else if(baseEnabled){
                                 RaspicadeSystem::getInstance()->disableWifi();
                            }
                        });
			mWindow->pushGui(s);


                });

	auto openScrapeNow = [this] { mWindow->pushGui(new GuiScraperStart(mWindow)); };
	addEntry("SCRAPER", 0x777777FF, true, 
		[this, openScrapeNow] { 
			auto s = new GuiSettings(mWindow, "SCRAPER");

			// scrape from
			auto scraper_list = std::make_shared< OptionListComponent< std::string > >(mWindow, "SCRAPE FROM", false);
			std::vector<std::string> scrapers = getScraperList();
			for(auto it = scrapers.begin(); it != scrapers.end(); it++)
				scraper_list->add(*it, *it, *it == Settings::getInstance()->getString("Scraper"));

			s->addWithLabel("SCRAPE FROM", scraper_list);
			s->addSaveFunc([scraper_list] { Settings::getInstance()->setString("Scraper", scraper_list->getSelected()); });

			// scrape ratings
			auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
			scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
			s->addWithLabel("SCRAPE RATINGS", scrape_ratings);
			s->addSaveFunc([scrape_ratings] { Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState()); });

			// scrape now
			ComponentListRow row;
			std::function<void()> openAndSave = openScrapeNow;
			openAndSave = [s, openAndSave] { s->save(); openAndSave(); };
			row.makeAcceptInputHandler(openAndSave);

			auto scrape_now = std::make_shared<TextComponent>(mWindow, "SCRAPE NOW", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
			auto bracket = makeArrow(mWindow);
			row.addElement(scrape_now, true);
			row.addElement(bracket, false);
			s->addRow(row);

			mWindow->pushGui(s);
	});

	addEntry("SOUND SETTINGS", 0x777777FF, true, 
		[this] {
			auto s = new GuiSettings(mWindow, "SOUND SETTINGS");

			// volume
			auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
			volume->setValue((float)VolumeControl::getInstance()->getVolume());
			s->addWithLabel("SYSTEM VOLUME", volume);
			s->addSaveFunc([volume] { VolumeControl::getInstance()->setVolume((int)round(volume->getValue())); });
/*			
			// disable sounds
			auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
			sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
			s->addWithLabel("ENABLE SOUNDS", sounds_enabled);
			s->addSaveFunc([sounds_enabled] { Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState()); });
*/
// disable sounds
			auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
			sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
			s->addWithLabel("ENABLE SOUNDS", sounds_enabled);
			s->addSaveFunc([sounds_enabled] {Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState()); 
                            if(!sounds_enabled->getState())
                                AudioManager::getInstance()->stopMusic();
                        });


			auto output_list = std::make_shared< OptionListComponent< std::string > >(mWindow, "OUTPUT DEVICE", false);
			
                        std::string currentDevice = Settings::getInstance()->getString("AudioOutputDevice");
                        output_list->add("HDMI", "hdmi", "hdmi" == currentDevice);
                        output_list->add("JACK", "jack", "jack" == currentDevice);
                        output_list->add("AUTO", "auto", "auto" == currentDevice);

			s->addWithLabel("OUTPUT DEVICE", output_list);
			s->addSaveFunc([output_list] { 
                            if(Settings::getInstance()->getString("AudioOutputDevice") != output_list->getSelected()){
                                Settings::getInstance()->setString("AudioOutputDevice", output_list->getSelected()); 
                                RaspicadeSystem::getInstance()->setAudioOutputDevice(output_list->getSelected());
                            }
                        });

			mWindow->pushGui(s);
	});

	addEntry("UI SETTINGS", 0x777777FF, true,
		[this] {
			auto s = new GuiSettings(mWindow, "UI SETTINGS");

			// screensaver time
			auto screensaver_time = std::make_shared<SliderComponent>(mWindow, 0.f, 30.f, 1.f, "m");
			screensaver_time->setValue((float)(Settings::getInstance()->getInt("ScreenSaverTime") / (1000 * 60)));
			s->addWithLabel("SCREENSAVER AFTER", screensaver_time);
			s->addSaveFunc([screensaver_time] { Settings::getInstance()->setInt("ScreenSaverTime", (int)round(screensaver_time->getValue()) * (1000 * 60)); });

			// screensaver behavior
			auto screensaver_behavior = std::make_shared< OptionListComponent<std::string> >(mWindow, "TRANSITION STYLE", false);
			std::vector<std::string> screensavers;
			screensavers.push_back("dim");
			screensavers.push_back("black");
			for(auto it = screensavers.begin(); it != screensavers.end(); it++)
				screensaver_behavior->add(*it, *it, Settings::getInstance()->getString("ScreenSaverBehavior") == *it);
			s->addWithLabel("SCREENSAVER BEHAVIOR", screensaver_behavior);
			s->addSaveFunc([screensaver_behavior] { Settings::getInstance()->setString("ScreenSaverBehavior", screensaver_behavior->getSelected()); });

			// framerate
			auto framerate = std::make_shared<SwitchComponent>(mWindow);
			framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
			s->addWithLabel("SHOW FRAMERATE", framerate);
			s->addSaveFunc([framerate] { Settings::getInstance()->setBool("DrawFramerate", framerate->getState()); });

			// show help
			auto show_help = std::make_shared<SwitchComponent>(mWindow);
			show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
			s->addWithLabel("ON-SCREEN HELP", show_help);
			s->addSaveFunc([show_help] { Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState()); });

			// quick system select (left/right in game list view)
			auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
			quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
			s->addWithLabel("QUICK SYSTEM SELECT", quick_sys_select);
			s->addSaveFunc([quick_sys_select] { Settings::getInstance()->setBool("QuickSystemSelect", quick_sys_select->getState()); });

			// transition style
			auto transition_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "TRANSITION STYLE", false);
			std::vector<std::string> transitions;
			transitions.push_back("fade");
			transitions.push_back("slide");
			for(auto it = transitions.begin(); it != transitions.end(); it++)
				transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
			s->addWithLabel("TRANSITION STYLE", transition_style);
			s->addSaveFunc([transition_style] { Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected()); });

			// theme set
			auto themeSets = ThemeData::getThemeSets();

			if(!themeSets.empty())
			{
				auto selectedSet = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
				if(selectedSet == themeSets.end())
					selectedSet = themeSets.begin();

				auto theme_set = std::make_shared< OptionListComponent<std::string> >(mWindow, "THEME SET", false);
				for(auto it = themeSets.begin(); it != themeSets.end(); it++)
					theme_set->add(it->first, it->first, it == selectedSet);
				s->addWithLabel("THEME SET", theme_set);

				Window* window = mWindow;
				s->addSaveFunc([window, theme_set] 
				{
					bool needReload = false;
					if(Settings::getInstance()->getString("ThemeSet") != theme_set->getSelected())
						needReload = true;

					Settings::getInstance()->setString("ThemeSet", theme_set->getSelected());

					if(needReload)
						ViewController::get()->reloadAll(); // TODO - replace this with some sort of signal-based implementation
				});
			}

			mWindow->pushGui(s);
	});

	addEntry("CONFIGURE INPUT", 0x777777FF, true, 
		[this] { 
			mWindow->pushGui(new GuiDetectDevice(mWindow, false, nullptr));
	});


	addEntry("QUIT", 0x777777FF, true, 
		[this] {
			auto s = new GuiSettings(mWindow, "QUIT");
			
			Window* window = mWindow;

			ComponentListRow row;
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES", 
				[] { 
					if(runRestartCommand() != 0)
						LOG(LogWarning) << "Restart terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "RESTART SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			row.elements.clear();
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY SHUTDOWN?", "YES", 
				[] { 
					if(runShutdownCommand() != 0)
						LOG(LogWarning) << "Shutdown terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "SHUTDOWN SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			if(Settings::getInstance()->getBool("ShowExit"))
			{
				row.elements.clear();
				row.makeAcceptInputHandler([window] {
					window->pushGui(new GuiMsgBox(window, "REALLY QUIT?", "YES", 
					[] { 
						SDL_Event ev;
						ev.type = SDL_QUIT;
						SDL_PushEvent(&ev);
					}, "NO", nullptr));
				});
				row.addElement(std::make_shared<TextComponent>(window, "QUIT EMULATIONSTATION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
				s->addRow(row);
			}

			mWindow->pushGui(s);
	});

	mVersion.setFont(Font::get(FONT_SIZE_SMALL));
	mVersion.setColor(0xC6C6C6FF);
	mVersion.setText("EMULATIONSTATION V" + strToUpper(PROGRAM_VERSION_STRING));
	mVersion.setAlignment(ALIGN_CENTER);

	addChild(&mMenu);
	addChild(&mVersion);

	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiMenu::createConfigInput(){
    
                GuiSettings *  s = new GuiSettings(mWindow, "CONFIGURE INPUT");

                Window* window = mWindow;

                ComponentListRow row;
                row.makeAcceptInputHandler([window, this, s] {
                    window->pushGui(new GuiMsgBox(window, "I18NMESSAGECONTROLLERS", "OK", 
                            [window, this, s] {
                                window->pushGui(new GuiDetectDevice(window, false, [this, s] {
                                    s->setSave(false);
                                    delete s;
                                    this->createConfigInput();
                                }));
                            }));
                });
                        
                
                row.addElement(std::make_shared<TextComponent>(window, "CONFIGURE A CONTROLLER", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
                s->addRow(row);

                
                row.elements.clear();
		// quick system select (left/right in game list view)
//                auto gpio_select = std::make_shared<SwitchComponent>(mWindow);
//                gpio_select->setState(Settings::getInstance()->getBool("GpioControllers"));
//                s->addWithLabel("GPIO CONTROLLERS", gpio_select);

                // Here we go; for each player                    
                std::list<int> alreadyTaken = std::list<int>();

                std::vector<std::shared_ptr<OptionListComponent<std::string>>> options;
                for (int player = 0; player < 4; player++) {
                    std::stringstream sstm;
                    sstm << "INPUT P" << player +1;
                    std::string confName = sstm.str();

                    auto inputOptionList = std::make_shared<OptionListComponent<std::string> >(mWindow, confName, false);
                    options.push_back(inputOptionList);
                    
                    // Checking if a setting has been saved, else setting to default
                    std::string configuratedName = Settings::getInstance()->getString(confName);
                    
                    bool found = false;
                    // For each available and configured input
                    for (auto it = 0; it < InputManager::getInstance()->getNumJoysticks(); it++) {
                        InputConfig * config = InputManager::getInstance()->getInputConfigByDevice(it);
                        if(config->isConfigured()) {
                            // create name
                            std::stringstream dispNameSS;
                            dispNameSS << "#" << config->getDeviceId() << " ";
                            std::string deviceName = config->getDeviceName();
                            if(deviceName.size() > 25){ 
                                dispNameSS << deviceName.substr(0, 16) << "..." << deviceName.substr(deviceName.size()-5, deviceName.size()-1);
                            }else {
                                dispNameSS << deviceName;
                            }
                            
                            std::string displayName = dispNameSS.str();
                            
                            
                            bool foundFromConfig = configuratedName == config->getDeviceName();
                            int deviceID = config->getDeviceId();
                            // Si la manette est configurée, qu'elle correspond a la configuration, et qu'elle n'est pas 
                            // deja selectionnée on l'ajoute en séléctionnée
                            if(foundFromConfig 
                                    && std::find(alreadyTaken.begin(), alreadyTaken.end(), deviceID) == alreadyTaken.end()
                                    && !found) {
                                found = true;
                                alreadyTaken.push_back(deviceID);
                                LOG(LogWarning) << "adding entry for player"<<player << " (selected): " << config->getDeviceName() << "  " << config->getDeviceGUIDString();
                                inputOptionList->add(displayName, config->getDeviceName(), true);
                            }else {
                                LOG(LogWarning) << "adding entry for player"<<player << " (not selected): " << config->getDeviceName() << "  " << config->getDeviceGUIDString();
                                inputOptionList->add(displayName, config->getDeviceName(), false);
                            }
                        }
                    }
                    if (configuratedName.compare("") == 0 || !found) {
                        LOG(LogWarning) << "adding default entry for player "<<player << "(selected : true)";
                        inputOptionList->add("DEFAULT", "", true);
                    }else {
                        LOG(LogWarning) << "adding default entry for player"<<player << "(selected : false)";
                        inputOptionList->add("DEFAULT", "", false);
                    }
                    
                    // ADD default config
                    
                    // Populate controllers list
                    s->addWithLabel(confName, inputOptionList);
                             
                }
                s->addSaveFunc([this, options, window] {
                      for (int player = 0; player < 4; player++) {
                            std::stringstream sstm;
                            sstm << "INPUT P" << player+1;
                            std::string confName = sstm.str();

                            auto input_p1 = options.at(player);
                            std::string name;
                            std::string selectedName = input_p1->getSelectedName();

                            if (selectedName.compare("DEFAULT") == 0) {
                                name = "DEFAULT";
                                Settings::getInstance()->setString(confName, name);                            
                            } else {
                                LOG(LogWarning) << "Found the selected controller ! : name in list  = "<< selectedName;
                                LOG(LogWarning) << "Found the selected controller ! : guid  = "<< input_p1->getSelected();

                                Settings::getInstance()->setString(confName, input_p1->getSelected());
                            }
                        }
                        
                        // GPIOS
//                        if(Settings::getInstance()->getBool("GpioControllers") != gpio_select->getState()){
//                            Settings::getInstance()->setBool("GpioControllers", gpio_select->getState()); 
//                            RetroboxSystem::getInstance()->setGPIOControllers(gpio_select->getState());
//                            
//                            if(gpio_select->getState()){
//                                window->pushGui(
//                                           new GuiMsgBox(window, "GPIO CONTROLLERS ARE NOW ON", "OK")
//                                        );
//                            }
//                        }
                        Settings::getInstance()->saveFile();

                });
                
                row.elements.clear();
                window->pushGui(s);
                
}

void GuiMenu::onSizeChanged()
{
	mVersion.setSize(mSize.x(), 0);
	mVersion.setPosition(0, mSize.y() - mVersion.getSize().y());
}

void GuiMenu::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);
	
	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
		row.addElement(bracket, false);
	}
	
	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("start", "close"));
	return prompts;
}
