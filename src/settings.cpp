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
#include "avconnect.h"
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
    }
    
    virtual void buildInterface() override {
        
        
        ImGui::PushItemWidth(-1);
        if(ImGui::BeginTable("DeviceListLayout", 2)) {
        	ImGui::TableSetupColumn("Device List", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 140);
        	ImGui::TableSetupColumn("Device Details", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort);
            
            ImGui::TableNextColumn();
        
            ImGui::PushItemWidth(-1);
            if(ImGui::Button("Add")) {
                avconnect_device_add();
            }
            ImGui::SameLine();
            if(ImGui::Button("Read")) {
                avconnect_conf_check_reload(true);
            }
            if(ImGui::BeginListBox("##Devices", ImVec2(-1, -2))) {
                
                for(int i = 0; i < avconnect_get_device_count(); ++i) {
                    av_device_t *dev = avconnect_device_get(i);
                    if(ImGui::Selectable(av_device_get_name(dev), i == sel_device_id)) {
                        sel_device_id = i;
                    }
                }
                
                ImGui::EndListBox();
            }
            
            ImGui::TableNextColumn();
            if(sel_device_id >= 0) {
                av_device_t *sel_device = avconnect_device_get(sel_device_id);
                ImGui::Text("%s", av_device_get_name(sel_device));
                portDropdown(sel_device);
                ImGui::SameLine();
                if(ImGui::Button("Scan")) {
                    updatePorts();
                }
                ImGui::BeginTabBar("Bindings");
                if(ImGui::BeginTabItem("Inputs")) {
                    if(ImGui::BeginChild("InputsScroll")) {
                        buildInputsTab(sel_device);
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
        
                if(ImGui::BeginTabItem("Outputs")) {
                    if(ImGui::BeginChild("OutputsScroll")) {
                        buildOutputsTab(sel_device);
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
        for(int i = 0; i < AV_MUX_MAX_PINS; ++i) {
            char buf[32];
            snprintf(buf, sizeof(buf), "Command #%d", i);
            commandField(buf, &mux->cmd[i]);
        }
    }
    
    void buildInputsTab(av_device_t *sel_device) {
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
            ImGui::InputText("ID", in->name, sizeof(in->name));
            
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
        
        if(to_delete >= 0 && to_delete < av_device_get_in_count(sel_device)) {
            av_device_delete_in(sel_device, to_delete);
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
        
        if(ImGui::Button("Save configuration")) {
            avconnect_conf_save(false);
        }
    }
    
#define COUNTOF(ar) (sizeof(ar) / sizeof(*ar))
    
    void buildPWMPad(av_out_pwm_t *pwm) {
        drefField("dataref", &pwm->dref);
        // ImGui::SameLine();
        dropdown("##mod_op", av_mod_str, COUNTOF(av_mod_str), (int&)pwm->mod_op);
        // ImGui::SameLine();
        ImGui::InputFloat("##mod_val", &pwm->mod_val);
    }
    
    void buildShiftRegPad(av_out_sreg_t *sreg) {
        for(int i = 0; i < AV_SREG_MAX_PINS; ++i) {
            ImGui::PushID(i);
            drefField("dataref", &sreg->pins[i].dref);
            // ImGui::SameLine();
            dropdown("##cmp_op", av_cmp_str, COUNTOF(av_cmp_str), (int&)sreg->pins[i].cmp_op);
            // ImGui::SameLine();
            ImGui::InputFloat("##cmp_val", &sreg->pins[i].cmp_val);
            ImGui::PopID();
        }
    }
    
    void buildOutputsTab(av_device_t *sel_device) {
        if(sel_device == nullptr)
            return;
        
        int to_delete = -1;
        for(int i = 0; i < av_device_get_out_count(sel_device); ++i) {
            av_out_t *out = av_device_get_out(sel_device, i);
            
            ImGui::PushID(i);
            const char *header = "Output";
            switch(out->type) {
            case AV_OUT_PWM: header = "PWM Output"; break;
            case AV_OUT_SHIFT_REG: header = "Shift Register"; break;
            }
            
            if(!ImGui::CollapsingHeader(header)) {
                ImGui::PopID();
                continue;
            }
            
            ImGui::InputInt("ID", &out->id);
            
            switch(out->type) {
            case AV_OUT_PWM:
                buildPWMPad((av_out_pwm_t *)out);
                break;
            case AV_OUT_SHIFT_REG:
                buildShiftRegPad((av_out_sreg_t *)out);
                break;
            }
            
            if(ImGui::Button("Delete")) {
                to_delete = i;
            }
            
            ImGui::PopID();
        }
        
        ImGui::Separator();
        
        if(to_delete >= 0 && to_delete < av_device_get_out_count(sel_device)) {
            av_device_delete_out(sel_device, to_delete);
        }
        
        if(ImGui::Button("Add PWM Output")) {
            av_device_add_out_pwm(sel_device);
        }
        ImGui::SameLine();
        if(ImGui::Button("Add Shift Register")) {
            av_device_add_out_sreg(sel_device);
        }
    }
    
    void portDropdown(av_device_t *dev) {
        const char *sel_port = av_device_get_address(dev);
        if(ImGui::BeginCombo("##Ports", sel_port)) {
            for(int i = 0; i < port_count; ++i) {
                if(ImGui::Selectable(ports[i].name)) {
                    av_device_set_address(dev, ports[i].address);
                }
            }
            ImGui::EndCombo();
        }
    }
    
    void updatePorts() {
        serial_free_list(ports, port_count);
        port_count = serial_list_devices(ports, max_ports);
    }
    
    void commandField(const char *label, av_cmd_t *cmd) {
        if(ImGui::InputText(label, cmd->path, sizeof(cmd->path))) {
            av_cmd_end(cmd);
            cmd->has_changed = true;
        }
    }
    
    void dropdown(const char *label, const char **options, int count, int& sel) {
        int value = -1;
        if(ImGui::BeginCombo(label, options[sel])) {
            for(int i = 0; i < count; ++i) {
                if(ImGui::Selectable(options[i], i == sel)) {
                    value = i;
                }
            }
            ImGui::EndCombo();
        }
        if(value >= 0)
            sel = value;
    }
    
    void drefField(const char *label, av_dref_t *dref) {
        if(!ImGui::InputText(label, dref->path, sizeof(dref->path)))
            return;
        dref->has_changed = true;
        dref->has_resolved = false;
        dref->type = AV_TYPE_INVALID;
    }
    
    static constexpr int max_ports = 64;
    serial_info_t   ports[max_ports];
    int             port_count = 0;
    
    int             sel_device_id = -1;
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
