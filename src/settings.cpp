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
        SetResizingLimits(500, 210, 1024, 1024);
        
        updatePorts();
    }
    
    virtual ~Settings() {
        serial_free_list(ports, port_count);
        for(int i = 0; i < device_count; ++i) {
            av_device_destroy(devices[i]);
        }
    }
    
    virtual void buildInterface() override {
        
        
        ImGui::PushItemWidth(-1);
        if(ImGui::BeginTable("DeviceListLayout", 2)) {
        	ImGui::TableSetupColumn("Device List", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 140);
        	ImGui::TableSetupColumn("Device Details", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort);
            
            ImGui::TableNextColumn();
        
            ImGui::PushItemWidth(-1);
            if(ImGui::Button("Add") && device_count < max_devices) {
                // TODO: we probably do not want our device register to exist in settings, we
                // should only be holding referebces
                devices[device_count] = av_device_new();
                sel_device = devices[device_count];
                device_count += 1;
            }
            if(ImGui::BeginListBox("##Devices", ImVec2(-1, -2))) {
                
                for(int i = 0; i < device_count; ++i) {
                    av_device_t *dev = devices[i];
                    const char *name = av_device_get_name(devices[i]);
                    if(ImGui::Selectable(name, dev == sel_device)) {
                        sel_device = dev;
                    }
                }
                
                ImGui::EndListBox();
            }
            
            ImGui::TableNextColumn();
            if(sel_device != nullptr) {
                ImGui::Text("%s", av_device_get_name(sel_device));
                if(portDropdown()) {
                    av_device_set_address(sel_device, sel_port->address);
                }
                ImGui::SameLine();
                if(ImGui::Button("Scan")) {
                    updatePorts();
                }
                ImGui::BeginTabBar("Bindings");
                if(ImGui::BeginTabItem("Inputs")) {
                    if(ImGui::BeginChild("InputsScroll")) {
                        buildInputsTab();
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
        
                if(ImGui::BeginTabItem("Outputs")) {
                    if(ImGui::BeginChild("OutputsScroll")) {
                        buildOutputsTab();
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            } else {
                ImGui::Spacing();
                ImGui::Text("No Device Selected");
            }
            
            ImGui::EndTable();
        }
        
        // TODO: move this to a flight loop callback
        for(int i = 0; i < device_count; ++i) {
            av_device_update(devices[i]);
        }
    }
    
private:
    
    void buildEncoderPad(av_in_encoder_t *encoder) {
        commandField("Command (down)", &encoder->cmd_dn);
        commandField("Command (up)", &encoder->cmd_up);
    }
    
    void buildButtonPad(av_in_button_t *button) {
        commandField("Command", &button->cmd);
    }
    
    void buildMuxPad(av_in_mux_t *mux) {
        for(int i = 0; i < mux->pin_count; ++i) {
            char buf[32];
            snprintf(buf, sizeof(buf), "Command #%d", i+1);
            commandField(buf, &mux->cmd[i]);
        }
        if(ImGui::Button("Add Multiplexer Input") && mux->pin_count < MUX_MAX_PINS) {
            av_cmd_init(&mux->cmd[mux->pin_count++]);
        }
    }
    
    void buildInputsTab() {
        if(sel_device == nullptr)
            return;
        
        int to_delete = -1;
        for(int i = 0; i < av_device_get_in_count(sel_device); ++i) {
            av_in_t *in = av_device_get_in(sel_device, i);
            
            
            ImGui::PushID(i);
            const char *header = "Input";
            switch(in->type) {
            case AV_IN_ENCODER: header = "Encoder"; break;
            case AV_IN_BUTTON: header = "Button"; break;
            case AV_IN_MUX: header = "Multiplexer"; break;
            }
            
            if(!ImGui::CollapsingHeader(header)) {
                ImGui::PopID();
                continue;
            }
            
            ImGui::SetNextItemWidth(100);
            ImGui::InputText("ID", in->name, sizeof(in->name)); ImGui::SameLine();
            ImGui::InputText("Comment", in->comment, sizeof(in->comment));
            
            switch(in->type) {
            case AV_IN_ENCODER:
                buildEncoderPad((av_in_encoder_t *)in);
                break;
            case AV_IN_BUTTON:
                buildButtonPad((av_in_button_t *)in);
                break;
            case AV_IN_MUX:
                buildMuxPad((av_in_mux_t *)in);
                break;
            }
            if(ImGui::Button("Delete")) {
                to_delete = i;
            }
            
            ImGui::PopID();
        }
        
        ImGui::Separator();
        
        if(to_delete >= 0 || to_delete < max_devices) {
            UNUSED(to_delete);
        }
        
        if(ImGui::Button("Add Encoder")) {
            av_device_add_in_encoder(sel_device);
        }
        ImGui::SameLine();
        if(ImGui::Button("Add Button")) {
            av_device_add_in_button(sel_device);
        }
        ImGui::SameLine();
        if(ImGui::Button("Add Multiplexer")) {
            av_device_add_in_mux(sel_device);
        }
    }
    
    void buildOutputsTab() {
    }
    
    bool portDropdown() {
        bool changed = false;
        
        if(ImGui::BeginCombo("##Ports", sel_port ? sel_port->name : "<none>")) {
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
    
    void commandField(const char *label, av_cmd_t *cmd) {
        if(ImGui::InputText(label, cmd->path, sizeof(cmd->path))) {
            cmd->has_changed = true;
        }
    }
    
    static constexpr int max_ports = 64;
    static constexpr int max_devices = 16;

    serial_info_t   *sel_port = nullptr;
    serial_info_t   ports[max_ports];
    int             port_count = 0;
    
    av_device_t     *devices[max_devices];
    int             device_count = 0;
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
        window = new Settings(200, 200+300, 50+1024, 50);
    window->SetVisible(true);
}
