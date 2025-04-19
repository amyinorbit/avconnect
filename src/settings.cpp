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
#include "device.h"
#include <serial/serial.h>
#include <ImgWindow.h>
#include <acfutils/helpers.h>

class Settings : public ImgWindow {
public:
    
    Settings(int left, int top, int right, int bottom) : ImgWindow(left, top, right, bottom) {
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        
        SetWindowTitle("AvConnect Settings");
        SetResizingLimits(500, 210, 500, 1024);
        
        updatePorts();
    }
    
    virtual ~Settings() {
        serial_free_list(ports, port_count);
    }
    
    virtual void buildInterface() override {
        
        
        ImGui::PushItemWidth(-1);
        if(ImGui::BeginTable("DeviceListLayout", 2)) {
        	ImGui::TableSetupColumn("Device List", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 140);
        	ImGui::TableSetupColumn("Device Details", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort);
            
            ImGui::TableNextColumn();
            if(ImGui::BeginListBox("##Devices", ImVec2(-1, -2))) {
                
                ImGui::EndListBox();
            }
            ImGui::TableNextColumn();
            if(sel_device == nullptr) {
                portDropdown();
                ImGui::SameLine();
                if(ImGui::Button("Scan")) {
                    updatePorts();
                }
                ImGui::BeginTabBar("Bindings");
                if(ImGui::BeginTabItem("Inputs")) {
                    buildInputsTab();
                    ImGui::EndTabItem();
                }
        
                if(ImGui::BeginTabItem("Outputs")) {
                    buildOutputsTab();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            } else {
                ImGui::Spacing();
                ImGui::Text("No Device Selected");
            }
            
            ImGui::EndTable();
        }
        
        
        
    }
    
private:
    
    void buildInputsTab() {
        
    }
    
    void buildOutputsTab() {
        
    }
    
    bool portDropdown() {
        bool changed = false;
        
        if(ImGui::BeginCombo("Ports", sel_port ? sel_port->name : "<none>")) {
            for(int i = 0; i < port_count; ++i) {
                if(ImGui::Selectable(ports[i].name, &ports[i] == sel_port)) {
                    sel_port = &ports[i];
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
        
        return changed;
    }
    
    void updatePorts() {
        sel_port = nullptr;
        serial_free_list(ports, port_count);
        port_count = serial_list_devices(ports, max_ports);
    }
    
    static constexpr int max_ports = 64;

    serial_info_t   *sel_port = nullptr;
    serial_info_t   ports[max_ports];
    int             port_count = 0;
    
    av_device_t     *sel_device = nullptr;
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
