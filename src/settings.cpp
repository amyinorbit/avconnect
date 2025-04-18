/*===--------------------------------------------------------------------------------------------===
 * settings.cpp
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "settings.h"
#include "xplane.h"
#include <ImgWindow.h>
#include <acfutils/helpers.h>

class Settings : public ImgWindow {
public:
    
    Settings(int left, int top, int right, int bottom) : ImgWindow(left, top, right, bottom) {
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        
        SetWindowTitle("AvConnect Settings");
        SetResizingLimits(500, 210, 500, 1024);
        
    }
    
    virtual ~Settings() {
        
    }
    
    virtual void buildInterface() override {
        
    }
    
private:
};


static Settings *window = nullptr;
static XPLMCommandRef show_settings = nullptr;
static int menu_item = -1;

static int handle_show(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) {
    UNUSED(cmd);
    UNUSED(ref);
    if(phase != xplm_CommandBegin)
        return 1;
    settings_open();
    return 1;
}

void settings_init() {
    window = nullptr;
    ImgWindow::sFontAtlas = std::make_shared<ImgFontAtlas>();
    
    show_settings = XPLMCreateCommand("avconnect/open_settings_window", "Open AvConnect Settings");
    VERIFY(show_settings);
    XPLMRegisterCommandHandler(show_settings, handle_show, false, nullptr);
    menu_item = XPLMAppendMenuItemWithCommand(get_plugin_menu(), "Settings…", show_settings);
}

void settings_fini() {
    XPLMRemoveMenuItem(nullptr, menu_item);
    XPLMUnregisterCommandHandler(show_settings, handle_show, false, nullptr);
    if(window)
        delete window;
    
    show_settings = nullptr;
    window = nullptr;
    menu_item = -1;
}

void settings_open() {
    if(window == nullptr)
        window = new Settings(50, 50+210, 50+500, 50);
    window->SetVisible(true);
}
